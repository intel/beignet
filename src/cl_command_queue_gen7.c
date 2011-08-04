/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "cl_command_queue.h"
#include "cl_context.h"
#include "cl_program.h"
#include "cl_kernel.h"
#include "cl_device_id.h"
#include "cl_mem.h"
#include "cl_utils.h"
#include "cl_alloc.h"

#include "intel_bufmgr.h"
#include "intel/intel_gpgpu.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static INLINE size_t
cl_kernel_compute_batch_sz(cl_kernel k)
{
  size_t sz = 256 + 16;
  return sz;
}

static cl_int
cl_set_local_ids(char *data,
                 const size_t *local_wk_sz,
                 size_t cst_sz,
                 size_t id_offset,
                 size_t thread_n)
{
  uint16_t *ids[3] = {NULL,NULL,NULL};
  size_t i, j, k, curr = 0;
  cl_int err = CL_SUCCESS;

  for (i = 0; i < 3; ++i)
    TRY_ALLOC(ids[i], (uint16_t*) cl_calloc(sizeof(uint16_t), thread_n*16));

  /* Compute the IDs */
  for (i = 0; i < local_wk_sz[0]; ++i)
  for (j = 0; j < local_wk_sz[1]; ++j)
  for (k = 0; k < local_wk_sz[2]; ++k, ++curr) {
    ((uint16_t*) ids[0])[curr] = i;
    ((uint16_t*) ids[1])[curr] = j;
    ((uint16_t*) ids[2])[curr] = k;
  }

  /* Copy them to the constant buffer */
  curr = 0;
  data += id_offset;
  for (i = 0; i < thread_n; ++i, data += cst_sz) {
    uint16_t *ids0 = (uint16_t *) (data +  0);
    uint16_t *ids1 = (uint16_t *) (data + 32);
    uint16_t *ids2 = (uint16_t *) (data + 64);
    for (j = 0; j < 16; ++j, ++curr) {/* SIMD16 */
      ids0[j] = ids[0][curr];
      ids1[j] = ids[1][curr];
      ids2[j] = ids[2][curr];
    }
  }

error:
  for (i = 0; i < 3; ++i)
    cl_free(ids[i]);
  return err;
}

LOCAL cl_int
cl_command_queue_ND_range_gen7(cl_command_queue queue,
                               cl_kernel ker,
                               const size_t *global_wk_off,
                               const size_t *global_wk_sz,
                               const size_t *local_wk_sz)
{
  cl_context ctx = queue->ctx;
  intel_gpgpu_t *gpgpu = queue->gpgpu;
  drm_intel_bo *private_bo = NULL, *scratch_bo = NULL;
  char *user = NULL;  /* User defined constants first */
  char *data = NULL;  /* Complete constant buffer to upload */
  genx_gpgpu_kernel_t kernel;
  const size_t local_mem_sz = cl_kernel_local_memory_sz(ker);
  size_t local_sz, batch_sz, cst_sz = ker->patch.curbe.sz;
  size_t i, thread_n, id_offset;
  cl_int err = CL_SUCCESS;

  /* Setup kernel */
  kernel.name = "OCL kernel";
  kernel.grf_blocks = 128;
  kernel.bin = NULL,
  kernel.size = 0,
  kernel.bo = ker->bo;
  kernel.barrierID = 0;

  /* All arguments must have been set */
  TRY (cl_kernel_check_args, ker);

  /* Check that the local work sizes are OK */
  TRY (cl_kernel_work_group_sz, ker, local_wk_sz, 3, &local_sz);
  thread_n = local_sz / 16; /* SIMD16 only */

  /* Fill the constant buffer. Basically, we have to build one set of
   * constants for each thread. The constants also includes the local ids we
   * append after all the other regular values (function parameters...)
   */
  if (cst_sz > 0) {
    assert(ker->cst_buffer);
    user = cl_kernel_create_cst_buffer(ker, global_wk_sz, local_wk_sz);
  }
  id_offset = cst_sz =  ALIGN(cst_sz, 32); /* Align the user data on 32 bytes */
  kernel.cst_sz = cst_sz += 3 * 32;        /* Add local IDs (16 words) */
  TRY_ALLOC (data, (char*) cl_calloc(thread_n, cst_sz));
  for (i = 0; i < thread_n; ++i)
    memcpy(data + cst_sz * i, user, cst_sz);
  TRY (cl_set_local_ids, data, local_wk_sz, cst_sz, id_offset, thread_n);

  /* Setup the kernel */
  gpgpu_state_init(gpgpu, ctx->device->max_compute_unit, 4, 64, cst_sz / 32, 64);
  if (queue->last_batch != NULL)
    drm_intel_bo_unreference(queue->last_batch);
  queue->last_batch = NULL;
  cl_command_queue_bind_surface(queue, ker, NULL, &private_bo, &scratch_bo, local_mem_sz);
  gpgpu_states_setup(gpgpu, &kernel, 1);

  /* We always have constant with Gen7 (local_ids are used) */
  gpgpu_upload_constants(gpgpu, data, thread_n*cst_sz);

  /* Start a new batch buffer */
  batch_sz = cl_kernel_compute_batch_sz(ker);
  gpgpu_batch_reset(gpgpu, batch_sz);
  gpgpu_batch_start(gpgpu);

  /* Issue the GPGPU_WALKER command */
  gpgpu_walker(gpgpu, thread_n, global_wk_off, global_wk_sz, local_wk_sz);

  /* Close the batch buffer and submit it */
  gpgpu_batch_end(gpgpu, 0);
  gpgpu_flush(gpgpu);

error:
  /* Release all temporary buffers */
  if (private_bo)
    drm_intel_bo_unreference(private_bo);
  if (scratch_bo)
    drm_intel_bo_unreference(scratch_bo);
  cl_free(data);
  cl_free(user);
  return err;
}


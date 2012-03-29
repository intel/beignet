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

#include <assert.h>
#include <stdio.h>
#include <string.h>

static INLINE size_t
cl_kernel_compute_batch_sz(cl_kernel k)
{
  size_t sz = 256 + 32;
  return sz;
}

static cl_int
cl_set_local_ids(char *data,
                 const size_t *local_wk_sz,
                 size_t simd_sz,
                 size_t cst_sz,
                 size_t id_offset,
                 size_t thread_n)
{
  uint16_t *ids[3] = {NULL,NULL,NULL};
  size_t i, j, k, curr = 0;
  cl_int err = CL_SUCCESS;

  for (i = 0; i < 3; ++i)
    TRY_ALLOC(ids[i], (uint16_t*) alloca(sizeof(uint16_t)*thread_n*simd_sz));

  /* Compute the IDs */
  for (k = 0; k < local_wk_sz[2]; ++k)
  for (j = 0; j < local_wk_sz[1]; ++j)
  for (i = 0; i < local_wk_sz[0]; ++i, ++curr) {
    ids[0][curr] = i;
    ids[1][curr] = j;
    ids[2][curr] = k;
  }

  /* Copy them to the constant buffer */
  curr = 0;
  data += id_offset;
  for (i = 0; i < thread_n; ++i, data += cst_sz) {
    /* Compiler use a GRF for each local ID (8 x 32 bits == 16 x 16 bits) */
    uint16_t *ids0 = (uint16_t *) (data + 0);
    uint16_t *ids1 = (uint16_t *) (data + 1*16*sizeof(uint16_t));
    uint16_t *ids2 = (uint16_t *) (data + 2*16*sizeof(uint16_t));
    for (j = 0; j < simd_sz; ++j, ++curr) {
      ids0[j] = ids[0][curr];
      ids1[j] = ids[1][curr];
      ids2[j] = ids[2][curr];
    }
  }

error:
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
  cl_gpgpu *gpgpu = queue->gpgpu;
  char *curbe = NULL;        /* Does not include per-thread local IDs */
  char *final_curbe = NULL;  /* Includes them */
  cl_buffer *private_bo = NULL, *scratch_bo = NULL;
  cl_gpgpu_kernel_t kernel;
  const uint32_t simd_sz = cl_kernel_get_simd_width(ker);
  size_t i, batch_sz = 0u, local_sz = 0u, thread_n = 0u, id_offset = 0u, cst_sz = 0u;
  cl_int err = CL_SUCCESS;

  /* Setup kernel */
  kernel.name = "OCL kernel";
  kernel.grf_blocks = 128;
  kernel.bo = ker->bo;
  kernel.barrierID = 0;
  kernel.use_barrier = 0;
  kernel.slm_sz = 0;
  kernel.cst_sz = 0;

  /* Compute the number of HW threads we are going to need */
  TRY (cl_kernel_work_group_sz, ker, local_wk_sz, 3, &local_sz);
  kernel.thread_n = thread_n = local_sz / simd_sz;
  id_offset = cst_sz = ALIGN(cst_sz, 32); /* Align the user data on 32 bytes */
  kernel.cst_sz = cst_sz += 3 * 32;       /* Add local IDs (16 words) */

  /* Setup the kernel */
  cl_gpgpu_state_init(gpgpu, ctx->device->max_compute_unit, cst_sz / 32);
  if (queue->last_batch != NULL)
    cl_buffer_unreference(queue->last_batch);
  queue->last_batch = NULL;
  cl_command_queue_bind_surface(queue, ker, curbe, NULL, &private_bo, &scratch_bo, 0);
  cl_gpgpu_states_setup(gpgpu, &kernel, 1);

  /* CURBE step 2. Give the localID and upload it to video memory */
  TRY_ALLOC (final_curbe, (char*) alloca(thread_n * cst_sz));
  if (curbe)
    for (i = 0; i < thread_n; ++i)
      memcpy(final_curbe + cst_sz * i, curbe, cst_sz - 3*32);
  TRY (cl_set_local_ids, final_curbe, local_wk_sz, simd_sz, cst_sz, id_offset, thread_n);
  cl_gpgpu_upload_constants(gpgpu, final_curbe, thread_n*cst_sz);

  /* Start a new batch buffer */
  batch_sz = cl_kernel_compute_batch_sz(ker);
  cl_gpgpu_batch_reset(gpgpu, batch_sz);
  cl_gpgpu_batch_start(gpgpu);

  /* Issue the GPGPU_WALKER command */
  cl_gpgpu_walker(gpgpu, simd_sz, thread_n, global_wk_off, global_wk_sz, local_wk_sz);

  /* Close the batch buffer and submit it */
  cl_gpgpu_batch_end(gpgpu, 0);
  cl_gpgpu_flush(gpgpu);

error:
  return err;
}


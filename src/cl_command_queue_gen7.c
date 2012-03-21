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

#ifdef _PLASMA
#include "plasma/plasma_export.h"
#else
#include "intel_bufmgr.h"
#include "intel/intel_gpgpu.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

static INLINE size_t
cl_kernel_compute_batch_sz(cl_kernel k)
{
#ifdef _PLASMA
    size_t sz = 0x1000; // _PLASMA
#else
    size_t sz = 256 + 32;
#endif
  return sz;
}

#if 0
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
    TRY_ALLOC(ids[i], (uint16_t*) cl_calloc(sizeof(uint16_t), thread_n*simd_sz));

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
  for (i = 0; i < 3; ++i)
    cl_free(ids[i]);
  return err;
}
#endif

LOCAL cl_int
cl_command_queue_ND_range_gen7(cl_command_queue queue,
                               cl_kernel ker,
                               const size_t *global_wk_off,
                               const size_t *global_wk_sz,
                               const size_t *local_wk_sz)
{
#if 0
  cl_context ctx = queue->ctx;
  intel_gpgpu_t *gpgpu = queue->gpgpu;
  drm_intel_bo *private_bo = NULL, *scratch_bo = NULL;
  char *curbe = NULL;        /* Does not include per-thread local IDs */
  char *final_curbe = NULL;  /* Includes them */
  genx_gpgpu_kernel_t kernel;
  //const size_t simd_sz = ker->patch.exec_env.largest_compiled_simd_sz;
  const size_t simd_sz = 16;
  size_t local_sz, batch_sz, cst_sz = ker->patch.curbe.sz;
  size_t i, thread_n, id_offset;
  cl_int err = CL_SUCCESS;

  /* Setup kernel */
  kernel.name = "OCL kernel";
  kernel.grf_blocks = 128;
  kernel.bin = ker->kernel_heap; // _PLASMA ; NULL
  kernel.size = ker->kernel_heap_sz; // _PLASMA ; 0
  kernel.bo = ker->bo;
  kernel.barrierID = 0;
  kernel.use_barrier = ker->patch.exec_env.has_barriers;
  kernel.slm_sz = cl_kernel_local_memory_sz(ker);

  /* All arguments must have been set */
  TRY (cl_kernel_check_args, ker);

  /* Check that the local work sizes are OK */
  TRY (cl_kernel_work_group_sz, ker, local_wk_sz, 3, &local_sz);
  //kernel.thread_n = thread_n = local_sz / simd_sz;
  kernel.thread_n = thread_n = local_sz / simd_sz;

  /* CURBE step 1. Allocate and fill fields shared by threads in workgroup */
  if (cst_sz > 0) {
    assert(ker->cst_buffer);
    curbe = cl_kernel_create_cst_buffer(ker,
                                        global_wk_off,
                                        global_wk_sz,
                                        local_wk_sz,
                                        3,
                                        thread_n);
  }
  id_offset = cst_sz = ALIGN(cst_sz, 32); /* Align the user data on 32 bytes */
  kernel.cst_sz = cst_sz += 3 * 32;       /* Add local IDs (16 words) */

  /* Setup the kernel */
  gpgpu_state_init(gpgpu, ctx->device->max_compute_unit, cst_sz / 32);
  if (queue->last_batch != NULL)
    drm_intel_bo_unreference(queue->last_batch);
  queue->last_batch = NULL;
  cl_command_queue_bind_surface(queue, ker, curbe, NULL, &private_bo, &scratch_bo, 0);
  gpgpu_states_setup(gpgpu, &kernel, 1);

  /* CURBE step 2. Give the localID and upload it to video memory */
  TRY_ALLOC (final_curbe, (char*) cl_calloc(thread_n, cst_sz));
  for (i = 0; i < thread_n; ++i)
    memcpy(final_curbe + cst_sz * i, curbe, cst_sz);
  TRY (cl_set_local_ids, final_curbe, local_wk_sz, simd_sz, cst_sz, id_offset, thread_n);
  gpgpu_upload_constants(gpgpu, final_curbe, thread_n*cst_sz);

  /* Start a new batch buffer */
  batch_sz = cl_kernel_compute_batch_sz(ker);
  gpgpu_batch_reset(gpgpu, batch_sz);
  gpgpu_batch_start(gpgpu);

  /* Issue the GPGPU_WALKER command */
  gpgpu_walker(gpgpu, simd_sz, thread_n, global_wk_off, global_wk_sz, local_wk_sz);

  /* Close the batch buffer and submit it */
  gpgpu_batch_end(gpgpu, 0);
  gpgpu_flush(gpgpu);

error:
  /* Release all temporary buffers */
  if (private_bo) drm_intel_bo_unreference(private_bo);
  if (scratch_bo) drm_intel_bo_unreference(scratch_bo);
  cl_free(final_curbe);
  cl_free(curbe);
  return err;
#endif
  return CL_SUCCESS;
}


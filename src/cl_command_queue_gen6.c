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

/* Header used by kernels */
typedef struct cl_inline_header {
  uint32_t grp_n[3];
  uint32_t local_sz[3];
  uint32_t exec_mask;
  uint32_t local_mem_sz;
} cl_inline_header_t;

/* ID inside the work group */
typedef struct cl_local_id {
  uint16_t data[16];
} cl_local_id_t;

static INLINE size_t
cl_kernel_compute_batch_sz(cl_kernel k, size_t wk_grp_n, size_t thread_n)
{
  size_t sz = 256; /* upper bound of the complete prelude */
  size_t media_obj_sz = 6 * 4; /* size of one MEDIA OBJECT */
  media_obj_sz += sizeof(cl_inline_header_t); /* header for all threads */
  media_obj_sz += 3 * sizeof(cl_local_id_t);/* for each dimension */ 
  if (k->patch.exec_env.has_barriers)
    media_obj_sz += 4 * 4; /* one barrier update per object */
  sz += media_obj_sz * wk_grp_n * thread_n;
  return sz;
}

static INLINE void
cl_command_queue_enqueue_wk_grp(cl_command_queue queue,
                                 cl_local_id_t **ids,
                                 const cl_inline_header_t *header,
                                 uint32_t thread_n,
                                 uint32_t barrierID)
{
  intel_gpgpu_t *gpgpu = queue->gpgpu;
  uint32_t i;
  for (i = 0; i < thread_n; ++i) {
    const size_t sz = sizeof(cl_inline_header_t) + 3*sizeof(cl_local_id_t);
    char *data = gpgpu_run_with_inline(gpgpu, barrierID, sz);
    size_t offset = 0;
    assert(data);
    *((cl_inline_header_t *) (data + offset)) = *header;
    offset += sizeof(cl_inline_header_t);
    *((cl_local_id_t *) (data + offset)) = ids[0][i];
    offset += sizeof(cl_local_id_t);
    *((cl_local_id_t *) (data + offset)) = ids[1][i];
    offset += sizeof(cl_local_id_t);
    *((cl_local_id_t *) (data + offset)) = ids[2][i];
  }
}

LOCAL cl_int
cl_command_queue_ND_range_gen6(cl_command_queue queue,
                                cl_kernel ker,
                                const size_t *global_wk_off,
                                const size_t *global_wk_sz,
                                const size_t *local_wk_sz)
{
  cl_context ctx = queue->ctx;
  intel_gpgpu_t *gpgpu = queue->gpgpu;
  drm_intel_bo *slm_bo = NULL, *private_bo = NULL, *scratch_bo = NULL;
  const size_t cst_sz = ker->patch.curbe.sz;
  size_t wk_grp_sz, wk_grp_n, batch_sz;
  uint32_t grp_end[3], offset[3], thread_n; /* per work group */
  uint32_t i, j, k, curr;
  uint32_t barrierID = 0;
  cl_inline_header_t header;
  cl_local_id_t *ids[3] = {NULL,NULL,NULL};
  cl_int err = CL_SUCCESS;

  /* Allocate 16 kernels (one per barrier) */
  genx_gpgpu_kernel_t kernels[16];
  for (i = 0; i < 16; ++i) {
    kernels[i].name = "OCL kernel";
    kernels[i].grf_blocks = 128;
    kernels[i].cst_sz = cst_sz;
    kernels[i].bin = NULL,
    kernels[i].size = 0,
    kernels[i].bo = ker->bo;
    kernels[i].barrierID = i;
    kernels[i].use_barrier = 0; /* unused in gen6 */
    kernels[i].thread_n = 0;    /* unused in gen6 */
  }

  /* All arguments must have been set */
  TRY (cl_kernel_check_args, ker);

  /* Check that the local work sizes are OK */
  TRY (cl_kernel_work_group_sz, ker, local_wk_sz, 3, &wk_grp_sz);

  /* Directly from the user defined values */
  header.local_sz[0] = local_wk_sz[0];
  header.local_sz[1] = local_wk_sz[1];
  header.local_sz[2] = local_wk_sz[2];
  offset[0] = header.grp_n[0] = 0;
  offset[1] = header.grp_n[1] = 0;
  offset[2] = header.grp_n[2] = 0;
  header.exec_mask = ~0;

  /* offsets are evenly divided by the local sizes */
  offset[0] = global_wk_off[0] / local_wk_sz[0];
  offset[1] = global_wk_off[1] / local_wk_sz[1];
  offset[2] = global_wk_off[2] / local_wk_sz[2];

  /* Compute the local size per wg and the offsets for each local buffer */
  header.local_mem_sz = cl_kernel_local_memory_sz(ker);

  if (queue->perf)
    gpgpu_set_perf_counters(gpgpu, queue->perf->bo);

  /* Setup the kernel */
  gpgpu_state_init(gpgpu, ctx->device->max_compute_unit, cst_sz / 32);
  if (queue->last_batch != NULL)
    drm_intel_bo_unreference(queue->last_batch);
  queue->last_batch = NULL;
  cl_command_queue_bind_surface(queue,
                                ker,
                                &slm_bo,
                                &private_bo,
                                &scratch_bo,
                                header.local_mem_sz);
  gpgpu_states_setup(gpgpu, kernels, 16);

  /* Fill the constant buffer */
  if (cst_sz > 0) {
    char *data = NULL;
    assert(ker->cst_buffer);
    data = cl_kernel_create_cst_buffer(ker,
                                       global_wk_off,
                                       global_wk_sz,
                                       local_wk_sz,
                                       0, 0); /* unused on Gen6 */
    gpgpu_upload_constants(gpgpu, data, cst_sz);
    cl_free(data);
  }

  wk_grp_n = 1;
  for (i = 0; i < 3; ++i) {
    TRY_ALLOC (ids[i], (cl_local_id_t*) cl_malloc(wk_grp_sz*sizeof(uint16_t)));
    grp_end[i] = offset[i] + global_wk_sz[i] / local_wk_sz[i];
    wk_grp_n *= grp_end[i]-offset[i];
  }
  thread_n = wk_grp_sz / 16;
  batch_sz = cl_kernel_compute_batch_sz(ker, wk_grp_n, thread_n);

  /* Start a new batch buffer */
  gpgpu_batch_reset(gpgpu, batch_sz);
  gpgpu_batch_start(gpgpu);
#if 1
  /* Push all media objects. We implement three paths to make it (a bit) faster.
   * Local IDs are shared from work group to work group. We allocate once the
   * buffers and reuse them
   */
  curr = 0;
  for (k = 0; k < local_wk_sz[2]; ++k)
  for (j = 0; j < local_wk_sz[1]; ++j)
  for (i = 0; i < local_wk_sz[0]; ++i, ++curr) {
    ((uint16_t*) ids[0])[curr] = i;
    ((uint16_t*) ids[1])[curr] = j;
    ((uint16_t*) ids[2])[curr] = k;
  }
  for (header.grp_n[0] = offset[0]; header.grp_n[0] < grp_end[0]; ++header.grp_n[0])
  for (header.grp_n[1] = offset[1]; header.grp_n[1] < grp_end[1]; ++header.grp_n[1])
  for (header.grp_n[2] = offset[2]; header.grp_n[2] < grp_end[2]; ++header.grp_n[2]) {
    if (ker->patch.exec_env.has_barriers)
      gpgpu_update_barrier(gpgpu, barrierID, thread_n);
    cl_command_queue_enqueue_wk_grp(queue, ids, &header, thread_n, barrierID);
    barrierID = (barrierID + 1) % 16;
  }
#endif
  gpgpu_batch_end(gpgpu, 0);
  gpgpu_flush(gpgpu);

  if (slm_bo)
    drm_intel_bo_unreference(slm_bo);
  if (private_bo)
    drm_intel_bo_unreference(private_bo);
  if (scratch_bo)
    drm_intel_bo_unreference(scratch_bo);

error:
  cl_free(ids[0]);
  cl_free(ids[1]);
  cl_free(ids[2]);
  return err;
}


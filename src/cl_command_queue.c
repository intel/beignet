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
#include "intel/genx_gpgpu.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

LOCAL cl_command_queue
cl_command_queue_new(cl_context ctx)
{
  cl_command_queue queue = NULL;

  assert(ctx);
  TRY_ALLOC_NO_ERR (queue, CALLOC(struct _cl_command_queue));
  queue->magic = CL_MAGIC_QUEUE_HEADER;
  queue->ref_n = 1;
  queue->ctx = ctx;
  TRY_ALLOC_NO_ERR (queue->gpgpu,
                    intel_gpgpu_new((struct intel_driver*) ctx->intel_drv));

  /* Append the command queue in the list */
  pthread_mutex_lock(&ctx->queue_lock);
    queue->next = ctx->queues;
    if (ctx->queues != NULL)
      ctx->queues->prev = queue;
    ctx->queues = queue;
  pthread_mutex_unlock(&ctx->queue_lock);

  /* The queue also belongs to its context */
  cl_context_add_ref(ctx);

exit:
  return queue;
error:
  cl_command_queue_delete(queue);
  queue = NULL;
  goto exit;
}

LOCAL void
cl_command_queue_delete(cl_command_queue queue)
{
  assert(queue);
  if (atomic_dec(&queue->ref_n) != 1)
    return;

  /* Remove it from the list */
  assert(queue->ctx);
  pthread_mutex_lock(&queue->ctx->queue_lock);
    if (queue->prev)
      queue->prev->next = queue->next;
    if (queue->next)
      queue->next->prev = queue->prev;
    if (queue->next == NULL && queue->prev == NULL)
      queue->ctx->queues = NULL;
  pthread_mutex_unlock(&queue->ctx->queue_lock);
  cl_mem_delete(queue->perf);
  cl_context_delete(queue->ctx);
  intel_gpgpu_delete(queue->gpgpu);
  queue->magic = CL_MAGIC_DEAD_HEADER; /* For safety */
  cl_free(queue);
}

LOCAL void
cl_command_queue_add_ref(cl_command_queue queue)
{
  atomic_inc(&queue->ref_n);
}

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

#define SURFACE_SZ 32

static cl_int
cl_command_queue_bind_surface(cl_command_queue queue,
                              cl_kernel k,
                              drm_intel_bo **local, 
                              uint32_t local_sz,
                              drm_intel_bo **priv,
                              drm_intel_bo **scratch)
{
  cl_context ctx = queue->ctx;
  intel_gpgpu_t *gpgpu = queue->gpgpu;
  drm_intel_bufmgr *bufmgr = cl_context_get_intel_bufmgr(ctx);
  cl_mem mem = NULL;
  drm_intel_bo *bo = NULL, *sync_bo = NULL;
  cl_int err = CL_SUCCESS;
  uint32_t i, index;
  const size_t max_thread = ctx->device->max_compute_unit;

  /* Bind user defined surface */
  for (i = 0; i < k->arg_info_n; ++i) {
    if (k->arg_info[i].type != OCLRT_ARG_TYPE_BUFFER)
      continue;
    assert(k->arg_info[i].offset % SURFACE_SZ == 0);
    index = k->arg_info[i].offset / SURFACE_SZ;
    mem = (cl_mem) k->args[k->arg_info[i].arg_index];
    assert(index != MAX_SURFACES - 1);
    CHECK_MEM(mem);
    bo = mem->bo;
    assert(bo);
    gpgpu_bind_buf(gpgpu, index, bo, 0, bo->size, cc_llc_mlc);
  }

  /* Allocate the constant surface (if any) */
  if (k->const_bo) {
    assert(k->const_bo_index != MAX_SURFACES - 1);
    gpgpu_bind_buf(gpgpu, k->const_bo_index,
                   k->const_bo,
                   0,
                   k->const_bo->size,
                   cc_llc_mlc);
  }

  /* Allocate local surface needed for SLM and bind it */
  if (local_sz != 0) {
    const size_t sz = 16 * local_sz; /* XXX 16 == maximum barrier number */
    assert(k->patch.local_surf.offset % SURFACE_SZ == 0);
    index = k->patch.local_surf.offset / SURFACE_SZ;
    assert(index != MAX_SURFACES - 1);
    *local = drm_intel_bo_alloc(bufmgr, "CL local surface", sz, 64);
    gpgpu_bind_buf(gpgpu, index, *local, 0, sz, cc_llc_mlc);
  }
  else
    *local = NULL;

  /* Allocate private surface and bind it */
  if (k->patch.private_surf.size != 0) {
    const size_t sz = max_thread *
                      k->patch.private_surf.size *
                      k->patch.exec_env.largest_compiled_simd_sz;
    assert(k->patch.exec_env.largest_compiled_simd_sz == 16);
    assert(k->patch.private_surf.offset % SURFACE_SZ == 0);
    index = k->patch.private_surf.offset / SURFACE_SZ;
    assert(index != MAX_SURFACES - 1);
    *priv = drm_intel_bo_alloc(bufmgr, "CL private surface", sz, 64);
    gpgpu_bind_buf(gpgpu, index, *priv, 0, sz, cc_llc_mlc);
  }
  else
    *priv = NULL;

  /* Allocate scratch surface and bind it */
  if (k->patch.scratch.size != 0) {
    const size_t sz = max_thread * /* XXX is it given per lane ??? */
                      k->patch.scratch.size *
                      k->patch.exec_env.largest_compiled_simd_sz;
    assert(k->patch.exec_env.largest_compiled_simd_sz == 16);
    assert(k->patch.scratch.offset % SURFACE_SZ == 0);
    assert(index != MAX_SURFACES - 1);
    index = k->patch.scratch.offset / SURFACE_SZ;
    *scratch = drm_intel_bo_alloc(bufmgr, "CL scratch surface", sz, 64);
    gpgpu_bind_buf(gpgpu, index, *scratch, 0, sz, cc_llc_mlc);
  }
  else
    *scratch = NULL;

  /* Now bind a bo used for synchronization */
  sync_bo = drm_intel_bo_alloc(bufmgr, "sync surface", 64, 64);
  gpgpu_bind_buf(gpgpu, MAX_SURFACES-1, sync_bo, 0, 64, cc_llc_mlc);
  if (queue->last_batch != NULL)
    drm_intel_bo_unreference(queue->last_batch);
  queue->last_batch = sync_bo;

error:
  assert(err == CL_SUCCESS); /* Cannot fail here */
  return err;
}

static INLINE cl_int
cl_kernel_check_args(cl_kernel k)
{
  uint32_t i;
  for (i = 0; i < k->arg_n; ++i)
    if (k->is_provided[i] == CL_FALSE)
      return CL_INVALID_KERNEL_ARGS;
  return CL_SUCCESS;
}

static INLINE void
cl_command_queue_enqueue_wrk_grp3(cl_command_queue queue,
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

static INLINE void
cl_command_queue_enqueue_wrk_grp2(cl_command_queue queue,
                                  cl_local_id_t **ids,
                                  const cl_inline_header_t *header,
                                  uint32_t thread_n,
                                  uint32_t barrierID)
{
  intel_gpgpu_t *gpgpu = queue->gpgpu;
  uint32_t i;
  for (i = 0; i < thread_n; ++i) {
    const size_t sz = sizeof(cl_inline_header_t) + 2*sizeof(cl_local_id_t);
    char *data = gpgpu_run_with_inline(gpgpu, barrierID, sz);
    size_t offset = 0;
    assert(data);
    *((cl_inline_header_t *) (data + offset)) = *header;
    offset += sizeof(cl_inline_header_t);
    *((cl_local_id_t *) (data + offset)) = ids[0][i];
    offset += sizeof(cl_local_id_t);
    *((cl_local_id_t *) (data + offset)) = ids[1][i];
  }
}

static INLINE void
cl_command_queue_enqueue_wrk_grp1(cl_command_queue queue,
                                  cl_local_id_t **ids,
                                  const cl_inline_header_t *header,
                                  uint32_t thread_n,
                                  uint32_t barrierID)
{
  intel_gpgpu_t *gpgpu = queue->gpgpu;
  uint32_t i;
  for (i = 0; i < thread_n; ++i) {
    const size_t sz = sizeof(cl_inline_header_t) + sizeof(cl_local_id_t);
    char *data = gpgpu_run_with_inline(gpgpu, barrierID, sz);
    size_t offset = 0;
    assert(data);
    *((cl_inline_header_t *) (data + offset)) = *header;
    offset += sizeof(cl_inline_header_t);
    *((cl_local_id_t *) (data + offset)) = ids[0][i];
  }
}

static INLINE int32_t
cl_kernel_get_first_local(cl_kernel k)
{
  int32_t i;
  for (i = 0; i < (int32_t) k->curbe_info_n; ++i)
    if (k->curbe_info[i].type == DATA_PARAMETER_SUM_OF_LOCAL_MEMORY_ARGUMENT_SIZES)
      return i;
  return k->curbe_info_n;
}

static void
cl_kernel_handle_local_memory(cl_kernel k, cl_inline_header_t *header)
{
  int32_t i;

  if (k->has_local_buffer) {
    header->local_mem_sz = 0;

    /* Look for all local surfaces offset to set */
    i = cl_kernel_get_first_local(k);

    /* Now, set the offsets for all local surfaces */
    for (; i < (int32_t) k->curbe_info_n; ++i) {
      cl_curbe_patch_info_t *info = k->curbe_info + i;
      const size_t offset = header->local_mem_sz;
      if (info->type != DATA_PARAMETER_SUM_OF_LOCAL_MEMORY_ARGUMENT_SIZES)
        break;
      assert(info->last == 0);
      assert(sizeof(int32_t) + info->offsets[0] <= k->patch.curbe.sz);
      memcpy(k->cst_buffer + info->offsets[0], &offset, sizeof(int32_t));
      header->local_mem_sz += info->sz;
    }
    header->local_mem_sz += k->patch.local_surf.sz;
  }
  else
    header->local_mem_sz = 0;
}

static INLINE size_t
cl_ker_compute_batch_sz(cl_kernel k,
                        size_t wrk_dim_n,
                        size_t wrk_grp_n,
                        size_t thread_n)
{
  size_t sz = 256; /* upper bound of the complete prelude */
  size_t media_obj_sz = 6 * 4; /* size of one MEDIA OBJECT */
  media_obj_sz += sizeof(cl_inline_header_t); /* header for all threads */
  media_obj_sz += wrk_dim_n * sizeof(cl_local_id_t);/* for each dimension */ 
  if (k->patch.exec_env.has_barriers)
    media_obj_sz += 4 * 4; /* one barrier update per object */
  sz += media_obj_sz * wrk_grp_n * thread_n;
  return sz;
}

LOCAL cl_int
cl_command_queue_set_report_buffer(cl_command_queue queue,
                                   cl_mem mem)
{
  cl_int err = CL_SUCCESS;
  if (queue->perf != NULL) {
    cl_mem_delete(queue->perf);
    queue->perf = NULL;
  }
  if (mem != NULL) {
    if (mem->bo->size < 1024) { /* 1K for the performance counters is enough */
      err = CL_INVALID_BUFFER_SIZE;
      goto error;
    }
    cl_mem_add_ref(mem);
    queue->perf = mem;
  }

error:
  return err;
}

static char*
cl_kernel_create_cst_buffer(cl_kernel k, 
                            cl_uint work_dim,
                            const size_t *global_wk_sz,
                            const size_t *local_wk_sz)
{
  cl_curbe_patch_info_t *info = NULL;
  const size_t sz = k->patch.curbe.sz;
  uint64_t key = 0;
  char *data = NULL;

  TRY_ALLOC_NO_ERR (data, (char *) cl_calloc(sz, 1));
  memcpy(data, k->cst_buffer, sz);

  /* Global work group size */
  key = cl_curbe_key(DATA_PARAMETER_GLOBAL_WORK_SIZE, 0, 0);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL)
    memcpy(data+info->offsets[0], global_wk_sz,   sizeof(uint32_t));
  key = cl_curbe_key(DATA_PARAMETER_GLOBAL_WORK_SIZE, 0, 4);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL)
    memcpy(data+info->offsets[0], global_wk_sz+1, sizeof(uint32_t));
  key = cl_curbe_key(DATA_PARAMETER_GLOBAL_WORK_SIZE, 0, 8);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL)
    memcpy(data+info->offsets[0], global_wk_sz+2, sizeof(uint32_t));

  /* Local work group size */
  key = cl_curbe_key(DATA_PARAMETER_LOCAL_WORK_SIZE, 0, 0);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL)
    memcpy(data+info->offsets[0], local_wk_sz,   sizeof(uint32_t));
  key = cl_curbe_key(DATA_PARAMETER_LOCAL_WORK_SIZE, 0, 4);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL)
    memcpy(data+info->offsets[0], local_wk_sz+1, sizeof(uint32_t));
  key = cl_curbe_key(DATA_PARAMETER_LOCAL_WORK_SIZE, 0, 8);
  if ((info = cl_kernel_get_curbe_info(k, key)) != NULL)
    memcpy(data+info->offsets[0], local_wk_sz+2, sizeof(uint32_t));

exit:
  return data;
error:
  cl_free(data);
  data = NULL;
  goto exit;
}

#if USE_FULSIM
LOCAL void
cl_run_fulsim(void)
{
  const char *run_it = getenv("OCL_FULSIM_RUN");
  const char *debug_mode = getenv("OCL_FULSIM_DEBUG_MODE");
  if (run_it == NULL || strcmp(run_it, "1"))
    return;
#if EMULATE_GEN == 6 /* SNB */
  if (debug_mode == NULL || strcmp(debug_mode, "1"))
    system("wine AubLoad.exe dump.aub -device sbrB0");
  else
    system("wine AubLoad.exe dump.aub -device sbrB0 -debug");
#elif EMULATE_GEN == 7
  if (debug_mode == NULL || strcmp(debug_mode, "1"))
    system("wine AubLoad.exe dump.aub -device ivb2");
  else
    system("wine AubLoad.exe dump.aub -device ivb2 -debug");
#endif
}
#endif /* USE_FULSIM */

LOCAL cl_int
cl_command_queue_ND_kernel(cl_command_queue queue,
                           cl_kernel ker,
                           cl_uint work_dim,
                           const size_t *global_work_offset,
                           const size_t *global_wk_sz,
                           const size_t *local_wk_sz)
{
  cl_context ctx = queue->ctx;
  intel_gpgpu_t *gpgpu = queue->gpgpu;
  drm_intel_bo *slm_bo = NULL, *private_bo = NULL, *scratch_bo = NULL;
  size_t cst_sz = ker->patch.curbe.sz;
  size_t wrk_grp_sz, wrk_grp_n, batch_sz;
  uint32_t grp_end[3], offset[3], thread_n; /* per work group */
  uint32_t i, j, k, curr;
  uint32_t barrierID = 0;
  genx_gpgpu_kernel_t *kernels = NULL;

  cl_inline_header_t header;
  cl_local_id_t *ids[3] = {NULL,NULL,NULL};
  cl_int err = CL_SUCCESS;

  /* Allocate 16 kernels (one for each barrier) */
  TRY_ALLOC (kernels, CALLOC_ARRAY(genx_gpgpu_kernel_t, 16));
  for (i = 0; i < 16; ++i) {
    kernels[i].name = "OCL kernel";
    kernels[i].grf_blocks = 128;
    kernels[i].cst_sz = cst_sz;
    kernels[i].bin = NULL,
    kernels[i].size = 0,
    kernels[i].bo = ker->bo;
    kernels[i].barrierID = i;
  }

  /* All arguments must have been set */
  TRY (cl_kernel_check_args, ker);

  /* Total number of elements in the work group */
  for (i = 0; i < work_dim; ++i)
    if ((&ker->patch.exec_env.required_wgr_sz_x)[i] &&
        (&ker->patch.exec_env.required_wgr_sz_x)[i] != local_wk_sz[i]) {
      err = CL_INVALID_WORK_ITEM_SIZE;
      goto error;
    }
  wrk_grp_sz = local_wk_sz[0];
  for (i = 1; i < work_dim; ++i)
    wrk_grp_sz *= local_wk_sz[i];
  FATAL_IF (wrk_grp_sz % 16, "Work group size must be a multiple of 16");
  if (wrk_grp_sz > ctx->device->max_work_group_size) {
    err = CL_INVALID_WORK_ITEM_SIZE;
    goto error;
  }

  /* Directly from the user defined values */
  header.local_sz[0] = local_wk_sz[0];
  header.local_sz[1] = local_wk_sz[1];
  header.local_sz[2] = local_wk_sz[2];
  offset[0] = header.grp_n[0] = 0;
  offset[1] = header.grp_n[1] = 0;
  offset[2] = header.grp_n[2] = 0;
  header.exec_mask = ~0;

  /* offsets are evenly divided by the local sizes */
  if (global_work_offset)
    for (i = 0; i < work_dim; ++i)
      offset[i] = global_work_offset[i]/local_wk_sz[i];

  /* Compute the local size per wg and the offsets for each local buffer */
  cl_kernel_handle_local_memory(ker, &header);

  if (queue->perf)
    gpgpu_set_perf_counters(gpgpu, queue->perf->bo);

  /* Setup the kernel */
  gpgpu_state_init(gpgpu, ctx->device->max_compute_unit, 4, 64, cst_sz / 32, 64);
  if (queue->last_batch != NULL)
    drm_intel_bo_unreference(queue->last_batch);
  queue->last_batch = NULL;
  cl_command_queue_bind_surface(queue,
                                ker,
                                &slm_bo,
                                header.local_mem_sz,
                                &private_bo,
                                &scratch_bo);
  gpgpu_states_setup(gpgpu, kernels, 16);

  /* Fill the constant buffer */
  if (cst_sz > 0) {
    char *data = NULL;
    assert(ker->cst_buffer);
    data = cl_kernel_create_cst_buffer(ker,work_dim,global_wk_sz,local_wk_sz);
    gpgpu_upload_constants(gpgpu, data, cst_sz);
    cl_free(data);
  }

  wrk_grp_n = 1;
  for (i = 0; i < work_dim; ++i) {
    TRY_ALLOC (ids[i], (cl_local_id_t*) cl_malloc(wrk_grp_sz*sizeof(uint16_t)));
    grp_end[i] = offset[i] + global_wk_sz[i] / local_wk_sz[i];
    wrk_grp_n *= grp_end[i]-offset[i];
  }
  thread_n = wrk_grp_sz / 16;
  batch_sz = cl_ker_compute_batch_sz(ker, work_dim, wrk_grp_n, thread_n);

  /* Start a new batch buffer */
  gpgpu_batch_reset(gpgpu, batch_sz);
  gpgpu_batch_start(gpgpu);
#if 1
  /* Push all media objects. We implement three paths to make it (a bit) faster.
   * Local IDs are shared from work group to work group. We allocate once the
   * buffers and reuse them
   */
  if (work_dim == 3) {
    curr = 0;
    for (i = 0; i < local_wk_sz[0]; ++i)
    for (j = 0; j < local_wk_sz[1]; ++j)
    for (k = 0; k < local_wk_sz[2]; ++k, ++curr) {
      ((uint16_t*) ids[0])[curr] = i;
      ((uint16_t*) ids[1])[curr] = j;
      ((uint16_t*) ids[2])[curr] = k;
    }
    for (header.grp_n[0] = offset[0]; header.grp_n[0] < grp_end[0]; ++header.grp_n[0])
    for (header.grp_n[1] = offset[1]; header.grp_n[1] < grp_end[1]; ++header.grp_n[1])
    for (header.grp_n[2] = offset[2]; header.grp_n[2] < grp_end[2]; ++header.grp_n[2]) {
      if (ker->patch.exec_env.has_barriers)
        gpgpu_update_barrier(gpgpu, barrierID, thread_n);
      cl_command_queue_enqueue_wrk_grp3(queue, ids, &header, thread_n, barrierID);
      barrierID = (barrierID + 1) % 16;
    }
  }
  else if (work_dim == 2) {
    curr = 0;
    for (i = 0; i < local_wk_sz[0]; ++i)
    for (j = 0; j < local_wk_sz[1]; ++j, ++curr) {
      ((uint16_t*) ids[0])[curr] = i;
      ((uint16_t*) ids[1])[curr] = j;
    }
    for (header.grp_n[0] = offset[0]; header.grp_n[0] < grp_end[0]; ++header.grp_n[0])
    for (header.grp_n[1] = offset[1]; header.grp_n[1] < grp_end[1]; ++header.grp_n[1]) {
      if (ker->patch.exec_env.has_barriers)
        gpgpu_update_barrier(gpgpu, barrierID, thread_n);
      cl_command_queue_enqueue_wrk_grp2(queue, ids, &header, thread_n, barrierID);
      barrierID = (barrierID + 1) % 16;
    }
  }
  else {
    for (i = 0; i < local_wk_sz[0]; ++i)
      ((uint16_t*) ids[0])[i] = i;
    for (header.grp_n[0] = offset[0]; header.grp_n[0] < grp_end[0]; ++header.grp_n[0]) {
      if (ker->patch.exec_env.has_barriers)
        gpgpu_update_barrier(gpgpu, barrierID, thread_n);
      cl_command_queue_enqueue_wrk_grp1(queue, ids, &header, thread_n, barrierID);
      barrierID = (barrierID + 1) % 16;
    }
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

#if USE_FULSIM
  cl_run_fulsim();
#endif /* USE_FULSIM */

error:
  cl_free(kernels);
  cl_free(ids[0]);
  cl_free(ids[1]);
  cl_free(ids[2]);
  return err;
}

LOCAL cl_int
cl_command_queue_finish(cl_command_queue queue)
{
  if (queue->last_batch == NULL)
    return CL_SUCCESS;
  drm_intel_bo_wait_rendering(queue->last_batch);
  drm_intel_bo_unreference(queue->last_batch);
  queue->last_batch = NULL;
  return CL_SUCCESS;
}

LOCAL cl_int
cl_command_queue_set_fulsim_buffer(cl_command_queue queue, cl_mem mem)
{
#if USE_FULSIM
  cl_context ctx = queue->ctx;
  drm_intel_bufmgr *bufmgr = cl_context_get_intel_bufmgr(ctx);
  drm_intel_aub_set_bo_to_dump(bufmgr, mem->bo);
#endif /* USE_FULSIM */

  queue->fulsim_out = mem;
  if (queue->fulsim_out != NULL) {
    cl_mem_delete(queue->fulsim_out);
    queue->fulsim_out = NULL;
  }
  if (mem != NULL) {
    cl_mem_add_ref(mem);
    queue->fulsim_out = mem;
  }

  return CL_SUCCESS;
}


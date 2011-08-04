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

#define SURFACE_SZ 32

extern cl_int
cl_command_queue_bind_surface(cl_command_queue queue,
                              cl_kernel k,
                              drm_intel_bo **local, 
                              drm_intel_bo **priv,
                              drm_intel_bo **scratch,
                              uint32_t local_sz)
{
  cl_context ctx = queue->ctx;
  intel_gpgpu_t *gpgpu = queue->gpgpu;
  drm_intel_bufmgr *bufmgr = cl_context_get_intel_bufmgr(ctx);
  cl_mem mem = NULL;
  drm_intel_bo *bo = NULL, *sync_bo = NULL;
  const size_t max_thread = ctx->device->max_compute_unit;
  cl_int err = CL_SUCCESS;
  uint32_t i, index;

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
  if (local && local_sz != 0) {
    const size_t sz = 16 * local_sz; /* XXX 16 == maximum barrier number */
    assert(k->patch.local_surf.offset % SURFACE_SZ == 0);
    index = k->patch.local_surf.offset / SURFACE_SZ;
    assert(index != MAX_SURFACES - 1);
    *local = drm_intel_bo_alloc(bufmgr, "CL local surface", sz, 64);
    gpgpu_bind_buf(gpgpu, index, *local, 0, sz, cc_llc_mlc);
  }
  else if (local)
    *local = NULL;

  /* Allocate private surface and bind it */
  if (priv && k->patch.private_surf.size != 0) {
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
  else if(priv)
    *priv = NULL;

  /* Allocate scratch surface and bind it */
  if (scratch && k->patch.scratch.size != 0) {
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
  else if (scratch)
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

LOCAL cl_int
cl_kernel_check_args(cl_kernel k)
{
  uint32_t i;
  for (i = 0; i < k->arg_n; ++i)
    if (k->is_provided[i] == CL_FALSE)
      return CL_INVALID_KERNEL_ARGS;
  return CL_SUCCESS;
}

LOCAL cl_int
cl_command_queue_set_report_buffer(cl_command_queue queue, cl_mem mem)
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

extern cl_int cl_command_queue_ND_range_gen6(cl_command_queue, cl_kernel, const size_t*, const size_t*, const size_t*);
extern cl_int cl_command_queue_ND_range_gen7(cl_command_queue, cl_kernel, const size_t *, const size_t *, const size_t *);

LOCAL cl_int
cl_command_queue_ND_range(cl_command_queue queue,
                          cl_kernel k,
                          const size_t *global_wk_off,
                          const size_t *global_wk_sz,
                          const size_t *local_wk_sz)
{
  intel_gpgpu_t *gpgpu = queue->gpgpu;
  const int32_t ver = intel_gpgpu_version(gpgpu);
  cl_int err = CL_SUCCESS;

  if (ver == 6)
    TRY (cl_command_queue_ND_range_gen6, queue, k, global_wk_off, global_wk_sz, local_wk_sz);
  else if (ver == 7)
    TRY (cl_command_queue_ND_range_gen7, queue, k, global_wk_off, global_wk_sz, local_wk_sz);
  else
    FATAL ("Unknown Gen Device");

#if USE_FULSIM
  cl_run_fulsim();
#endif /* USE_FULSIM */

error:
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

/* We added this function in libdrm_intel to dump a binary buffer */
extern int drm_intel_aub_set_bo_to_dump(drm_intel_bufmgr*, drm_intel_bo*);

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


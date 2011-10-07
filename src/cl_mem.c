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

#include "cl_mem.h"
#include "cl_context.h"
#include "cl_utils.h"
#include "cl_alloc.h"

#include "intel_bufmgr.h" /* libdrm_intel */

#include "CL/cl.h"
#include "CL/cl_intel.h"
#include <assert.h>
#include <stdio.h>

LOCAL cl_mem
cl_mem_new(cl_context ctx,
           cl_mem_flags flags,
           size_t sz,
           void *data,
           cl_int *errcode_ret)
{
  drm_intel_bufmgr *bufmgr = NULL;
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  size_t alignment = 64;

  assert(ctx);
  FATAL_IF (flags & CL_MEM_ALLOC_HOST_PTR,
            "CL_MEM_ALLOC_HOST_PTR unsupported"); /* XXX */
  FATAL_IF (flags & CL_MEM_USE_HOST_PTR,
            "CL_MEM_USE_HOST_PTR unsupported");   /* XXX */
  if (UNLIKELY(sz == 0)) {
    err = CL_INVALID_BUFFER_SIZE;
    goto error;
  }
  if (UNLIKELY(flags & CL_MEM_COPY_HOST_PTR && data == NULL)) {
    err = CL_INVALID_HOST_PTR;
    goto error;
  }

  /* Allocate and inialize the structure itself */
  TRY_ALLOC (mem, CALLOC(struct _cl_mem));
  mem->ref_n = 1;
  mem->magic = CL_MAGIC_MEM_HEADER;
  mem->flags = flags;

  /* Pinning will require stricter alignment rules */
  if (flags & CL_MEM_PINNABLE)
    alignment = 4096;

  /* Allocate space in memory */
  bufmgr = cl_context_get_intel_bufmgr(ctx);
  assert(bufmgr);
  mem->bo = drm_intel_bo_alloc(bufmgr, "CL memory object", sz, alignment);
  if (UNLIKELY(mem->bo == NULL)) {
    err = CL_MEM_ALLOCATION_FAILURE;
    goto error;
  }
  /* Copy the data if required */
  if (flags & CL_MEM_COPY_HOST_PTR) /* TODO check other flags too */
	drm_intel_bo_subdata(mem->bo, 0, sz, data);
   #if 0	
    if (UNLIKELY(drm_intel_bo_subdata(mem->bo, 0, sz, data) != 0)) {
      err = CL_MEM_ALLOCATION_FAILURE;
      goto error;
    }
   #endif
	

  /* Append the buffer in the context buffer list */
  pthread_mutex_lock(&ctx->buffer_lock);
    mem->next = ctx->buffers;
    if (ctx->buffers != NULL)
      ctx->buffers->prev = mem;
    ctx->buffers = mem;
  pthread_mutex_unlock(&ctx->buffer_lock);
  mem->ctx = ctx;
  cl_context_add_ref(ctx);

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
error:
  cl_mem_delete(mem);
  mem = NULL;
  goto exit;
}

LOCAL void
cl_mem_delete(cl_mem mem)
{
  if (UNLIKELY(mem == NULL))
    return;
  if (atomic_dec(&mem->ref_n) > 1)
    return;
  if (LIKELY(mem->bo != NULL))
    drm_intel_bo_unreference(mem->bo);

  /* Remove it from the list */
  assert(mem->ctx);
  pthread_mutex_lock(&mem->ctx->buffer_lock);
    if (mem->prev)
      mem->prev->next = mem->next;
    if (mem->next)
      mem->next->prev = mem->prev;
    if (mem->prev == NULL && mem->next == NULL)
      mem->ctx->buffers = NULL;
  pthread_mutex_unlock(&mem->ctx->buffer_lock);
  cl_context_delete(mem->ctx);

  cl_free(mem);
}

LOCAL void
cl_mem_add_ref(cl_mem mem)
{
  assert(mem);
  atomic_inc(&mem->ref_n);
}

LOCAL void*
cl_mem_map(cl_mem mem)
{
  drm_intel_bo_map(mem->bo, 1);
  assert(mem->bo->virtual);
  return mem->bo->virtual;
}

LOCAL cl_int
cl_mem_unmap(cl_mem mem)
{
  drm_intel_bo_unmap(mem->bo);
  return CL_SUCCESS;
}

LOCAL cl_int
cl_mem_pin(cl_mem mem)
{
  assert(mem);
  if ((mem->flags & CL_MEM_PINNABLE) == 0)
    return CL_INVALID_MEM;
  printf("%x\n", mem->bo->offset / 16);
  drm_intel_bo_pin(mem->bo, 4096);
  printf("%x\n", mem->bo->offset / 16);
  return CL_SUCCESS;
}

LOCAL cl_int
cl_mem_unpin(cl_mem mem)
{
  assert(mem);
  if ((mem->flags & CL_MEM_PINNABLE) == 0)
    return CL_INVALID_MEM;
  drm_intel_bo_unpin(mem->bo);
  return CL_SUCCESS;
}


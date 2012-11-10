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

#include "cl_kernel.h"
#include "cl_program.h"
#include "cl_device_id.h"
#include "cl_context.h"
#include "cl_mem.h"
#include "cl_alloc.h"
#include "cl_utils.h"
#include "CL/cl.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

LOCAL void
cl_kernel_delete(cl_kernel k)
{
  uint32_t i;
  if (k == NULL) return;

  /* We are not done with the kernel */
  if (atomic_dec(&k->ref_n) > 1) return;
  /* Release one reference on all bos we own */
  if (k->bo)       cl_buffer_unreference(k->bo);
  if (k->const_bo) cl_buffer_unreference(k->const_bo);
  /* This will be true for kernels created by clCreateKernel */
  if (k->ref_its_program) cl_program_delete(k->program);
  /* Release the curbe if allocated */
  if (k->curbe) cl_free(k->curbe);
  /* Release the argument array if required */
  if (k->args) {
    for (i = 0; i < k->arg_n; ++i)
      if (k->args[i].mem != NULL)
        cl_mem_delete(k->args[i].mem);
    cl_free(k->args);
  }
  k->magic = CL_MAGIC_DEAD_HEADER; /* For safety */
  cl_free(k);
}

LOCAL cl_kernel
cl_kernel_new(cl_program p)
{
  cl_kernel k = NULL;
  TRY_ALLOC_NO_ERR (k, CALLOC(struct _cl_kernel));
  k->ref_n = 1;
  k->magic = CL_MAGIC_KERNEL_HEADER;
  k->program = p;

exit:
  return k;
error:
  cl_kernel_delete(k);
  k = NULL;
  goto exit;
}

LOCAL const char*
cl_kernel_get_name(cl_kernel k)
{
  if (UNLIKELY(k == NULL)) return NULL;
  return gbe_kernel_get_name(k->opaque);
}

LOCAL void
cl_kernel_add_ref(cl_kernel k)
{
  atomic_inc(&k->ref_n);
}

LOCAL cl_int
cl_kernel_set_arg(cl_kernel k, cl_uint index, size_t sz, const void *value)
{
  uint32_t offset;            /* where to patch */
  enum gbe_arg_type arg_type; /* kind of argument */
  size_t arg_sz;              /* size of the argument */
  cl_mem mem;                 /* for __global, __constant and image arguments */

  if (UNLIKELY(index >= k->arg_n))
    return CL_INVALID_ARG_INDEX;
  arg_type = gbe_kernel_get_arg_type(k->opaque, index);
  arg_sz = gbe_kernel_get_arg_size(k->opaque, index);
  if (UNLIKELY(arg_type != GBE_ARG_LOCAL_PTR && arg_sz != sz))
    return CL_INVALID_ARG_SIZE;

  /* Copy the structure or the value directly into the curbe */
  if (arg_type == GBE_ARG_VALUE) {
    if (UNLIKELY(value == NULL))
      return CL_INVALID_KERNEL_ARGS;
    offset = gbe_kernel_get_curbe_offset(k->opaque, GBE_CURBE_KERNEL_ARGUMENT, index);
    assert(offset + sz <= k->curbe_sz);
    memcpy(k->curbe + offset, value, sz);
    k->args[index].local_sz = 0;
    k->args[index].is_set = 1;
    k->args[index].mem = NULL;
    return CL_SUCCESS;
  }

  /* For a local pointer just save the size */
  if (arg_type == GBE_ARG_LOCAL_PTR) {
    if (UNLIKELY(value != NULL))
      return CL_INVALID_KERNEL_ARGS;
    k->args[index].local_sz = sz;
    k->args[index].is_set = 1;
    k->args[index].mem = NULL;
    return CL_SUCCESS;
  }

  /* Otherwise, we just need to check that this is a buffer */
  if (UNLIKELY(value == NULL))
    return CL_INVALID_KERNEL_ARGS;
  mem = *(cl_mem*) value;
  if (UNLIKELY(mem->magic != CL_MAGIC_MEM_HEADER))
    return CL_INVALID_ARG_VALUE;
  if (mem->is_image)
    if (UNLIKELY(arg_type == GBE_ARG_IMAGE))
      return CL_INVALID_ARG_VALUE;
  cl_mem_add_ref(mem);
  if (k->args[index].mem)
    cl_mem_delete(k->args[index].mem);
  k->args[index].mem = mem;
  k->args[index].is_set = 1;
  k->args[index].local_sz = 0;

  return CL_SUCCESS;
}

LOCAL uint32_t
cl_kernel_get_simd_width(cl_kernel k)
{
  assert(k != NULL);
  return gbe_kernel_get_simd_width(k->opaque);
}

LOCAL void
cl_kernel_setup(cl_kernel k, gbe_kernel opaque)
{
  cl_context ctx = k->program->ctx;
  cl_buffer_mgr bufmgr = cl_context_get_bufmgr(ctx);

  /* Allocate the gen code here */
  const uint32_t code_sz = gbe_kernel_get_code_size(opaque);
  const char *code = gbe_kernel_get_code(opaque);
  k->bo = cl_buffer_alloc(bufmgr, "CL kernel", code_sz, 64u);
  k->arg_n = gbe_kernel_get_arg_num(opaque);

  /* Upload the code */
  cl_buffer_subdata(k->bo, 0, code_sz, code);
  k->opaque = opaque;

  /* Create the curbe */
  k->curbe_sz = gbe_kernel_get_curbe_size(k->opaque);
}

LOCAL cl_kernel
cl_kernel_dup(cl_kernel from)
{
  cl_kernel to = NULL;

  if (UNLIKELY(from == NULL))
    return NULL;
  TRY_ALLOC_NO_ERR (to, CALLOC(struct _cl_kernel));
  to->bo = from->bo;
  to->const_bo = from->const_bo;
  to->opaque = from->opaque;
  to->ref_n = 1;
  to->magic = CL_MAGIC_KERNEL_HEADER;
  to->program = from->program;
  to->arg_n = from->arg_n;
  to->curbe_sz = from->curbe_sz;
  TRY_ALLOC_NO_ERR(to->args, cl_calloc(to->arg_n, sizeof(cl_argument)));
  if (to->curbe_sz) TRY_ALLOC_NO_ERR(to->curbe, cl_calloc(1, to->curbe_sz));

  /* Retain the bos */
  if (from->bo)       cl_buffer_reference(from->bo);
  if (from->const_bo) cl_buffer_reference(from->const_bo);

  /* We retain the program destruction since this kernel (user allocated)
   * depends on the program for some of its pointers
   */
  assert(from->program);
  cl_program_add_ref(from->program);
  to->ref_its_program = CL_TRUE;

exit:
  return to;
error:
  cl_kernel_delete(to);
  to = NULL;
  goto exit;
}

LOCAL cl_int
cl_kernel_work_group_sz(cl_kernel ker,
                        const size_t *local_wk_sz,
                        uint32_t wk_dim,
                        size_t *wk_grp_sz)
{
  cl_int err = CL_SUCCESS;
  size_t sz = 0;
  cl_uint i;

  for (i = 0; i < wk_dim; ++i) {
    const uint32_t required_sz = gbe_kernel_get_required_work_group_size(ker->opaque, i);
    if (required_sz != 0 && required_sz != local_wk_sz[i]) {
      err = CL_INVALID_WORK_ITEM_SIZE;
      goto error;
    }
  }
  sz = local_wk_sz[0];
  for (i = 1; i < wk_dim; ++i)
    sz *= local_wk_sz[i];
  FATAL_IF (sz % 16, "Work group size must be a multiple of 16");
  if (sz > ker->program->ctx->device->max_work_group_size) {
    err = CL_INVALID_WORK_ITEM_SIZE;
    goto error;
  }

error:
  if (wk_grp_sz) *wk_grp_sz = sz;
  return err;
}



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
#include "gen/program.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

LOCAL void
cl_kernel_delete(cl_kernel k)
{
  if (k == NULL)
    return;

  /* We are not done with the kernel */
  if (atomic_dec(&k->ref_n) > 1) return;

  /* Release one reference on all bos we own */
  if (k->bo)       cl_buffer_unreference(k->bo);
  if (k->const_bo) cl_buffer_unreference(k->const_bo);

  /* This will be true for kernels created by clCreateKernel */
  if (k->ref_its_program) cl_program_delete(k->program);

  k->magic = CL_MAGIC_DEAD_HEADER; /* For safety */
  cl_free(k);
}

LOCAL cl_kernel
cl_kernel_new(const cl_program p)
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
cl_kernel_get_name(const cl_kernel k)
{
  if (UNLIKELY(k == NULL)) return NULL;
  return GenKernelGetName(k->gen_kernel);
}

LOCAL void
cl_kernel_add_ref(cl_kernel k)
{
  atomic_inc(&k->ref_n);
}

LOCAL cl_int
cl_kernel_set_arg(cl_kernel k, cl_uint index, size_t sz, const void *value)
{
  cl_int err = CL_SUCCESS;

  return err;
}

LOCAL uint32_t
cl_kernel_get_simd_width(const cl_kernel k)
{
  assert(k != NULL);
  return GenKernelGetSIMDWidth(k->gen_kernel);
}

LOCAL void
cl_kernel_setup(cl_kernel k, const struct GenKernel *gen_kernel)
{
  cl_context ctx = k->program->ctx;
  cl_buffer_mgr *bufmgr = cl_context_get_bufmgr(ctx);

  /* Allocate the gen code here */
  const uint32_t code_sz = GenKernelGetCodeSize(gen_kernel);
  const char *code = GenKernelGetCode(gen_kernel);
  k->bo = cl_buffer_alloc(bufmgr, "CL kernel", code_sz, 64u);

  /* Upload the code */
  cl_buffer_subdata(k->bo, 0, code_sz, code);
  k->gen_kernel = gen_kernel;
}

LOCAL cl_kernel
cl_kernel_dup(const cl_kernel from)
{
  cl_kernel to = NULL;

  if (UNLIKELY(from == NULL))
    return NULL;
  TRY_ALLOC_NO_ERR (to, CALLOC(struct _cl_kernel));
  to->bo = from->bo;
  to->const_bo = from->const_bo;
  to->gen_kernel = from->gen_kernel;
  to->ref_n = 1;
  to->magic = CL_MAGIC_KERNEL_HEADER;
  to->program = from->program;

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
    const uint32_t required_sz = GenKernelGetRequiredWorkGroupSize(ker->gen_kernel, i);
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
  if (wk_grp_sz)
    *wk_grp_sz = sz;
  return err;
}



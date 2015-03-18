/*
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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
#include "cl_khr_icd.h"
#include "CL/cl.h"
#include "cl_sampler.h"

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
  if (k->image_sz)
    cl_free(k->images);
  k->magic = CL_MAGIC_DEAD_HEADER; /* For safety */
  cl_free(k);
}

LOCAL cl_kernel
cl_kernel_new(cl_program p)
{
  cl_kernel k = NULL;
  TRY_ALLOC_NO_ERR (k, CALLOC(struct _cl_kernel));
  SET_ICD(k->dispatch)
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
  return interp_kernel_get_name(k->opaque);
}

LOCAL const char*
cl_kernel_get_attributes(cl_kernel k)
{
  if (UNLIKELY(k == NULL)) return NULL;
  return interp_kernel_get_attributes(k->opaque);
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
  cl_mem mem = NULL;          /* for __global, __constant and image arguments */
  cl_context ctx = k->program->ctx;

  if (UNLIKELY(index >= k->arg_n))
    return CL_INVALID_ARG_INDEX;
  arg_type = interp_kernel_get_arg_type(k->opaque, index);
  arg_sz = interp_kernel_get_arg_size(k->opaque, index);

  if (UNLIKELY(arg_type != GBE_ARG_LOCAL_PTR && arg_sz != sz)) {
    if (arg_type != GBE_ARG_SAMPLER ||
        (arg_type == GBE_ARG_SAMPLER && sz != sizeof(cl_sampler)))
      return CL_INVALID_ARG_SIZE;
  }

  if(UNLIKELY(arg_type == GBE_ARG_LOCAL_PTR && sz == 0))
    return CL_INVALID_ARG_SIZE;
  if(arg_type == GBE_ARG_VALUE) {
    if(UNLIKELY(value == NULL))
      return CL_INVALID_ARG_VALUE;
  } else if(arg_type == GBE_ARG_LOCAL_PTR) {
    if(UNLIKELY(value != NULL))
      return CL_INVALID_ARG_VALUE;
  } else if(arg_type == GBE_ARG_SAMPLER) {
    if (UNLIKELY(value == NULL))
      return CL_INVALID_ARG_VALUE;

    cl_sampler s = *(cl_sampler*)value;
    if(s->magic != CL_MAGIC_SAMPLER_HEADER)
      return CL_INVALID_SAMPLER;
  } else {
    // should be image, GLOBAL_PTR, CONSTANT_PTR
    if (UNLIKELY(value == NULL && arg_type == GBE_ARG_IMAGE))
      return CL_INVALID_ARG_VALUE;
    if(value != NULL)
      mem = *(cl_mem*)value;
    if(value != NULL && mem) {
      if( CL_SUCCESS != is_valid_mem(mem, ctx->buffers))
        return CL_INVALID_MEM_OBJECT;

      if (UNLIKELY((arg_type == GBE_ARG_IMAGE && !IS_IMAGE(mem))
         || (arg_type != GBE_ARG_IMAGE && IS_IMAGE(mem))))
          return CL_INVALID_ARG_VALUE;
    }
  }

  /* Copy the structure or the value directly into the curbe */
  if (arg_type == GBE_ARG_VALUE) {
    offset = interp_kernel_get_curbe_offset(k->opaque, GBE_CURBE_KERNEL_ARGUMENT, index);
    assert(offset + sz <= k->curbe_sz);
    memcpy(k->curbe + offset, value, sz);
    k->args[index].local_sz = 0;
    k->args[index].is_set = 1;
    k->args[index].mem = NULL;
    return CL_SUCCESS;
  }

  /* For a local pointer just save the size */
  if (arg_type == GBE_ARG_LOCAL_PTR) {
    k->args[index].local_sz = sz;
    k->args[index].is_set = 1;
    k->args[index].mem = NULL;
    return CL_SUCCESS;
  }

  /* Is it a sampler*/
  if (arg_type == GBE_ARG_SAMPLER) {
    cl_sampler sampler;
    memcpy(&sampler, value, sz);
    k->args[index].local_sz = 0;
    k->args[index].is_set = 1;
    k->args[index].mem = NULL;
    k->args[index].sampler = sampler;
    cl_set_sampler_arg_slot(k, index, sampler);
    offset = interp_kernel_get_curbe_offset(k->opaque, GBE_CURBE_KERNEL_ARGUMENT, index);
    //assert(arg_sz == 4);
    assert(offset + 4 <= k->curbe_sz);
    memcpy(k->curbe + offset, &sampler->clkSamplerValue, 4);
    return CL_SUCCESS;
  }

  if(value != NULL)
    mem = *(cl_mem*) value;

  if(value == NULL || mem == NULL) {
    /* for buffer object GLOBAL_PTR CONSTANT_PTR, it maybe NULL */
    int32_t offset = interp_kernel_get_curbe_offset(k->opaque, GBE_CURBE_KERNEL_ARGUMENT, index);
    *((uint32_t *)(k->curbe + offset)) = 0;
    assert(arg_type == GBE_ARG_GLOBAL_PTR || arg_type == GBE_ARG_CONSTANT_PTR);

    if (k->args[index].mem)
      cl_mem_delete(k->args[index].mem);
    k->args[index].mem = NULL;
    k->args[index].is_set = 1;
    k->args[index].local_sz = 0;
    return CL_SUCCESS;
  }

  mem = *(cl_mem*) value;

  cl_mem_add_ref(mem);
  if (k->args[index].mem)
    cl_mem_delete(k->args[index].mem);
  k->args[index].mem = mem;
  k->args[index].is_set = 1;
  k->args[index].local_sz = 0;
  k->args[index].bti = interp_kernel_get_arg_bti(k->opaque, index);
  return CL_SUCCESS;
}

LOCAL int
cl_get_kernel_arg_info(cl_kernel k, cl_uint arg_index, cl_kernel_arg_info param_name,
                       size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
  assert(k != NULL);
  void *ret_info = interp_kernel_get_arg_info(k->opaque, arg_index,
                           param_name - CL_KERNEL_ARG_ADDRESS_QUALIFIER);
  uint32_t arg_type = interp_kernel_get_arg_type(k->opaque, arg_index);
  int str_len = 0;
  cl_kernel_arg_type_qualifier type_qual = CL_KERNEL_ARG_TYPE_NONE;

  switch (param_name) {
  case CL_KERNEL_ARG_ADDRESS_QUALIFIER:
    if (param_value_size < sizeof(cl_kernel_arg_address_qualifier))
      return CL_INVALID_VALUE;
    if (param_value_size_ret)
      *param_value_size_ret = sizeof(cl_kernel_arg_address_qualifier);
    if (!param_value) return CL_SUCCESS;
    if ((cl_ulong)ret_info == 0) {
      *(cl_kernel_arg_address_qualifier *)param_value = CL_KERNEL_ARG_ADDRESS_PRIVATE;
    } else if ((cl_ulong)ret_info == 1 || (cl_ulong)ret_info == 4) {
      *(cl_kernel_arg_address_qualifier *)param_value = CL_KERNEL_ARG_ADDRESS_GLOBAL;
    } else if ((cl_ulong)ret_info == 2) {
      *(cl_kernel_arg_address_qualifier *)param_value = CL_KERNEL_ARG_ADDRESS_CONSTANT;
    } else if ((cl_ulong)ret_info == 3) {
      *(cl_kernel_arg_address_qualifier *)param_value = CL_KERNEL_ARG_ADDRESS_LOCAL;
    } else {
      /* If no address qualifier is specified, the default address qualifier
         which is CL_KERNEL_ARG_ADDRESS_PRIVATE is returned. */
      *(cl_kernel_arg_address_qualifier *)param_value = CL_KERNEL_ARG_ADDRESS_LOCAL;
    }
    return CL_SUCCESS;

  case CL_KERNEL_ARG_ACCESS_QUALIFIER:
    if (param_value_size < sizeof(cl_kernel_arg_access_qualifier))
      return CL_INVALID_VALUE;
    if (param_value_size_ret)
      *param_value_size_ret = sizeof(cl_kernel_arg_access_qualifier);
    if (!param_value) return CL_SUCCESS;
    if (!strcmp((char*)ret_info, "write_only")) {
      *(cl_kernel_arg_address_qualifier *)param_value = CL_KERNEL_ARG_ACCESS_WRITE_ONLY;
    } else if (!strcmp((char*)ret_info, "read_only")) {
      *(cl_kernel_arg_address_qualifier *)param_value = CL_KERNEL_ARG_ACCESS_READ_ONLY;
    } else if (!strcmp((char*)ret_info, "read_write")) {
      *(cl_kernel_arg_address_qualifier *)param_value = CL_KERNEL_ARG_ACCESS_READ_WRITE;
    } else {
      *(cl_kernel_arg_address_qualifier *)param_value = CL_KERNEL_ARG_ACCESS_NONE;
    }
    return CL_SUCCESS;

  case CL_KERNEL_ARG_TYPE_NAME:
  case CL_KERNEL_ARG_NAME:
    str_len = strlen(ret_info);
    if (param_value_size < str_len + 1)
      return CL_INVALID_VALUE;
    if (param_value_size_ret)
      *param_value_size_ret = str_len + 1;
    if (!param_value) return CL_SUCCESS;
    memcpy(param_value, ret_info, str_len);
    ((char *)param_value)[str_len] = 0;
    return CL_SUCCESS;

  case CL_KERNEL_ARG_TYPE_QUALIFIER:
    if (param_value_size < sizeof(cl_kernel_arg_type_qualifier))
      return CL_INVALID_VALUE;
    if (param_value_size_ret)
      *param_value_size_ret = sizeof(cl_kernel_arg_type_qualifier);
    if (!param_value) return CL_SUCCESS;
    if (strstr((char*)ret_info, "const") &&
         (arg_type == GBE_ARG_GLOBAL_PTR   ||
          arg_type == GBE_ARG_CONSTANT_PTR ||
          arg_type == GBE_ARG_LOCAL_PTR))
      type_qual = type_qual | CL_KERNEL_ARG_TYPE_CONST;
    if (strstr((char*)ret_info, "volatile"))
      type_qual = type_qual | CL_KERNEL_ARG_TYPE_VOLATILE;
    if (strstr((char*)ret_info, "restrict"))
      type_qual = type_qual | CL_KERNEL_ARG_TYPE_RESTRICT;
    *(cl_kernel_arg_type_qualifier *)param_value = type_qual;
    return CL_SUCCESS;

  default:
    assert(0);
  }

  return CL_SUCCESS;
}

LOCAL uint32_t
cl_kernel_get_simd_width(cl_kernel k)
{
  assert(k != NULL);
  return interp_kernel_get_simd_width(k->opaque);
}

LOCAL void
cl_kernel_setup(cl_kernel k, gbe_kernel opaque)
{
  cl_context ctx = k->program->ctx;
  cl_buffer_mgr bufmgr = cl_context_get_bufmgr(ctx);

  if(k->bo != NULL)
    cl_buffer_unreference(k->bo);

  /* Allocate the gen code here */
  const uint32_t code_sz = interp_kernel_get_code_size(opaque);
  const char *code = interp_kernel_get_code(opaque);
  k->bo = cl_buffer_alloc(bufmgr, "CL kernel", code_sz, 64u);
  k->arg_n = interp_kernel_get_arg_num(opaque);

  /* Upload the code */
  cl_buffer_subdata(k->bo, 0, code_sz, code);
  k->opaque = opaque;

  /* Create the curbe */
  k->curbe_sz = interp_kernel_get_curbe_size(k->opaque);

  /* Get sampler data & size */
  k->sampler_sz = interp_kernel_get_sampler_size(k->opaque);
  assert(k->sampler_sz <= GEN_MAX_SAMPLERS);
  if (k->sampler_sz > 0)
    interp_kernel_get_sampler_data(k->opaque, k->samplers);
  interp_kernel_get_compile_wg_size(k->opaque, k->compile_wg_sz);
  k->stack_size = interp_kernel_get_stack_size(k->opaque);
  /* Get image data & size */
  k->image_sz = interp_kernel_get_image_size(k->opaque);
  assert(k->sampler_sz <= GEN_MAX_SURFACES);
  assert(k->image_sz <= ctx->device->max_read_image_args + ctx->device->max_write_image_args);
  if (k->image_sz > 0) {
    TRY_ALLOC_NO_ERR(k->images, cl_calloc(k->image_sz, sizeof(k->images[0])));
    interp_kernel_get_image_data(k->opaque, k->images);
  } else
    k->images = NULL;
  return;
error:
  cl_buffer_unreference(k->bo);
  k->bo = NULL;
}

LOCAL cl_kernel
cl_kernel_dup(cl_kernel from)
{
  cl_kernel to = NULL;

  if (UNLIKELY(from == NULL))
    return NULL;
  TRY_ALLOC_NO_ERR (to, CALLOC(struct _cl_kernel));
  SET_ICD(to->dispatch)
  to->bo = from->bo;
  to->opaque = from->opaque;
  to->ref_n = 1;
  to->magic = CL_MAGIC_KERNEL_HEADER;
  to->program = from->program;
  to->arg_n = from->arg_n;
  to->curbe_sz = from->curbe_sz;
  to->sampler_sz = from->sampler_sz;
  to->image_sz = from->image_sz;
  memcpy(to->compile_wg_sz, from->compile_wg_sz, sizeof(from->compile_wg_sz));
  to->stack_size = from->stack_size;
  if (to->sampler_sz)
    memcpy(to->samplers, from->samplers, to->sampler_sz * sizeof(uint32_t));
  if (to->image_sz) {
    TRY_ALLOC_NO_ERR(to->images, cl_calloc(to->image_sz, sizeof(to->images[0])));
    memcpy(to->images, from->images, to->image_sz * sizeof(to->images[0]));
  } else
    to->images = NULL;
  TRY_ALLOC_NO_ERR(to->args, cl_calloc(to->arg_n, sizeof(cl_argument)));
  if (to->curbe_sz) TRY_ALLOC_NO_ERR(to->curbe, cl_calloc(1, to->curbe_sz));

  /* Retain the bos */
  if (from->bo)       cl_buffer_reference(from->bo);

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
    const uint32_t required_sz = interp_kernel_get_required_work_group_size(ker->opaque, i);
    if (required_sz != 0 && required_sz != local_wk_sz[i]) {
      err = CL_INVALID_WORK_ITEM_SIZE;
      goto error;
    }
  }
  sz = local_wk_sz[0];
  for (i = 1; i < wk_dim; ++i)
    sz *= local_wk_sz[i];

  if (sz > cl_get_kernel_max_wg_sz(ker)) {
    err = CL_INVALID_WORK_ITEM_SIZE;
    goto error;
  }

error:
  if (wk_grp_sz) *wk_grp_sz = sz;
  return err;
}



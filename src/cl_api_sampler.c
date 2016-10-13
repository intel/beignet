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
 */

#include "cl_sampler.h"
#include "cl_context.h"
#include "cl_device_id.h"

cl_sampler
clCreateSampler(cl_context context,
                cl_bool normalized,
                cl_addressing_mode addressing,
                cl_filter_mode filter,
                cl_int *errcode_ret)
{
  cl_sampler sampler = NULL;
  cl_int err = CL_SUCCESS;
  cl_uint i;

  do {
    if (!CL_OBJECT_IS_CONTEXT(context)) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (addressing < CL_ADDRESS_NONE || addressing > CL_ADDRESS_MIRRORED_REPEAT) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (filter < CL_FILTER_NEAREST || filter > CL_FILTER_LINEAR) {
      err = CL_INVALID_VALUE;
      break;
    }

    /* Check if images are not supported by any device associated with context */
    for (i = 0; i < context->device_num; i++) {
      if (context->devices[i]->image_support == CL_FALSE) {
        err = CL_INVALID_OPERATION;
        break;
      }
    }
    if (err != CL_SUCCESS)
      break;

    sampler = cl_create_sampler(context, normalized, addressing, filter, &err);
  } while (0);

  if (errcode_ret)
    *errcode_ret = err;
  return sampler;
}

cl_int
clGetSamplerInfo(cl_sampler sampler,
                 cl_sampler_info param_name,
                 size_t param_value_size,
                 void *param_value,
                 size_t *param_value_size_ret)
{
  const void *src_ptr = NULL;
  size_t src_size = 0;
  cl_int ref;

  if (!CL_OBJECT_IS_SAMPLER(sampler)) {
    return CL_INVALID_SAMPLER;
  }

  if (param_name == CL_SAMPLER_REFERENCE_COUNT) {
    ref = CL_OBJECT_GET_REF(sampler);
    src_ptr = &ref;
    src_size = sizeof(cl_int);
  } else if (param_name == CL_SAMPLER_CONTEXT) {
    src_ptr = &sampler->ctx;
    src_size = sizeof(cl_context);
  } else if (param_name == CL_SAMPLER_NORMALIZED_COORDS) {
    src_ptr = &sampler->normalized_coords;
    src_size = sizeof(cl_bool);
  } else if (param_name == CL_SAMPLER_ADDRESSING_MODE) {
    src_ptr = &sampler->address;
    src_size = sizeof(cl_addressing_mode);
  } else if (param_name == CL_SAMPLER_FILTER_MODE) {
    src_ptr = &sampler->filter;
    src_size = sizeof(cl_filter_mode);
  } else {
    return CL_INVALID_VALUE;
  }

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
}

cl_int
clRetainSampler(cl_sampler sampler)
{
  if (!CL_OBJECT_IS_SAMPLER(sampler)) {
    return CL_INVALID_SAMPLER;
  }

  cl_sampler_add_ref(sampler);
  return CL_SUCCESS;
}

cl_int
clReleaseSampler(cl_sampler sampler)
{
  if (!CL_OBJECT_IS_SAMPLER(sampler)) {
    return CL_INVALID_SAMPLER;
  }

  cl_sampler_delete(sampler);
  return CL_SUCCESS;
}

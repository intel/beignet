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

#include "cl_context.h"

cl_int
clGetContextInfo(cl_context context,
                 cl_context_info param_name,
                 size_t param_value_size,
                 void *param_value,
                 size_t *param_value_size_ret)
{
  const void *src_ptr = NULL;
  size_t src_size = 0;
  cl_uint n, ref;
  cl_context_properties p;

  if (!CL_OBJECT_IS_CONTEXT(context)) {
    return CL_INVALID_CONTEXT;
  }

  if (param_name == CL_CONTEXT_DEVICES) {
    src_ptr = &context->device;
    src_size = sizeof(cl_device_id);
  } else if (param_name == CL_CONTEXT_NUM_DEVICES) {
    n = 1;
    src_ptr = &n;
    src_size = sizeof(cl_uint);
  } else if (param_name == CL_CONTEXT_REFERENCE_COUNT) {
    ref = CL_OBJECT_GET_REF(context);
    src_ptr = &ref;
    src_size = sizeof(cl_uint);
  } else if (param_name == CL_CONTEXT_PROPERTIES) {
    if (context->prop_len > 0) {
      src_ptr = context->prop_user;
      src_size = sizeof(cl_context_properties) * context->prop_len;
    } else {
      p = 0;
      src_ptr = &p;
      src_size = sizeof(cl_context_properties);
    }
  } else {
    return CL_INVALID_VALUE;
  }

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
}

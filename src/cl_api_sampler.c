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

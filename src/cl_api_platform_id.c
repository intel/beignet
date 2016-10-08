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

#include "cl_platform_id.h"
#include "CL/cl_ext.h"

cl_int
clGetPlatformInfo(cl_platform_id platform,
                  cl_platform_info param_name,
                  size_t param_value_size,
                  void *param_value,
                  size_t *param_value_size_ret)
{
  const void *src_ptr = NULL;
  size_t src_size = 0;

  if (!CL_OBJECT_IS_PLATFORM(platform)) {
    return CL_INVALID_PLATFORM;
  }

  /* Only one platform now. */
  if (platform != cl_get_platform_default()) {
    return CL_INVALID_PLATFORM;
  }

  if (param_name == CL_PLATFORM_PROFILE) {
    src_ptr = platform->profile;
    src_size = platform->profile_sz;
  } else if (param_name == CL_PLATFORM_VERSION) {
    src_ptr = platform->version;
    src_size = platform->version_sz;
  } else if (param_name == CL_PLATFORM_NAME) {
    src_ptr = platform->name;
    src_size = platform->name_sz;
  } else if (param_name == CL_PLATFORM_VENDOR) {
    src_ptr = platform->vendor;
    src_size = platform->vendor_sz;
  } else if (param_name == CL_PLATFORM_EXTENSIONS) {
    src_ptr = platform->extensions;
    src_size = platform->extensions_sz;
  } else if (param_name == CL_PLATFORM_ICD_SUFFIX_KHR) {
    src_ptr = platform->icd_suffix_khr;
    src_size = platform->icd_suffix_khr_sz;
  } else {
    return CL_INVALID_VALUE;
  }

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
}

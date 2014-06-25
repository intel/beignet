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

#include "cl_platform_id.h"
#include "cl_internals.h"
#include "cl_utils.h"
#include "CL/cl.h"
#include "CL/cl_ext.h"

#include <stdlib.h>
#include <string.h>

#define DECL_INFO_STRING(FIELD, STRING) \
    .FIELD = STRING,                    \
    .JOIN(FIELD,_sz) = sizeof(STRING),

static struct _cl_platform_id intel_platform_data = {
  INIT_ICD(dispatch)
  DECL_INFO_STRING(profile, "FULL_PROFILE")
  DECL_INFO_STRING(version, LIBCL_VERSION_STRING)
  DECL_INFO_STRING(name, "Intel Gen OCL Driver")
  DECL_INFO_STRING(vendor, "Intel")
  DECL_INFO_STRING(icd_suffix_khr, "Intel")
};

#undef DECL_INFO_STRING

/* Intel platform (only GPU now) */
cl_platform_id const intel_platform = &intel_platform_data;

LOCAL cl_int
cl_get_platform_ids(cl_uint          num_entries,
                    cl_platform_id * platforms,
                    cl_uint *        num_platforms)
{
  if (num_platforms != NULL)
    *num_platforms = 1;

  cl_intel_platform_extension_init(intel_platform);
  /* Easy right now, only one platform is supported */
  if(platforms)
    *platforms = intel_platform;
  intel_platform->extensions_sz = strlen(intel_platform->extensions) + 1;
  return CL_SUCCESS;
}

#define DECL_FIELD(CASE,FIELD)                                  \
  case JOIN(CL_,CASE):                                          \
    if (param_value_size < intel_platform->JOIN(FIELD,_sz))     \
      return CL_INVALID_VALUE;                                  \
    if (param_value_size_ret != NULL)                           \
      *param_value_size_ret = intel_platform->JOIN(FIELD,_sz);  \
    memcpy(param_value,                                         \
           intel_platform->FIELD,                               \
           intel_platform->JOIN(FIELD,_sz));                    \
      return CL_SUCCESS;

#define GET_FIELD_SZ(CASE,FIELD)                                \
  case JOIN(CL_,CASE):                                          \
    if (param_value_size_ret != NULL)                           \
      *param_value_size_ret = intel_platform->JOIN(FIELD,_sz);  \
    return CL_SUCCESS;

LOCAL cl_int
cl_get_platform_info(cl_platform_id    platform,
                     cl_platform_info  param_name,
                     size_t            param_value_size,
                     void *            param_value,
                     size_t *          param_value_size_ret)
{
  if (param_value == NULL) {
    switch (param_name) {
      GET_FIELD_SZ (PLATFORM_PROFILE,    profile);
      GET_FIELD_SZ (PLATFORM_VERSION,    version);
      GET_FIELD_SZ (PLATFORM_NAME,       name);
      GET_FIELD_SZ (PLATFORM_VENDOR,     vendor);
      GET_FIELD_SZ (PLATFORM_EXTENSIONS, extensions);
      GET_FIELD_SZ (PLATFORM_ICD_SUFFIX_KHR, icd_suffix_khr);
      default: return CL_INVALID_VALUE;
    }
  }

  /* Fetch the platform inform */
  switch (param_name) {
    DECL_FIELD (PLATFORM_PROFILE,    profile);
    DECL_FIELD (PLATFORM_VERSION,    version);
    DECL_FIELD (PLATFORM_NAME,       name);
    DECL_FIELD (PLATFORM_VENDOR,     vendor);
    DECL_FIELD (PLATFORM_EXTENSIONS, extensions);
    DECL_FIELD (PLATFORM_ICD_SUFFIX_KHR, icd_suffix_khr);
    default: return CL_INVALID_VALUE;
  }
}

#undef DECL_FIELD


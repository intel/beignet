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
  DECL_INFO_STRING(profile, "FULL_PROFILE")
  DECL_INFO_STRING(version, LIBCL_VERSION_STRING)
  DECL_INFO_STRING(name, "Intel Gen OCL Driver")
  DECL_INFO_STRING(vendor, "Intel")
  DECL_INFO_STRING(icd_suffix_khr, "Intel")
};

#undef DECL_INFO_STRING

/* Intel platform (only GPU now).
   It is used as default when the API's platform ptr is NULL */
static cl_platform_id intel_platform = NULL;
LOCAL cl_platform_id
cl_get_platform_default(void)
{
  if (intel_platform)
    return intel_platform;

  intel_platform = &intel_platform_data;
  CL_OBJECT_INIT_BASE(intel_platform, CL_OBJECT_PLATFORM_MAGIC);
  cl_intel_platform_extension_init(intel_platform);
  return intel_platform;
}

LOCAL cl_int
cl_get_platform_ids(cl_uint          num_entries,
                    cl_platform_id * platforms,
                    cl_uint *        num_platforms)
{
  if (num_platforms != NULL)
    *num_platforms = 1;

  /* Easy right now, only one platform is supported */
  if(platforms)
    *platforms = cl_get_platform_default();

  return CL_SUCCESS;
}

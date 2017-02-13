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

#ifndef __CL_PLATFORM_ID_H__
#define __CL_PLATFORM_ID_H__

#include "CL/cl.h"
#include "cl_internals.h"
#include "cl_extensions.h"
#include "cl_base_object.h"
#include "src/OCLConfig.h"
#include "src/git_sha1.h"

struct _cl_platform_id {
  _cl_base_object base;
  const char *profile;
  const char *version;
  const char *name;
  const char *vendor;
  char *extensions;
  const char *icd_suffix_khr;
  size_t profile_sz;
  size_t version_sz;
  size_t name_sz;
  size_t vendor_sz;
  size_t extensions_sz;
  size_t icd_suffix_khr_sz;
  struct cl_extensions *internal_extensions;
};

#define CL_OBJECT_PLATFORM_MAGIC 0xaacdbb00123ccd85LL
#define CL_OBJECT_IS_PLATFORM(obj) ((obj &&                           \
         ((cl_base_object)obj)->magic == CL_OBJECT_PLATFORM_MAGIC &&  \
         CL_OBJECT_GET_REF(obj) >= 1))

/* Return the default platform */
extern cl_platform_id cl_get_platform_default(void);

/* Return the valid platform */
extern cl_int cl_get_platform_ids(cl_uint          num_entries,
                                  cl_platform_id * platforms,
                                  cl_uint *        num_platforms);

#define _STR(x) #x
#define _JOINT(x, y) _STR(x) "." _STR(y)
#define _JOINT3(x, y, z) _STR(x) "." _STR(y) "." _STR(z)

#ifdef BEIGNET_GIT_SHA1
       #define BEIGNET_GIT_SHA1_STRING " (" BEIGNET_GIT_SHA1 ")"
#else
       #define BEIGNET_GIT_SHA1_STRING
#endif

#ifdef LIBCL_DRIVER_VERSION_PATCH
#define LIBCL_DRIVER_VERSION_STRING _JOINT3(LIBCL_DRIVER_VERSION_MAJOR, LIBCL_DRIVER_VERSION_MINOR, LIBCL_DRIVER_VERSION_PATCH)
#else
#define LIBCL_DRIVER_VERSION_STRING _JOINT(LIBCL_DRIVER_VERSION_MAJOR, LIBCL_DRIVER_VERSION_MINOR)
#endif
#define GEN9_LIBCL_VERSION_STRING "OpenCL " _JOINT(LIBCL_C_VERSION_MAJOR, LIBCL_C_VERSION_MINOR) " beignet " LIBCL_DRIVER_VERSION_STRING BEIGNET_GIT_SHA1_STRING
#define GEN9_LIBCL_C_VERSION_STRING "OpenCL C " _JOINT(LIBCL_C_VERSION_MAJOR, LIBCL_C_VERSION_MINOR) " beignet " LIBCL_DRIVER_VERSION_STRING BEIGNET_GIT_SHA1_STRING
#define NONGEN9_LIBCL_VERSION_STRING "OpenCL 1.2 beignet " LIBCL_DRIVER_VERSION_STRING BEIGNET_GIT_SHA1_STRING
#define NONGEN9_LIBCL_C_VERSION_STRING "OpenCL C 1.2 beignet " LIBCL_DRIVER_VERSION_STRING BEIGNET_GIT_SHA1_STRING

#endif /* __CL_PLATFORM_ID_H__ */


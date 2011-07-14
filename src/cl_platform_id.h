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

#ifndef __CL_PLATFORM_ID_H__
#define __CL_PLATFORM_ID_H__

#include "cl_internals.h"
#include "CL/cl.h"

/* Platform implemented by this run-time */
extern cl_platform_id const intel_platform;

/* Return the valid platform */
extern cl_int cl_get_platform_ids(cl_uint          num_entries,
                                  cl_platform_id * platforms,
                                  cl_uint *        num_platforms);

/* Return information for the current platform */
extern cl_int cl_get_platform_into(cl_platform_id    platform,
                                   cl_platform_info  param_name,
                                   size_t            param_value_size,
                                   void *            param_value,
                                   size_t *          param_value_size_ret);

#endif /* __CL_PLATFORM_ID_H__ */


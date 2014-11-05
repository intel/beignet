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

#ifndef __CL_IMAGE_H__
#define __CL_IMAGE_H__

#include "cl_internals.h"
#include "CL/cl.h"
#include <stdint.h>

/* Returned when the OCL format is not supported */
#define INTEL_UNSUPPORTED_FORMAT ((uint32_t) ~0x0u)

/* Compute the number of bytes per pixel if the format is supported */
extern cl_int cl_image_byte_per_pixel(const cl_image_format *fmt, uint32_t *bpp);

/* Return the intel format for the given OCL format */
extern uint32_t cl_image_get_intel_format(const cl_image_format *fmt);

/* Return the list of formats supported by the API */
extern cl_int cl_image_get_supported_fmt(cl_context context,
                                         cl_mem_object_type image_type,
                                         cl_uint num_entries,
                                         cl_image_format *image_formats,
                                         cl_uint *num_image_formats);

#endif /* __CL_IMAGE_H__ */


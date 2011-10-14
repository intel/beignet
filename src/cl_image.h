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

#ifndef __CL_IMAGE_H__
#define __CL_IMAGE_H__

#include "cl_internals.h"
#include "CL/cl.h"
#include <stdint.h>

/* Compute the number of bytes per pixel if the format is supported */
extern cl_int cl_image_byte_per_pixel(const cl_image_format *fmt, uint32_t *bpp);

#endif /* __CL_IMAGE_H__ */


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

#include "cl_image.h"
#include "cl_utils.h"

#include <assert.h>

LOCAL cl_int
cl_image_byte_per_pixel(const cl_image_format *fmt, uint32_t *bpp)
{
  assert(bpp);

  const uint32_t type = fmt->image_channel_data_type;
  const uint32_t order = fmt->image_channel_order;
  switch (type) {
#define DECL_BPP(DATA_TYPE, VALUE) case DATA_TYPE: *bpp = VALUE;
    DECL_BPP(CL_SNORM_INT8, 1); break;
    DECL_BPP(CL_SNORM_INT16, 2); break;
    DECL_BPP(CL_UNORM_INT8, 1); break;
    DECL_BPP(CL_UNORM_INT16, 2); break;
    DECL_BPP(CL_UNORM_SHORT_565, 2);
      if (order != CL_RGBx && order != CL_RGB)
        return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    break;
    DECL_BPP(CL_UNORM_SHORT_555, 2);
      if (order != CL_RGBx && order != CL_RGB)
        return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    break;
    DECL_BPP(CL_UNORM_INT_101010, 4);
      if (order != CL_RGBx && order != CL_RGB)
        return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    break;
    DECL_BPP(CL_SIGNED_INT8, 1); break;
    DECL_BPP(CL_SIGNED_INT16, 2); break;
    DECL_BPP(CL_SIGNED_INT32, 4); break;
    DECL_BPP(CL_UNSIGNED_INT8, 1); break;
    DECL_BPP(CL_UNSIGNED_INT16, 2); break;
    DECL_BPP(CL_UNSIGNED_INT32, 4); break;
    DECL_BPP(CL_HALF_FLOAT, 2); break;
    DECL_BPP(CL_FLOAT, 4); break;
#undef DECL_BPP
    default: return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
  };

  switch (order) {
    case CL_R: break;
    case CL_A: break;
    case CL_RA: *bpp *= 2; break;
    case CL_RG: *bpp *= 2; break;
    case CL_Rx: *bpp *= 2; break;
    case CL_INTENSITY:
    case CL_LUMINANCE:
      if (type != CL_UNORM_INT8 && type != CL_UNORM_INT16 &&
          type != CL_SNORM_INT8 && type != CL_SNORM_INT16 &&
          type != CL_HALF_FLOAT && type != CL_FLOAT)
        return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    break;
    case CL_RGB:
    case CL_RGBx:
      if (type != CL_UNORM_SHORT_555 &&
          type != CL_UNORM_SHORT_565 &&
          type != CL_UNORM_INT_101010)
        return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    break;
    case CL_RGBA: *bpp *= 4; break;
    case CL_ARGB:
    case CL_BGRA:
      if (type != CL_UNORM_INT8 && type != CL_SIGNED_INT8 &&
          type != CL_SNORM_INT8 && type != CL_UNSIGNED_INT8)
        return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
      *bpp *= 4;
    break;
    default: return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
  };

  return CL_SUCCESS;
}


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
#include "intel/intel_defines.h"

#include <assert.h>

LOCAL cl_int
cl_image_byte_per_pixel(const cl_image_format *fmt, uint32_t *bpp)
{
  assert(bpp);

  if(fmt == NULL)
    return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;

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
    case CL_Rx: break;
    case CL_R: break;
    case CL_A: break;
    case CL_RA: *bpp *= 2; break;
    case CL_RG: *bpp *= 2; break;
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

LOCAL uint32_t
cl_image_get_intel_format(const cl_image_format *fmt)
{
  const uint32_t type = fmt->image_channel_data_type;
  const uint32_t order = fmt->image_channel_order;
  switch (order) {
    case CL_R:
#if 0
    case CL_Rx:
    case CL_A:
    case CL_INTENSITY:
    case CL_LUMINANCE:
      if ((order == CL_INTENSITY || order == CL_LUMINANCE)
          && (type != CL_UNORM_INT8 && type != CL_UNORM_INT16
              && type != CL_SNORM_INT8 && type != CL_SNORM_INT16
              && type != CL_HALF_FLOAT && type != CL_FLOAT))
        return INTEL_UNSUPPORTED_FORMAT;
#endif

/* XXX it seems we have some acuracy compatible issue with snomr_int8/16,
 * have to disable those formats currently. */

      switch (type) {
        case CL_HALF_FLOAT:     return I965_SURFACEFORMAT_R16_FLOAT;
        case CL_FLOAT:          return I965_SURFACEFORMAT_R32_FLOAT;
//        case CL_SNORM_INT16:    return I965_SURFACEFORMAT_R16_SNORM;
//        case CL_SNORM_INT8:     return I965_SURFACEFORMAT_R8_SNORM;
        case CL_UNORM_INT8:     return I965_SURFACEFORMAT_R8_UNORM;
        case CL_UNORM_INT16:    return I965_SURFACEFORMAT_R16_UNORM;
        case CL_SIGNED_INT8:    return I965_SURFACEFORMAT_R8_SINT;
        case CL_SIGNED_INT16:   return I965_SURFACEFORMAT_R16_SINT;
        case CL_SIGNED_INT32:   return I965_SURFACEFORMAT_R32_SINT;
        case CL_UNSIGNED_INT8:  return I965_SURFACEFORMAT_R8_UINT;
        case CL_UNSIGNED_INT16: return I965_SURFACEFORMAT_R16_UINT;
        case CL_UNSIGNED_INT32: return I965_SURFACEFORMAT_R32_UINT;
        default: return INTEL_UNSUPPORTED_FORMAT;
      };
#if 0
    case CL_RG:
    case CL_RA:
      switch (type) {
        case CL_HALF_FLOAT:     return I965_SURFACEFORMAT_R16G16_FLOAT;
        case CL_FLOAT:          return I965_SURFACEFORMAT_R32G32_FLOAT;
        case CL_SNORM_INT16:    return I965_SURFACEFORMAT_R16G16_SNORM;
        case CL_SNORM_INT8:     return I965_SURFACEFORMAT_R8G8_SNORM;
        case CL_UNORM_INT8:     return I965_SURFACEFORMAT_R8G8_UNORM;
        case CL_UNORM_INT16:    return I965_SURFACEFORMAT_R16G16_UNORM;
        case CL_SIGNED_INT8:    return I965_SURFACEFORMAT_R8G8_SINT;
        case CL_SIGNED_INT16:   return I965_SURFACEFORMAT_R16G16_SINT;
        case CL_SIGNED_INT32:   return I965_SURFACEFORMAT_R32G32_SINT;
        case CL_UNSIGNED_INT8:  return I965_SURFACEFORMAT_R8G8_UINT;
        case CL_UNSIGNED_INT16: return I965_SURFACEFORMAT_R16G16_UINT;
        case CL_UNSIGNED_INT32: return I965_SURFACEFORMAT_R32G32_UINT;
        default: return INTEL_UNSUPPORTED_FORMAT;
      };
    case CL_RGB:
    case CL_RGBx:
      switch (type) {
        case CL_UNORM_INT_101010: return I965_SURFACEFORMAT_R10G10B10A2_UNORM;
        case CL_UNORM_SHORT_565:
        case CL_UNORM_SHORT_555:
        default: return INTEL_UNSUPPORTED_FORMAT;
      };
#endif
    case CL_RGBA:
      switch (type) {
        case CL_HALF_FLOAT:     return I965_SURFACEFORMAT_R16G16B16A16_FLOAT;
        case CL_FLOAT:          return I965_SURFACEFORMAT_R32G32B32A32_FLOAT;
//        case CL_SNORM_INT16:    return I965_SURFACEFORMAT_R16G16B16A16_SNORM;
//        case CL_SNORM_INT8:     return I965_SURFACEFORMAT_R8G8B8A8_SNORM;
        case CL_UNORM_INT8:     return I965_SURFACEFORMAT_R8G8B8A8_UNORM;
        case CL_UNORM_INT16:    return I965_SURFACEFORMAT_R16G16B16A16_UNORM;
        case CL_SIGNED_INT8:    return I965_SURFACEFORMAT_R8G8B8A8_SINT;
        case CL_SIGNED_INT16:   return I965_SURFACEFORMAT_R16G16B16A16_SINT;
        case CL_SIGNED_INT32:   return I965_SURFACEFORMAT_R32G32B32A32_SINT;
        case CL_UNSIGNED_INT8:  return I965_SURFACEFORMAT_R8G8B8A8_UINT;
        case CL_UNSIGNED_INT16: return I965_SURFACEFORMAT_R16G16B16A16_UINT;
        case CL_UNSIGNED_INT32: return I965_SURFACEFORMAT_R32G32B32A32_UINT;
        default: return INTEL_UNSUPPORTED_FORMAT;
      };
    case CL_ARGB: return INTEL_UNSUPPORTED_FORMAT;
    case CL_BGRA:
      switch (type) {
        case CL_UNORM_INT8:     return I965_SURFACEFORMAT_B8G8R8A8_UNORM;
        default: return INTEL_UNSUPPORTED_FORMAT;
      };
    default: return INTEL_UNSUPPORTED_FORMAT;
  };
}

static const uint32_t cl_image_order[] = {
  CL_R, CL_A, CL_RG, CL_RA, CL_RGB, CL_RGBA, CL_BGRA, CL_ARGB,
  CL_INTENSITY, CL_LUMINANCE, CL_Rx, CL_RGx, CL_RGBx
};

static const uint32_t cl_image_type[] = {
  CL_SNORM_INT8, CL_SNORM_INT16, CL_UNORM_INT8, CL_UNORM_INT16,
  CL_UNORM_SHORT_565, CL_UNORM_SHORT_555, CL_UNORM_INT_101010,
  CL_SIGNED_INT8, CL_SIGNED_INT16, CL_SIGNED_INT32,
  CL_UNSIGNED_INT8, CL_UNSIGNED_INT16, CL_UNSIGNED_INT32,
  CL_HALF_FLOAT, CL_FLOAT
};

static const size_t cl_image_order_n = SIZEOF32(cl_image_order);
static const size_t cl_image_type_n = SIZEOF32(cl_image_type);

cl_int
cl_image_get_supported_fmt(cl_context ctx,
                           cl_mem_object_type image_type,
                           cl_uint num_entries,
                           cl_image_format *image_formats,
                           cl_uint *num_image_formats)
{
  size_t i, j, n = 0;
  for (i = 0; i < cl_image_order_n; ++i)
  for (j = 0; j < cl_image_type_n; ++j) {
    const cl_image_format fmt = {
      .image_channel_order = cl_image_order[i],
      .image_channel_data_type = cl_image_type[j]
    };
    const uint32_t intel_fmt = cl_image_get_intel_format(&fmt);
    if (intel_fmt == INTEL_UNSUPPORTED_FORMAT)
      continue;
    if (n < num_entries && image_formats) image_formats[n] = fmt;
    n++;
  }
  if (num_image_formats) *num_image_formats = n;
  return CL_SUCCESS;
}


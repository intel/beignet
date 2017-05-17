/*
 * Copyright © 2012 - 2014 Intel Corporation
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
#include "ocl_image.h"
#if (__OPENCL_C_VERSION__ >= 200)
#include "ocl_math_20.h"
#else
#include "ocl_math.h"
#endif
#include "ocl_integer.h"
#include "ocl_common.h"
#include "ocl_convert.h"

#define int1 int
#define float1 float

///////////////////////////////////////////////////////////////////////////////
// Beignet builtin functions.
///////////////////////////////////////////////////////////////////////////////

#define DECL_GEN_OCL_RW_IMAGE(image_type, n) \
  OVERLOADABLE int4 __gen_ocl_read_imagei(read_only image_type image, sampler_t sampler,            \
                                          float ##n coord, uint sampler_offset);          \
  OVERLOADABLE int4 __gen_ocl_read_imagei(read_only image_type image, sampler_t sampler,            \
                                          int ##n coord, uint sampler_offset);            \
  OVERLOADABLE uint4 __gen_ocl_read_imageui(read_only image_type image, sampler_t sampler,          \
                                            float ##n coord, uint sampler_offset);        \
  OVERLOADABLE uint4 __gen_ocl_read_imageui(read_only image_type image, sampler_t sampler,          \
                                            int ##n coord, uint sampler_offset);          \
  OVERLOADABLE float4 __gen_ocl_read_imagef(read_only image_type image, sampler_t sampler,          \
                                            float ##n coord, uint sampler_offset);        \
  OVERLOADABLE float4 __gen_ocl_read_imagef(read_only image_type image, sampler_t sampler,          \
                                            int ##n coord, uint sampler_offset);          \
  OVERLOADABLE void __gen_ocl_write_imagei(write_only image_type image, int ##n coord , int4 color); \
  OVERLOADABLE void __gen_ocl_write_imageui(write_only image_type image, int ##n coord, uint4 color);\
  OVERLOADABLE void __gen_ocl_write_imagef(write_only image_type image, int ##n coord, float4 color);

#define DECL_GEN_OCL_QUERY_IMAGE(image_type) \
  OVERLOADABLE int __gen_ocl_get_image_width(image_type image);                           \
  OVERLOADABLE int __gen_ocl_get_image_height(image_type image);                          \
  OVERLOADABLE int __gen_ocl_get_image_channel_data_type(image_type image);               \
  OVERLOADABLE int __gen_ocl_get_image_channel_order(image_type image);                   \
  OVERLOADABLE int __gen_ocl_get_image_depth(image_type image);                           \

DECL_GEN_OCL_RW_IMAGE(image1d_t, 1)
DECL_GEN_OCL_RW_IMAGE(image1d_buffer_t, 2)
DECL_GEN_OCL_RW_IMAGE(image1d_array_t, 2)
DECL_GEN_OCL_RW_IMAGE(image1d_array_t, 4)
DECL_GEN_OCL_RW_IMAGE(image2d_t, 2)
DECL_GEN_OCL_RW_IMAGE(image2d_array_t, 3)
DECL_GEN_OCL_RW_IMAGE(image3d_t, 3)
DECL_GEN_OCL_RW_IMAGE(image2d_array_t, 4)
DECL_GEN_OCL_RW_IMAGE(image3d_t, 4)

DECL_GEN_OCL_QUERY_IMAGE(read_only image1d_t)
DECL_GEN_OCL_QUERY_IMAGE(read_only image1d_buffer_t)
DECL_GEN_OCL_QUERY_IMAGE(read_only image1d_array_t)
DECL_GEN_OCL_QUERY_IMAGE(read_only image2d_t)
DECL_GEN_OCL_QUERY_IMAGE(read_only image2d_array_t)
DECL_GEN_OCL_QUERY_IMAGE(read_only image3d_t)

#if __clang_major__*10 + __clang_minor__ >= 39
DECL_GEN_OCL_QUERY_IMAGE(write_only image1d_t)
DECL_GEN_OCL_QUERY_IMAGE(write_only image1d_buffer_t)
DECL_GEN_OCL_QUERY_IMAGE(write_only image1d_array_t)
DECL_GEN_OCL_QUERY_IMAGE(write_only image2d_t)
DECL_GEN_OCL_QUERY_IMAGE(write_only image2d_array_t)
DECL_GEN_OCL_QUERY_IMAGE(write_only image3d_t)
#endif

#if (__OPENCL_C_VERSION__ >= 200)
#define DECL_GEN_OCL_RW_IMAGE_WR(image_type, n) \
  OVERLOADABLE int4 __gen_ocl_read_imagei(read_write image_type image, sampler_t sampler,            \
                                          float ##n coord, uint sampler_offset);          \
  OVERLOADABLE int4 __gen_ocl_read_imagei(read_write image_type image, sampler_t sampler,            \
                                          int ##n coord, uint sampler_offset);            \
  OVERLOADABLE uint4 __gen_ocl_read_imageui(read_write image_type image, sampler_t sampler,          \
                                            float ##n coord, uint sampler_offset);        \
  OVERLOADABLE uint4 __gen_ocl_read_imageui(read_write image_type image, sampler_t sampler,          \
                                            int ##n coord, uint sampler_offset);          \
  OVERLOADABLE float4 __gen_ocl_read_imagef(read_write image_type image, sampler_t sampler,          \
                                            float ##n coord, uint sampler_offset);        \
  OVERLOADABLE float4 __gen_ocl_read_imagef(read_write image_type image, sampler_t sampler,          \
                                            int ##n coord, uint sampler_offset);          \
  OVERLOADABLE void __gen_ocl_write_imagei(read_write image_type image, int ##n coord , int4 color); \
  OVERLOADABLE void __gen_ocl_write_imageui(read_write image_type image, int ##n coord, uint4 color);\
  OVERLOADABLE void __gen_ocl_write_imagef(read_write image_type image, int ##n coord, float4 color);

DECL_GEN_OCL_RW_IMAGE_WR(image1d_t, 1)
DECL_GEN_OCL_RW_IMAGE_WR(image1d_buffer_t, 2)
DECL_GEN_OCL_RW_IMAGE_WR(image1d_array_t, 2)
DECL_GEN_OCL_RW_IMAGE_WR(image1d_array_t, 4)
DECL_GEN_OCL_RW_IMAGE_WR(image2d_t, 2)
DECL_GEN_OCL_RW_IMAGE_WR(image2d_array_t, 3)
DECL_GEN_OCL_RW_IMAGE_WR(image3d_t, 3)
DECL_GEN_OCL_RW_IMAGE_WR(image2d_array_t, 4)
DECL_GEN_OCL_RW_IMAGE_WR(image3d_t, 4)

DECL_GEN_OCL_QUERY_IMAGE(read_write image1d_t)
DECL_GEN_OCL_QUERY_IMAGE(read_write image1d_buffer_t)
DECL_GEN_OCL_QUERY_IMAGE(read_write image1d_array_t)
DECL_GEN_OCL_QUERY_IMAGE(read_write image2d_t)
DECL_GEN_OCL_QUERY_IMAGE(read_write image2d_array_t)
DECL_GEN_OCL_QUERY_IMAGE(read_write image3d_t)
#endif
///////////////////////////////////////////////////////////////////////////////
// helper functions to validate array index.
///////////////////////////////////////////////////////////////////////////////
INLINE_OVERLOADABLE float2 __gen_validate_array_index(float2 coord, read_only image1d_array_t image)
{
  float array_size = __gen_ocl_get_image_depth(image);
  coord.s1 = clamp(rint(coord.s1), 0.f, array_size - 1.f);
  return coord;
}

INLINE_OVERLOADABLE float4 __gen_validate_array_index(float4 coord, read_only image2d_array_t image)
{
  float array_size = __gen_ocl_get_image_depth(image);
  coord.s2 = clamp(rint(coord.s2), 0.f, array_size - 1.f);
  return coord;
}

INLINE_OVERLOADABLE float3 __gen_validate_array_index(float3 coord, read_only image2d_array_t image)
{
  float array_size = __gen_ocl_get_image_depth(image);
  coord.s2 = clamp(rint(coord.s2), 0.f, array_size - 1.f);
  return coord;
}

INLINE_OVERLOADABLE int2 __gen_validate_array_index(int2 coord, read_only image1d_array_t image)
{
  int array_size = __gen_ocl_get_image_depth(image);
  coord.s1 = clamp(coord.s1, 0, array_size - 1);
  return coord;
}

INLINE_OVERLOADABLE int4 __gen_validate_array_index(int4 coord, read_only image2d_array_t image)
{
  int array_size = __gen_ocl_get_image_depth(image);
  coord.s2 = clamp(coord.s2, 0, array_size - 1);
  return coord;
}

INLINE_OVERLOADABLE int3 __gen_validate_array_index(int3 coord, read_only image2d_array_t image)
{
  int array_size = __gen_ocl_get_image_depth(image);
  coord.s2 = clamp(coord.s2, 0, array_size - 1);
  return coord;
}

#if __clang_major__*10 + __clang_minor__ >= 39
INLINE_OVERLOADABLE float2 __gen_validate_array_index(float2 coord, write_only image1d_array_t image)
{
  float array_size = __gen_ocl_get_image_depth(image);
  coord.s1 = clamp(rint(coord.s1), 0.f, array_size - 1.f);
  return coord;
}
INLINE_OVERLOADABLE float4 __gen_validate_array_index(float4 coord, write_only image2d_array_t image)
{
  float array_size = __gen_ocl_get_image_depth(image);
  coord.s2 = clamp(rint(coord.s2), 0.f, array_size - 1.f);
  return coord;
}
INLINE_OVERLOADABLE float3 __gen_validate_array_index(float3 coord, write_only image2d_array_t image)
{
  float array_size = __gen_ocl_get_image_depth(image);
  coord.s2 = clamp(rint(coord.s2), 0.f, array_size - 1.f);
  return coord;
}
INLINE_OVERLOADABLE int2 __gen_validate_array_index(int2 coord, write_only image1d_array_t image)
{
  int array_size = __gen_ocl_get_image_depth(image);
  coord.s1 = clamp(coord.s1, 0, array_size - 1);
  return coord;
}
INLINE_OVERLOADABLE int4 __gen_validate_array_index(int4 coord, write_only image2d_array_t image)
{
  int array_size = __gen_ocl_get_image_depth(image);
  coord.s2 = clamp(coord.s2, 0, array_size - 1);
  return coord;
}
INLINE_OVERLOADABLE int3 __gen_validate_array_index(int3 coord, write_only image2d_array_t image)
{
  int array_size = __gen_ocl_get_image_depth(image);
  coord.s2 = clamp(coord.s2, 0, array_size - 1);
  return coord;
}
#endif

#if (__OPENCL_C_VERSION__ >= 200)
INLINE_OVERLOADABLE float2 __gen_validate_array_index(float2 coord, read_write image1d_array_t image)
{
  float array_size = __gen_ocl_get_image_depth(image);
  coord.s1 = clamp(rint(coord.s1), 0.f, array_size - 1.f);
  return coord;
}

INLINE_OVERLOADABLE float4 __gen_validate_array_index(float4 coord, read_write image2d_array_t image)
{
  float array_size = __gen_ocl_get_image_depth(image);
  coord.s2 = clamp(rint(coord.s2), 0.f, array_size - 1.f);
  return coord;
}

INLINE_OVERLOADABLE float3 __gen_validate_array_index(float3 coord, read_write image2d_array_t image)
{
  float array_size = __gen_ocl_get_image_depth(image);
  coord.s2 = clamp(rint(coord.s2), 0.f, array_size - 1.f);
  return coord;
}

INLINE_OVERLOADABLE int2 __gen_validate_array_index(int2 coord, read_write image1d_array_t image)
{
  int array_size = __gen_ocl_get_image_depth(image);
  coord.s1 = clamp(coord.s1, 0, array_size - 1);
  return coord;
}

INLINE_OVERLOADABLE int4 __gen_validate_array_index(int4 coord, read_write image2d_array_t image)
{
  int array_size = __gen_ocl_get_image_depth(image);
  coord.s2 = clamp(coord.s2, 0, array_size - 1);
  return coord;
}

INLINE_OVERLOADABLE int3 __gen_validate_array_index(int3 coord, read_write image2d_array_t image)
{
  int array_size = __gen_ocl_get_image_depth(image);
  coord.s2 = clamp(coord.s2, 0, array_size - 1);
  return coord;
}
#endif

// For non array image type, we need to do nothing.
#define GEN_VALIDATE_ARRAY_INDEX(coord_type, image_type) \
INLINE_OVERLOADABLE coord_type __gen_validate_array_index(coord_type coord, image_type image) \
{ \
  return coord; \
}

GEN_VALIDATE_ARRAY_INDEX(float, read_only image1d_t)
GEN_VALIDATE_ARRAY_INDEX(int, read_only image1d_t)
GEN_VALIDATE_ARRAY_INDEX(float2, read_only image2d_t)
GEN_VALIDATE_ARRAY_INDEX(int2, read_only image2d_t)
GEN_VALIDATE_ARRAY_INDEX(float4, read_only image3d_t)
GEN_VALIDATE_ARRAY_INDEX(int4, read_only image3d_t)
GEN_VALIDATE_ARRAY_INDEX(float3, read_only image3d_t)
GEN_VALIDATE_ARRAY_INDEX(int3, read_only image3d_t)
GEN_VALIDATE_ARRAY_INDEX(float, read_only image1d_buffer_t)
GEN_VALIDATE_ARRAY_INDEX(int, read_only image1d_buffer_t)

#if __clang_major__*10 + __clang_minor__ >= 39
GEN_VALIDATE_ARRAY_INDEX(float, write_only image1d_t)
GEN_VALIDATE_ARRAY_INDEX(int, write_only image1d_t)
GEN_VALIDATE_ARRAY_INDEX(float2, write_only image2d_t)
GEN_VALIDATE_ARRAY_INDEX(int2, write_only image2d_t)
GEN_VALIDATE_ARRAY_INDEX(float4, write_only image3d_t)
GEN_VALIDATE_ARRAY_INDEX(int4, write_only image3d_t)
GEN_VALIDATE_ARRAY_INDEX(float3, write_only image3d_t)
GEN_VALIDATE_ARRAY_INDEX(int3, write_only image3d_t)
GEN_VALIDATE_ARRAY_INDEX(float, write_only image1d_buffer_t)
GEN_VALIDATE_ARRAY_INDEX(int, write_only image1d_buffer_t)
#endif

#if (__OPENCL_C_VERSION__ >= 200)
GEN_VALIDATE_ARRAY_INDEX(float, read_write image1d_t)
GEN_VALIDATE_ARRAY_INDEX(int, read_write image1d_t)
GEN_VALIDATE_ARRAY_INDEX(float2, read_write image2d_t)
GEN_VALIDATE_ARRAY_INDEX(int2, read_write image2d_t)
GEN_VALIDATE_ARRAY_INDEX(float4, read_write image3d_t)
GEN_VALIDATE_ARRAY_INDEX(int4, read_write image3d_t)
GEN_VALIDATE_ARRAY_INDEX(float3, read_write image3d_t)
GEN_VALIDATE_ARRAY_INDEX(int3, read_write image3d_t)
GEN_VALIDATE_ARRAY_INDEX(float, read_write image1d_buffer_t)
GEN_VALIDATE_ARRAY_INDEX(int, read_write image1d_buffer_t)
#endif
///////////////////////////////////////////////////////////////////////////////
// Helper functions to work around some coordiate boundary issues.
// The major issue on Gen7/Gen7.5 are the sample message could not sampling
// integer type surfaces correctly with CLK_ADDRESS_CLAMP and CLK_FILTER_NEAREST.
// The work around is to use a LD message instead of normal sample message.
///////////////////////////////////////////////////////////////////////////////

bool __gen_ocl_sampler_need_fix(sampler_t);
bool __gen_ocl_sampler_need_rounding_fix(sampler_t);

bool __gen_sampler_need_fix(const sampler_t sampler)
{
  return __gen_ocl_sampler_need_fix(sampler);
}

bool __gen_sampler_need_rounding_fix(const sampler_t sampler)
{
  return __gen_ocl_sampler_need_rounding_fix(sampler);
}

INLINE_OVERLOADABLE float __gen_fixup_float_coord(float tmpCoord)
{
  if (tmpCoord < 0 && tmpCoord > -0x1p-20f)
    tmpCoord += -0x1p-9f;
  return tmpCoord;
}

INLINE_OVERLOADABLE float2 __gen_fixup_float_coord(float2 tmpCoord)
{
  if (tmpCoord.s0 < 0 && tmpCoord.s0 > -0x1p-20f)
    tmpCoord.s0 += -0x1p-9f;
  if (tmpCoord.s1 < 0 && tmpCoord.s1 > -0x1p-20f)
    tmpCoord.s1 += -0x1p-9f;
  return tmpCoord;
}

INLINE_OVERLOADABLE float3 __gen_fixup_float_coord(float3 tmpCoord)
{
  if (tmpCoord.s0 < 0 && tmpCoord.s0 > -0x1p-20f)
    tmpCoord.s0 += -0x1p-9f;
  if (tmpCoord.s1 < 0 && tmpCoord.s1 > -0x1p-20f)
    tmpCoord.s1 += -0x1p-9f;
  if (tmpCoord.s2 < 0 && tmpCoord.s2 > -0x1p-20f)
    tmpCoord.s2 += -0x1p-9f;
  return tmpCoord;
}

INLINE_OVERLOADABLE float4 __gen_fixup_float_coord(float4 tmpCoord)
{
  if (tmpCoord.s0 < 0 && tmpCoord.s0 > -0x1p-20f)
    tmpCoord.s0 += -0x1p-9f;
  if (tmpCoord.s1 < 0 && tmpCoord.s1 > -0x1p-20f)
    tmpCoord.s1 += -0x1p-9f;
  if (tmpCoord.s2 < 0 && tmpCoord.s2 > -0x1p-20f)
    tmpCoord.s2 += -0x1p-9f;
  return tmpCoord;
}

// Functions to denormalize coordiates, it's needed when we need to use LD
// message (sampler offset is non-zero) and the coordiates are normalized
// coordiates.
INLINE_OVERLOADABLE float __gen_denormalize_coord(const image1d_t image, float srcCoord)
{
  return srcCoord * __gen_ocl_get_image_width(image);
}

INLINE_OVERLOADABLE float2 __gen_denormalize_coord(const image1d_array_t image, float2 srcCoord)
{
  srcCoord.s0 = srcCoord.s0 * __gen_ocl_get_image_width(image);
  return srcCoord;
}

INLINE_OVERLOADABLE float __gen_denormalize_coord(const image1d_buffer_t image, float srcCoord)
{
  return srcCoord * __gen_ocl_get_image_width(image);
}

INLINE_OVERLOADABLE float2 __gen_denormalize_coord(const image2d_t image, float2 srcCoord)
{
  srcCoord.s0 = srcCoord.s0 * __gen_ocl_get_image_width(image);
  srcCoord.s1 = srcCoord.s1 * __gen_ocl_get_image_height(image);
  return srcCoord;
}

INLINE_OVERLOADABLE float3 __gen_denormalize_coord(const image2d_array_t image, float3 srcCoord)
{
  srcCoord.s0 = srcCoord.s0 * __gen_ocl_get_image_width(image);
  srcCoord.s1 = srcCoord.s1 * __gen_ocl_get_image_height(image);
  return srcCoord;
}

INLINE_OVERLOADABLE float3 __gen_denormalize_coord(const image3d_t image, float3 srcCoord)
{
  srcCoord.s0 = srcCoord.s0 * __gen_ocl_get_image_width(image);
  srcCoord.s1 = srcCoord.s1 * __gen_ocl_get_image_height(image);
  srcCoord.s2 = srcCoord.s2 * __gen_ocl_get_image_depth(image);
  return srcCoord;
}

INLINE_OVERLOADABLE float4 __gen_denormalize_coord(const image2d_array_t image, float4 srcCoord)
{
  srcCoord.s0 = srcCoord.s0 * __gen_ocl_get_image_width(image);
  srcCoord.s1 = srcCoord.s1 * __gen_ocl_get_image_height(image);
  return srcCoord;
}

INLINE_OVERLOADABLE float4 __gen_denormalize_coord(const image3d_t image, float4 srcCoord)
{
  srcCoord.s0 = srcCoord.s0 * __gen_ocl_get_image_width(image);
  srcCoord.s1 = srcCoord.s1 * __gen_ocl_get_image_height(image);
  srcCoord.s2 = srcCoord.s2 * __gen_ocl_get_image_depth(image);
  return srcCoord;
}

// After denormalize, we have to fixup the negative boundary.
INLINE_OVERLOADABLE float __gen_fixup_neg_boundary(float coord)
{
  return coord < 0 ? -1 : coord;
}

INLINE_OVERLOADABLE float2 __gen_fixup_neg_boundary(float2 coord)
{
  coord.s0 = coord.s0 < 0 ? -1 : coord.s0;
  coord.s1 = coord.s1 < 0 ? -1 : coord.s1;
  return coord;
}

INLINE_OVERLOADABLE float4 __gen_fixup_neg_boundary(float4 coord)
{
  coord.s0 = coord.s0 < 0 ? -1 : coord.s0;
  coord.s1 = coord.s1 < 0 ? -1 : coord.s1;
  coord.s2 = coord.s2 < 0 ? -1 : coord.s2;
  return coord;
}

INLINE_OVERLOADABLE float3 __gen_fixup_neg_boundary(float3 coord)
{
  coord.s0 = coord.s0 < 0 ? -1 : coord.s0;
  coord.s1 = coord.s1 < 0 ? -1 : coord.s1;
  coord.s2 = coord.s2 < 0 ? -1 : coord.s2;
  return coord;
}

///////////////////////////////////////////////////////////////////////////////
// Built-in Image Read/Write Functions
///////////////////////////////////////////////////////////////////////////////

// 2D 3D Image Common Macro
#ifdef GEN7_SAMPLER_CLAMP_BORDER_WORKAROUND
#define GEN_FIX_FLOAT_ROUNDING 1
#define GEN_FIX_INT_CLAMPING 1
#else
#define GEN_FIX_FLOAT_ROUNDING 0
#define GEN_FIX_INT_CLAMPING 0
#endif

#define convert_float1 convert_float
#define convert_int1 convert_int

// For integer coordinates
#define DECL_READ_IMAGE0(int_clamping_fix, image_type,                        \
                         image_data_type, suffix, coord_type, n)              \
  OVERLOADABLE image_data_type read_image ##suffix(read_only image_type cl_image,       \
                                        const sampler_t sampler,              \
                                        coord_type coord)                     \
  {                                                                           \
    coord = __gen_validate_array_index(coord, cl_image);                      \
    if (int_clamping_fix && __gen_sampler_need_fix(sampler))                  \
      return __gen_ocl_read_image ##suffix(cl_image, sampler,                 \
                                           convert_int ##n(coord), 1);        \
    return __gen_ocl_read_image ##suffix(cl_image, sampler,                   \
                                         convert_float ##n (coord), 0);       \
  }

// For float coordinates
#define DECL_READ_IMAGE1(int_clamping_fix, image_type,                        \
                         image_data_type, suffix, coord_type, n)              \
  OVERLOADABLE image_data_type read_image ##suffix(read_only image_type cl_image,       \
                                        const sampler_t sampler,              \
                                        coord_type coord)                     \
  {                                                                           \
    coord_type tmpCoord = __gen_validate_array_index(coord, cl_image);        \
    if (GEN_FIX_FLOAT_ROUNDING | int_clamping_fix) {                          \
      if (__gen_sampler_need_fix(sampler)) {                                  \
        if (GEN_FIX_FLOAT_ROUNDING &&                                         \
            __gen_sampler_need_rounding_fix(sampler))                         \
          tmpCoord = __gen_fixup_float_coord(tmpCoord);                       \
        if (int_clamping_fix) {                                               \
            if (!__gen_sampler_need_rounding_fix(sampler))                    \
              tmpCoord = __gen_denormalize_coord(cl_image, tmpCoord);         \
            tmpCoord = __gen_fixup_neg_boundary(tmpCoord);                    \
            return __gen_ocl_read_image ##suffix(                             \
                     cl_image, sampler, convert_int ##n(tmpCoord), 1);        \
        }                                                                     \
      }                                                                       \
    }                                                                         \
    return  __gen_ocl_read_image ##suffix(cl_image, sampler,                  \
                                          convert_float ##n (tmpCoord), 0);   \
  }

#define DECL_READ_IMAGE_NOSAMPLER(access_qual, image_type, image_data_type,   \
                                  suffix, coord_type, n)                      \
  OVERLOADABLE image_data_type read_image ##suffix(access_qual image_type cl_image, \
                                               coord_type coord)              \
  {                                                                           \
    coord = __gen_validate_array_index(coord, cl_image);                      \
    sampler_t defaultSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE \
                               | CLK_FILTER_NEAREST;                          \
    return __gen_ocl_read_image ##suffix(                                     \
             cl_image, defaultSampler, convert_float ##n (coord), 0);         \
  }

#define DECL_WRITE_IMAGE(access_qual, image_type, image_data_type, suffix, coord_type)   \
  OVERLOADABLE void write_image ##suffix(access_qual image_type cl_image,    \
                                         coord_type coord,                    \
                                         image_data_type color)               \
  {                                                                           \
    coord_type fixedCoord = __gen_validate_array_index(coord, cl_image);      \
    __gen_ocl_write_image ##suffix(cl_image, fixedCoord, color);              \
  }

#if (__OPENCL_C_VERSION__ >= 200)
#define DECL_IMAGE(int_clamping_fix, image_type, image_data_type, suffix, n)  \
  DECL_READ_IMAGE0(int_clamping_fix, image_type,                              \
                   image_data_type, suffix, int ##n, n)                       \
  DECL_READ_IMAGE1(int_clamping_fix, image_type,                              \
                   image_data_type, suffix, float ##n, n)                     \
  DECL_READ_IMAGE_NOSAMPLER(read_only, image_type, image_data_type, suffix, int ##n, n)  \
  DECL_READ_IMAGE_NOSAMPLER(read_write, image_type, image_data_type, suffix, int ##n, n) \
  DECL_WRITE_IMAGE(write_only, image_type, image_data_type, suffix, int ##n) \
  DECL_WRITE_IMAGE(read_write, image_type, image_data_type, suffix, int ##n)
#else
#define DECL_IMAGE(int_clamping_fix, image_type, image_data_type, suffix, n)  \
  DECL_READ_IMAGE0(int_clamping_fix, image_type,                              \
                   image_data_type, suffix, int ##n, n)                       \
  DECL_READ_IMAGE1(int_clamping_fix, image_type,                              \
                   image_data_type, suffix, float ##n, n)                     \
  DECL_READ_IMAGE_NOSAMPLER(read_only, image_type, image_data_type, suffix, int ##n, n)  \
  DECL_WRITE_IMAGE(write_only, image_type, image_data_type, suffix, int ##n)
#endif

// 1D
#define DECL_IMAGE_TYPE(image_type, n)                                        \
  DECL_IMAGE(GEN_FIX_INT_CLAMPING, image_type, int4, i, n)                    \
  DECL_IMAGE(GEN_FIX_INT_CLAMPING, image_type, uint4, ui, n)                  \
  DECL_IMAGE(0, image_type, float4, f, n)

DECL_IMAGE_TYPE(image1d_t, 1)
DECL_IMAGE_TYPE(image2d_t, 2)
DECL_IMAGE_TYPE(image3d_t, 4)
DECL_IMAGE_TYPE(image3d_t, 3)
DECL_IMAGE_TYPE(image2d_array_t, 4)
DECL_IMAGE_TYPE(image2d_array_t, 3)

#define DECL_READ_IMAGE1D_BUFFER_NOSAMPLER(access_qual, image_type, image_data_type, \
                                  suffix, coord_type)                         \
  OVERLOADABLE image_data_type read_image ##suffix(access_qual image_type cl_image,       \
                                               coord_type coord)              \
  {                                                                           \
    sampler_t defaultSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE \
                               | CLK_FILTER_NEAREST;                          \
    int2 effectCoord;                                                         \
    effectCoord.s0 = coord % 8192;                                            \
    effectCoord.s1 = coord / 8192;                                            \
    return __gen_ocl_read_image ##suffix(                                     \
             cl_image, defaultSampler, convert_float2(effectCoord), 0);       \
  }

#define DECL_WRITE_IMAGE1D_BUFFER(access_qual, image_type, image_data_type, suffix, coord_type) \
  OVERLOADABLE void write_image ##suffix(access_qual image_type cl_image,                 \
                                         coord_type coord,                    \
                                         image_data_type color)               \
  {                                                                           \
    int2 effectCoord;                                                         \
    effectCoord.s0 = coord %8192;                                             \
    effectCoord.s1 = coord / 8192;                                            \
    __gen_ocl_write_image ##suffix(cl_image, effectCoord, color);              \
  }

#if (__OPENCL_C_VERSION__ >= 200)
#define DECL_IMAGE_1DBuffer(int_clamping_fix, image_data_type, suffix)        \
  DECL_READ_IMAGE1D_BUFFER_NOSAMPLER(read_only, image1d_buffer_t, image_data_type,  \
                                     suffix, int)                             \
  DECL_READ_IMAGE1D_BUFFER_NOSAMPLER(read_write, image1d_buffer_t, image_data_type, \
                                     suffix, int)                             \
  DECL_WRITE_IMAGE1D_BUFFER(write_only, image1d_buffer_t, image_data_type, suffix, int) \
  DECL_WRITE_IMAGE1D_BUFFER(read_write, image1d_buffer_t, image_data_type, suffix, int)
#else
#define DECL_IMAGE_1DBuffer(int_clamping_fix, image_data_type, suffix)        \
  DECL_READ_IMAGE1D_BUFFER_NOSAMPLER(read_only, image1d_buffer_t, image_data_type,       \
                                     suffix, int)                             \
  DECL_WRITE_IMAGE1D_BUFFER(write_only, image1d_buffer_t, image_data_type, suffix, int)
#endif

DECL_IMAGE_1DBuffer(GEN_FIX_INT_CLAMPING, int4, i)
DECL_IMAGE_1DBuffer(GEN_FIX_INT_CLAMPING, uint4, ui)
DECL_IMAGE_1DBuffer(0, float4, f)

// For 1D Array:
// fixup_1darray_coord functions are to convert 1d array coord to 2d array coord
// and the caller must set the sampler offset to 2 by using this converted coord.
// It is used to work around an image 1d array restrication which could not set
// ai in the LD message. We solve it by fake the same image as a 2D array, and
// then access it by LD message as a 3D sufface, treat the ai as the w coordinate.
INLINE_OVERLOADABLE float4 __gen_fixup_1darray_coord(float2 coord, image1d_array_t image)
{
  float4 newCoord;
  newCoord.s0 = coord.s0 < 0 ? -1 : coord.s0;
  newCoord.s1 = 0;
  newCoord.s2 = coord.s1;
  newCoord.s3 = 0;
  return newCoord;
}

INLINE_OVERLOADABLE int4 __gen_fixup_1darray_coord(int2 coord, image1d_array_t image)
{
  int4 newCoord;
  newCoord.s0 = coord.s0;
  newCoord.s1 = 0;
  newCoord.s2 = coord.s1;
  newCoord.s3 = 0;
  return newCoord;
}

// For integer coordinates
#define DECL_READ_IMAGE0_1DArray(int_clamping_fix,                            \
                                 image_data_type, suffix, coord_type)         \
  OVERLOADABLE image_data_type read_image ##suffix(image1d_array_t cl_image,  \
                                        const sampler_t sampler,              \
                                        coord_type coord)                     \
  {                                                                           \
    coord = __gen_validate_array_index(coord, cl_image);                      \
    if (int_clamping_fix && __gen_sampler_need_fix(sampler)) {                \
      int4 newCoord = __gen_fixup_1darray_coord(coord, cl_image);             \
      return __gen_ocl_read_image ##suffix(cl_image, sampler, newCoord, 2);   \
    }                                                                         \
    return  __gen_ocl_read_image ##suffix(cl_image, sampler,                  \
                                          convert_float2 (coord), 0);         \
  }

// For float coordiates
#define DECL_READ_IMAGE1_1DArray(int_clamping_fix, image_data_type,           \
                                 suffix, coord_type)                          \
  OVERLOADABLE image_data_type read_image ##suffix(image1d_array_t cl_image,  \
                                        const sampler_t sampler,              \
                                        coord_type coord)                     \
  {                                                                           \
    coord_type tmpCoord = __gen_validate_array_index(coord, cl_image);        \
    if (GEN_FIX_FLOAT_ROUNDING | int_clamping_fix) {                          \
      if (__gen_sampler_need_fix(sampler)) {                                  \
        if (GEN_FIX_FLOAT_ROUNDING &&                                         \
            __gen_sampler_need_rounding_fix(sampler))                         \
          tmpCoord = __gen_fixup_float_coord(tmpCoord);                       \
        if (int_clamping_fix) {                                               \
            if (!__gen_sampler_need_rounding_fix(sampler))                    \
              tmpCoord = __gen_denormalize_coord(cl_image, tmpCoord);         \
            float4 newCoord = __gen_fixup_1darray_coord(tmpCoord, cl_image);  \
            return __gen_ocl_read_image ##suffix(                             \
                     cl_image, sampler, convert_int4(newCoord), 2);         \
        }                                                                     \
      }                                                                       \
    }                                                                         \
    return  __gen_ocl_read_image ##suffix(cl_image, sampler,                \
                                          convert_float2 (tmpCoord), 0);      \
  }

#if (__OPENCL_C_VERSION__ >= 200)
#define DECL_IMAGE_1DArray(int_clamping_fix, image_data_type, suffix)         \
  DECL_READ_IMAGE0_1DArray(int_clamping_fix, image_data_type, suffix, int2)   \
  DECL_READ_IMAGE1_1DArray(int_clamping_fix, image_data_type,                 \
                           suffix, float2)                                    \
  DECL_READ_IMAGE_NOSAMPLER(read_only, image1d_array_t, image_data_type, suffix, int2, 2) \
  DECL_READ_IMAGE_NOSAMPLER(read_write, image1d_array_t, image_data_type, suffix, int2, 2)\
  DECL_WRITE_IMAGE(write_only, image1d_array_t, image_data_type, suffix, int2) \
  DECL_WRITE_IMAGE(read_write, image1d_array_t, image_data_type, suffix, int2)
#else
#define DECL_IMAGE_1DArray(int_clamping_fix, image_data_type, suffix)         \
  DECL_READ_IMAGE0_1DArray(int_clamping_fix, image_data_type, suffix, int2)   \
  DECL_READ_IMAGE1_1DArray(int_clamping_fix, image_data_type,                 \
                           suffix, float2)                                    \
  DECL_READ_IMAGE_NOSAMPLER(read_only, image1d_array_t, image_data_type, suffix, int2, 2) \
  DECL_WRITE_IMAGE(write_only, image1d_array_t, image_data_type, suffix, int2)
#endif

DECL_IMAGE_1DArray(GEN_FIX_INT_CLAMPING, int4, i)
DECL_IMAGE_1DArray(GEN_FIX_INT_CLAMPING, uint4, ui)
DECL_IMAGE_1DArray(0, float4, f)

///////////////////////////////////////////////////////////////////////////////
// Built-in Image Query Functions
///////////////////////////////////////////////////////////////////////////////
#define DECL_IMAGE_INFO_COMMON(image_type)                                    \
  OVERLOADABLE  int get_image_channel_data_type(image_type image)             \
  {                                                                           \
    return __gen_ocl_get_image_channel_data_type(image);                      \
  }                                                                           \
  OVERLOADABLE  int get_image_channel_order(image_type image)                 \
  {                                                                           \
    return __gen_ocl_get_image_channel_order(image);                          \
  }                                                                           \
  OVERLOADABLE int get_image_width(image_type image)                          \
  {                                                                           \
    return __gen_ocl_get_image_width(image);                                  \
  }

DECL_IMAGE_INFO_COMMON(read_only image1d_t)
DECL_IMAGE_INFO_COMMON(read_only image1d_buffer_t)
DECL_IMAGE_INFO_COMMON(read_only image1d_array_t)
DECL_IMAGE_INFO_COMMON(read_only image2d_t)
DECL_IMAGE_INFO_COMMON(read_only image3d_t)
DECL_IMAGE_INFO_COMMON(read_only image2d_array_t)

#if __clang_major__*10 + __clang_minor__ >= 39
DECL_IMAGE_INFO_COMMON(write_only image1d_t)
DECL_IMAGE_INFO_COMMON(write_only image1d_buffer_t)
DECL_IMAGE_INFO_COMMON(write_only image1d_array_t)
DECL_IMAGE_INFO_COMMON(write_only image2d_t)
DECL_IMAGE_INFO_COMMON(write_only image3d_t)
DECL_IMAGE_INFO_COMMON(write_only image2d_array_t)
#endif

#if (__OPENCL_C_VERSION__ >= 200)
DECL_IMAGE_INFO_COMMON(read_write image1d_t)
DECL_IMAGE_INFO_COMMON(read_write image1d_buffer_t)
DECL_IMAGE_INFO_COMMON(read_write image1d_array_t)
DECL_IMAGE_INFO_COMMON(read_write image2d_t)
DECL_IMAGE_INFO_COMMON(read_write image3d_t)
DECL_IMAGE_INFO_COMMON(read_write image2d_array_t)
#endif

// 2D extra Info
OVERLOADABLE int get_image_height(read_only image2d_t image)
{
  return __gen_ocl_get_image_height(image);
}
OVERLOADABLE int2 get_image_dim(read_only image2d_t image)
{
  return (int2){get_image_width(image), get_image_height(image)};
}
#if __clang_major__*10 + __clang_minor__ >= 39
OVERLOADABLE int get_image_height(write_only image2d_t image)
{
  return __gen_ocl_get_image_height(image);
}
OVERLOADABLE int2 get_image_dim(write_only image2d_t image)
{
  return (int2){get_image_width(image), get_image_height(image)};
}
#endif

#if (__OPENCL_C_VERSION__ >= 200)
OVERLOADABLE int get_image_height(read_write image2d_t image)
{
  return __gen_ocl_get_image_height(image);
}
OVERLOADABLE int2 get_image_dim(read_write image2d_t image)
{
  return (int2){get_image_width(image), get_image_height(image)};
}
#endif
// End of 2D

// 3D extra Info
OVERLOADABLE int get_image_height(read_only image3d_t image)
{
  return __gen_ocl_get_image_height(image);
}
OVERLOADABLE int get_image_depth(read_only image3d_t image)
{
  return __gen_ocl_get_image_depth(image);
}
OVERLOADABLE int4 get_image_dim(read_only image3d_t image)
{
  return (int4) (get_image_width(image),
                 get_image_height(image),
                 get_image_depth(image),
                 0);
}
#if __clang_major__*10 + __clang_minor__ >= 39
OVERLOADABLE int get_image_height(write_only image3d_t image)
{
  return __gen_ocl_get_image_height(image);
}
OVERLOADABLE int get_image_depth(write_only image3d_t image)
{
  return __gen_ocl_get_image_depth(image);
}
OVERLOADABLE int4 get_image_dim(write_only image3d_t image)
{
  return (int4) (get_image_width(image),
                 get_image_height(image),
                 get_image_depth(image),
                 0);
}
#endif

#if (__OPENCL_C_VERSION__ >= 200)
OVERLOADABLE int get_image_height(read_write image3d_t image)
{
  return __gen_ocl_get_image_height(image);
}
OVERLOADABLE int get_image_depth(read_write image3d_t image)
{
  return __gen_ocl_get_image_depth(image);
}
OVERLOADABLE int4 get_image_dim(read_write image3d_t image)
{
  return (int4) (get_image_width(image),
                 get_image_height(image),
                 get_image_depth(image),
                 0);
}
#endif
// 2D Array extra Info
OVERLOADABLE int get_image_height(read_only image2d_array_t image)
{
  return __gen_ocl_get_image_height(image);
}
OVERLOADABLE int2 get_image_dim(read_only image2d_array_t image)
{
  return (int2){get_image_width(image), get_image_height(image)};
}
OVERLOADABLE size_t get_image_array_size(read_only image2d_array_t image)
{
  return __gen_ocl_get_image_depth(image);
}
#if __clang_major__*10 + __clang_minor__ >= 39
OVERLOADABLE int get_image_height(write_only image2d_array_t image)
{
  return __gen_ocl_get_image_height(image);
}
OVERLOADABLE int2 get_image_dim(write_only image2d_array_t image)
{
  return (int2){get_image_width(image), get_image_height(image)};
}
OVERLOADABLE size_t get_image_array_size(write_only image2d_array_t image)
{
  return __gen_ocl_get_image_depth(image);
}
#endif
#if (__OPENCL_C_VERSION__ >= 200)
OVERLOADABLE int get_image_height(read_write image2d_array_t image)
{
  return __gen_ocl_get_image_height(image);
}
OVERLOADABLE int2 get_image_dim(read_write image2d_array_t image)
{
  return (int2){get_image_width(image), get_image_height(image)};
}
OVERLOADABLE size_t get_image_array_size(read_write image2d_array_t image)
{
  return __gen_ocl_get_image_depth(image);
}
#endif
// 1D Array info
OVERLOADABLE size_t get_image_array_size(read_only image1d_array_t image)
{
  return __gen_ocl_get_image_depth(image);
}
#if __clang_major__*10 + __clang_minor__ >= 39
OVERLOADABLE size_t get_image_array_size(write_only image1d_array_t image)
{
  return __gen_ocl_get_image_depth(image);
}
#endif
#if (__OPENCL_C_VERSION__ >= 200)
OVERLOADABLE size_t get_image_array_size(read_write image1d_array_t image)
{
  return __gen_ocl_get_image_depth(image);
}
#endif
// End of 1DArray

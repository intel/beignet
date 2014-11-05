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
#include "ocl_math.h"
#include "ocl_integer.h"
#include "ocl_common.h"

// 1D read
OVERLOADABLE int4 __gen_ocl_read_imagei(uint surface_id, sampler_t sampler, float u, uint sampler_offset);
OVERLOADABLE int4 __gen_ocl_read_imagei(uint surface_id, sampler_t sampler, int u, uint sampler_offset);
OVERLOADABLE uint4 __gen_ocl_read_imageui(uint surface_id, sampler_t sampler, float u, uint sampler_offset);
OVERLOADABLE uint4 __gen_ocl_read_imageui(uint surface_id, sampler_t sampler, int u, uint sampler_offset);
OVERLOADABLE float4 __gen_ocl_read_imagef(uint surface_id, sampler_t sampler, float u, uint sampler_offset);
OVERLOADABLE float4 __gen_ocl_read_imagef(uint surface_id, sampler_t sampler, int u, uint sampler_offset);

// 2D & 1D Array read
OVERLOADABLE int4 __gen_ocl_read_imagei(uint surface_id, sampler_t sampler, float u, float v, uint sampler_offset);
OVERLOADABLE int4 __gen_ocl_read_imagei(uint surface_id, sampler_t sampler, int u, int v, uint sampler_offset);
OVERLOADABLE uint4 __gen_ocl_read_imageui(uint surface_id, sampler_t sampler, float u, float v, uint sampler_offset);
OVERLOADABLE uint4 __gen_ocl_read_imageui(uint surface_id, sampler_t sampler, int u, int v, uint sampler_offset);
OVERLOADABLE float4 __gen_ocl_read_imagef(uint surface_id, sampler_t sampler, float u, float v, uint sampler_offset);
OVERLOADABLE float4 __gen_ocl_read_imagef(uint surface_id, sampler_t sampler, int u, int v, uint sampler_offset);

// 3D & 2D Array read
OVERLOADABLE int4 __gen_ocl_read_imagei(uint surface_id, sampler_t sampler, float u, float v, float w, uint sampler_offset);
OVERLOADABLE int4 __gen_ocl_read_imagei(uint surface_id, sampler_t sampler, int u, int v, int w, uint sampler_offset);
OVERLOADABLE uint4 __gen_ocl_read_imageui(uint surface_id, sampler_t sampler, float u, float v, float w, uint sampler_offset);
OVERLOADABLE uint4 __gen_ocl_read_imageui(uint surface_id, sampler_t sampler, int u, int v, int w, uint sampler_offset);
OVERLOADABLE float4 __gen_ocl_read_imagef(uint surface_id, sampler_t sampler, float u, float v, float w, uint sampler_offset);
OVERLOADABLE float4 __gen_ocl_read_imagef(uint surface_id, sampler_t sampler, int u, int v, int w, uint sampler_offset);

// 1D write
OVERLOADABLE void __gen_ocl_write_imagei(uint surface_id, int u, int4 color);
OVERLOADABLE void __gen_ocl_write_imageui(uint surface_id, int u, uint4 color);
OVERLOADABLE void __gen_ocl_write_imagef(uint surface_id, int u, float4 color);

// 2D & 1D Array write
OVERLOADABLE void __gen_ocl_write_imagei(uint surface_id, int u, int v, int4 color);
OVERLOADABLE void __gen_ocl_write_imageui(uint surface_id, int u, int v, uint4 color);
OVERLOADABLE void __gen_ocl_write_imagef(uint surface_id, int u, int v, float4 color);

// 3D & 2D Array write
OVERLOADABLE void __gen_ocl_write_imagei(uint surface_id, int u, int v, int w, int4 color);
OVERLOADABLE void __gen_ocl_write_imageui(uint surface_id, int u, int v, int w, uint4 color);
OVERLOADABLE void __gen_ocl_write_imagef(uint surface_id, int u, int v, int w, float4 color);

int __gen_ocl_get_image_width(uint surface_id);
int __gen_ocl_get_image_height(uint surface_id);
int __gen_ocl_get_image_channel_data_type(uint surface_id);
int __gen_ocl_get_image_channel_order(uint surface_id);
int __gen_ocl_get_image_depth(uint surface_id);

// 2D 3D Image Common Macro
#ifdef GEN7_SAMPLER_CLAMP_BORDER_WORKAROUND
#define GEN_FIX_1 1
#else
#define GEN_FIX_1 0
#endif

#define GET_IMAGE(cl_image, surface_id) \
    uint surface_id = (uint)cl_image
OVERLOADABLE float __gen_compute_array_index(const float index, image1d_array_t image)
{
  GET_IMAGE(image, surface_id);
  float array_size = __gen_ocl_get_image_depth(surface_id);
  return clamp(rint(index), 0.f, array_size - 1.f);
}

OVERLOADABLE float __gen_compute_array_index(float index, image2d_array_t image)
{
  GET_IMAGE(image, surface_id);
  float array_size = __gen_ocl_get_image_depth(surface_id);
  return clamp(rint(index), 0.f, array_size - 1.f);
}

OVERLOADABLE int __gen_compute_array_index(int index, image1d_array_t image)
{
  GET_IMAGE(image, surface_id);
  int array_size = __gen_ocl_get_image_depth(surface_id);
  return clamp(index, 0, array_size - 1);
}

OVERLOADABLE int __gen_compute_array_index(int index, image2d_array_t image)
{
  GET_IMAGE(image, surface_id);
  int array_size = __gen_ocl_get_image_depth(surface_id);
  return clamp(index, 0, array_size - 1);
}

#define DECL_READ_IMAGE0(int_clamping_fix,                                   \
                        image_type, type, suffix, coord_type, n)             \
  OVERLOADABLE type read_image ##suffix(image_type cl_image,          \
                                               const sampler_t sampler,      \
                                               coord_type coord)             \
  {                                                                          \
    GET_IMAGE(cl_image, surface_id);                                         \
    GET_IMAGE_ARRAY_SIZE(cl_image, coord, int, ai);                          \
    if (int_clamping_fix &&                                                  \
        ((sampler & __CLK_ADDRESS_MASK) == CLK_ADDRESS_CLAMP) &&             \
        ((sampler & __CLK_FILTER_MASK) == CLK_FILTER_NEAREST))               \
            return   __gen_ocl_read_image ##suffix(                          \
                        EXPEND_READ_COORD(surface_id, sampler, coord));      \
    return  __gen_ocl_read_image ##suffix(                                   \
                    EXPEND_READ_COORDF(surface_id, sampler, coord), 0);      \
  }

#define DECL_READ_IMAGE1(float_coord_rounding_fix, int_clamping_fix,         \
                        image_type, type, suffix, coord_type, n)             \
  OVERLOADABLE type read_image ##suffix(image_type cl_image,          \
                                               const sampler_t sampler,      \
                                               coord_type coord)             \
  {                                                                          \
    GET_IMAGE(cl_image, surface_id);                                         \
    GET_IMAGE_ARRAY_SIZE(cl_image, coord, float, ai)                         \
    coord_type tmpCoord = coord;                                             \
    if (float_coord_rounding_fix | int_clamping_fix) {                       \
      if (((sampler & __CLK_ADDRESS_MASK) == CLK_ADDRESS_CLAMP)              \
          && ((sampler & __CLK_FILTER_MASK) == CLK_FILTER_NEAREST)) {        \
        if (float_coord_rounding_fix                                         \
            && ((sampler & CLK_NORMALIZED_COORDS_TRUE) == 0)) {              \
          FIXUP_FLOAT_COORD(tmpCoord);                                       \
        }                                                                    \
        if (int_clamping_fix) {                                              \
            coord_type intCoord;                                             \
            if (sampler & CLK_NORMALIZED_COORDS_TRUE) {                      \
              DENORMALIZE_COORD(surface_id, intCoord, tmpCoord);             \
            } else                                                           \
              intCoord = tmpCoord;                                           \
            return   __gen_ocl_read_image ##suffix(                          \
                       EXPEND_READ_COORDI(surface_id, sampler, intCoord));\
       }                                                                     \
      }                                                                      \
    }                                                                        \
    return  __gen_ocl_read_image ##suffix(                                   \
                        EXPEND_READ_COORDF(surface_id, sampler, tmpCoord), 0);\
  }

#define DECL_READ_IMAGE_NOSAMPLER(image_type, type, suffix, coord_type, n)   \
  OVERLOADABLE type read_image ##suffix(image_type cl_image,          \
                                               coord_type coord)             \
  {                                                                          \
    GET_IMAGE(cl_image, surface_id);                                         \
    GET_IMAGE_ARRAY_SIZE(cl_image, coord, int, ai)                           \
    return __gen_ocl_read_image ##suffix(                                    \
           EXPEND_READ_COORDF(surface_id,                                    \
                             CLK_NORMALIZED_COORDS_FALSE                     \
                             | CLK_ADDRESS_NONE                              \
                             | CLK_FILTER_NEAREST, (float)coord), 0);        \
  }

#define DECL_WRITE_IMAGE(image_type, type, suffix, coord_type) \
  OVERLOADABLE void write_image ##suffix(image_type cl_image, coord_type coord, type color)\
  {\
    GET_IMAGE(cl_image, surface_id);\
    __gen_ocl_write_image ##suffix(EXPEND_WRITE_COORD(surface_id, coord, color));\
  }

#define DECL_IMAGE_INFO_COMMON(image_type)    \
  OVERLOADABLE  int get_image_channel_data_type(image_type image)\
  { \
    GET_IMAGE(image, surface_id);\
    return __gen_ocl_get_image_channel_data_type(surface_id); \
  }\
  OVERLOADABLE  int get_image_channel_order(image_type image)\
  { \
    GET_IMAGE(image, surface_id);\
    return __gen_ocl_get_image_channel_order(surface_id); \
  } \
  OVERLOADABLE int get_image_width(image_type image) \
  { \
    GET_IMAGE(image, surface_id); \
    return __gen_ocl_get_image_width(surface_id);  \
  }

// 1D
#define DECL_IMAGE(int_clamping_fix, image_type, type, suffix)                       \
  DECL_READ_IMAGE0(int_clamping_fix, image_type, type, suffix, int, 1)               \
  DECL_READ_IMAGE1(GEN_FIX_1, int_clamping_fix, image_type, type, suffix, float, 1)  \
  DECL_READ_IMAGE_NOSAMPLER(image_type, type, suffix, int, 1)                        \
  DECL_WRITE_IMAGE(image_type, type, suffix, int)                                    \
  DECL_WRITE_IMAGE(image_type, type, suffix, float)

#define EXPEND_READ_COORD(id, sampler, coord) id, sampler, coord, 1
#define EXPEND_READ_COORDF(id, sampler, coord) id, sampler, (float)coord
#define EXPEND_READ_COORDI(id, sampler, coord) id, sampler, (int)(coord < 0 ? -1 : coord), 1
#define DENORMALIZE_COORD(id, dstCoord, srcCoord) dstCoord = srcCoord * __gen_ocl_get_image_width(id);
#define EXPEND_WRITE_COORD(id, coord, color) id, coord, color
#define GET_IMAGE_ARRAY_SIZE(a,b,c,d)

#define FIXUP_FLOAT_COORD(tmpCoord)                            \
  {                                                            \
    if (tmpCoord < 0 && tmpCoord > -0x1p-20f)                  \
      tmpCoord += -0x1p-9f;                                     \
  }

DECL_IMAGE(GEN_FIX_1, image1d_t, int4, i)
DECL_IMAGE(GEN_FIX_1, image1d_t, uint4, ui)
DECL_IMAGE(0, image1d_t, float4, f)
DECL_IMAGE(GEN_FIX_1, image1d_buffer_t, int4, i)
DECL_IMAGE(GEN_FIX_1, image1d_buffer_t, uint4, ui)
DECL_IMAGE(0, image1d_buffer_t, float4, f)

// 1D Info
DECL_IMAGE_INFO_COMMON(image1d_t)
DECL_IMAGE_INFO_COMMON(image1d_buffer_t)

#undef EXPEND_READ_COORD
#undef EXPEND_READ_COORDF
#undef EXPEND_READ_COORDI
#undef DENORMALIZE_COORD
#undef EXPEND_WRITE_COORD
#undef FIXUP_FLOAT_COORD
#undef DECL_IMAGE
// End of 1D

#define DECL_IMAGE(int_clamping_fix, image_type, type, suffix, n)                       \
  DECL_READ_IMAGE0(int_clamping_fix, image_type, type, suffix, int ##n, n)              \
  DECL_READ_IMAGE1(GEN_FIX_1, int_clamping_fix, image_type, type, suffix, float ##n, n) \
  DECL_READ_IMAGE_NOSAMPLER(image_type, type, suffix, int ##n, n)                       \
  DECL_WRITE_IMAGE(image_type, type, suffix, int ## n)                                  \
  DECL_WRITE_IMAGE(image_type, type, suffix, float ## n)
// 2D
#define EXPEND_READ_COORD(id, sampler, coord) id, sampler, coord.s0, coord.s1, 1
#define EXPEND_READ_COORDF(id, sampler, coord) id, sampler, (float)coord.s0, (float)coord.s1
#define EXPEND_READ_COORDI(id, sampler, coord) id, sampler, (int)(coord.s0 < 0 ? -1 : coord.s0), \
                                               (int)(coord.s1 < 0 ? -1 : coord.s1), 1
#define DENORMALIZE_COORD(id, dstCoord, srcCoord) dstCoord.x = srcCoord.x * __gen_ocl_get_image_width(id); \
                                                  dstCoord.y = srcCoord.y * __gen_ocl_get_image_height(id);
#define EXPEND_WRITE_COORD(id, coord, color) id, coord.s0, coord.s1, color

#define FIXUP_FLOAT_COORD(tmpCoord)                            \
  {                                                            \
    if (tmpCoord.s0 < 0 && tmpCoord.s0 > -0x1p-20f)            \
      tmpCoord.s0 += -0x1p-9f;                                  \
    if (tmpCoord.s1 < 0 && tmpCoord.s1 > -0x1p-20f)            \
      tmpCoord.s1 += -0x1p-9f;                                 \
  }

DECL_IMAGE(GEN_FIX_1, image2d_t, int4, i, 2)
DECL_IMAGE(GEN_FIX_1, image2d_t, uint4, ui, 2)
DECL_IMAGE(0, image2d_t, float4, f, 2)

// 1D Array
#undef GET_IMAGE_ARRAY_SIZE
#undef EXPEND_READ_COORD
#undef EXPEND_READ_COORDF
#undef EXPEND_READ_COORDI
#undef DENORMALIZE_COORD
#undef EXPEND_WRITE_COORD
#undef FIXUP_FLOAT_COORD

#define EXPEND_READ_COORD(id, sampler, coord) id, sampler, coord.s0, (int)0, ai, 2
#define EXPEND_READ_COORDF(id, sampler, coord) id, sampler, (float)coord.s0, (float)ai
#define EXPEND_READ_COORDI(id, sampler, coord) id, sampler, (int)(coord.s0 < 0 ? -1 : coord.s0), 0, (int)ai, 2
#define DENORMALIZE_COORD(id, dstCoord, srcCoord) dstCoord.x = srcCoord.x * __gen_ocl_get_image_width(id);
#define EXPEND_WRITE_COORD(id, coord, color) id, coord.s0, __gen_compute_array_index(coord.s1, cl_image), color
#define GET_IMAGE_ARRAY_SIZE(image, coord, coord_type, ai) \
  coord_type ai = __gen_compute_array_index(coord.s1, image);

#define FIXUP_FLOAT_COORD(tmpCoord)                            \
  {                                                            \
    if (tmpCoord.s0 < 0 && tmpCoord.s0 > -0x1p-20f)            \
      tmpCoord.s0 += -0x1p-9f;                                  \
  }

DECL_IMAGE(GEN_FIX_1, image1d_array_t, int4, i, 2)
DECL_IMAGE(GEN_FIX_1, image1d_array_t, uint4, ui, 2)
DECL_IMAGE(0, image1d_array_t, float4, f, 2)

// 2D Info
DECL_IMAGE_INFO_COMMON(image2d_t)
OVERLOADABLE int get_image_height(image2d_t image)
{
  GET_IMAGE(image, surface_id);
  return __gen_ocl_get_image_height(surface_id);
}
OVERLOADABLE int2 get_image_dim(image2d_t image)
{
  return (int2){get_image_width(image), get_image_height(image)};
}

// 1D Array info
DECL_IMAGE_INFO_COMMON(image1d_array_t)
OVERLOADABLE size_t get_image_array_size(image1d_array_t image)
{
  GET_IMAGE(image, surface_id);
  return __gen_ocl_get_image_depth(surface_id);
}

#undef EXPEND_READ_COORD
#undef EXPEND_READ_COORDI
#undef EXPEND_READ_COORDF
#undef DENORMALIZE_COORD
#undef EXPEND_WRITE_COORD
#undef FIXUP_FLOAT_COORD
#undef GET_IMAGE_ARRAY_SIZE
// End of 2D and 1D Array

// 3D
#define EXPEND_READ_COORD(id, sampler, coord) id, sampler, coord.s0, coord.s1, coord.s2, 1
#define EXPEND_READ_COORDF(id, sampler, coord) id, sampler, (float)coord.s0, (float)coord.s1, (float)coord.s2
#define EXPEND_READ_COORDI(id, sampler, coord) id, sampler, (int) (coord.s0 < 0 ? -1 : coord.s0), \
                                               (int)(coord.s1 < 0 ? -1 : coord.s1), (int)(coord.s2 < 0 ? -1 : coord.s2), 1
#define DENORMALIZE_COORD(id, dstCoord, srcCoord) dstCoord.x = srcCoord.x * __gen_ocl_get_image_width(id); \
                                                  dstCoord.y = srcCoord.y * __gen_ocl_get_image_height(id); \
                                                  dstCoord.z = srcCoord.z * __gen_ocl_get_image_depth(id);
#define EXPEND_WRITE_COORD(id, coord, color) id, coord.s0, coord.s1, coord.s2, color

#define FIXUP_FLOAT_COORD(tmpCoord)                             \
  {                                                             \
    if (tmpCoord.s0 < 0 && tmpCoord.s0 > -0x1p-20f)              \
      tmpCoord.s0 += -0x1p-9f;                                   \
    if (tmpCoord.s1 < 0 && tmpCoord.s1 > -0x1p-20f)              \
      tmpCoord.s1 += -0x1p-9f;                                   \
    if (tmpCoord.s2 < 0 && tmpCoord.s2 > -0x1p-20f)              \
      tmpCoord.s2 += -0x1p-9f;                                   \
  }
#define GET_IMAGE_ARRAY_SIZE(a,b,c,d)

DECL_IMAGE(GEN_FIX_1, image3d_t, int4, i, 4)
DECL_IMAGE(GEN_FIX_1, image3d_t, uint4, ui, 4)
DECL_IMAGE(0, image3d_t, float4, f, 4)

DECL_IMAGE(GEN_FIX_1, image3d_t, int4, i, 3)
DECL_IMAGE(GEN_FIX_1, image3d_t, uint4, ui, 3)
DECL_IMAGE(0, image3d_t, float4, f, 3)

#undef EXPEND_READ_COORD
#undef EXPEND_READ_COORDF
#undef EXPEND_READ_COORDI
#undef DENORMALIZE_COORD
#undef EXPEND_WRITE_COORD
#undef FIXUP_FLOAT_COORD
#undef GET_IMAGE_ARRAY_SIZE

#define EXPEND_READ_COORD(id, sampler, coord) id, sampler, coord.s0, coord.s1, ai, 1
#define EXPEND_READ_COORDF(id, sampler, coord) id, sampler, (float)coord.s0, (float)coord.s1, (float)ai
#define EXPEND_READ_COORDI(id, sampler, coord) id, sampler, (int) (coord.s0 < 0 ? -1 : coord.s0), \
                                               (int)(coord.s1 < 0 ? -1 : coord.s1), (int)ai, 1
#define DENORMALIZE_COORD(id, dstCoord, srcCoord) dstCoord.x = srcCoord.x * __gen_ocl_get_image_width(id); \
                                                  dstCoord.y = srcCoord.y * __gen_ocl_get_image_height(id);
#define EXPEND_WRITE_COORD(id, coord, color) id, coord.s0, coord.s1, __gen_compute_array_index(coord.s2, cl_image), color

#define FIXUP_FLOAT_COORD(tmpCoord)                             \
  {                                                             \
    if (tmpCoord.s0 < 0 && tmpCoord.s0 > -0x1p-20f)              \
      tmpCoord.s0 += -0x1p-9f;                                   \
    if (tmpCoord.s1 < 0 && tmpCoord.s1 > -0x1p-20f)              \
      tmpCoord.s1 += -0x1p-9f;                                   \
  }
#define GET_IMAGE_ARRAY_SIZE(image, coord, coord_type, ai) \
  coord_type ai = __gen_compute_array_index(coord.s2, image);

// 2D Array
DECL_IMAGE(GEN_FIX_1, image2d_array_t, int4, i, 4)
DECL_IMAGE(GEN_FIX_1, image2d_array_t, uint4, ui, 4)
DECL_IMAGE(0, image2d_array_t, float4, f, 4)

DECL_IMAGE(GEN_FIX_1, image2d_array_t, int4, i, 3)
DECL_IMAGE(GEN_FIX_1, image2d_array_t, uint4, ui, 3)
DECL_IMAGE(0, image2d_array_t, float4, f, 3)

// 3D Info
DECL_IMAGE_INFO_COMMON(image3d_t)
OVERLOADABLE int get_image_height(image3d_t image)
{
  GET_IMAGE(image, surface_id);
  return __gen_ocl_get_image_height(surface_id);
}
OVERLOADABLE int get_image_depth(image3d_t image)
{
  GET_IMAGE(image, surface_id);
  return __gen_ocl_get_image_depth(surface_id);
}
OVERLOADABLE int4 get_image_dim(image3d_t image)
{
  return (int4){get_image_width(image), get_image_height(image), get_image_depth(image), 0};
}

// 2D Array Info
DECL_IMAGE_INFO_COMMON(image2d_array_t)
OVERLOADABLE int get_image_height(image2d_array_t image)
{
  GET_IMAGE(image, surface_id);
  return __gen_ocl_get_image_height(surface_id);
}
OVERLOADABLE int2 get_image_dim(image2d_array_t image)
{
  return (int2){get_image_width(image), get_image_height(image)};
}
OVERLOADABLE size_t get_image_array_size(image2d_array_t image)
{
  GET_IMAGE(image, surface_id);
  return __gen_ocl_get_image_depth(surface_id);
}

#undef EXPEND_READ_COORD
#undef EXPEND_READ_COORDF
#undef EXPEND_READ_COORDI
#undef DENORMALIZE_COORD
#undef EXPEND_WRITE_COORD
#undef FIXUP_FLOAT_COORD
#undef GET_IMAGE_ARRAY_SIZE
// End of 3D and 2D Array

#undef DECL_IMAGE
#undef DECL_READ_IMAGE
#undef DECL_READ_IMAGE_NOSAMPLER
#undef DECL_WRITE_IMAGE
#undef GEN_FIX_1
// End of Image


#undef GET_IMAGE

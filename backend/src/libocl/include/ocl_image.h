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
#ifndef __OCL_IMAGE_H__
#define __OCL_IMAGE_H__

#include "ocl_types.h"

#define int1 int
#define float1 float

#define DECL_IMAGE_READ_SAMPLE_RETTYPE(IMG_TYPE, DATA_YPE, SUFFIX, N) \
  OVERLOADABLE DATA_YPE read_image ## SUFFIX(IMG_TYPE cl_image, const sampler_t sampler, int##N coord); \
  OVERLOADABLE DATA_YPE read_image ## SUFFIX(IMG_TYPE cl_image, const sampler_t sampler, float##N coord);

#define DECL_IMAGE_READ_NO_SAMPLE_RETTYPE(IMG_TYPE, DATA_YPE, SUFFIX, N) \
  OVERLOADABLE DATA_YPE read_image ## SUFFIX(IMG_TYPE cl_image, int##N coord);

#define DECL_IMAGE_WRITE_RETTYPE(IMG_TYPE, DATA_YPE, SUFFIX, N) \
  OVERLOADABLE void write_image ## SUFFIX(IMG_TYPE cl_image, int##N coord, DATA_YPE color);

#define DECL_IMAGE_TYPE_READ_NO_SAMPLE(IMG_TYPE, N)\
    DECL_IMAGE_READ_NO_SAMPLE_RETTYPE(IMG_TYPE, int4, i, N) \
    DECL_IMAGE_READ_NO_SAMPLE_RETTYPE(IMG_TYPE, uint4, ui, N) \
    DECL_IMAGE_READ_NO_SAMPLE_RETTYPE(IMG_TYPE, float4, f, N)

#define DECL_IMAGE_TYPE_READ_SAMPLE(IMG_TYPE, N)\
    DECL_IMAGE_READ_SAMPLE_RETTYPE(IMG_TYPE, int4, i, N) \
    DECL_IMAGE_READ_SAMPLE_RETTYPE(IMG_TYPE, uint4, ui, N) \
    DECL_IMAGE_READ_SAMPLE_RETTYPE(IMG_TYPE, float4, f, N)

#define DECL_IMAGE_TYPE_WRITE(IMG_TYPE, N)\
    DECL_IMAGE_WRITE_RETTYPE(IMG_TYPE, int4, i, N) \
    DECL_IMAGE_WRITE_RETTYPE(IMG_TYPE, uint4, ui, N) \
    DECL_IMAGE_WRITE_RETTYPE(IMG_TYPE, float4, f, N)

#if (__OPENCL_C_VERSION__ >= 200)
#define DECL_IMAGE(IMG_TYPE, N) \
    DECL_IMAGE_TYPE_READ_NO_SAMPLE(read_only IMG_TYPE, N) \
    DECL_IMAGE_TYPE_READ_NO_SAMPLE(read_write IMG_TYPE, N) \
    DECL_IMAGE_TYPE_READ_SAMPLE(read_only IMG_TYPE, N) \
    DECL_IMAGE_TYPE_WRITE(write_only IMG_TYPE, N) \
    DECL_IMAGE_TYPE_WRITE(read_write IMG_TYPE, N)
#else
#define DECL_IMAGE(IMG_TYPE, N) \
    DECL_IMAGE_TYPE_READ_NO_SAMPLE(read_only IMG_TYPE, N) \
    DECL_IMAGE_TYPE_READ_SAMPLE(read_only IMG_TYPE, N) \
    DECL_IMAGE_TYPE_WRITE(write_only IMG_TYPE, N)
#endif

DECL_IMAGE(image1d_t, 1)
DECL_IMAGE(image2d_t, 2)
DECL_IMAGE(image1d_array_t, 2)
DECL_IMAGE(image3d_t, 3)
DECL_IMAGE(image3d_t, 4)
DECL_IMAGE(image2d_array_t, 3)
DECL_IMAGE(image2d_array_t, 4)

#undef DECL_IMAGE

#if (__OPENCL_C_VERSION__ >= 200)
#define DECL_IMAGE(IMG_TYPE, N) \
    DECL_IMAGE_TYPE_READ_NO_SAMPLE(read_only IMG_TYPE, N) \
    DECL_IMAGE_TYPE_READ_NO_SAMPLE(read_write IMG_TYPE, N) \
    DECL_IMAGE_TYPE_WRITE(write_only IMG_TYPE, N) \
    DECL_IMAGE_TYPE_WRITE(read_write IMG_TYPE, N)
#else
#define DECL_IMAGE(IMG_TYPE, N) \
    DECL_IMAGE_TYPE_READ_NO_SAMPLE(read_only IMG_TYPE, N) \
    DECL_IMAGE_TYPE_WRITE(write_only IMG_TYPE, N)
#endif

DECL_IMAGE(image1d_buffer_t, 1)

#undef int1
#undef float1
#undef DECL_IMAGE_TYPE_READ_NO_SAMPLE
#undef DECL_IMAGE_TYPE_WRITE
#undef DECL_IMAGE

OVERLOADABLE int get_image_channel_data_type(read_only image1d_t image);
OVERLOADABLE int get_image_channel_order(read_only image1d_t image);
OVERLOADABLE int get_image_width(read_only image1d_t image);

OVERLOADABLE int get_image_channel_data_type(read_only image1d_buffer_t image);
OVERLOADABLE int get_image_channel_order(read_only image1d_buffer_t image);
OVERLOADABLE int get_image_width(read_only image1d_buffer_t image);

OVERLOADABLE int get_image_channel_data_type(read_only image2d_t image);
OVERLOADABLE int get_image_channel_order(read_only image2d_t image);
OVERLOADABLE int get_image_width(read_only image2d_t image);
OVERLOADABLE int get_image_height(read_only image2d_t image);
OVERLOADABLE int2 get_image_dim(read_only image2d_t image);

OVERLOADABLE int get_image_channel_data_type(read_only image1d_array_t image);
OVERLOADABLE int get_image_channel_order(read_only image1d_array_t image);
OVERLOADABLE int get_image_width(read_only image1d_array_t image);
OVERLOADABLE size_t get_image_array_size(read_only image1d_array_t image);

OVERLOADABLE int get_image_channel_data_type(read_only image3d_t image);
OVERLOADABLE int get_image_channel_order(read_only image3d_t image);
OVERLOADABLE int get_image_width(read_only image3d_t image);
OVERLOADABLE int get_image_height(read_only image3d_t image);
OVERLOADABLE int get_image_depth(read_only image3d_t image);
OVERLOADABLE int4 get_image_dim(read_only image3d_t image);

OVERLOADABLE int get_image_channel_data_type(read_only image2d_array_t image);
OVERLOADABLE int get_image_channel_order(read_only image2d_array_t image);
OVERLOADABLE int get_image_width(read_only image2d_array_t image);
OVERLOADABLE int get_image_height(read_only image2d_array_t image);
OVERLOADABLE int2 get_image_dim(read_only image2d_array_t image);
OVERLOADABLE size_t get_image_array_size(read_only image2d_array_t image);

#if __clang_major__*10 + __clang_minor__ >= 39
OVERLOADABLE int get_image_channel_data_type(write_only image1d_t image);
OVERLOADABLE int get_image_channel_order(write_only image1d_t image);
OVERLOADABLE int get_image_width(write_only image1d_t image);

OVERLOADABLE int get_image_channel_data_type(write_only image1d_buffer_t image);
OVERLOADABLE int get_image_channel_order(write_only image1d_buffer_t image);
OVERLOADABLE int get_image_width(write_only image1d_buffer_t image);

OVERLOADABLE int get_image_channel_data_type(write_only image2d_t image);
OVERLOADABLE int get_image_channel_order(write_only image2d_t image);
OVERLOADABLE int get_image_width(write_only image2d_t image);
OVERLOADABLE int get_image_height(write_only image2d_t image);
OVERLOADABLE int2 get_image_dim(write_only image2d_t image);

OVERLOADABLE int get_image_channel_data_type(write_only image1d_array_t image);
OVERLOADABLE int get_image_channel_order(write_only image1d_array_t image);
OVERLOADABLE int get_image_width(write_only image1d_array_t image);
OVERLOADABLE size_t get_image_array_size(write_only image1d_array_t image);

OVERLOADABLE int get_image_channel_data_type(write_only image3d_t image);
OVERLOADABLE int get_image_channel_order(write_only image3d_t image);
OVERLOADABLE int get_image_width(write_only image3d_t image);
OVERLOADABLE int get_image_height(write_only image3d_t image);
OVERLOADABLE int get_image_depth(write_only image3d_t image);
OVERLOADABLE int4 get_image_dim(write_only image3d_t image);

OVERLOADABLE int get_image_channel_data_type(write_only image2d_array_t image);
OVERLOADABLE int get_image_channel_order(write_only image2d_array_t image);
OVERLOADABLE int get_image_width(write_only image2d_array_t image);
OVERLOADABLE int get_image_height(write_only image2d_array_t image);
OVERLOADABLE int2 get_image_dim(write_only image2d_array_t image);
OVERLOADABLE size_t get_image_array_size(write_only image2d_array_t image);
#endif

#if (__OPENCL_C_VERSION__ >= 200)
OVERLOADABLE int get_image_channel_data_type(read_write image1d_t image);
OVERLOADABLE int get_image_channel_order(read_write image1d_t image);
OVERLOADABLE int get_image_width(read_write image1d_t image);

OVERLOADABLE int get_image_channel_data_type(read_write image1d_buffer_t image);
OVERLOADABLE int get_image_channel_order(read_write image1d_buffer_t image);
OVERLOADABLE int get_image_width(read_write image1d_buffer_t image);

OVERLOADABLE int get_image_channel_data_type(read_write image2d_t image);
OVERLOADABLE int get_image_channel_order(read_write image2d_t image);
OVERLOADABLE int get_image_width(read_write image2d_t image);
OVERLOADABLE int get_image_height(read_write image2d_t image);
OVERLOADABLE int2 get_image_dim(read_write image2d_t image);

OVERLOADABLE int get_image_channel_data_type(read_write image1d_array_t image);
OVERLOADABLE int get_image_channel_order(read_write image1d_array_t image);
OVERLOADABLE int get_image_width(read_write image1d_array_t image);
OVERLOADABLE size_t get_image_array_size(read_write image1d_array_t image);

OVERLOADABLE int get_image_channel_data_type(read_write image3d_t image);
OVERLOADABLE int get_image_channel_order(read_write image3d_t image);
OVERLOADABLE int get_image_width(read_write image3d_t image);
OVERLOADABLE int get_image_height(read_write image3d_t image);
OVERLOADABLE int get_image_depth(read_write image3d_t image);
OVERLOADABLE int4 get_image_dim(read_write image3d_t image);

OVERLOADABLE int get_image_channel_data_type(read_write image2d_array_t image);
OVERLOADABLE int get_image_channel_order(read_write image2d_array_t image);
OVERLOADABLE int get_image_width(read_write image2d_array_t image);
OVERLOADABLE int get_image_height(read_write image2d_array_t image);
OVERLOADABLE int2 get_image_dim(read_write image2d_array_t image);
OVERLOADABLE size_t get_image_array_size(read_write image2d_array_t image);
#endif

#endif

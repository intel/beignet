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

OVERLOADABLE int4 read_imagei(read_only image1d_t cl_image, const sampler_t sampler, int coord);
OVERLOADABLE int4 read_imagei(read_only image1d_t cl_image, const sampler_t sampler, float coord);
OVERLOADABLE int4 read_imagei(read_only image1d_t cl_image, int coord);
OVERLOADABLE void write_imagei(write_only image1d_t cl_image, int coord, int4 color);
OVERLOADABLE void write_imagei(write_only image1d_t cl_image, float coord, int4 color);
OVERLOADABLE uint4 read_imageui(read_only image1d_t cl_image, const sampler_t sampler, int coord);
OVERLOADABLE uint4 read_imageui(read_only image1d_t cl_image, const sampler_t sampler, float coord);
OVERLOADABLE uint4 read_imageui(read_only image1d_t cl_image, int coord);
OVERLOADABLE void write_imageui(write_only image1d_t cl_image, int coord, uint4 color);
OVERLOADABLE void write_imageui(write_only image1d_t cl_image, float coord, uint4 color);
OVERLOADABLE float4 read_imagef(read_only image1d_t cl_image, const sampler_t sampler, int coord);
OVERLOADABLE float4 read_imagef(read_only image1d_t cl_image, const sampler_t sampler, float coord);
OVERLOADABLE float4 read_imagef(read_only image1d_t cl_image, int coord);
OVERLOADABLE void write_imagef(write_only image1d_t cl_image, int coord, float4 color);
OVERLOADABLE void write_imagef(write_only image1d_t cl_image, float coord, float4 color);
OVERLOADABLE int4 read_imagei(read_only image1d_buffer_t cl_image, int coord);
OVERLOADABLE void write_imagei(write_only image1d_buffer_t cl_image, int coord, int4 color);
OVERLOADABLE uint4 read_imageui(read_only image1d_buffer_t cl_image, int coord);
OVERLOADABLE void write_imageui(write_only image1d_buffer_t cl_image, int coord, uint4 color);
OVERLOADABLE void write_imageui(write_only image1d_buffer_t cl_image, float coord, uint4 color);
OVERLOADABLE float4 read_imagef(read_only image1d_buffer_t cl_image, int coord);
OVERLOADABLE void write_imagef(write_only image1d_buffer_t cl_image, int coord, float4 color);

OVERLOADABLE int get_image_channel_data_type(read_only image1d_t image);
OVERLOADABLE int get_image_channel_order(read_only image1d_t image);
OVERLOADABLE int get_image_width(read_only image1d_t image);

OVERLOADABLE int get_image_channel_data_type(read_only image1d_buffer_t image);
OVERLOADABLE int get_image_channel_order(read_only image1d_buffer_t image);
OVERLOADABLE int get_image_width(read_only image1d_buffer_t image);

OVERLOADABLE int4 read_imagei(read_only image2d_t cl_image, const sampler_t sampler, int2 coord);
OVERLOADABLE int4 read_imagei(read_only image2d_t cl_image, const sampler_t sampler, float2 coord);
OVERLOADABLE int4 read_imagei(read_only image2d_t cl_image, int2 coord);
OVERLOADABLE void write_imagei(write_only image2d_t cl_image, int2 coord, int4 color);
OVERLOADABLE void write_imagei(write_only image2d_t cl_image, float2 coord, int4 color);
OVERLOADABLE uint4 read_imageui(read_only image2d_t cl_image, const sampler_t sampler, int2 coord);
OVERLOADABLE uint4 read_imageui(read_only image2d_t cl_image, const sampler_t sampler, float2 coord);
OVERLOADABLE uint4 read_imageui(read_only image2d_t cl_image, int2 coord);
OVERLOADABLE void write_imageui(write_only image2d_t cl_image, int2 coord, uint4 color);
OVERLOADABLE void write_imageui(write_only image2d_t cl_image, float2 coord, uint4 color);
OVERLOADABLE float4 read_imagef(read_only image2d_t cl_image, const sampler_t sampler, int2 coord);
OVERLOADABLE float4 read_imagef(read_only image2d_t cl_image, const sampler_t sampler, float2 coord);
OVERLOADABLE float4 read_imagef(read_only image2d_t cl_image, int2 coord);
OVERLOADABLE void write_imagef(write_only image2d_t cl_image, int2 coord, float4 color);
OVERLOADABLE void write_imagef(write_only image2d_t cl_image, float2 coord, float4 color);
OVERLOADABLE int4 read_imagei(read_only image1d_array_t cl_image, const sampler_t sampler, int2 coord);
OVERLOADABLE int4 read_imagei(read_only image1d_array_t cl_image, const sampler_t sampler, float2 coord);
OVERLOADABLE int4 read_imagei(read_only image1d_array_t cl_image, int2 coord);
OVERLOADABLE void write_imagei(write_only image1d_array_t cl_image, int2 coord, int4 color);
OVERLOADABLE void write_imagei(write_only image1d_array_t cl_image, float2 coord, int4 color);
OVERLOADABLE uint4 read_imageui(read_only image1d_array_t cl_image, const sampler_t sampler, int2 coord);
OVERLOADABLE uint4 read_imageui(read_only image1d_array_t cl_image, const sampler_t sampler, float2 coord);
OVERLOADABLE uint4 read_imageui(read_only image1d_array_t cl_image, int2 coord);
OVERLOADABLE void write_imageui(write_only image1d_array_t cl_image, int2 coord, uint4 color);
OVERLOADABLE void write_imageui(write_only image1d_array_t cl_image, float2 coord, uint4 color);
OVERLOADABLE float4 read_imagef(read_only image1d_array_t cl_image, const sampler_t sampler, int2 coord);
OVERLOADABLE float4 read_imagef(read_only image1d_array_t cl_image, const sampler_t sampler, float2 coord);
OVERLOADABLE float4 read_imagef(read_only image1d_array_t cl_image, int2 coord);
OVERLOADABLE void write_imagef(write_only image1d_array_t cl_image, int2 coord, float4 color);
OVERLOADABLE void write_imagef(write_only image1d_array_t cl_image, float2 coord, float4 color);

OVERLOADABLE int get_image_channel_data_type(read_only image2d_t image);
OVERLOADABLE int get_image_channel_order(read_only image2d_t image);
OVERLOADABLE int get_image_width(read_only image2d_t image);
OVERLOADABLE int get_image_height(read_only image2d_t image);
OVERLOADABLE int2 get_image_dim(read_only image2d_t image);

OVERLOADABLE int get_image_channel_data_type(read_only image1d_array_t image);
OVERLOADABLE int get_image_channel_order(read_only image1d_array_t image);
OVERLOADABLE int get_image_width(read_only image1d_array_t image);
OVERLOADABLE size_t get_image_array_size(read_only image1d_array_t image);

OVERLOADABLE int4 read_imagei(read_only image3d_t cl_image, const sampler_t sampler, int4 coord);
OVERLOADABLE int4 read_imagei(read_only image3d_t cl_image, const sampler_t sampler, float4 coord);
OVERLOADABLE int4 read_imagei(read_only image3d_t cl_image, int4 coord);
OVERLOADABLE void write_imagei(write_only image3d_t cl_image, int4 coord, int4 color);
OVERLOADABLE void write_imagei(write_only image3d_t cl_image, float4 coord, int4 color);
OVERLOADABLE uint4 read_imageui(read_only image3d_t cl_image, const sampler_t sampler, int4 coord);
OVERLOADABLE uint4 read_imageui(read_only image3d_t cl_image, const sampler_t sampler, float4 coord);
OVERLOADABLE uint4 read_imageui(read_only image3d_t cl_image, int4 coord);
OVERLOADABLE void write_imageui(write_only image3d_t cl_image, int4 coord, uint4 color);
OVERLOADABLE void write_imageui(write_only image3d_t cl_image, float4 coord, uint4 color);
OVERLOADABLE float4 read_imagef(read_only image3d_t cl_image, const sampler_t sampler, int4 coord);
OVERLOADABLE float4 read_imagef(read_only image3d_t cl_image, const sampler_t sampler, float4 coord);
OVERLOADABLE float4 read_imagef(read_only image3d_t cl_image, int4 coord);
OVERLOADABLE void write_imagef(write_only image3d_t cl_image, int4 coord, float4 color);
OVERLOADABLE void write_imagef(write_only image3d_t cl_image, float4 coord, float4 color);

OVERLOADABLE int4 read_imagei(read_only image3d_t cl_image, const sampler_t sampler, int3 coord);
OVERLOADABLE int4 read_imagei(read_only image3d_t cl_image, const sampler_t sampler, float3 coord);
OVERLOADABLE int4 read_imagei(read_only image3d_t cl_image, int3 coord);
OVERLOADABLE void write_imagei(write_only image3d_t cl_image, int3 coord, int4 color);
OVERLOADABLE void write_imagei(write_only image3d_t cl_image, float3 coord, int4 color);
OVERLOADABLE uint4 read_imageui(read_only image3d_t cl_image, const sampler_t sampler, int3 coord);
OVERLOADABLE uint4 read_imageui(read_only image3d_t cl_image, const sampler_t sampler, float3 coord);
OVERLOADABLE uint4 read_imageui(read_only image3d_t cl_image, int3 coord);
OVERLOADABLE void write_imageui(write_only image3d_t cl_image, int3 coord, uint4 color);
OVERLOADABLE void write_imageui(write_only image3d_t cl_image, float3 coord, uint4 color);
OVERLOADABLE float4 read_imagef(read_only image3d_t cl_image, const sampler_t sampler, int3 coord);
OVERLOADABLE float4 read_imagef(read_only image3d_t cl_image, const sampler_t sampler, float3 coord);
OVERLOADABLE float4 read_imagef(read_only image3d_t cl_image, int3 coord);
OVERLOADABLE void write_imagef(write_only image3d_t cl_image, int3 coord, float4 color);
OVERLOADABLE void write_imagef(write_only image3d_t cl_image, float3 coord, float4 color);
OVERLOADABLE int4 read_imagei(read_only image2d_array_t cl_image, const sampler_t sampler, int4 coord);
OVERLOADABLE int4 read_imagei(read_only image2d_array_t cl_image, const sampler_t sampler, float4 coord);
OVERLOADABLE int4 read_imagei(read_only image2d_array_t cl_image, int4 coord);
OVERLOADABLE void write_imagei(write_only image2d_array_t cl_image, int4 coord, int4 color);
OVERLOADABLE void write_imagei(write_only image2d_array_t cl_image, float4 coord, int4 color);
OVERLOADABLE uint4 read_imageui(read_only image2d_array_t cl_image, const sampler_t sampler, int4 coord);
OVERLOADABLE uint4 read_imageui(read_only image2d_array_t cl_image, const sampler_t sampler, float4 coord);
OVERLOADABLE uint4 read_imageui(read_only image2d_array_t cl_image, int4 coord);
OVERLOADABLE void write_imageui(write_only image2d_array_t cl_image, int4 coord, uint4 color);
OVERLOADABLE void write_imageui(write_only image2d_array_t cl_image, float4 coord, uint4 color);
OVERLOADABLE float4 read_imagef(read_only image2d_array_t cl_image, const sampler_t sampler, int4 coord);
OVERLOADABLE float4 read_imagef(read_only image2d_array_t cl_image, const sampler_t sampler, float4 coord);
OVERLOADABLE float4 read_imagef(read_only image2d_array_t cl_image, int4 coord);
OVERLOADABLE void write_imagef(write_only image2d_array_t cl_image, int4 coord, float4 color);
OVERLOADABLE void write_imagef(write_only image2d_array_t cl_image, float4 coord, float4 color);

OVERLOADABLE int4 read_imagei(read_only image2d_array_t cl_image, const sampler_t sampler, int3 coord);
OVERLOADABLE int4 read_imagei(read_only image2d_array_t cl_image, const sampler_t sampler, float3 coord);
OVERLOADABLE int4 read_imagei(read_only image2d_array_t cl_image, int3 coord);
OVERLOADABLE void write_imagei(write_only image2d_array_t cl_image, int3 coord, int4 color);
OVERLOADABLE void write_imagei(write_only image2d_array_t cl_image, float3 coord, int4 color);
OVERLOADABLE uint4 read_imageui(read_only image2d_array_t cl_image, const sampler_t sampler, int3 coord);
OVERLOADABLE uint4 read_imageui(read_only image2d_array_t cl_image, const sampler_t sampler, float3 coord);
OVERLOADABLE uint4 read_imageui(read_only image2d_array_t cl_image, int3 coord);
OVERLOADABLE void write_imageui(write_only image2d_array_t cl_image, int3 coord, uint4 color);
OVERLOADABLE void write_imageui(write_only image2d_array_t cl_image, float3 coord, uint4 color);
OVERLOADABLE float4 read_imagef(read_only image2d_array_t cl_image, const sampler_t sampler, int3 coord);
OVERLOADABLE float4 read_imagef(read_only image2d_array_t cl_image, const sampler_t sampler, float3 coord);
OVERLOADABLE float4 read_imagef(read_only image2d_array_t cl_image, int3 coord);
OVERLOADABLE void write_imagef(write_only image2d_array_t cl_image, int3 coord, float4 color);
OVERLOADABLE void write_imagef(write_only image2d_array_t cl_image, float3 coord, float4 color);

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

#endif

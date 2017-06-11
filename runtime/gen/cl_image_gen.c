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
 */

#include "cl_gen.h"
#include "intel_defines.h"
#include <math.h>

#define LOCAL_SZ_0 16
#define LOCAL_SZ_1 4
#define LOCAL_SZ_2 4

LOCAL cl_int
cl_image_format_support_gen(cl_device_id device, cl_mem_object_type image_type,
                            cl_image_format *image_format)
{
  uint32_t fmt = cl_image_get_gen_format(image_format);
  if (fmt == INTEL_UNSUPPORTED_FORMAT)
    return CL_FALSE;

  return CL_TRUE;
}

LOCAL uint32_t
cl_image_get_gen_format(const cl_image_format *fmt)
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
    case CL_HALF_FLOAT:
      return I965_SURFACEFORMAT_R16_FLOAT;
    case CL_FLOAT:
      return I965_SURFACEFORMAT_R32_FLOAT;
    //        case CL_SNORM_INT16:    return I965_SURFACEFORMAT_R16_SNORM;
    //        case CL_SNORM_INT8:     return I965_SURFACEFORMAT_R8_SNORM;
    case CL_UNORM_INT8:
      return I965_SURFACEFORMAT_R8_UNORM;
    case CL_UNORM_INT16:
      return I965_SURFACEFORMAT_R16_UNORM;
    case CL_SIGNED_INT8:
      return I965_SURFACEFORMAT_R8_SINT;
    case CL_SIGNED_INT16:
      return I965_SURFACEFORMAT_R16_SINT;
    case CL_SIGNED_INT32:
      return I965_SURFACEFORMAT_R32_SINT;
    case CL_UNSIGNED_INT8:
      return I965_SURFACEFORMAT_R8_UINT;
    case CL_UNSIGNED_INT16:
      return I965_SURFACEFORMAT_R16_UINT;
    case CL_UNSIGNED_INT32:
      return I965_SURFACEFORMAT_R32_UINT;
    default:
      return INTEL_UNSUPPORTED_FORMAT;
    };
  case CL_RG:
    switch (type) {
    case CL_UNORM_INT8:
      return I965_SURFACEFORMAT_R8G8_UNORM;
    case CL_UNORM_INT16:
      return I965_SURFACEFORMAT_R16G16_UNORM;
    case CL_UNSIGNED_INT8:
      return I965_SURFACEFORMAT_R8G8_UINT;
    case CL_UNSIGNED_INT16:
      return I965_SURFACEFORMAT_R16G16_UINT;
    default:
      return INTEL_UNSUPPORTED_FORMAT;
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
    case CL_HALF_FLOAT:
      return I965_SURFACEFORMAT_R16G16B16A16_FLOAT;
    case CL_FLOAT:
      return I965_SURFACEFORMAT_R32G32B32A32_FLOAT;
    //        case CL_SNORM_INT16:    return I965_SURFACEFORMAT_R16G16B16A16_SNORM;
    //        case CL_SNORM_INT8:     return I965_SURFACEFORMAT_R8G8B8A8_SNORM;
    case CL_UNORM_INT8:
      return I965_SURFACEFORMAT_R8G8B8A8_UNORM;
    case CL_UNORM_INT16:
      return I965_SURFACEFORMAT_R16G16B16A16_UNORM;
    case CL_SIGNED_INT8:
      return I965_SURFACEFORMAT_R8G8B8A8_SINT;
    case CL_SIGNED_INT16:
      return I965_SURFACEFORMAT_R16G16B16A16_SINT;
    case CL_SIGNED_INT32:
      return I965_SURFACEFORMAT_R32G32B32A32_SINT;
    case CL_UNSIGNED_INT8:
      return I965_SURFACEFORMAT_R8G8B8A8_UINT;
    case CL_UNSIGNED_INT16:
      return I965_SURFACEFORMAT_R16G16B16A16_UINT;
    case CL_UNSIGNED_INT32:
      return I965_SURFACEFORMAT_R32G32B32A32_UINT;
    default:
      return INTEL_UNSUPPORTED_FORMAT;
    };
  case CL_ARGB:
    return INTEL_UNSUPPORTED_FORMAT;
  case CL_BGRA:
    switch (type) {
    case CL_UNORM_INT8:
      return I965_SURFACEFORMAT_B8G8R8A8_UNORM;
    default:
      return INTEL_UNSUPPORTED_FORMAT;
    };
  case CL_sRGBA:
    switch (type) {
    case CL_UNORM_INT8:
      return I965_SURFACEFORMAT_R8G8B8A8_UNORM_SRGB;
    default:
      return INTEL_UNSUPPORTED_FORMAT;
    };
  case CL_sBGRA:
    switch (type) {
    case CL_UNORM_INT8:
      return I965_SURFACEFORMAT_B8G8R8A8_UNORM_SRGB;
    default:
      return INTEL_UNSUPPORTED_FORMAT;
    };
  case CL_NV12_INTEL:
    switch (type) {
    case CL_UNORM_INT8:
      return I965_SURFACEFORMAT_PLANAR_420_8;
    default:
      return INTEL_UNSUPPORTED_FORMAT;
    };
  default:
    return INTEL_UNSUPPORTED_FORMAT;
  };
}

LOCAL cl_int
cl_enqueue_image_fill_gen(cl_event event, cl_int status)
{
  cl_int ret = CL_SUCCESS;
  assert(event->exec_data.type == EnqueueFillImage);

  if (status == CL_QUEUED) {
    cl_command_queue queue = event->queue;
    const void *pattern = event->exec_data.fill_image.pattern;
    cl_mem mem = event->exec_data.fill_image.image;
    const size_t *origin = event->exec_data.fill_image.origin;
    const size_t *region = event->exec_data.fill_image.region;

    cl_kernel ker = NULL;
    size_t global_off[] = {0, 0, 0};
    size_t global_sz[] = {1, 1, 1};
    size_t local_sz[] = {LOCAL_SZ_0, LOCAL_SZ_1, LOCAL_SZ_2};
    cl_mem_image src_image = cl_mem_to_image(mem);
    cl_mem_gen mem_gen = (cl_mem_gen)mem->each_device[0];
    assert(mem_gen);
    uint32_t savedIntelFmt = mem_gen->image.intel_fmt;

    if (region[1] == 1)
      local_sz[1] = 1;
    if (region[2] == 1)
      local_sz[2] = 1;
    global_sz[0] = ((region[0] + local_sz[0] - 1) / local_sz[0]) * local_sz[0];
    global_sz[1] = ((region[1] + local_sz[1] - 1) / local_sz[1]) * local_sz[1];
    global_sz[2] = ((region[2] + local_sz[2] - 1) / local_sz[2]) * local_sz[2];

    if (src_image->image_type == CL_MEM_OBJECT_IMAGE1D) {
      ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device, CL_ENQUEUE_FILL_IMAGE_1D);
    } else if (src_image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
      ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device, CL_ENQUEUE_FILL_IMAGE_1D_ARRAY);
    } else if (src_image->image_type == CL_MEM_OBJECT_IMAGE2D) {
      ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device, CL_ENQUEUE_FILL_IMAGE_2D);
    } else if (src_image->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
      ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device, CL_ENQUEUE_FILL_IMAGE_2D_ARRAY);
    } else if (src_image->image_type == CL_MEM_OBJECT_IMAGE3D) {
      ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device, CL_ENQUEUE_FILL_IMAGE_3D);
    } else {
      return CL_IMAGE_FORMAT_NOT_SUPPORTED;
    }

    assert(ker);

    cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &src_image);
    if (src_image->fmt.image_channel_order >= CL_sRGBA) {
#define RGB2sRGB(linear) (linear <= 0.0031308f) ? (12.92f * linear) : (1.055f * powf(linear, 1.0f / 2.4f) - 0.055f);
      cl_image_format fmt;
      float newpattern[4] = {0.0, 0.0, 0.0, ((float *)pattern)[3]};
      int i;
      for (i = 0; i < 3; i++) {
        if (src_image->fmt.image_channel_order == CL_sRGBA) {
          newpattern[i] = RGB2sRGB(((float *)pattern)[i]);
        } else
          newpattern[2 - i] = RGB2sRGB(((float *)pattern)[i]);
      }
      cl_kernel_set_arg(ker, 1, sizeof(float) * 4, newpattern);
      fmt.image_channel_order = CL_RGBA;
      fmt.image_channel_data_type = CL_UNORM_INT8;
      mem_gen->image.intel_fmt = cl_image_get_gen_format(&fmt);
#undef RGB2sRGB
    } else
      cl_kernel_set_arg(ker, 1, sizeof(float) * 4, pattern);
    cl_kernel_set_arg(ker, 2, sizeof(cl_int), &region[0]);
    cl_kernel_set_arg(ker, 3, sizeof(cl_int), &region[1]);
    cl_kernel_set_arg(ker, 4, sizeof(cl_int), &region[2]);
    cl_kernel_set_arg(ker, 5, sizeof(cl_int), &origin[0]);
    cl_kernel_set_arg(ker, 6, sizeof(cl_int), &origin[1]);
    cl_kernel_set_arg(ker, 7, sizeof(cl_int), &origin[2]);

    ret = cl_command_queue_ND_range_wrap(queue, ker, event, 3, global_off, global_sz, local_sz);
    mem_gen->image.intel_fmt = savedIntelFmt;
    return ret;
  }

  if (status == CL_SUBMITTED) {
    assert(event->exec_data.exec_ctx);
    ret = cl_command_queue_flush_gpgpu(event->exec_data.exec_ctx);
    return ret;
  }

  if (status == CL_RUNNING) {
    /* Nothing to do */
    return CL_SUCCESS;
  }

  assert(status == CL_COMPLETE);
  assert(event->exec_data.exec_ctx);
  ret = cl_command_queue_finish_gpgpu(event->exec_data.exec_ctx);
  return ret;
}

LOCAL cl_int
cl_enqueue_image_copy_gen(cl_event event, cl_int status)
{
  cl_int ret = CL_SUCCESS;
  assert(event->exec_data.type == EnqueueCopyImage);

  if (status == CL_QUEUED) {
    cl_command_queue queue = event->queue;
    cl_mem src = event->exec_data.copy_image.src_image;
    cl_mem dst = event->exec_data.copy_image.dst_image;
    const size_t *src_origin = event->exec_data.copy_image.src_origin;
    const size_t *dst_origin = event->exec_data.copy_image.dst_origin;
    const size_t *region = event->exec_data.copy_image.region;
    cl_kernel ker = NULL;
    size_t global_off[] = {0, 0, 0};
    size_t global_sz[] = {1, 1, 1};
    size_t local_sz[] = {LOCAL_SZ_0, LOCAL_SZ_1, LOCAL_SZ_2};
    uint32_t fixupDataType;
    uint32_t savedIntelFmt;
    cl_mem_image src_image = cl_mem_to_image(src);
    cl_mem_image dst_image = cl_mem_to_image(dst);
    cl_mem_gen src_mem_gen = (cl_mem_gen)src->each_device[0];
    cl_mem_gen dst_mem_gen = (cl_mem_gen)dst->each_device[0];
    assert(src_mem_gen);
    assert(dst_mem_gen);

    if (region[1] == 1)
      local_sz[1] = 1;
    if (region[2] == 1)
      local_sz[2] = 1;
    global_sz[0] = ((region[0] + local_sz[0] - 1) / local_sz[0]) * local_sz[0];
    global_sz[1] = ((region[1] + local_sz[1] - 1) / local_sz[1]) * local_sz[1];
    global_sz[2] = ((region[2] + local_sz[2] - 1) / local_sz[2]) * local_sz[2];

    switch (src_image->fmt.image_channel_data_type) {
    case CL_SNORM_INT8:
    case CL_UNORM_INT8:
      fixupDataType = CL_UNSIGNED_INT8;
      break;
    case CL_HALF_FLOAT:
    case CL_SNORM_INT16:
    case CL_UNORM_INT16:
      fixupDataType = CL_UNSIGNED_INT16;
      break;
    case CL_FLOAT:
      fixupDataType = CL_UNSIGNED_INT32;
      break;
    default:
      fixupDataType = 0;
    }

    if (fixupDataType) {
      cl_image_format fmt;
      if (src_image->fmt.image_channel_order != CL_BGRA &&
          src_image->fmt.image_channel_order != CL_sBGRA &&
          src_image->fmt.image_channel_order != CL_sRGBA)
        fmt.image_channel_order = src_image->fmt.image_channel_order;
      else
        fmt.image_channel_order = CL_RGBA;

      fmt.image_channel_data_type = fixupDataType;
      savedIntelFmt = src_mem_gen->image.intel_fmt;
      src_mem_gen->image.intel_fmt = cl_image_get_gen_format(&fmt);
      dst_mem_gen->image.intel_fmt = src_mem_gen->image.intel_fmt;
    }

    /* We use one kernel to copy the data. The kernel is lazily created. */
    assert(src_image->base.ctx == dst_image->base.ctx);

    /* setup the kernel and run. */
    if (src_image->image_type == CL_MEM_OBJECT_IMAGE1D) {
      if (dst_image->image_type == CL_MEM_OBJECT_IMAGE1D) {
        ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device, CL_ENQUEUE_COPY_IMAGE_1D_TO_1D);
      }
    } else if (src_image->image_type == CL_MEM_OBJECT_IMAGE2D) {
      if (dst_image->image_type == CL_MEM_OBJECT_IMAGE2D) {
        ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device, CL_ENQUEUE_COPY_IMAGE_2D_TO_2D);
      } else if (dst_image->image_type == CL_MEM_OBJECT_IMAGE3D) {
        ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device, CL_ENQUEUE_COPY_IMAGE_2D_TO_3D);
      } else if (dst_image->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
        ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device, CL_ENQUEUE_COPY_IMAGE_2D_TO_2D_ARRAY);
      }
    } else if (src_image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
      if (dst_image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
        ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                                CL_ENQUEUE_COPY_IMAGE_1D_ARRAY_TO_1D_ARRAY);
      }
    } else if (src_image->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
      if (dst_image->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
        ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                                CL_ENQUEUE_COPY_IMAGE_2D_ARRAY_TO_2D_ARRAY);
      } else if (dst_image->image_type == CL_MEM_OBJECT_IMAGE2D) {
        ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                                CL_ENQUEUE_COPY_IMAGE_2D_ARRAY_TO_2D);
      } else if (dst_image->image_type == CL_MEM_OBJECT_IMAGE3D) {
        ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                                CL_ENQUEUE_COPY_IMAGE_2D_ARRAY_TO_3D);
      }
    } else if (src_image->image_type == CL_MEM_OBJECT_IMAGE3D) {
      if (dst_image->image_type == CL_MEM_OBJECT_IMAGE2D) {
        ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device, CL_ENQUEUE_COPY_IMAGE_3D_TO_2D);
      } else if (dst_image->image_type == CL_MEM_OBJECT_IMAGE3D) {
        ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device, CL_ENQUEUE_COPY_IMAGE_3D_TO_3D);
      } else if (dst_image->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
        ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                                CL_ENQUEUE_COPY_IMAGE_3D_TO_2D_ARRAY);
      }
    }
    assert(ker);

    cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &src_image);
    cl_kernel_set_arg(ker, 1, sizeof(cl_mem), &dst_image);
    cl_kernel_set_arg(ker, 2, sizeof(cl_int), &region[0]);
    cl_kernel_set_arg(ker, 3, sizeof(cl_int), &region[1]);
    cl_kernel_set_arg(ker, 4, sizeof(cl_int), &region[2]);
    cl_kernel_set_arg(ker, 5, sizeof(cl_int), &src_origin[0]);
    cl_kernel_set_arg(ker, 6, sizeof(cl_int), &src_origin[1]);
    cl_kernel_set_arg(ker, 7, sizeof(cl_int), &src_origin[2]);
    cl_kernel_set_arg(ker, 8, sizeof(cl_int), &dst_origin[0]);
    cl_kernel_set_arg(ker, 9, sizeof(cl_int), &dst_origin[1]);
    cl_kernel_set_arg(ker, 10, sizeof(cl_int), &dst_origin[2]);

    ret = cl_command_queue_ND_range_wrap(queue, ker, event, 1, global_off, global_sz, local_sz);

    if (fixupDataType) {
      src_mem_gen->image.intel_fmt = savedIntelFmt;
      dst_mem_gen->image.intel_fmt = savedIntelFmt;
    }
    return ret;
  }

  if (status == CL_SUBMITTED) {
    assert(event->exec_data.exec_ctx);
    ret = cl_command_queue_flush_gpgpu(event->exec_data.exec_ctx);
    return ret;
  }

  if (status == CL_RUNNING) {
    /* Nothing to do */
    return CL_SUCCESS;
  }

  assert(status == CL_COMPLETE);
  assert(event->exec_data.exec_ctx);
  ret = cl_command_queue_finish_gpgpu(event->exec_data.exec_ctx);
  return ret;
}

LOCAL cl_int
cl_enqueue_copy_image_to_buffer_gen(cl_event event, cl_int status)
{
  cl_int ret = CL_SUCCESS;
  assert(event->exec_data.type == EnqueueCopyImageToBuffer);

  if (status == CL_QUEUED) {
    cl_command_queue queue = event->queue;
    cl_mem the_image = event->exec_data.copy_image_and_buffer.image;
    cl_mem buffer = event->exec_data.copy_image_and_buffer.buffer;
    const size_t *src_origin = event->exec_data.copy_image_and_buffer.origin;
    const size_t *region = event->exec_data.copy_image_and_buffer.region;
    const size_t dst_offset = event->exec_data.copy_image_and_buffer.offset;
    cl_kernel ker = NULL;
    size_t global_off[] = {0, 0, 0};
    size_t global_sz[] = {1, 1, 1};
    size_t local_sz[] = {LOCAL_SZ_0, LOCAL_SZ_1, LOCAL_SZ_2};
    uint32_t intel_fmt, bpp;
    cl_image_format fmt;
    size_t origin0, region0;
    size_t kn_dst_offset;
    int align16 = 0;
    size_t align_size = 1;
    size_t w_saved;
    cl_mem_image image = cl_mem_to_image(the_image);
    cl_mem_gen image_gen = (cl_mem_gen)the_image->each_device[0];
    assert(image_gen);

    if (region[1] == 1)
      local_sz[1] = 1;
    if (region[2] == 1)
      local_sz[2] = 1;
    global_sz[0] = ((region[0] + local_sz[0] - 1) / local_sz[0]) * local_sz[0];
    global_sz[1] = ((region[1] + local_sz[1] - 1) / local_sz[1]) * local_sz[1];
    global_sz[2] = ((region[2] + local_sz[2] - 1) / local_sz[2]) * local_sz[2];

    /* We use one kernel to copy the data. The kernel is lazily created. */
    assert(image->base.ctx == buffer->ctx);

    intel_fmt = image_gen->image.intel_fmt;
    bpp = image->bpp;
    w_saved = image->w;
    region0 = region[0] * bpp;
    kn_dst_offset = dst_offset;
    if ((image->image_type == CL_MEM_OBJECT_IMAGE2D) && ((image->w * image->bpp) % 16 == 0) &&
        ((src_origin[0] * bpp) % 16 == 0) && (region0 % 16 == 0) && (dst_offset % 16 == 0)) {
      fmt.image_channel_order = CL_RGBA;
      fmt.image_channel_data_type = CL_UNSIGNED_INT32;
      align16 = 1;
      align_size = 16;
    } else {
      fmt.image_channel_order = CL_R;
      fmt.image_channel_data_type = CL_UNSIGNED_INT8;
      align_size = 1;
    }
    image_gen->image.intel_fmt = cl_image_get_gen_format(&fmt);
    image->w = (image->w * image->bpp) / align_size;
    image->bpp = align_size;
    region0 = (region[0] * bpp) / align_size;
    origin0 = (src_origin[0] * bpp) / align_size;
    kn_dst_offset /= align_size;
    global_sz[0] = ((region0 + local_sz[0] - 1) / local_sz[0]) * local_sz[0];

    /* setup the kernel and run. */
    if (image->image_type == CL_MEM_OBJECT_IMAGE2D) {
      if (align16) {
        ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                                CL_ENQUEUE_COPY_IMAGE_2D_TO_BUFFER_ALIGN16);
      } else {
        ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                                CL_ENQUEUE_COPY_IMAGE_2D_TO_BUFFER);
      }
    } else if (image->image_type == CL_MEM_OBJECT_IMAGE3D) {
      ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                              CL_ENQUEUE_COPY_IMAGE_3D_TO_BUFFER);
    }

    assert(ker);

    cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &image);
    cl_kernel_set_arg(ker, 1, sizeof(cl_mem), &buffer);
    cl_kernel_set_arg(ker, 2, sizeof(cl_int), &region0);
    cl_kernel_set_arg(ker, 3, sizeof(cl_int), &region[1]);
    cl_kernel_set_arg(ker, 4, sizeof(cl_int), &region[2]);
    cl_kernel_set_arg(ker, 5, sizeof(cl_int), &origin0);
    cl_kernel_set_arg(ker, 6, sizeof(cl_int), &src_origin[1]);
    cl_kernel_set_arg(ker, 7, sizeof(cl_int), &src_origin[2]);
    cl_kernel_set_arg(ker, 8, sizeof(cl_int), &kn_dst_offset);

    ret = cl_command_queue_ND_range_wrap(queue, ker, event, 1, global_off, global_sz, local_sz);

    image_gen->image.intel_fmt = intel_fmt;
    image->bpp = bpp;
    image->w = w_saved;
    return ret;
  }

  if (status == CL_SUBMITTED) {
    assert(event->exec_data.exec_ctx);
    ret = cl_command_queue_flush_gpgpu(event->exec_data.exec_ctx);
    return ret;
  }

  if (status == CL_RUNNING) {
    /* Nothing to do */
    return CL_SUCCESS;
  }

  assert(status == CL_COMPLETE);
  assert(event->exec_data.exec_ctx);
  ret = cl_command_queue_finish_gpgpu(event->exec_data.exec_ctx);
  return ret;
}

LOCAL cl_int
cl_enqueue_copy_buffer_to_image_gen(cl_event event, cl_int status)
{
  cl_int ret = CL_SUCCESS;
  assert(event->exec_data.type == EnqueueCopyBufferToImage);

  if (status == CL_QUEUED) {
    cl_command_queue queue = event->queue;
    cl_mem buffer = event->exec_data.copy_image_and_buffer.buffer;
    cl_mem the_image = event->exec_data.copy_image_and_buffer.image;
    const size_t src_offset = event->exec_data.copy_image_and_buffer.offset;
    const size_t *dst_origin = event->exec_data.copy_image_and_buffer.origin;
    const size_t *region = event->exec_data.copy_image_and_buffer.region;
    cl_kernel ker = NULL;
    size_t global_off[] = {0, 0, 0};
    size_t global_sz[] = {1, 1, 1};
    size_t local_sz[] = {LOCAL_SZ_0, LOCAL_SZ_1, LOCAL_SZ_2};
    uint32_t intel_fmt, bpp;
    cl_image_format fmt;
    size_t origin0, region0;
    size_t kn_src_offset;
    int align16 = 0;
    size_t align_size = 1;
    size_t w_saved = 0;
    cl_mem_image image = cl_mem_to_image(the_image);
    cl_mem_gen image_gen = (cl_mem_gen)the_image->each_device[0];
    assert(image_gen);

    if (region[1] == 1)
      local_sz[1] = 1;
    if (region[2] == 1)
      local_sz[2] = 1;
    global_sz[0] = ((region[0] + local_sz[0] - 1) / local_sz[0]) * local_sz[0];
    global_sz[1] = ((region[1] + local_sz[1] - 1) / local_sz[1]) * local_sz[1];
    global_sz[2] = ((region[2] + local_sz[2] - 1) / local_sz[2]) * local_sz[2];

    /* We use one kernel to copy the data. The kernel is lazily created. */
    assert(image->base.ctx == buffer->ctx);

    intel_fmt = image_gen->image.intel_fmt;
    bpp = image->bpp;
    w_saved = image->w;
    region0 = region[0] * bpp;
    kn_src_offset = src_offset;
    if ((image->image_type == CL_MEM_OBJECT_IMAGE2D) && ((image->w * image->bpp) % 16 == 0) &&
        ((dst_origin[0] * bpp) % 16 == 0) && (region0 % 16 == 0) && (src_offset % 16 == 0)) {
      fmt.image_channel_order = CL_RGBA;
      fmt.image_channel_data_type = CL_UNSIGNED_INT32;
      align16 = 1;
      align_size = 16;
    } else {
      fmt.image_channel_order = CL_R;
      fmt.image_channel_data_type = CL_UNSIGNED_INT8;
      align_size = 1;
    }
    image_gen->image.intel_fmt = cl_image_get_gen_format(&fmt);
    image->w = (image->w * image->bpp) / align_size;
    image->bpp = align_size;
    region0 = (region[0] * bpp) / align_size;
    origin0 = (dst_origin[0] * bpp) / align_size;
    kn_src_offset /= align_size;
    global_sz[0] = ((region0 + local_sz[0] - 1) / local_sz[0]) * local_sz[0];

    /* setup the kernel and run. */
    if (image->image_type == CL_MEM_OBJECT_IMAGE2D) {
      if (align16) {
        ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device,
                                                CL_ENQUEUE_COPY_BUFFER_TO_IMAGE_2D_ALIGN16);
      } else {
        ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device, CL_ENQUEUE_COPY_BUFFER_TO_IMAGE_2D);
      }
    } else if (image->image_type == CL_MEM_OBJECT_IMAGE3D) {
      ker = cl_context_get_builtin_kernel_gen(queue->ctx, queue->device, CL_ENQUEUE_COPY_BUFFER_TO_IMAGE_3D);
    }

    assert(ker);

    cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &image);
    cl_kernel_set_arg(ker, 1, sizeof(cl_mem), &buffer);
    cl_kernel_set_arg(ker, 2, sizeof(cl_int), &region0);
    cl_kernel_set_arg(ker, 3, sizeof(cl_int), &region[1]);
    cl_kernel_set_arg(ker, 4, sizeof(cl_int), &region[2]);
    cl_kernel_set_arg(ker, 5, sizeof(cl_int), &origin0);
    cl_kernel_set_arg(ker, 6, sizeof(cl_int), &dst_origin[1]);
    cl_kernel_set_arg(ker, 7, sizeof(cl_int), &dst_origin[2]);
    cl_kernel_set_arg(ker, 8, sizeof(cl_int), &kn_src_offset);

    ret = cl_command_queue_ND_range_wrap(queue, ker, event, 1, global_off, global_sz, local_sz);

    image_gen->image.intel_fmt = intel_fmt;
    image->bpp = bpp;
    image->w = w_saved;

    return ret;
  }

  if (status == CL_SUBMITTED) {
    assert(event->exec_data.exec_ctx);
    ret = cl_command_queue_flush_gpgpu(event->exec_data.exec_ctx);
    return ret;
  }

  if (status == CL_RUNNING) {
    /* Nothing to do */
    return CL_SUCCESS;
  }

  assert(status == CL_COMPLETE);
  assert(event->exec_data.exec_ctx);
  ret = cl_command_queue_finish_gpgpu(event->exec_data.exec_ctx);
  return ret;
}

static cl_image_gen_tiling
cl_gen_get_default_tiling(cl_device_id device)
{
  static int initialized = 0;
  static cl_image_gen_tiling tiling = CL_TILE_X;

  if (!initialized) {
    // FIXME, need to find out the performance diff's root cause on BDW.
    // SKL's 3D Image can't use TILE_X, so use TILE_Y as default
    if (IS_GEN9(device->device_id) || IS_GEN8(device->device_id))
      tiling = CL_TILE_Y;

    char *tilingStr = getenv("OCL_TILING");
    if (tilingStr != NULL) {
      switch (tilingStr[0]) {
      case '0':
        tiling = CL_NO_TILE;
        break;
      case '1':
        tiling = CL_TILE_X;
        break;
      case '2':
        tiling = CL_TILE_Y;
        break;
      default:
        break;
      }
    }
    initialized = 1;
  }

  return tiling;
}

static uint32_t
cl_gen_get_tiling_align(cl_device_id device, uint32_t tiling_mode, uint32_t dim)
{
  uint32_t ret = 0;

  switch (tiling_mode) {
  case CL_TILE_X:
    if (dim == 0) { //tileX width in bytes
      ret = 512;
    } else if (dim == 1) { //tileX height in number of rows
      ret = 8;
    } else if (dim == 2) {            //height to calculate slice pitch
      if (IS_GEN9(device->device_id)) //SKL same as tileY height
        ret = 8;
      else if (IS_GEN8(device->device_id)) //IVB, HSW, BDW same as CL_NO_TILE vertical alignment
        ret = 4;
      else
        ret = 2;
    } else
      assert(0);
    break;

  case CL_TILE_Y:
    if (dim == 0) { //tileY width in bytes
      ret = 128;
    } else if (dim == 1) { //tileY height in number of rows
      ret = 32;
    } else if (dim == 2) {            //height to calculate slice pitch
      if (IS_GEN9(device->device_id)) //SKL same as tileY height
        ret = 32;
      else if (IS_GEN8(device->device_id)) //IVB, HSW, BDW same as CL_NO_TILE vertical alignment
        ret = 4;
      else
        ret = 2;
    } else
      assert(0);
    break;

  case CL_NO_TILE:
    if (dim == 1 || dim == 2) {                                     //vertical alignment
      if (IS_GEN8(device->device_id) || IS_GEN9(device->device_id)) //SKL 1D array need 4 alignment qpitch
        ret = 4;
      else
        ret = 2;
    } else
      assert(0);
    break;
  }

  return ret;
}

static void
cl_mem_gen_upload_image(cl_mem_image image, cl_mem_gen mem_gen)
{
  cl_mem mem = &image->base;
  size_t origin[3] = {0, 0, 0};
  size_t region[3] = {image->w, image->h, image->depth};
  void *dst_ptr;

  assert(mem_gen->drm_bo);
  assert(mem_gen->drm_bo->host_coherent == CL_FALSE);
  assert(image->mem_from == NULL); // If image from buffer, no need to upload

  if ((mem->flags & CL_MEM_COPY_HOST_PTR) || (mem->flags & CL_MEM_USE_HOST_PTR)) {
    assert(mem->host_ptr);

    dst_ptr = cl_mem_gen_drm_bo_map(mem_gen->drm_bo, CL_FALSE);
    assert(dst_ptr);
    cl_mem_copy_image_region_helper(origin, region,
                                    dst_ptr, mem_gen->image.gpu_row_pitch, mem_gen->image.gpu_slice_pitch,
                                    mem->host_ptr, image->row_pitch, image->slice_pitch,
                                    image->bpp, image->w, image->h, CL_FALSE, CL_FALSE);
    cl_mem_gen_drm_bo_unmap(mem_gen->drm_bo);

    if (mem->flags & CL_MEM_COPY_HOST_PTR)
      mem->host_ptr = NULL; // Clear the content set by user
  }
}

static void
cl_mem_gen_image_parameter_init(cl_mem_image image, cl_mem_gen mem_gen)
{
  mem_gen->image.gpu_w = image->w;
  mem_gen->image.gpu_h = image->h;
  mem_gen->image.gpu_depth = image->depth;
  mem_gen->image.gpu_row_pitch = image->row_pitch;
  if (image->slice_pitch == 0)
    mem_gen->image.gpu_slice_pitch = mem_gen->image.gpu_h * mem_gen->image.gpu_row_pitch;
}

/* 1D and 1D array, never tiling and never real user ptr,
   because CL_NO_TILE need at least 2 alignment for height */
static cl_int
cl_mem_allocate_image_gen_1D(cl_device_id device, cl_mem mem)
{
  cl_context_gen ctx_gen;
  cl_mem_gen mem_gen = (cl_mem_gen)mem->each_device[0];
  cl_mem_image image = cl_mem_to_image(mem);
  size_t alignment = 64;

  DEV_PRIVATE_DATA(mem->ctx, device, ctx_gen);
  assert(ctx_gen);
  assert(image->mem_from == NULL);
  assert(mem_gen->drm_bo == NULL);

  /* Allocate the real mem bo */
  if (mem->flags & CL_MEM_PINNABLE)
    alignment = 4096;

  cl_mem_gen_image_parameter_init(image, mem_gen);
  mem_gen->image.gpu_row_pitch = image->w * image->bpp;
  assert(mem_gen->image.gpu_row_pitch <= image->row_pitch);

  assert(image->h == 1);
  mem_gen->image.gpu_slice_pitch = image->h * mem_gen->image.gpu_row_pitch;

  if (CL_OBJECT_IS_IMAGE_ARRAY(mem))
    mem_gen->image.gpu_slice_pitch = mem_gen->image.gpu_row_pitch *
                                     ALIGN(image->h, cl_gen_get_tiling_align(device, CL_NO_TILE, 2));

  mem_gen->drm_bo = cl_mem_gen_create_drm_bo(ctx_gen->drv->bufmgr,
                                             mem_gen->image.gpu_slice_pitch * mem_gen->image.gpu_depth,
                                             alignment, CL_NO_TILE, 0, NULL);

  cl_mem_gen_upload_image(image, mem_gen);
  return CL_SUCCESS;
}

/* No tiling and no real host ptr */
static cl_int
cl_mem_allocate_image_gen_1D_buffer(cl_device_id device, cl_mem mem)
{
  cl_context_gen ctx_gen;
  cl_mem_gen mem_gen = (cl_mem_gen)mem->each_device[0];
  cl_mem_image image = cl_mem_to_image(mem);
  cl_mem mem_from = image->mem_from;
  size_t alignment = 64;
  size_t aligned_h;
  DEV_PRIVATE_DATA(mem->ctx, device, ctx_gen);

  assert(CL_OBJECT_IS_BUFFER(mem_from));
  if (CL_OBJECT_IS_SUB_BUFFER(mem_from)) {
    mem_gen->image.sub_offset = cl_mem_to_buffer(mem_from)->sub_offset;
  }
  cl_mem_gen mem_from_gen = (cl_mem_gen)(mem_from->each_device[0]);
  assert(mem_from_gen);
  assert(mem_from);
  assert(ctx_gen);
  assert(image->h == 1);
  assert(image->depth == 1);
  assert(mem_gen->drm_bo == NULL);

  cl_mem_gen_image_parameter_init(image, mem_gen);
  /* This is an image1d buffer which exceeds normal image size restrication
     We have to use a 2D image to simulate this 1D image. */
  mem_gen->image.gpu_h = (image->w + device->image2d_max_width - 1) / device->image2d_max_width;
  mem_gen->image.gpu_w = image->w > device->image2d_max_width ? device->image2d_max_width : image->w;

  mem_gen->image.gpu_row_pitch = mem_gen->image.gpu_w * image->bpp;
  assert(mem_gen->image.gpu_row_pitch <= image->row_pitch);

  aligned_h = ALIGN(mem_gen->image.gpu_h, cl_gen_get_tiling_align(device, CL_NO_TILE, 1));
  mem_gen->image.gpu_slice_pitch = aligned_h * mem_gen->image.gpu_row_pitch;

  /* FIXME, we use 2D to imitate the 1D image for 1D buffer, the drm bo size is different and so
     we need to replace the old one with a new drm bo. Some risk if some is using it */
  if (mem_from_gen->buffer.already_convert_image) { // Already do the convert
    mem_gen->drm_bo = mem_from_gen->drm_bo;
    cl_mem_gen_drm_bo_ref(mem_gen->drm_bo);
    assert(mem_gen->drm_bo->tiling == CL_NO_TILE);
    return CL_SUCCESS;
  }

  /* Allocate the real mem bo */
  if (mem->flags & CL_MEM_PINNABLE)
    alignment = 4096;

  /* Just calculate a big enough size, later image from this buffer is always enough */
  size_t max_h = (mem_from->size + device->image2d_max_width - 1) / device->image2d_max_width;
  size_t max_sz = ALIGN(max_h, cl_gen_get_tiling_align(device, CL_NO_TILE, 1)) * device->image2d_max_width;
  assert(mem_gen->image.gpu_row_pitch * mem_gen->image.gpu_h <= max_sz);

  if (cl_mem_gen_drm_bo_expand(mem_from_gen->drm_bo, max_sz, alignment) == CL_FALSE) {
    return CL_MEM_OBJECT_ALLOCATION_FAILURE;
  }

  mem_from_gen->buffer.already_convert_image = CL_TRUE;
  mem_gen->drm_bo = mem_from_gen->drm_bo;
  cl_mem_gen_drm_bo_ref(mem_gen->drm_bo);
  assert(mem_gen->drm_bo->tiling == CL_NO_TILE);
  return CL_SUCCESS;
}

#define MAX_TILING_SIZE 128 * MB

/* 2D,3D and 2D array image. */
static cl_int
cl_mem_allocate_image_gen_2D_3D(cl_device_id device, cl_mem mem)
{
  cl_context_gen ctx_gen;
  cl_mem_gen mem_gen = (cl_mem_gen)mem->each_device[0];
  cl_mem_image image = cl_mem_to_image(mem);
  int enableUserptr = 0;
  int enable_true_hostptr = 0;
  cl_uint cacheline_size = 0;
  size_t alignment = 64;
  cl_image_gen_tiling tiling;
  size_t total_gpu_size = 0;
  size_t aligned_h;

  DEV_PRIVATE_DATA(mem->ctx, device, ctx_gen);
  assert(ctx_gen);
  assert(image->mem_from == NULL);
  assert(mem_gen->drm_bo == NULL);

  cl_mem_gen_image_parameter_init(image, mem_gen);

#ifdef HAS_USERPTR
  /* Only enable real user ptr if user set */
  const char *env = getenv("OCL_IMAGE_HOSTPTR");
  if (env != NULL) {
    sscanf(env, "%i", &enable_true_hostptr);
  }
#endif

  enableUserptr = 0;
  tiling = cl_gen_get_default_tiling(device);

  if (enable_true_hostptr && device->host_unified_memory && (mem->flags & CL_MEM_USE_HOST_PTR)) {
    cacheline_size = device->global_mem_cache_line_size;
    if (ALIGN((unsigned long)mem->host_ptr, cacheline_size) == (unsigned long)mem->host_ptr &&
        ALIGN(image->h, cl_gen_get_tiling_align(device, CL_NO_TILE, 1)) == image->h &&
        ALIGN(image->h * image->row_pitch * image->depth, cacheline_size) ==
          image->h * image->row_pitch * image->depth &&
        /* If 3D and 2D array, slice pitch must match */
        ((image->image_type == CL_MEM_OBJECT_IMAGE3D || image->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) &&
         image->row_pitch * image->h == image->slice_pitch)) {
      tiling = CL_NO_TILE;
      enableUserptr = 1;
    }
  }

  if (enableUserptr) {
    total_gpu_size = mem_gen->image.gpu_slice_pitch * mem_gen->image.gpu_depth;
    mem_gen->drm_bo = cl_mem_gen_create_drm_bo_from_hostptr(
      ctx_gen->drv->bufmgr, CL_FALSE, total_gpu_size, cacheline_size, mem->host_ptr);
  } else { // change for the param.
    mem_gen->image.gpu_row_pitch = image->w * image->bpp;
    if (tiling != CL_NO_TILE)
      mem_gen->image.gpu_row_pitch = ALIGN(mem_gen->image.gpu_row_pitch,
                                           cl_gen_get_tiling_align(device, tiling, 0));

    if (CL_OBJECT_IS_IMAGE_ARRAY(mem) || CL_OBJECT_IS_3D_IMAGE(mem))
      aligned_h = ALIGN(image->h, cl_gen_get_tiling_align(device, tiling, 2));
    else
      aligned_h = ALIGN(image->h, cl_gen_get_tiling_align(device, tiling, 1));

    mem_gen->image.gpu_slice_pitch = mem_gen->image.gpu_row_pitch * aligned_h;
    total_gpu_size = mem_gen->image.gpu_slice_pitch * mem_gen->image.gpu_depth;

    /* If sz is large than 128MB, map gtt may fail in some system.
       Because there is no obviours performance drop, disable tiling. */
    if (tiling != CL_NO_TILE && total_gpu_size > MAX_TILING_SIZE) {
      tiling = CL_NO_TILE;

      mem_gen->image.gpu_row_pitch = image->w * image->bpp;

      if (CL_OBJECT_IS_IMAGE_ARRAY(mem) || CL_OBJECT_IS_3D_IMAGE(mem))
        aligned_h = ALIGN(image->h, cl_gen_get_tiling_align(device, tiling, 2));
      else
        aligned_h = ALIGN(image->h, cl_gen_get_tiling_align(device, tiling, 1));

      mem_gen->image.gpu_slice_pitch = mem_gen->image.gpu_row_pitch * aligned_h;
      total_gpu_size = mem_gen->image.gpu_slice_pitch * mem_gen->image.gpu_depth;
    }
  }

  /* Allocate the real mem bo */
  if (mem->flags & CL_MEM_PINNABLE || tiling != CL_NO_TILE)
    alignment = 4096;

  if (mem_gen->drm_bo == NULL)
    mem_gen->drm_bo = cl_mem_gen_create_drm_bo(ctx_gen->drv->bufmgr, total_gpu_size, alignment,
                                               tiling, mem_gen->image.gpu_row_pitch, NULL);

  assert(mem_gen->drm_bo);
  cl_mem_gen_upload_image(image, mem_gen);

  return CL_SUCCESS;
}

static cl_int
cl_mem_allocate_image_gen_2D_buffer(cl_device_id device, cl_mem mem)
{
  cl_context_gen ctx_gen;
  cl_mem_gen mem_gen = (cl_mem_gen)mem->each_device[0];
  cl_mem_image image = cl_mem_to_image(mem);
  cl_mem mem_from = image->mem_from;
  if (CL_OBJECT_IS_SUB_BUFFER(mem_from)) {
    mem_gen->image.sub_offset = cl_mem_to_buffer(mem_from)->sub_offset;
  }
  cl_mem_gen mem_from_gen = (cl_mem_gen)(mem_from->each_device[0]);

  DEV_PRIVATE_DATA(mem->ctx, device, ctx_gen);
  assert(mem_from);
  assert(ctx_gen);
  assert(mem_gen->drm_bo == NULL);
  cl_mem_gen_image_parameter_init(image, mem_gen);

  if (CL_OBJECT_IS_2D_IMAGE(mem_from)) {
    assert(mem_gen->image.sub_offset == 0);
    /* According to spec, if from another image, just the channel order
       is different, so we can inherit all parameters of the old image */
    mem_gen->image.gpu_w = mem_from_gen->image.gpu_w;
    mem_gen->image.gpu_h = mem_from_gen->image.gpu_h;
    mem_gen->image.gpu_row_pitch = mem_from_gen->image.gpu_row_pitch;
    mem_gen->image.gpu_slice_pitch = mem_from_gen->image.gpu_slice_pitch;
    mem_gen->drm_bo = mem_from_gen->drm_bo;
    cl_mem_gen_drm_bo_ref(mem_gen->drm_bo);
    return CL_SUCCESS;
  }

  assert(CL_OBJECT_IS_BUFFER(mem_from));
  /* Image from a real buffer */
  mem_gen->image.gpu_row_pitch = image->row_pitch;
  mem_gen->image.gpu_slice_pitch = mem_gen->image.gpu_row_pitch *
                                   ALIGN(image->h, cl_gen_get_tiling_align(device, CL_NO_TILE, 1));
  mem_gen->drm_bo = mem_from_gen->drm_bo;
  cl_mem_gen_drm_bo_ref(mem_gen->drm_bo);
  assert(mem_gen->drm_bo->tiling == CL_NO_TILE);
  return CL_SUCCESS;
}

LOCAL cl_int
cl_mem_allocate_image_gen(cl_device_id device, cl_mem mem)
{
  cl_mem_gen mem_gen;
  cl_mem_image image = cl_mem_to_image(mem);
  cl_int err = CL_SUCCESS;

  mem_gen = CL_CALLOC(1, sizeof(_cl_mem_gen));
  if (mem_gen == NULL)
    return CL_OUT_OF_HOST_MEMORY;

  mem_gen->mem_base.device = device;
  mem->each_device[0] = (cl_mem_for_device)mem_gen;

  /* Only a sub-set of the formats are supported */
  mem_gen->image.intel_fmt = cl_image_get_gen_format(&image->fmt);
  if (mem_gen->image.intel_fmt == INTEL_UNSUPPORTED_FORMAT) {
    mem->each_device[0] = NULL;
    CL_FREE(mem_gen);
    return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
  }

  if (image->image_type == CL_MEM_OBJECT_IMAGE1D ||
      image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
    err = cl_mem_allocate_image_gen_1D(device, mem);
  } else if (image->image_type == CL_MEM_OBJECT_IMAGE3D ||
             image->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY ||
             image->image_type == CL_MEM_OBJECT_IMAGE2D) {
    if (image->mem_from) {
      err = cl_mem_allocate_image_gen_2D_buffer(device, mem);
    } else {
      err = cl_mem_allocate_image_gen_2D_3D(device, mem);
    }
  } else if (image->image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER) {
    err = cl_mem_allocate_image_gen_1D_buffer(device, mem);
  } else
    assert(0);

#if 0
  printf("---- Create image with width: %ld, height: %ld, depth %ld, row_pitch: %ld, slice_pitch %ld, \n"
         "--- GPU Real size: width: %ld, height: %ld, depth %ld, row_pitch: %ld, slice_pitch %ld, tiling is %d\n",
         image->w, image->h, image->depth, image->row_pitch, image->slice_pitch,
         mem_gen->image.gpu_w, mem_gen->image.gpu_h, mem_gen->image.gpu_depth, mem_gen->image.gpu_row_pitch,
         mem_gen->image.gpu_slice_pitch, mem_gen->drm_bo->tiling);
#endif

  if (err != CL_SUCCESS) {
    mem->each_device[0] = NULL;
    CL_FREE(mem_gen);
  }

  return err;
}

LOCAL cl_int
cl_enqueue_handle_map_image_gen(cl_event event, cl_int status)
{
  cl_mem mem = event->exec_data.map_image.mem_obj;
  cl_mem_image image = cl_mem_to_image(event->exec_data.map_image.mem_obj);
  cl_mem_gen mem_gen = (cl_mem_gen)mem->each_device[0];
  void *ptr = NULL;
  assert(mem_gen);
  assert(event->exec_data.map_image.origin[0] + event->exec_data.map_image.region[0] <= image->w);
  assert(event->exec_data.map_image.origin[1] + event->exec_data.map_image.region[1] <= image->h);
  assert(event->exec_data.map_image.origin[2] + event->exec_data.map_image.region[2] <= image->depth);

  if (status == CL_SUBMITTED || status == CL_RUNNING)
    return CL_SUCCESS;

  if (status == CL_QUEUED) {
    ptr = cl_mem_gen_drm_bo_map(mem_gen->drm_bo, event->exec_data.map_image.unsync_map);
    assert(ptr);
    ptr += mem_gen->image.sub_offset;

    if (mem->flags & CL_MEM_USE_HOST_PTR) {
      assert(mem->host_ptr);
      event->exec_data.map_image.ptr = mem->host_ptr + image->bpp * event->exec_data.map_image.origin[0] +
                                       image->row_pitch * event->exec_data.map_image.origin[1] +
                                       image->slice_pitch * event->exec_data.map_image.origin[2];
      event->exec_data.map_image.row_pitch = image->row_pitch;
      event->exec_data.map_image.slice_pitch = image->slice_pitch;
    } else {
      event->exec_data.map_image.ptr = ptr + image->bpp * event->exec_data.map_image.origin[0] +
                                       mem_gen->image.gpu_row_pitch * event->exec_data.map_image.origin[1] +
                                       mem_gen->image.gpu_slice_pitch * event->exec_data.map_image.origin[2];
      event->exec_data.map_image.row_pitch = mem_gen->image.gpu_row_pitch;
      event->exec_data.map_image.slice_pitch = mem_gen->image.gpu_slice_pitch;
      if (image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
        event->exec_data.map_image.row_pitch = event->exec_data.map_image.slice_pitch;
      if (image->image_type == CL_MEM_OBJECT_IMAGE1D || image->image_type == CL_MEM_OBJECT_IMAGE2D)
        event->exec_data.map_image.slice_pitch = 0;
    }

    event->exec_data.exec_ctx = ptr; // Find a place to store the mapped ptr temp
    return CL_SUCCESS;
  }

  assert(status == CL_COMPLETE);
  /* Assure the map complete */
  if (event->exec_data.map_image.unsync_map)
    cl_mem_gen_drm_bo_sync(mem_gen->drm_bo);

  ptr = event->exec_data.exec_ctx;
  assert(ptr);

  /* Sync back the data to host if fake USE_HOST_PTR */
  if ((mem->flags & CL_MEM_USE_HOST_PTR) && ptr != mem->host_ptr) {
    assert(event->exec_data.map_image.ptr == ((char *)mem->host_ptr +
                                              image->bpp * event->exec_data.map_image.origin[0] +
                                              image->row_pitch * event->exec_data.map_image.origin[1] +
                                              image->slice_pitch * event->exec_data.map_image.origin[2]));

    cl_mem_copy_image_region_helper(event->exec_data.map_image.origin, event->exec_data.map_image.region,
                                    event->exec_data.map_image.ptr, image->row_pitch, image->slice_pitch,
                                    ptr, mem_gen->image.gpu_row_pitch, mem_gen->image.gpu_slice_pitch,
                                    image->bpp, image->w, image->h, CL_FALSE, CL_TRUE);
  }

  return CL_SUCCESS;
}

LOCAL cl_int
cl_enqueue_handle_unmap_image_gen(cl_event event, cl_int status)
{
  cl_mem mem = event->exec_data.unmap.mem_obj;
  cl_mem_image image = cl_mem_to_image(event->exec_data.unmap.mem_obj);
  cl_mem_gen mem_gen = (cl_mem_gen)mem->each_device[0];

  assert(mem_gen);
  assert(mem_gen->drm_bo);
  assert(event->exec_data.unmap.ptr);

  if (status == CL_QUEUED || status == CL_RUNNING || status == CL_SUBMITTED)
    return CL_SUCCESS;

  /* Sync back the content if fake USE_HOST_PTR */
  void *host_offset_ptr = mem->host_ptr + image->bpp * event->exec_data.map_image.origin[0] +
                          image->row_pitch * event->exec_data.map_image.origin[1] +
                          image->slice_pitch * event->exec_data.map_image.origin[2];
  if ((mem->flags & CL_MEM_USE_HOST_PTR) && (host_offset_ptr != mem_gen->drm_bo->mapped_ptr)) {
    assert(mem_gen->drm_bo->mapped_ptr);

    void *dst_ptr = mem_gen->drm_bo->mapped_ptr + mem_gen->image.sub_offset;
    cl_mem_copy_image_region_helper(event->exec_data.unmap.origin, event->exec_data.unmap.region,
                                    dst_ptr, mem_gen->image.gpu_row_pitch, mem_gen->image.gpu_slice_pitch,
                                    event->exec_data.unmap.ptr, image->row_pitch, image->slice_pitch,
                                    image->bpp, image->w, image->h, CL_TRUE, CL_FALSE);
  }

  cl_mem_gen_drm_bo_unmap(mem_gen->drm_bo);
  return CL_SUCCESS;
}

LOCAL cl_int
cl_enqueue_read_image_gen(cl_event event, cl_int status)
{
  cl_mem mem = event->exec_data.read_write_image.image;
  cl_mem_gen mem_gen = (cl_mem_gen)mem->each_device[0];
  void *data_ptr = NULL;
  const size_t *origin = event->exec_data.read_write_image.origin;
  const size_t *region = event->exec_data.read_write_image.region;
  cl_mem_image image;

  assert(mem_gen);
  assert(mem_gen->drm_bo);
  assert(CL_OBJECT_IS_IMAGE(mem));
  assert(event->exec_data.type == EnqueueReadImage);

  if (status == CL_QUEUED || status == CL_RUNNING || status == CL_SUBMITTED)
    return CL_SUCCESS;

  image = cl_mem_to_image(mem);
  data_ptr = cl_mem_gen_drm_bo_map(mem_gen->drm_bo, CL_FALSE);
  if (data_ptr == NULL)
    return CL_OUT_OF_RESOURCES;

  data_ptr += mem_gen->image.sub_offset;

  cl_mem_copy_image_region_helper(origin, region,
                                  event->exec_data.read_write_image.ptr, event->exec_data.read_write_image.row_pitch,
                                  event->exec_data.read_write_image.slice_pitch,
                                  data_ptr, mem_gen->image.gpu_row_pitch, mem_gen->image.gpu_slice_pitch,
                                  image->bpp, image->w, image->h, CL_FALSE, CL_TRUE);

  cl_mem_gen_drm_bo_unmap(mem_gen->drm_bo);
  return CL_SUCCESS;
}

LOCAL cl_int
cl_enqueue_write_image_gen(cl_event event, cl_int status)
{
  cl_mem mem = event->exec_data.read_write_image.image;
  cl_mem_gen mem_gen = (cl_mem_gen)mem->each_device[0];
  void *data_ptr = NULL;
  const size_t *origin = event->exec_data.read_write_image.origin;
  const size_t *region = event->exec_data.read_write_image.region;
  cl_mem_image image;
  void *src_ptr = event->exec_data.read_write_image.ptr;

  assert(mem_gen);
  assert(mem_gen->drm_bo);
  assert(CL_OBJECT_IS_IMAGE(mem));
  assert(event->exec_data.type == EnqueueWriteImage);

  if (status == CL_QUEUED || status == CL_RUNNING || status == CL_SUBMITTED)
    return CL_SUCCESS;

  image = cl_mem_to_image(mem);
  data_ptr = cl_mem_gen_drm_bo_map(mem_gen->drm_bo, CL_FALSE);
  if (data_ptr == NULL)
    return CL_OUT_OF_RESOURCES;

  data_ptr += mem_gen->image.sub_offset;

  cl_mem_copy_image_region_helper(origin, region,
                                  data_ptr, mem_gen->image.gpu_row_pitch, mem_gen->image.gpu_slice_pitch,
                                  src_ptr, event->exec_data.read_write_image.row_pitch,
                                  event->exec_data.read_write_image.slice_pitch,
                                  image->bpp, image->w, image->h, CL_TRUE, CL_FALSE);

  cl_mem_gen_drm_bo_unmap(mem_gen->drm_bo);
  return CL_SUCCESS;
}

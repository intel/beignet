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
 * Author: Zhigang Gong <zhigang.gong@intel.com>
 */
#include <GL/gl.h>
#include <GL/glext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <assert.h>
#include <stdio.h>

#include "cl_mem.h"
#include "cl_image.h"
#include "cl_context.h"
#include "cl_utils.h"
#include "cl_alloc.h"
#include "cl_device_id.h"
#include "cl_driver.h"
#include "cl_platform_id.h"
#include "cl_mem_gl.h"

#include "CL/cl.h"
#include "CL/cl_intel.h"
#include "CL/cl_gl.h"

#ifndef CL_VERSION_1_2
#define CL_INVALID_IMAGE_DESCRIPTOR -65
#endif

static int cl_get_clformat_from_texture(GLint tex_format, cl_image_format * cl_format)
{
  cl_int ret = CL_SUCCESS;

  switch (tex_format) {
  case GL_RGBA8:
  case GL_RGBA:
  case GL_RGBA16:
  case GL_RGBA8I:
  case GL_RGBA16I:
  case GL_RGBA32I:
  case GL_RGBA8UI:
  case GL_RGBA16UI:
  case GL_RGBA32UI:
  case GL_RGBA16F:
  case GL_RGBA32F:
    cl_format->image_channel_order = CL_RGBA;
    break;
  case GL_BGRA:
    cl_format->image_channel_order = CL_BGRA;
    break;
  default:
    ret = CL_INVALID_IMAGE_DESCRIPTOR;
    goto error;
  }

  switch (tex_format) {
  case GL_RGBA8:
  case GL_RGBA:
  case GL_BGRA:
    cl_format->image_channel_data_type = CL_UNORM_INT8;
    break;
  case GL_RGBA16:
    cl_format->image_channel_data_type = CL_UNORM_INT16;
    break;
  case GL_RGBA8I:
    cl_format->image_channel_data_type = CL_SIGNED_INT8;
    break;
  case GL_RGBA16I:
    cl_format->image_channel_data_type = CL_SIGNED_INT16;
    break;
  case GL_RGBA32I:
    cl_format->image_channel_data_type = CL_SIGNED_INT32;
    break;
  case GL_RGBA8UI:
    cl_format->image_channel_data_type = CL_UNSIGNED_INT8;
    break;
  case GL_RGBA16UI:
    cl_format->image_channel_data_type = CL_UNSIGNED_INT16;
    break;
  case GL_RGBA32UI:
    cl_format->image_channel_data_type = CL_UNSIGNED_INT32;
    break;
  case GL_RGBA16F:
    cl_format->image_channel_data_type = CL_HALF_FLOAT;
    break;
  case GL_RGBA32F:
    cl_format->image_channel_order = CL_FLOAT;
    break;
  default:
    ret = CL_INVALID_IMAGE_DESCRIPTOR;
    goto error;
  }

error:
  return ret;
}

static cl_mem_object_type
get_mem_type_from_target(GLenum texture_target)
{
  switch(texture_target) {
  case GL_TEXTURE_1D: return CL_MEM_OBJECT_IMAGE1D;
  case GL_TEXTURE_2D: return CL_MEM_OBJECT_IMAGE2D;
  case GL_TEXTURE_3D: return CL_MEM_OBJECT_IMAGE3D;
  case GL_TEXTURE_1D_ARRAY: return CL_MEM_OBJECT_IMAGE1D_ARRAY;
  case GL_TEXTURE_2D_ARRAY: return CL_MEM_OBJECT_IMAGE2D_ARRAY;
  default:
    assert(0);
  }
  return 0;
}

LOCAL cl_mem cl_mem_new_gl_buffer(cl_context ctx,
                                  cl_mem_flags flags,
                                  GLuint buf_obj, 
                                  cl_int *errcode_ret)
{
  NOT_IMPLEMENTED;
}

EGLImageKHR cl_create_textured_egl_image(cl_context ctx,
                                         GLenum texture_target,
                                         GLint miplevel,
                                         GLuint texture)
{
  struct cl_gl_ext_deps *egl_funcs;
  EGLDisplay egl_display;
  EGLContext egl_context;
  EGLint egl_attribs[] = { EGL_GL_TEXTURE_LEVEL_KHR, miplevel, EGL_NONE};

  assert(ctx->props.gl_type == CL_GL_EGL_DISPLAY);
  egl_funcs =  CL_EXTENSION_GET_FUNCS(ctx, khr_gl_sharing, gl_ext_deps);
  assert(egl_funcs != NULL);
  egl_display = (EGLDisplay)ctx->props.egl_display;
  egl_context = (EGLDisplay)ctx->props.gl_context;
  return egl_funcs->eglCreateImageKHR_func(egl_display, egl_context,
                                           EGL_GL_TEXTURE_2D_KHR,
                                           (EGLClientBuffer)(uintptr_t)texture,
                                           &egl_attribs[0]);
}

LOCAL cl_mem cl_mem_new_gl_texture(cl_context ctx,
                                   cl_mem_flags flags,
                                   GLenum texture_target,
                                   GLint miplevel,
                                   GLuint texture,
                                   cl_int *errcode_ret)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;
  EGLImageKHR egl_image;
  int w, h, pitch, tiling;
  unsigned int bpp, intel_fmt;
  cl_image_format cl_format;
  unsigned int gl_format;
  /* Check flags consistency */
  if (UNLIKELY(flags & CL_MEM_COPY_HOST_PTR)) {
    err = CL_INVALID_ARG_VALUE;
    goto error;
  }

  TRY_ALLOC (mem, CALLOC(struct _cl_mem));
  mem->ctx = ctx;
  cl_context_add_ref(ctx);

  egl_image = cl_create_textured_egl_image(ctx, texture_target, miplevel, texture);

  if (egl_image == NULL) {
    err = CL_INVALID_GL_OBJECT;
    goto error;
  }
  mem->egl_image = egl_image;
  mem->bo = cl_buffer_alloc_from_eglimage(ctx, (void*)egl_image, &gl_format, &w, &h, &pitch, &tiling);
  if (UNLIKELY(mem->bo == NULL)) {
    err = CL_MEM_OBJECT_ALLOCATION_FAILURE;
    goto error;
  }
  cl_get_clformat_from_texture(gl_format, &cl_format);

  /* XXX Maybe we'd better to check the hw format in driver? */
  intel_fmt = cl_image_get_intel_format(&cl_format);

  if (intel_fmt == INTEL_UNSUPPORTED_FORMAT) {
    err = CL_INVALID_IMAGE_DESCRIPTOR;
    goto error;
  }
  cl_image_byte_per_pixel(&cl_format, &bpp);

  mem->type = get_mem_type_from_target(texture_target);
  mem->w = w;
  mem->h = h;
  mem->depth = 1;
  mem->fmt = cl_format;
  mem->intel_fmt = intel_fmt;
  mem->bpp = bpp;
  mem->is_image = 1;
  mem->row_pitch = pitch;
  mem->slice_pitch = 0;
  mem->tiling = tiling;
  mem->ref_n = 1;
  mem->magic = CL_MAGIC_MEM_HEADER;
  mem->flags = flags;

  /* Append the buffer in the context buffer list */
  pthread_mutex_lock(&ctx->buffer_lock);
    mem->next = ctx->buffers;
    if (ctx->buffers != NULL)
      ctx->buffers->prev = mem;
    ctx->buffers = mem;
  pthread_mutex_unlock(&ctx->buffer_lock);

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
error:
  cl_mem_delete(mem);
  mem = NULL;
  goto exit;

}

LOCAL void cl_mem_gl_delete(cl_mem mem)
{
  struct cl_gl_ext_deps *egl_funcs;
  EGLDisplay egl_display = (EGLDisplay)mem->ctx->props.egl_display;
  egl_funcs =  CL_EXTENSION_GET_FUNCS(mem->ctx, khr_gl_sharing, gl_ext_deps);
  egl_funcs->eglDestroyImageKHR_func(egl_display, mem->egl_image);
}

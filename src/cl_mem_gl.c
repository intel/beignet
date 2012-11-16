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

#include "cl_mem.h"
#include "cl_image.h"
#include "cl_context.h"
#include "cl_utils.h"
#include "cl_alloc.h"
#include "cl_device_id.h"
#include "cl_driver.h"

#include <GL/gl.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glext.h>
#include "CL/cl.h"
#include "CL/cl_intel.h"
#include "CL/cl_gl.h"
#include <assert.h>
#include <stdio.h>

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
    ret = CL_INVALID_TEXTURE;
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
    ret = CL_INVALID_TEXTURE;
    goto error;
  }

error:
  return ret;
}

static int cl_mem_process_texture(cl_context ctx,
                                  cl_mem_flags flags,
                                  GLenum texture_target,
                                  GLint miplevel,
                                  GLuint texture,
                                  GLuint dim,
                                  cl_mem mem)
{
  cl_int err = CL_SUCCESS;
  GLint tex_format;
  cl_image_format cl_format;
  /* XXX why we use vendor related structure in this layer? */
  uint32_t intel_fmt, bpp, aligned_pitch;
  int w,h;

  if ((dim == 2 && texture_target != GL_TEXTURE_2D)
      || (dim == 3 && texture_target != GL_TEXTURE_3D)) {
    err = CL_INVALID_TEXTURE;
    goto error;
  }

  if (dim == 2)
    glBindTexture(GL_TEXTURE_2D, texture);
  else if (dim == 3)
    glBindTexture(GL_TEXTURE_3D, texture);

        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MIN_FILTER,
                        GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MAG_FILTER,
                        GL_NEAREST);

  glGetTexLevelParameteriv(texture_target, miplevel, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv(texture_target, miplevel, GL_TEXTURE_HEIGHT, &h);
  glGetTexLevelParameteriv(texture_target, miplevel, GL_TEXTURE_INTERNAL_FORMAT, &tex_format);

  cl_get_clformat_from_texture(tex_format, &cl_format);

  intel_fmt = cl_image_get_intel_format(&cl_format);

  if (intel_fmt == INTEL_UNSUPPORTED_FORMAT) {
    err = CL_INVALID_TEXTURE;
    goto error;
  }

  cl_image_byte_per_pixel(&cl_format, &bpp);

  /* XXX What's the tiling? */
  aligned_pitch = w * bpp;
  mem->w = w;
  mem->h = h;
  mem->fmt = cl_format;
  mem->intel_fmt = intel_fmt;
  mem->bpp = bpp;
  mem->is_image = 1;
  mem->pitch = aligned_pitch;
  mem->tiling = 0;
 
error:
  return err;
}

LOCAL cl_mem cl_mem_new_gl_buffer(cl_context ctx,
                                  cl_mem_flags flags,
                                  GLuint buf_obj, 
                                  cl_int *errcode_ret)
{
  NOT_IMPLEMENTED;
}


LOCAL cl_mem cl_mem_new_gl_texture(cl_context ctx,
                                     cl_mem_flags flags,
                                     GLenum texture_target,
                                     GLint miplevel,
                                     GLuint texture,
                                     GLuint dim,
                                     cl_int *errcode_ret)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;

  /* Check flags consistency */
  if (UNLIKELY(flags & CL_MEM_COPY_HOST_PTR)) {
    err = CL_INVALID_ARG_VALUE;
    goto error;
  }

  TRY_ALLOC (mem, CALLOC(struct _cl_mem));
  if (cl_mem_process_texture(ctx, flags, texture_target, miplevel, texture, dim, mem) != CL_SUCCESS) {
    printf("invalid texture.\n");
    err = CL_INVALID_TEXTURE;
    goto error;
  }

  mem->bo = cl_buffer_alloc_from_texture(ctx, flags, texture_target, miplevel, texture, dim);

  if (UNLIKELY(mem->bo == NULL)) {
    err = CL_MEM_ALLOCATION_FAILURE;
    goto error;
  }

  mem->ref_n = 1;
  mem->magic = CL_MAGIC_MEM_HEADER;
  mem->flags = flags;
  mem->ctx = ctx;

  /* Append the buffer in the context buffer list */
  pthread_mutex_lock(&ctx->buffer_lock);
    mem->next = ctx->buffers;
    if (ctx->buffers != NULL)
      ctx->buffers->prev = mem;
    ctx->buffers = mem;
  pthread_mutex_unlock(&ctx->buffer_lock);
  mem->ctx = ctx;
  cl_context_add_ref(ctx);

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
error:
  cl_mem_delete(mem);
  mem = NULL;
  goto exit;

}

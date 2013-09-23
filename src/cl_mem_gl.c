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


LOCAL cl_mem
cl_mem_new_gl_buffer(cl_context ctx,
                     cl_mem_flags flags,
                     GLuint buf_obj,
                     cl_int *errcode_ret)
{
  NOT_IMPLEMENTED;
}

LOCAL cl_mem
cl_mem_new_gl_texture(cl_context ctx,
                      cl_mem_flags flags,
                      GLenum texture_target,
                      GLint miplevel,
                      GLuint texture,
                      cl_int *errcode_ret)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;
  /* Check flags consistency */
  if (UNLIKELY(flags & CL_MEM_COPY_HOST_PTR)) {
    err = CL_INVALID_ARG_VALUE;
    goto error;
  }

  mem = cl_mem_allocate(CL_MEM_GL_IMAGE_TYPE, ctx, flags, 0, 0, &err);
  if (mem == NULL || err != CL_SUCCESS)
    goto error;

  mem->bo = cl_buffer_alloc_from_texture(ctx, texture_target, miplevel,
                                         texture, cl_mem_image(mem));
  if (UNLIKELY(mem->bo == NULL)) {
    err = CL_MEM_OBJECT_ALLOCATION_FAILURE;
    goto error;
  }

  cl_mem_gl_image(mem)->target = texture_target;
  cl_mem_gl_image(mem)->miplevel = miplevel;
  cl_mem_gl_image(mem)->texture = texture;

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
error:
  cl_mem_delete(mem);
  mem = NULL;
  goto exit;

}

LOCAL void cl_mem_gl_delete(struct _cl_mem_gl_image *gl_image)
{
  if (gl_image->base.base.bo != NULL)
    cl_buffer_release_from_texture(gl_image->base.base.ctx, gl_image->target,
                                   gl_image->miplevel, gl_image->texture);
}

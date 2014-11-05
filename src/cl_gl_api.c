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
 * Author: Zhigang Gong <zhigang.gong@intel.com>
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#ifdef HAS_EGL
#include <GL/gl.h>
#endif

#include "cl_platform_id.h"
#include "cl_device_id.h" 
#include "cl_context.h"
#include "cl_command_queue.h"
#include "cl_program.h"
#include "cl_kernel.h"
#include "cl_mem.h"
#include "cl_image.h"
#include "cl_sampler.h"
#include "cl_alloc.h"
#include "cl_utils.h"

#include "CL/cl.h"
#include "CL/cl_gl.h"
#include "CL/cl_intel.h"
#include "cl_mem_gl.h"

#define CHECK_GL_CONTEXT(CTX)                             \
do {                                                      \
  if (UNLIKELY(CTX->props.gl_type == CL_GL_NOSHARE)) {    \
    err = CL_INVALID_CONTEXT;                             \
    goto error;                                           \
  }                                                       \
} while (0)

cl_mem
clCreateFromGLBuffer(cl_context    context,
                     cl_mem_flags  flags,
                     GLuint        bufobj,
                     cl_int *      errcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);
  CHECK_GL_CONTEXT (context);

  mem = cl_mem_new_gl_buffer(context, flags, bufobj, &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
}

cl_mem
clCreateFromGLTexture2D(cl_context    context,
                        cl_mem_flags  flags,
                        GLenum texture_target,
                        GLint miplevel,
                        GLuint texture,
                        cl_int *      errcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);
  CHECK_GL_CONTEXT (context);

  mem = cl_mem_new_gl_texture(context, flags, texture_target, miplevel, texture, &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
}

cl_mem
clCreateFromGLTexture3D(cl_context    context,
                        cl_mem_flags  flags,
                        GLenum texture_target,
                        GLint miplevel,
                        GLuint texture,
                        cl_int *      errcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);
  CHECK_GL_CONTEXT (context);

  mem = cl_mem_new_gl_texture(context, flags, texture_target, miplevel, texture, &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
}

cl_mem
clCreateFromGLTexture(cl_context      context,
                      cl_mem_flags    flags,
                      cl_GLenum       target,
                      cl_GLint        miplevel,
                      cl_GLuint       texture,
                      cl_int *        errcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);
  CHECK_GL_CONTEXT (context);

  mem = cl_mem_new_gl_texture(context, flags, target, miplevel, texture, &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;

}

/* XXX NULL function currently. */
cl_int clEnqueueAcquireGLObjects (cl_command_queue command_queue,
                                  cl_uint num_objects,
                                  const cl_mem *mem_objects,
                                  cl_uint num_events_in_wait_list,
                                  const cl_event *event_wait_list,
                                  cl_event *event)
{
  cl_int err = CL_SUCCESS;
  return err;
}

/* XXX NULL function currently. */
cl_int clEnqueueReleaseGLObjects (cl_command_queue command_queue,
                                  cl_uint num_objects,
                                  const cl_mem *mem_objects,
                                  cl_uint num_events_in_wait_list,
                                  const cl_event *event_wait_list,
                                  cl_event *event)
{
  cl_int err = CL_SUCCESS;
  return err;
}

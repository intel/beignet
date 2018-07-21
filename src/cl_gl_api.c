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
#ifdef HAS_GL_EGL
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
#include "cl_enqueue.h"
#include "cl_event.h"

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
  NOT_IMPLEMENTED;
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

  //We just support GL_TEXTURE_2D now.
  if(target != GL_TEXTURE_2D){
    err = CL_INVALID_VALUE;
    goto error;
  }

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
  cl_int e_status, i;
  cl_event e = NULL;
  enqueue_data *data = NULL;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (UNLIKELY(command_queue->ctx->props.gl_type == CL_GL_NOSHARE)) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if ((num_objects == 0 && mem_objects != NULL) ||
        (num_objects > 0 && mem_objects == NULL)) {
      err = CL_INVALID_VALUE;
      break;
    }

    for (i = 0; i < num_objects; i++) {
      if (!cl_mem_image(mem_objects[i])) {
        err = CL_INVALID_MEM_OBJECT;
        break;
      }
      if (!IS_GL_IMAGE(mem_objects[i])) {
        err = CL_INVALID_GL_OBJECT;
        break;
      }
    }
    if (err != CL_SUCCESS) {
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_ACQUIRE_GL_OBJECTS, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e_status = cl_event_is_ready(e);

    data = &e->exec_data;
    data->type = EnqueueReturnSuccesss;

    if (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      err = cl_event_exec(e, CL_SUBMITTED, CL_TRUE); // Submit to get the address.
      if (err != CL_SUCCESS) {
        break;
      }

      cl_command_queue_enqueue_event(command_queue, e);
    }
  } while (0);

  if (err == CL_SUCCESS && event) {
    *event = e;
  } else {
    cl_event_delete(e);
  }

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
  cl_int e_status, i;
  cl_event e = NULL;
  enqueue_data *data = NULL;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (UNLIKELY(command_queue->ctx->props.gl_type == CL_GL_NOSHARE)) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if ((num_objects == 0 && mem_objects != NULL) ||
        (num_objects > 0 && mem_objects == NULL)) {
      err = CL_INVALID_VALUE;
      break;
    }

    for (i = 0; i < num_objects; i++) {
      if (!cl_mem_image(mem_objects[i])) {
        err = CL_INVALID_MEM_OBJECT;
        break;
      }
      if (!IS_GL_IMAGE(mem_objects[i])) {
        err = CL_INVALID_GL_OBJECT;
        break;
      }
    }
    if (err != CL_SUCCESS) {
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_ACQUIRE_GL_OBJECTS, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e_status = cl_event_is_ready(e);

    data = &e->exec_data;
    data->type = EnqueueReturnSuccesss;

    if (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      err = cl_event_exec(e, CL_SUBMITTED, CL_TRUE); // Submit to get the address.
      if (err != CL_SUCCESS) {
        break;
      }

      cl_command_queue_enqueue_event(command_queue, e);
    }
  } while (0);

  if (err == CL_SUCCESS && event) {
    *event = e;
  } else {
    cl_event_delete(e);
  }

  return err;
}

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

#ifndef __CL_CONTEXT_H__
#define __CL_CONTEXT_H__

#include "cl_base_object.h"
#include "CL/cl.h"
#include "CL/cl_ext.h"

enum _cl_gl_context_type {
  CL_GL_NOSHARE,
  CL_GL_EGL_DISPLAY,
  CL_GL_GLX_DISPLAY,
  CL_GL_WGL_HDC,
  CL_GL_CGL_SHAREGROUP
};

typedef struct _cl_context_prop {
  cl_context_properties platform_id;
  enum _cl_gl_context_type gl_type;
  cl_context_properties gl_context;
  union {
    cl_context_properties egl_display;
    cl_context_properties glx_display;
    cl_context_properties wgl_hdc;
    cl_context_properties cgl_sharegroup;
  };
} _cl_context_prop;
typedef _cl_context_prop *cl_context_prop;

typedef struct _cl_context_for_device {
  cl_device_id device; /* Point to the device it belong to */
} _cl_context_for_device;
typedef _cl_context_for_device *cl_context_for_device;

typedef struct _cl_context {
  _cl_base_object base;
  cl_device_id *devices;        /* All devices belong to this context */
  cl_uint device_num;           /* Devices number of this context */
  list_head queues;             /* All command queues currently allocated */
  cl_uint queue_num;            /* All queue number currently allocated */
  cl_uint queue_modify_disable; /* Temp disable queue list change. */
  list_head mem_objects;        /* All memory object currently allocated */
  cl_uint mem_object_num;       /* All memory number currently allocated */
  list_head samplers;           /* All sampler object currently allocated */
  cl_uint sampler_num;          /* All sampler number currently allocated */
  list_head events;             /* All event object currently allocated */
  cl_uint event_num;            /* All event number currently allocated */
  list_head programs;           /* All programs currently allocated */
  cl_uint program_num;          /* All program number currently allocated */

  struct _cl_context_prop props;
  cl_context_properties *prop_user; /* a copy of user passed context properties when create context */
  cl_uint prop_len;                 /* count of the properties */

  cl_uint each_device_num;            /* Each device number */
  cl_context_for_device *each_device; /* Context content interpreted by device */

  void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *);
  /* User's callback when error occur in context */
  void *user_data; /* A pointer to user supplied data */
} _cl_context;

#define CL_OBJECT_CONTEXT_MAGIC 0x20BBCADE993134AALL
#define CL_OBJECT_IS_CONTEXT(obj) ((obj &&                                                     \
                                    ((cl_base_object)obj)->magic == CL_OBJECT_CONTEXT_MAGIC && \
                                    CL_OBJECT_GET_REF(obj) >= 1))
#define CL_OBJECT_IS_CONTEXT_EGL(obj) ((obj &&                                                     \
                                        ((cl_base_object)obj)->magic == CL_OBJECT_CONTEXT_MAGIC && \
                                        CL_OBJECT_GET_REF(obj) >= 1) &&                            \
                                       ((cl_context)obj)->props.gl_type == CL_GL_EGL_DISPLAY)

extern void cl_context_add_queue(cl_context ctx, cl_command_queue queue);
extern void cl_context_remove_queue(cl_context ctx, cl_command_queue queue);
extern void cl_context_add_mem(cl_context ctx, cl_mem mem);
extern void cl_context_remove_mem(cl_context ctx, cl_mem mem);
extern void cl_context_add_sampler(cl_context ctx, cl_sampler sampler);
extern void cl_context_remove_sampler(cl_context ctx, cl_sampler sampler);
extern void cl_context_add_event(cl_context ctx, cl_event sampler);
extern void cl_context_remove_event(cl_context ctx, cl_event sampler);
extern void cl_context_add_program(cl_context ctx, cl_program program);
extern void cl_context_remove_program(cl_context ctx, cl_program program);
extern cl_context cl_context_create(const cl_context_properties *, cl_uint, const cl_device_id *,
                                    void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                                    void *, cl_int *);
extern void cl_context_delete(cl_context);
extern void cl_context_add_ref(cl_context);
extern cl_mem cl_context_get_svm_by_ptr(cl_context ctx, const void *p, cl_bool no_offset);
#endif /* __CL_CONTEXT_H__ */

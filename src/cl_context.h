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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __CL_CONTEXT_H__
#define __CL_CONTEXT_H__

#include "CL/cl.h"
#include "CL/cl_ext.h"
#include "cl_internals.h"
#include "cl_driver.h"
#include "cl_base_object.h"

#include <stdint.h>
#include <pthread.h>

/* DRI device created at create context */
struct intel_driver;

enum _cl_gl_context_type {
  CL_GL_NOSHARE,
  CL_GL_EGL_DISPLAY,
  CL_GL_GLX_DISPLAY,
  CL_GL_WGL_HDC,
  CL_GL_CGL_SHAREGROUP
};

enum _cl_internal_ker_type {
  CL_INTERNAL_KERNEL_MIN = 0,
  CL_ENQUEUE_COPY_BUFFER_ALIGN4 = 0,
  CL_ENQUEUE_COPY_BUFFER_ALIGN16,
  CL_ENQUEUE_COPY_BUFFER_UNALIGN_SAME_OFFSET,
  CL_ENQUEUE_COPY_BUFFER_UNALIGN_DST_OFFSET,
  CL_ENQUEUE_COPY_BUFFER_UNALIGN_SRC_OFFSET,
  CL_ENQUEUE_COPY_BUFFER_RECT,
  CL_ENQUEUE_COPY_BUFFER_RECT_ALIGN4,
  CL_ENQUEUE_COPY_IMAGE_1D_TO_1D,             //copy image 1d to image 1d
  CL_ENQUEUE_COPY_IMAGE_2D_TO_2D,             //copy image 2d to image 2d
  CL_ENQUEUE_COPY_IMAGE_3D_TO_2D,             //copy image 3d to image 2d
  CL_ENQUEUE_COPY_IMAGE_2D_TO_3D,             //copy image 2d to image 3d
  CL_ENQUEUE_COPY_IMAGE_3D_TO_3D,             //copy image 3d to image 3d
  CL_ENQUEUE_COPY_IMAGE_2D_TO_2D_ARRAY,       //copy image 2d to image 2d array
  CL_ENQUEUE_COPY_IMAGE_1D_ARRAY_TO_1D_ARRAY, //copy image 1d array to image 1d array
  CL_ENQUEUE_COPY_IMAGE_2D_ARRAY_TO_2D_ARRAY, //copy image 2d array to image 2d array
  CL_ENQUEUE_COPY_IMAGE_2D_ARRAY_TO_2D,       //copy image 2d array to image 2d
  CL_ENQUEUE_COPY_IMAGE_2D_ARRAY_TO_3D,       //copy image 2d array to image 3d
  CL_ENQUEUE_COPY_IMAGE_3D_TO_2D_ARRAY,       //copy image 3d to image 2d array
  CL_ENQUEUE_COPY_IMAGE_2D_TO_BUFFER,   //copy image 2d to buffer
  CL_ENQUEUE_COPY_IMAGE_2D_TO_BUFFER_ALIGN16,
  CL_ENQUEUE_COPY_IMAGE_3D_TO_BUFFER,   //copy image 3d tobuffer
  CL_ENQUEUE_COPY_BUFFER_TO_IMAGE_2D,   //copy buffer to image 2d
  CL_ENQUEUE_COPY_BUFFER_TO_IMAGE_2D_ALIGN16,
  CL_ENQUEUE_COPY_BUFFER_TO_IMAGE_3D,   //copy buffer to image 3d
  CL_ENQUEUE_FILL_BUFFER_UNALIGN,      //fill buffer with 1 aligne pattern, pattern size=1
  CL_ENQUEUE_FILL_BUFFER_ALIGN2,       //fill buffer with 2 aligne pattern, pattern size=2
  CL_ENQUEUE_FILL_BUFFER_ALIGN4,       //fill buffer with 4 aligne pattern, pattern size=4
  CL_ENQUEUE_FILL_BUFFER_ALIGN8_8,     //fill buffer with 8 aligne pattern, pattern size=8
  CL_ENQUEUE_FILL_BUFFER_ALIGN8_16,    //fill buffer with 16 aligne pattern, pattern size=16
  CL_ENQUEUE_FILL_BUFFER_ALIGN8_32,    //fill buffer with 16 aligne pattern, pattern size=32
  CL_ENQUEUE_FILL_BUFFER_ALIGN8_64,    //fill buffer with 16 aligne pattern, pattern size=64
  CL_ENQUEUE_FILL_BUFFER_ALIGN128,     //fill buffer with 128 aligne pattern, pattern size=128
  CL_ENQUEUE_FILL_IMAGE_1D,             //fill image 1d
  CL_ENQUEUE_FILL_IMAGE_1D_ARRAY,       //fill image 1d array
  CL_ENQUEUE_FILL_IMAGE_2D,             //fill image 2d
  CL_ENQUEUE_FILL_IMAGE_2D_ARRAY,       //fill image 2d array
  CL_ENQUEUE_FILL_IMAGE_3D,             //fill image 3d
  CL_INTERNAL_KERNEL_MAX
};

struct _cl_context_prop {
  cl_context_properties platform_id;
  enum _cl_gl_context_type gl_type;
  cl_context_properties gl_context;
  union {
    cl_context_properties egl_display;
    cl_context_properties glx_display;
    cl_context_properties wgl_hdc;
    cl_context_properties cgl_sharegroup;
  };
};

#define IS_EGL_CONTEXT(ctx)  (ctx->props.gl_type == CL_GL_EGL_DISPLAY)
#define EGL_DISP(ctx)   (EGLDisplay)(ctx->props.egl_display)
#define EGL_CTX(ctx)    (EGLContext)(ctx->props.gl_context)
/* Encapsulate the whole device */
struct _cl_context {
  _cl_base_object base;
  cl_driver drv;                    /* Handles HW or simulator */
  cl_device_id* devices;            /* All devices belong to this context */
  cl_uint device_num;               /* Devices number of this context */
  list_head queues;                 /* All command queues currently allocated */
  cl_uint queue_num;                /* All queue number currently allocated */
  cl_uint queue_modify_disable;     /* Temp disable queue list change. */
  list_head mem_objects;            /* All memory object currently allocated */
  cl_uint mem_object_num;           /* All memory number currently allocated */
  list_head samplers;               /* All sampler object currently allocated */
  cl_uint sampler_num;              /* All sampler number currently allocated */
  list_head events;                 /* All event object currently allocated */
  cl_uint event_num;                /* All event number currently allocated */
  list_head programs;               /* All programs currently allocated */
  cl_uint program_num;              /* All program number currently allocated */

  cl_accelerator_intel accels;      /* All accelerator_intel object currently allocated */
  cl_program internal_prgs[CL_INTERNAL_KERNEL_MAX];
                                    /* All programs internal used, for example clEnqueuexxx api use */
  cl_kernel  internal_kernels[CL_INTERNAL_KERNEL_MAX];
                                    /* All kernels  for clenqueuexxx api, for example clEnqueuexxx api use */
  cl_program built_in_prgs;  /*all built-in kernels belongs to this program only*/
  cl_kernel  built_in_kernels[CL_INTERNAL_KERNEL_MAX];
  uint32_t ver;                     /* Gen version */
  struct _cl_context_prop props;
  cl_context_properties * prop_user; /* a copy of user passed context properties when create context */
  cl_uint                 prop_len;  /* count of the properties */
  void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *);
                                     /* User's callback when error occur in context */
  void *user_data;                   /* A pointer to user supplied data */

};

#define CL_OBJECT_CONTEXT_MAGIC 0x20BBCADE993134AALL
#define CL_OBJECT_IS_CONTEXT(obj) ((obj &&                           \
         ((cl_base_object)obj)->magic == CL_OBJECT_CONTEXT_MAGIC &&  \
         CL_OBJECT_GET_REF(obj) >= 1))

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

/* Implement OpenCL function */
extern cl_context cl_create_context(const cl_context_properties*,
                                    cl_uint,
                                    const cl_device_id*,
                                    void (CL_CALLBACK * pfn_notify) (const char*, const void*, size_t, void*),
                                    void *,
                                    cl_int*);

/* Allocate and initialize a context */
extern cl_context cl_context_new(struct _cl_context_prop *prop, cl_uint dev_num, cl_device_id* all_dev);

/* Destroy and deallocate a context */
extern void cl_context_delete(cl_context);

/* Increment the context reference counter */
extern void cl_context_add_ref(cl_context);

/* Enqueue a ND Range kernel */
extern cl_int cl_context_ND_kernel(cl_context,
                                   cl_command_queue,
                                   cl_kernel,
                                   cl_uint,
                                   const size_t*,
                                   const size_t*,
                                   const size_t*);

/* Used for allocation */
extern cl_buffer_mgr cl_context_get_bufmgr(cl_context ctx);

/* Get the internal used kernel from binary*/
extern cl_kernel cl_context_get_static_kernel_from_bin(cl_context ctx, cl_int index,
                  const char * str_kernel, size_t size, const char * str_option);

/* Get the SVM from pointer, return NULL if pointer is not from SVM */
extern cl_mem cl_context_get_svm_from_ptr(cl_context ctx, const void *p);
/* Get the mem from pointer, return NULL if pointer is not from mem*/
extern cl_mem cl_context_get_mem_from_ptr(cl_context ctx, const void *p);

#endif /* __CL_CONTEXT_H__ */


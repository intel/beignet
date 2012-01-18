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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __CL_CONTEXT_H__
#define __CL_CONTEXT_H__

#include "cl_internals.h"
#include "CL/cl.h"

#include <stdint.h>
#include <pthread.h>

/* DRI device created at create context */
struct intel_driver;

/* Encapsulate the whole device */
struct _cl_context {
  uint64_t magic;                   /* To identify it as a context */
  volatile int ref_n;               /* We reference count this object */
  struct intel_driver *intel_drv;   /* Handles the real HW */
  cl_device_id device;              /* All information about the GPU device */
  cl_command_queue queues;          /* All command queues currently allocated */
  cl_program programs;              /* All programs currently allocated */
  cl_mem buffers;                   /* All memory object currently allocated */
  cl_sampler samplers;              /* All sampler object currently allocated */
  pthread_mutex_t queue_lock;       /* To allocate and deallocate queues */
  pthread_mutex_t program_lock;     /* To allocate and deallocate programs */
  pthread_mutex_t buffer_lock;      /* To allocate and deallocate buffers */
  pthread_mutex_t sampler_lock;     /* To allocate and deallocate samplers */
  uint32_t ver;                     /* Gen version */
};

/* Implement OpenCL function */
extern cl_context cl_create_context(const cl_context_properties*,
                                    cl_uint,
                                    const cl_device_id*,
                                    void (CL_CALLBACK * pfn_notify) (const char*, const void*, size_t, void*),
                                    void *,
                                    cl_int*);

/* Allocate and initialize a context */
extern cl_context cl_context_new(void);

/* Destroy and deallocate a context */
extern void cl_context_delete(cl_context);

/* Increment the context reference counter */
extern void cl_context_add_ref(cl_context);

/* Create the command queue from the given context and device */
extern cl_command_queue cl_context_create_queue(cl_context,
                                                cl_device_id,
                                                cl_command_queue_properties,
                                                cl_int*);

/* Use for all GPU buffers */
extern struct _drm_intel_bufmgr* cl_context_get_intel_bufmgr(cl_context);

/* Enqueue a ND Range kernel */
extern cl_int cl_context_ND_kernel(cl_context,
                                   cl_command_queue,
                                   cl_kernel,
                                   cl_uint,
                                   const size_t*,
                                   const size_t*,
                                   const size_t*);

/* Used for allocation */
extern struct _drm_intel_bufmgr*
cl_context_get_intel_bufmgr(cl_context ctx);

#endif /* __CL_CONTEXT_H__ */


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

#ifndef __CL_COMMAND_QUEUE_H__
#define __CL_COMMAND_QUEUE_H__

#include "cl_internals.h"
#include "cl_driver.h"
#include "cl_thread.h"
#include "CL/cl.h"
#include <stdint.h>

struct intel_gpgpu;

/* Basically, this is a (kind-of) batch buffer */
struct _cl_command_queue {
  DEFINE_ICD(dispatch)
  uint64_t magic;                      /* To identify it as a command queue */
  volatile int ref_n;                  /* We reference count this object */
  cl_context ctx;                      /* Its parent context */
  cl_event* barrier_events;               /* Point to array of non-complete user events that block this command queue */
  cl_int    barrier_events_num;           /* Number of Non-complete user events */
  cl_int    barrier_events_size;          /* The size of array that wait_events point to */
  cl_event* wait_events;               /* Point to array of non-complete user events that block this command queue */
  cl_int    wait_events_num;           /* Number of Non-complete user events */
  cl_int    wait_events_size;          /* The size of array that wait_events point to */
  cl_event  last_event;                /* The last event in the queue, for enqueue mark used */
  cl_event  current_event;             /* Current event. */
  cl_command_queue_properties  props;  /* Queue properties */
  cl_command_queue prev, next;         /* We chain the command queues together */
  void *thread_data;                   /* Used to store thread context data */
  cl_mem perf;                         /* Where to put the perf counters */
  cl_mem fulsim_out;                   /* Fulsim will output this buffer */
};

/* The macro to get the thread specified gpgpu struct. */
#define GET_QUEUE_THREAD_GPGPU(queue) \
	cl_gpgpu gpgpu = queue ? cl_get_thread_gpgpu(queue) : NULL;  \
	if (queue) \
	  assert(gpgpu);

/* Allocate and initialize a new command queue. Also insert it in the list of
 * command queue in the associated context
 */
extern cl_command_queue cl_command_queue_new(cl_context);

/* Destroy and deallocate the command queue */
extern void cl_command_queue_delete(cl_command_queue);

/* Keep one more reference on the queue */
extern void cl_command_queue_add_ref(cl_command_queue);

/* Map ND range kernel from OCL API */
extern cl_int cl_command_queue_ND_range(cl_command_queue queue,
                                        cl_kernel ker,
                                        const uint32_t work_dim,
                                        const size_t *global_work_offset,
                                        const size_t *global_work_size,
                                        const size_t *local_work_size);

/* The memory object where to report the performance */
extern cl_int cl_command_queue_set_report_buffer(cl_command_queue, cl_mem);

/* Fulsim will dump this buffer (mostly to check its consistency */
cl_int cl_command_queue_set_fulsim_buffer(cl_command_queue, cl_mem);

/* Flush for the command queue */
extern cl_int cl_command_queue_flush(cl_command_queue);

/* Flush for the specified gpgpu */
extern void cl_command_queue_flush_gpgpu(cl_command_queue, cl_gpgpu);

/* Wait for the completion of the command queue */
extern cl_int cl_command_queue_finish(cl_command_queue);

/* Bind all the surfaces in the GPGPU state */
extern cl_int cl_command_queue_bind_surface(cl_command_queue, cl_kernel);

/* Bind all the image surfaces in the GPGPU state */
extern cl_int cl_command_queue_bind_image(cl_command_queue, cl_kernel);

/* Insert a user event to command's wait_events */
extern void cl_command_queue_insert_event(cl_command_queue, cl_event);

/* Remove a user event from command's wait_events */
extern void cl_command_queue_remove_event(cl_command_queue, cl_event);

extern void cl_command_queue_insert_barrier_event(cl_command_queue queue, cl_event event);

extern void cl_command_queue_remove_barrier_event(cl_command_queue queue, cl_event event);

#endif /* __CL_COMMAND_QUEUE_H__ */


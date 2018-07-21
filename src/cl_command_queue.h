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

#ifndef __CL_COMMAND_QUEUE_H__
#define __CL_COMMAND_QUEUE_H__

#include "cl_internals.h"
#include "cl_driver.h"
#include "cl_base_object.h"
#include "CL/cl.h"
#include <stdint.h>

struct intel_gpgpu;

typedef struct _cl_command_queue_enqueue_worker {
  cl_command_queue queue;
  pthread_t tid;
  cl_uint cookie;
  cl_bool quit;
  list_head enqueued_events;
  cl_uint in_exec_status; // Same value as CL_COMPLETE, CL_SUBMITTED ...
} _cl_command_queue_enqueue_worker;

typedef _cl_command_queue_enqueue_worker *cl_command_queue_enqueue_worker;

/* Basically, this is a (kind-of) batch buffer */
typedef struct _cl_command_queue {
  _cl_base_object base;
  _cl_command_queue_enqueue_worker worker;
  cl_context ctx;                      /* Its parent context */
  cl_device_id device;                 /* Its device */
  cl_event* barrier_events;            /* Point to array of non-complete user events that block this command queue */
  cl_int barrier_events_num;           /* Number of Non-complete user events */
  cl_int barrier_events_size;          /* The size of array that wait_events point to */
  cl_command_queue_properties props;   /* Queue properties */
  cl_mem perf;                         /* Where to put the perf counters */
  cl_uint size;                        /* Store the specified size for queueu */
} _cl_command_queue;;

#define CL_OBJECT_COMMAND_QUEUE_MAGIC 0x83650a12b79ce4efLL
#define CL_OBJECT_IS_COMMAND_QUEUE(obj) ((obj &&                           \
         ((cl_base_object)obj)->magic == CL_OBJECT_COMMAND_QUEUE_MAGIC &&  \
         CL_OBJECT_GET_REF(obj) >= 1))

/* Allocate and initialize a new command queue. Also insert it in the list of
 * command queue in the associated context */
extern cl_command_queue cl_create_command_queue(cl_context, cl_device_id,
                                                cl_command_queue_properties, cl_uint, cl_int*);
/* Destroy and deallocate the command queue */
extern void cl_command_queue_delete(cl_command_queue);
/* Keep one more reference on the queue */
extern void cl_command_queue_add_ref(cl_command_queue);
/* Map ND range kernel from OCL API */
extern cl_int cl_command_queue_ND_range(cl_command_queue queue,
                                        cl_kernel ker,
                                        cl_event event,
                                        const uint32_t work_dim,
                                        const size_t *global_wk_off,
                                        const size_t *global_dim_off,
                                        const size_t *global_wk_sz,
                                        const size_t *global_wk_sz_use,
                                        const size_t *local_wk_sz,
                                        const size_t *local_wk_sz_use);

/* The memory object where to report the performance */
extern cl_int cl_command_queue_set_report_buffer(cl_command_queue, cl_mem);
/* Flush for the specified gpgpu */
extern int cl_command_queue_flush_gpgpu(cl_gpgpu);
/* Bind all the surfaces in the GPGPU state */
extern cl_int cl_command_queue_bind_surface(cl_command_queue, cl_kernel, cl_gpgpu, uint32_t *);
/* Bind all the image surfaces in the GPGPU state */
extern cl_int cl_command_queue_bind_image(cl_command_queue, cl_kernel, cl_gpgpu, uint32_t *);
/* Bind all exec info to bind table */
extern cl_int cl_command_queue_bind_exec_info(cl_command_queue, cl_kernel, cl_gpgpu, uint32_t *);

/* Insert a user event to command's wait_events */
extern void cl_command_queue_insert_event(cl_command_queue, cl_event);
/* Remove a user event from command's wait_events */
extern void cl_command_queue_remove_event(cl_command_queue, cl_event);
extern void cl_command_queue_insert_barrier_event(cl_command_queue queue, cl_event event);
extern void cl_command_queue_remove_barrier_event(cl_command_queue queue, cl_event event);
extern void cl_command_queue_notify(cl_command_queue queue);
extern void cl_command_queue_enqueue_event(cl_command_queue queue, cl_event event);
extern cl_int cl_command_queue_init_enqueue(cl_command_queue queue);
extern void cl_command_queue_destroy_enqueue(cl_command_queue queue);
extern cl_int cl_command_queue_wait_finish(cl_command_queue queue);
extern cl_int cl_command_queue_wait_flush(cl_command_queue queue);
/* Note: Must call this function with queue's lock. */
extern cl_event *cl_command_queue_record_in_queue_events(cl_command_queue queue, cl_uint *list_num);
/* Whether it is valid to call cl_event_exec directly, instead of cl_command_queue_enqueue_event */
static inline cl_bool cl_command_queue_allow_bypass_submit(cl_command_queue queue){
  return (queue->props & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)/* if out-of-order, always */
    || list_empty(&queue->worker.enqueued_events);/* if in-order, only if empty */
}

#endif /* __CL_COMMAND_QUEUE_H__ */


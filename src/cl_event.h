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

#ifndef __CL_EVENT_H_
#define __CL_EVENT_H_

#include <semaphore.h>

#include "cl_base_object.h"
#include "cl_enqueue.h"
#include "CL/cl.h"

typedef void(CL_CALLBACK *cl_event_notify_cb)(cl_event event, cl_int event_command_exec_status, void *user_data);

typedef struct _cl_event_user_callback {
  cl_int status;                 /* The execution status */
  cl_bool executed;              /* Indicat the callback function been called or not */
  cl_event_notify_cb pfn_notify; /* Callback function */
  void *user_data;               /* Callback user data */
  list_node node;                /* Event callback list node */
} _cl_event_user_callback;

typedef _cl_event_user_callback *cl_event_user_callback;

typedef struct _cl_event {
  _cl_base_object base;
  cl_context ctx;             /* The context associated with event */
  cl_command_queue queue;     /* The command queue associated with event */
  cl_command_type event_type; /* Event type. */
  cl_bool is_barrier;         /* Is this event a barrier */
  cl_int status;              /* The execution status */
  cl_event *depend_events;    /* The events must complete before this. May disappear after they have completed - see cl_event_delete_depslist*/
  cl_uint depend_event_num;   /* The depend events number. */
  list_head callbacks;        /* The events The event callback functions */
  list_node enqueue_node;     /* The node in the enqueue list. */
  cl_ulong timestamp[5];      /* The time stamps for profiling. */
  enqueue_data exec_data; /* Context for execute this event. */
} _cl_event;

#define CL_OBJECT_EVENT_MAGIC 0x8324a9f810ebf90fLL
#define CL_OBJECT_IS_EVENT(obj) ((obj &&                           \
         ((cl_base_object)obj)->magic == CL_OBJECT_EVENT_MAGIC &&  \
         CL_OBJECT_GET_REF(obj) >= 1))

#define CL_EVENT_STATE_UNKNOWN 0x4

#define CL_EVENT_IS_MARKER(E) (E->event_type == CL_COMMAND_MARKER)
#define CL_EVENT_IS_BARRIER(E) (E->event_type == CL_COMMAND_BARRIER)
#define CL_EVENT_IS_USER(E) (E->event_type == CL_COMMAND_USER)

#define CL_EVENT_INVALID_TIMESTAMP 0xFFFFFFFFFFFFFFFF

/* Create a new event object */
extern cl_event cl_event_create(cl_context ctx, cl_command_queue queue, cl_uint num_events,
                                const cl_event *event_list, cl_command_type type, cl_int *errcode_ret);
extern cl_int cl_event_check_waitlist(cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                                      cl_event* event, cl_context ctx);
extern cl_uint cl_event_exec(cl_event event, cl_int exec_to_status, cl_bool ignore_depends);
/* 0 means ready, >0 means not ready, <0 means error. */
extern cl_int cl_event_is_ready(cl_event event);
extern cl_int cl_event_get_status(cl_event event);
extern void cl_event_add_ref(cl_event event);
extern void cl_event_delete(cl_event event);
extern cl_int cl_event_set_status(cl_event event, cl_int status);
extern cl_int cl_event_set_callback(cl_event event, cl_int exec_type,
                                    cl_event_notify_cb pfn_notify, void *user_data);
extern cl_int cl_event_wait_for_events_list(cl_uint num_events, const cl_event *event_list);
extern cl_int cl_event_wait_for_event_ready(cl_event event);
extern cl_event cl_event_create_marker_or_barrier(cl_command_queue queue, cl_uint num_events_in_wait_list,
                                                  const cl_event *event_wait_list, cl_bool is_barrier,
                                                  cl_int* error);
extern void cl_event_update_timestamp(cl_event event, cl_int status);
#endif /* __CL_EVENT_H__ */

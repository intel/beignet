/*
 * Copyright Â© 2012 Intel Corporation
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
#include "cl_event.h"
#include "cl_context.h"
#include "cl_command_queue.h"
#include "CL/cl.h"
#include <stdio.h>

cl_event
clCreateUserEvent(cl_context context,
                  cl_int *errcode_ret)
{
  cl_int err = CL_SUCCESS;
  cl_event event = NULL;

  do {
    if (!CL_OBJECT_IS_CONTEXT(context)) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    event = cl_event_create(context, NULL, 0, NULL, CL_COMMAND_USER, &err);
  } while (0);

  if (errcode_ret)
    *errcode_ret = err;
  return event;
}

cl_int
clSetUserEventStatus(cl_event event,
                     cl_int execution_status)
{
  cl_int err = CL_SUCCESS;

  if (!CL_OBJECT_IS_EVENT(event)) {
    return CL_INVALID_EVENT;
  }

  if (execution_status > CL_COMPLETE) {
    return CL_INVALID_VALUE;
  }

  err = cl_event_set_status(event, execution_status);
  return err;
}

/* 1.1 API, depreciated */
cl_int
clEnqueueMarker(cl_command_queue command_queue,
                cl_event *event)
{
  return clEnqueueMarkerWithWaitList(command_queue, 0, NULL, event);
}

cl_int
clEnqueueMarkerWithWaitList(cl_command_queue command_queue,
                            cl_uint num_events_in_wait_list,
                            const cl_event *event_wait_list,
                            cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_event e = NULL;
  cl_int e_status;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    if (event == NULL) { /* Create a anonymous event, it can not be waited on and useless. */
      return CL_SUCCESS;
    }

    e = cl_event_create_marker_or_barrier(command_queue, num_events_in_wait_list,
                                          event_wait_list, CL_FALSE, &err);
    if (err != CL_SUCCESS) {
      return err;
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    } else if (e_status == CL_COMPLETE) {
      err = cl_enqueue_handle(&e->exec_data, CL_COMPLETE);
      if (err != CL_SUCCESS) {
        break;
      }

      e->status = CL_COMPLETE;
    } else {
      cl_command_queue_enqueue_event(command_queue, e);
    }
  } while (0);

  if (event) {
    *event = e;
  } else {
    cl_event_delete(e);
  }
  return err;
}

/* 1.1 API, depreciated */
cl_int
clEnqueueBarrier(cl_command_queue command_queue)
{
  return clEnqueueBarrierWithWaitList(command_queue, 0, NULL, NULL);
}

cl_int
clEnqueueBarrierWithWaitList(cl_command_queue command_queue,
                             cl_uint num_events_in_wait_list,
                             const cl_event *event_wait_list,
                             cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_event e = NULL;
  cl_int e_status;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create_marker_or_barrier(command_queue, num_events_in_wait_list,
                                          event_wait_list, CL_TRUE, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    } else if (e_status == CL_COMPLETE) {
      err = cl_enqueue_handle(&e->exec_data, CL_COMPLETE);
      if (err != CL_SUCCESS) {
        break;
      }

      e->status = CL_COMPLETE;
      /* Already a completed barrier, no need to insert to queue. */
    } else {
      cl_command_queue_insert_barrier_event(command_queue, e);
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

cl_int
clWaitForEvents(cl_uint num_events,
                const cl_event *event_list)
{
  cl_int err = CL_SUCCESS;
  cl_uint i;

  if (num_events == 0 || event_list == NULL) {
    return CL_INVALID_VALUE;
  }

  err = cl_event_check_waitlist(num_events, event_list, NULL, NULL);
  if (err != CL_SUCCESS) {
    return err;
  }

  for (i = 0; i < num_events; i++) {
    if (cl_event_get_status(event_list[i]) < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      return err;
    }
  }

  err = cl_event_wait_for_events_list(num_events, event_list);
  return err;
}

/* 1.1 API, depreciated */
cl_int
clEnqueueWaitForEvents(cl_command_queue command_queue,
                       cl_uint num_events,
                       const cl_event *event_list)
{
  cl_int err = CL_SUCCESS;

  if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
    return CL_INVALID_COMMAND_QUEUE;
  }

  err = clWaitForEvents(num_events, event_list);
  return err;
}

cl_int
clSetEventCallback(cl_event event,
                   cl_int command_exec_callback_type,
                   void(CL_CALLBACK *pfn_notify)(cl_event, cl_int, void *),
                   void *user_data)
{
  cl_int err = CL_SUCCESS;

  if (!CL_OBJECT_IS_EVENT(event)) {
    return CL_INVALID_EVENT;
  }

  if ((pfn_notify == NULL) ||
      (command_exec_callback_type > CL_SUBMITTED) ||
      (command_exec_callback_type < CL_COMPLETE)) {
    return CL_INVALID_VALUE;
  }

  err = cl_event_set_callback(event, command_exec_callback_type, pfn_notify, user_data);
  return err;
}

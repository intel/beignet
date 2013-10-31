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
 * Author: Rong Yang <rong.r.yang@intel.com>
 */

#include "cl_event.h"
#include "cl_context.h"
#include "cl_utils.h"
#include "cl_alloc.h"
#include "cl_khr_icd.h"
#include "cl_kernel.h"
#include "cl_command_queue.h"

#include <assert.h>
#include <stdio.h>

inline cl_bool
cl_event_is_gpu_command_type(cl_command_type type)
{
  switch(type) {
    case CL_COMMAND_COPY_BUFFER:
    case CL_COMMAND_COPY_IMAGE:
    case CL_COMMAND_COPY_IMAGE_TO_BUFFER:
    case CL_COMMAND_COPY_BUFFER_TO_IMAGE:
    case CL_COMMAND_COPY_BUFFER_RECT:
    case CL_COMMAND_TASK:
    case CL_COMMAND_NDRANGE_KERNEL:
      return CL_TRUE;
    default:
      return CL_FALSE;
  }
}

cl_event cl_event_new(cl_context ctx, cl_command_queue queue, cl_command_type type, cl_bool emplict)
{
  cl_event event = NULL;

  /* Allocate and inialize the structure itself */
  TRY_ALLOC_NO_ERR (event, CALLOC(struct _cl_event));
  SET_ICD(event->dispatch)
  event->magic = CL_MAGIC_EVENT_HEADER;
  event->ref_n = 1;

  /* Append the event in the context event list */
  pthread_mutex_lock(&ctx->event_lock);
    event->next = ctx->events;
    if (ctx->events != NULL)
      ctx->events->prev = event;
    ctx->events = event;
  pthread_mutex_unlock(&ctx->event_lock);
  event->ctx   = ctx;
  cl_context_add_ref(ctx);

  /* Initialize all members and create GPGPU event object */
  event->queue = queue;
  event->type  = type;
  event->gpgpu_event = NULL;
  if(type == CL_COMMAND_USER) {
    event->status = CL_SUBMITTED;
  }
  else {
    event->status = CL_QUEUED;
    if(cl_event_is_gpu_command_type(event->type))
      event->gpgpu_event = cl_gpgpu_event_new(queue->gpgpu);
  }
  cl_event_add_ref(event);       //dec when complete
  event->user_cb = NULL;
  event->enqueue_cb = NULL;
  event->waits_head = NULL;
  event->emplict = emplict;
  if(queue && event->gpgpu_event)
    queue->last_event = event;

exit:
  return event;
error:
  cl_event_delete(event);
  event = NULL;
  goto exit;
}

void cl_event_delete(cl_event event)
{
  if (UNLIKELY(event == NULL))
    return;

  cl_event_update_status(event);

  if (atomic_dec(&event->ref_n) > 1)
    return;

  if(event->queue && event->queue->last_event == event)
    event->queue->last_event = NULL;

  /* Call all user's callback if haven't execute */
  user_callback *cb = event->user_cb;
  while(event->user_cb) {
    cb = event->user_cb;
    if(cb->executed == CL_FALSE) {
      cb->pfn_notify(event, event->status, cb->user_data);
    }
    event->user_cb = cb->next;
    cl_free(cb);
  }

  /* delete gpgpu event object */
  if(event->gpgpu_event)
    cl_gpgpu_event_delete(event->gpgpu_event);

  /* Remove it from the list */
  assert(event->ctx);
  pthread_mutex_lock(&event->ctx->event_lock);

  if (event->prev)
    event->prev->next = event->next;
  if (event->next)
    event->next->prev = event->prev;
  /* if this is the head, update head pointer ctx->events */
  if (event->ctx->events == event)
    event->ctx->events = event->next;

  pthread_mutex_unlock(&event->ctx->event_lock);
  cl_context_delete(event->ctx);

  cl_free(event);
}

void cl_event_add_ref(cl_event event)
{
  assert(event);
  atomic_inc(&event->ref_n);
}

cl_int cl_event_set_callback(cl_event event ,
                                  cl_int command_exec_callback_type,
                                  EVENT_NOTIFY pfn_notify,
                                  void* user_data)
{
  assert(event);
  assert(pfn_notify);

  cl_int err = CL_SUCCESS;
  user_callback *cb;
  TRY_ALLOC(cb, CALLOC(user_callback));

  cb->pfn_notify  = pfn_notify;
  cb->user_data   = user_data;
  cb->status      = command_exec_callback_type;
  cb->executed    = CL_FALSE;

  cb->next        = event->user_cb;
  event->user_cb  = cb;

exit:
  return err;
error:
  err = CL_OUT_OF_HOST_MEMORY;
  cl_free(cb);
  goto exit;
};

cl_int cl_event_check_waitlist(cl_uint num_events_in_wait_list,
                                    const cl_event *event_wait_list,
                                    cl_event *event,cl_context ctx)
{
  cl_int err = CL_SUCCESS;
  cl_int i;
  /* check the event_wait_list and num_events_in_wait_list */
  if((event_wait_list == NULL) &&
     (num_events_in_wait_list > 0))
    goto error;

  if ((event_wait_list != NULL) &&
      (num_events_in_wait_list == 0)){
    goto error;
  }

  /* check the event and context */
  for(i=0; i<num_events_in_wait_list; i++) {
    CHECK_EVENT(event_wait_list[i]);
    if(event_wait_list[i]->status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      goto exit;
    }
    if(event && *event == event_wait_list[i])
      goto error;
    if(event_wait_list[i]->ctx != ctx)
      goto error;
  }

exit:
  return err;
error:
  err = CL_INVALID_EVENT_WAIT_LIST;  //reset error
  goto exit;
}

cl_int cl_event_wait_events(cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                            cl_command_queue queue)
{
  cl_int i, j;

  /* Check whether wait user events */
  for(i=0; i<num_events_in_wait_list; i++) {
    if(event_wait_list[i]->status <= CL_COMPLETE)
      continue;

    /* Need wait on user event, return and do enqueue defer */
    if((event_wait_list[i]->type == CL_COMMAND_USER) ||
       (event_wait_list[i]->enqueue_cb &&
       (event_wait_list[i]->enqueue_cb->wait_user_events != NULL))){
      for(j=0; j<num_events_in_wait_list; j++)
        cl_event_add_ref(event_wait_list[j]);  //add defer enqueue's wait event reference
      return CL_ENQUEUE_EXECUTE_DEFER;
    }
  }

  if(queue && queue->barrier_index > 0) {
    return CL_ENQUEUE_EXECUTE_DEFER;
  }

  /* Non user events or all user event finished, wait all enqueue events finish */
  for(i=0; i<num_events_in_wait_list; i++) {
    if(event_wait_list[i]->status <= CL_COMPLETE)
      continue;

    //enqueue callback haven't finish, in another thread, wait
    if(event_wait_list[i]->enqueue_cb != NULL)
      return CL_ENQUEUE_EXECUTE_DEFER;
    if(event_wait_list[i]->gpgpu_event)
      cl_gpgpu_event_update_status(event_wait_list[i]->gpgpu_event, 1);
    cl_event_set_status(event_wait_list[i], CL_COMPLETE);  //Execute user's callback
  }
  return CL_ENQUEUE_EXECUTE_IMM;
}

void cl_event_new_enqueue_callback(cl_event event,
                                            enqueue_data *data,
                                            cl_uint num_events_in_wait_list,
                                            const cl_event *event_wait_list)
{
  enqueue_callback *cb, *node;
  user_event *user_events, *u_ev;
  cl_command_queue queue = event->queue;
  cl_int i;

  /* Allocate and inialize the structure itself */
  TRY_ALLOC_NO_ERR (cb, CALLOC(enqueue_callback));
  cb->num_events = num_events_in_wait_list;
  TRY_ALLOC_NO_ERR (cb->wait_list, CALLOC_ARRAY(cl_event, num_events_in_wait_list));
  for(i=0; i<num_events_in_wait_list; i++)
    cb->wait_list[i] = event_wait_list[i];
  cb->event = event;
  cb->next = NULL;
  cb->wait_user_events = NULL;

  if(queue && queue->barrier_index > 0) {
    for(i=0; i<queue->barrier_index; i++) {
      /* Insert the enqueue_callback to user event list */
      node = queue->wait_events[i]->waits_head;
      if(node == NULL)
        queue->wait_events[i]->waits_head = cb;
      else
        while((node != cb) && node->next)
          node = node->next;
        if(node == cb)   //wait on dup user event
          continue;
        node->next = cb;

      /* Insert the user event to enqueue_callback's wait_user_events */
      TRY_ALLOC_NO_ERR (u_ev, CALLOC(user_event));
      u_ev->event = queue->wait_events[i];
      u_ev->next = cb->wait_user_events;
      cb->wait_user_events = u_ev;
    }
  }

  /* Find out all user events that events in event_wait_list wait */
  for(i=0; i<num_events_in_wait_list; i++) {
    if(event_wait_list[i]->status <= CL_COMPLETE)
      continue;

    if(event_wait_list[i]->type == CL_COMMAND_USER) {
      /* Insert the enqueue_callback to user event list */
      node = event_wait_list[i]->waits_head;
      if(node == NULL)
        event_wait_list[i]->waits_head = cb;
      else {
        while((node != cb) && node->next)
          node = node->next;
        if(node == cb)   //wait on dup user event
          continue;
        node->next = cb;
      }
      /* Insert the user event to enqueue_callback's wait_user_events */
      TRY_ALLOC_NO_ERR (u_ev, CALLOC(user_event));
      u_ev->event = event_wait_list[i];
      u_ev->next = cb->wait_user_events;
      cb->wait_user_events = u_ev;
      cl_command_queue_insert_event(event->queue, event_wait_list[i]);
    } else if(event_wait_list[i]->enqueue_cb != NULL) {
      user_events = event_wait_list[i]->enqueue_cb->wait_user_events;
      while(user_events != NULL) {
        /* Insert the enqueue_callback to user event's  waits_tail */
        node = user_events->event->waits_head;
        while((node != cb) && node->next)
          node = node->next;
        if(node == cb) {  //wait on dup user event
          user_events = user_events->next;
          continue;
        }
        node->next = cb;

        /* Insert the user event to enqueue_callback's wait_user_events */
        TRY_ALLOC_NO_ERR (u_ev, CALLOC(user_event));
        u_ev->event = user_events->event;
        u_ev->next = cb->wait_user_events;
        cb->wait_user_events = u_ev;
        user_events = user_events->next;
        cl_command_queue_insert_event(event->queue, event_wait_list[i]);
      }
    }
  }
  if(data->queue != NULL && event->gpgpu_event != NULL) {
    cl_gpgpu_event_pending(data->queue->gpgpu, event->gpgpu_event);
    data->ptr = (void *)event->gpgpu_event;
  }
  cb->data = *data;
  event->enqueue_cb = cb;

exit:
  return;
error:
  if(cb) {
    while(cb->wait_user_events) {
      u_ev = cb->wait_user_events;
      cb->wait_user_events = cb->wait_user_events->next;
      cl_free(u_ev);
    }
    if(cb->wait_list)
      cl_free(cb->wait_list);
    cl_free(cb);
  }
  goto exit;
}

void cl_event_set_status(cl_event event, cl_int status)
{
  user_callback *user_cb;
  user_event    *u_ev, *u_ev_next;
  cl_int ret, i;
  cl_event evt;

  pthread_mutex_lock(&event->ctx->event_lock);
  if(status >= event->status) {
    pthread_mutex_unlock(&event->ctx->event_lock);
    return;
  }
  if(event->status <= CL_COMPLETE) {
    event->status = status;    //have done enqueue before or doing in another thread
    pthread_mutex_unlock(&event->ctx->event_lock);
    return;
  }

  if(status <= CL_COMPLETE) {
    if(event->enqueue_cb) {
      cl_enqueue_handle(&event->enqueue_cb->data);
      if(event->gpgpu_event)
        cl_gpgpu_event_update_status(event->gpgpu_event, 1);  //now set complet, need refine
      event->status = status;  //Change the event status after enqueue and befor unlock

      pthread_mutex_unlock(&event->ctx->event_lock);
      for(i=0; i<event->enqueue_cb->num_events; i++)
        cl_event_delete(event->enqueue_cb->wait_list[i]);
      pthread_mutex_lock(&event->ctx->event_lock);

      if(event->enqueue_cb->wait_list)
        cl_free(event->enqueue_cb->wait_list);
      cl_free(event->enqueue_cb);
      event->enqueue_cb = NULL;
    }
  }
  if(event->status >= status)  //maybe changed in other threads
    event->status = status;
  pthread_mutex_unlock(&event->ctx->event_lock);

  if(event->status <= CL_COMPLETE)
    cl_event_delete(event);

  /* Call user callback */
  user_cb = event->user_cb;
  while(user_cb) {
    if(user_cb->status >= status) {
      user_cb->pfn_notify(event, event->status, user_cb->user_data);
      user_cb->executed = CL_TRUE;
    }
    user_cb = user_cb->next;
  }

  if(event->type != CL_COMMAND_USER)
    return;

  /* Check all defer enqueue */
  enqueue_callback *cb, *enqueue_cb = event->waits_head;
  while(enqueue_cb) {
    /* Remove this user event in enqueue_cb */
    while(enqueue_cb->wait_user_events &&
          enqueue_cb->wait_user_events->event == event) {
      u_ev = enqueue_cb->wait_user_events;
      enqueue_cb->wait_user_events = enqueue_cb->wait_user_events->next;
      cl_free(u_ev);
    }

    u_ev = enqueue_cb->wait_user_events;
    while(u_ev) {
      u_ev_next = u_ev->next;
      if(u_ev_next && u_ev_next->event == event) {
        u_ev->next = u_ev_next->next;
        cl_free(u_ev_next);
      } else
        u_ev->next = u_ev_next;
    }

    /* Still wait on other user events */
    if(enqueue_cb->wait_user_events != NULL) {
      enqueue_cb = enqueue_cb->next;
      continue;
    }

    //remove user event frome enqueue_cb's ctx
    cl_command_queue_remove_event(enqueue_cb->event->queue, event);

    /* All user events complete, now wait enqueue events */
    ret = cl_event_wait_events(enqueue_cb->num_events, enqueue_cb->wait_list,
                               enqueue_cb->event->queue);
    assert(ret != CL_ENQUEUE_EXECUTE_DEFER);

    cb = enqueue_cb;
    enqueue_cb = enqueue_cb->next;

    /* Call the pending operation */
    evt = cb->event;
    cl_event_set_status(cb->event, CL_COMPLETE);
    if(evt->emplict == CL_FALSE) {
      cl_event_delete(evt);
    }
  }
  event->waits_head = NULL;
}

void cl_event_update_status(cl_event event)
{
  if(event->status <= CL_COMPLETE)
    return;
  if((event->gpgpu_event) &&
     (cl_gpgpu_event_update_status(event->gpgpu_event, 0) == command_complete))
    cl_event_set_status(event, CL_COMPLETE);
}

cl_int cl_event_marker(cl_command_queue queue, cl_event* event)
{
  enqueue_data data;

  *event = cl_event_new(queue->ctx, queue, CL_COMMAND_MARKER, CL_TRUE);
  if(event == NULL)
    return CL_OUT_OF_HOST_MEMORY;

  //if wait_events_num>0, the marker event need wait queue->wait_events
  if(queue->wait_events_num > 0) {
    data.type = EnqueueMarker;
    cl_event_new_enqueue_callback(*event, &data, queue->wait_events_num, queue->wait_events);
    return CL_SUCCESS;
  }

  if(queue->last_event && queue->last_event->gpgpu_event) {
    cl_gpgpu_event_update_status(queue->last_event->gpgpu_event, 1);
  }

  cl_event_set_status(*event, CL_COMPLETE);
  return CL_SUCCESS;
}

cl_int cl_event_profiling(cl_event event, cl_profiling_info param_name, cl_ulong *ret_val)
{
  if (!event->gpgpu_event) {
    /* Some event like read buffer do not need GPU involved, so
       we just return all the profiling to 0 now. */
    *ret_val = 0;
    return CL_SUCCESS;
  }

  if(param_name == CL_PROFILING_COMMAND_START ||
     param_name == CL_PROFILING_COMMAND_QUEUED ||
     param_name == CL_PROFILING_COMMAND_SUBMIT) {
    cl_gpgpu_event_get_timestamp(event->gpgpu_event, 0, ret_val);
    return CL_SUCCESS;
  } else if (param_name == CL_PROFILING_COMMAND_END) {
    cl_gpgpu_event_get_timestamp(event->gpgpu_event, 1, ret_val);
    return CL_SUCCESS;
  } else {
    return CL_INVALID_VALUE;
  }
}

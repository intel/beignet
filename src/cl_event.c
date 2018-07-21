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

#include "cl_event.h"
#include "cl_context.h"
#include "cl_command_queue.h"
#include "cl_alloc.h"
#include <string.h>
#include <stdio.h>

// TODO: Need to move it to some device related file later.
static void
cl_event_update_timestamp_gen(cl_event event, cl_int status)
{
  cl_ulong ts = 0;

  if ((event->exec_data.type == EnqueueCopyBufferRect) ||
      (event->exec_data.type == EnqueueCopyBuffer) ||
      (event->exec_data.type == EnqueueCopyImage) ||
      (event->exec_data.type == EnqueueCopyBufferToImage) ||
      (event->exec_data.type == EnqueueCopyImageToBuffer) ||
      (event->exec_data.type == EnqueueNDRangeKernel) ||
      (event->exec_data.type == EnqueueFillBuffer) ||
      (event->exec_data.type == EnqueueFillImage)) {

    if (status == CL_QUEUED || status == CL_SUBMITTED) {
      cl_gpgpu_event_get_gpu_cur_timestamp(event->queue->ctx->drv, &ts);

      if (ts == CL_EVENT_INVALID_TIMESTAMP)
        ts++;
      event->timestamp[CL_QUEUED - status] = ts;
      return;
    } else if (status == CL_RUNNING) {
      assert(event->exec_data.gpgpu);
      return; // Wait for the event complete and get run and complete then.
    } else {
      assert(event->exec_data.gpgpu);
      cl_gpgpu_event_get_exec_timestamp(event->exec_data.gpgpu, 0, &ts);
      if (ts == CL_EVENT_INVALID_TIMESTAMP)
        ts++;
      event->timestamp[2] = ts;
      cl_gpgpu_event_get_exec_timestamp(event->exec_data.gpgpu, 1, &ts);
      if (ts == CL_EVENT_INVALID_TIMESTAMP)
        ts++;
      event->timestamp[3] = ts;

      /* Set the submit time the same as running time if it is later. */
      if (event->timestamp[1] > event->timestamp[2] ||
          event->timestamp[2] - event->timestamp[1] > 0x0FFFFFFFFFF /*Overflowed */)
        event->timestamp[1] = event->timestamp[2];

      return;
    }
  } else {
    cl_gpgpu_event_get_gpu_cur_timestamp(event->queue->ctx->drv, &ts);
    if (ts == CL_EVENT_INVALID_TIMESTAMP)
      ts++;
    event->timestamp[CL_QUEUED - status] = ts;
    return;
  }
}

LOCAL void
cl_event_update_timestamp(cl_event event, cl_int state)
{
  int i;
  cl_bool re_cal = CL_FALSE;
  cl_ulong ts[4];

  assert(state >= CL_COMPLETE || state <= CL_QUEUED);

  if (event->event_type == CL_COMMAND_USER)
    return;

  assert(event->queue);
  if ((event->queue->props & CL_QUEUE_PROFILING_ENABLE) == 0)
    return;

  /* Should not record the timestamp twice. */
  assert(event->timestamp[CL_QUEUED - state] == CL_EVENT_INVALID_TIMESTAMP);
  cl_event_update_timestamp_gen(event, state);

  if (state == CL_COMPLETE) {
    // TODO: Need to set the CL_PROFILING_COMMAND_COMPLETE when enable child enqueue.
    // Just a duplicate of event complete time now.
    event->timestamp[4] = event->timestamp[3];

    /* If timestamp overflow, set queued time to 0 and re-calculate. */
    for (i = 0; i < 4; i++) {
      if (event->timestamp[i + 1] < event->timestamp[i]) {
        re_cal = CL_TRUE;
        break;
      }
    }

    if (re_cal) {
      for (i = 3; i >= 0; i--) {
        if (event->timestamp[i + 1] < event->timestamp[i]) { //overflow
          ts[i] = event->timestamp[i + 1] + (CL_EVENT_INVALID_TIMESTAMP - event->timestamp[i]);
        } else {
          ts[i] = event->timestamp[i + 1] - event->timestamp[i];
        }
      }

      event->timestamp[0] = 0;
      for (i = 1; i < 5; i++) {
        event->timestamp[i] = event->timestamp[i - 1] + ts[i - 1];
      }
    }
  }
}

LOCAL void
cl_event_add_ref(cl_event event)
{
  assert(event);
  CL_OBJECT_INC_REF(event);
}

LOCAL cl_int
cl_event_get_status(cl_event event)
{
  cl_int ret;

  assert(event);
  CL_OBJECT_LOCK(event);
  ret = event->status;
  CL_OBJECT_UNLOCK(event);
  return ret;
}

static cl_event
cl_event_new(cl_context ctx, cl_command_queue queue, cl_command_type type,
             cl_uint num_events, cl_event *event_list)
{
  int i;
  cl_event e = cl_calloc(1, sizeof(_cl_event));
  if (e == NULL)
    return NULL;

  CL_OBJECT_INIT_BASE(e, CL_OBJECT_EVENT_MAGIC);

  /* Append the event in the context event list */
  cl_context_add_event(ctx, e);
  e->queue = queue;

  list_init(&e->callbacks);
  list_node_init(&e->enqueue_node);

  assert(type >= CL_COMMAND_NDRANGE_KERNEL && type <= CL_COMMAND_SVM_UNMAP);
  e->event_type = type;
  if (type == CL_COMMAND_USER) {
    e->status = CL_SUBMITTED;
  } else {
    e->status = CL_EVENT_STATE_UNKNOWN;
  }

  if (type == CL_COMMAND_USER) {
    assert(queue == NULL);
  }

  e->depend_events = event_list;
  e->depend_event_num = num_events;
  for (i = 0; i < 4; i++) {
    e->timestamp[i] = CL_EVENT_INVALID_TIMESTAMP;
  }

  return e;
}

/* This exists to prevent long chains of events from filling up memory (https://bugs.launchpad.net/ubuntu/+source/beignet/+bug/1354086).  Call only after the dependencies are complete, or failed and marked as such in this event's status, or when this event is being destroyed */
LOCAL void
cl_event_delete_depslist(cl_event event)
{
  CL_OBJECT_LOCK(event);
  cl_event *old_depend_events = event->depend_events;
  int depend_count = event->depend_event_num;
  event->depend_event_num = 0;
  event->depend_events = NULL;
  CL_OBJECT_UNLOCK(event);
  if (old_depend_events) {
    assert(depend_count);
    for (int i = 0; i < depend_count; i++) {
      cl_event_delete(old_depend_events[i]);
    }
    cl_free(old_depend_events);
  }
}

LOCAL void
cl_event_delete(cl_event event)
{
  int i;
  cl_event_user_callback cb;

  if (UNLIKELY(event == NULL))
    return;

  if (CL_OBJECT_DEC_REF(event) > 1)
    return;

  cl_enqueue_delete(&event->exec_data);

  assert(list_node_out_of_list(&event->enqueue_node));

  cl_event_delete_depslist(event);

  /* Free all the callbacks. Last ref, no need to lock. */
  while (!list_empty(&event->callbacks)) {
    cb = list_entry(event->callbacks.head_node.n, _cl_event_user_callback, node);
    list_node_del(&cb->node);
    cl_free(cb);
  }

  /* Remove it from the list */
  assert(event->ctx);
  cl_context_remove_event(event->ctx, event);

  CL_OBJECT_DESTROY_BASE(event);
  cl_free(event);
}

LOCAL cl_event
cl_event_create(cl_context ctx, cl_command_queue queue, cl_uint num_events,
                const cl_event *event_list, cl_command_type type, cl_int *errcode_ret)
{
  cl_event e = NULL;
  cl_event *depend_events = NULL;
  cl_int err = CL_SUCCESS;
  cl_uint total_events = 0;
  int i;

  assert(ctx);

  do {
    if (event_list)
      assert(num_events);

    if (queue == NULL) {
      assert(type == CL_COMMAND_USER);
      assert(event_list == NULL);
      assert(num_events == 0);

      e = cl_event_new(ctx, queue, type, 0, NULL);
      if (e == NULL) {
        err = CL_OUT_OF_HOST_MEMORY;
        break;
      }
    } else {
      CL_OBJECT_LOCK(queue);
      total_events = queue->barrier_events_num + num_events;

      if (total_events) {
        depend_events = cl_calloc(total_events, sizeof(cl_event));
        if (depend_events == NULL) {
          CL_OBJECT_UNLOCK(queue);
          err = CL_OUT_OF_HOST_MEMORY;
          break;
        }
      }

      /* Add all the barrier events as depend events. */
      for (i = 0; i < queue->barrier_events_num; i++) {
        assert(CL_EVENT_IS_BARRIER(queue->barrier_events[i]));
        cl_event_add_ref(queue->barrier_events[i]);
        depend_events[num_events + i] = queue->barrier_events[i];
      }

      CL_OBJECT_UNLOCK(queue);

      for (i = 0; i < num_events; i++) {
        assert(event_list && event_list[i]);
        assert(event_list[i]->ctx == ctx);
        assert(CL_OBJECT_IS_EVENT(event_list[i]));
        cl_event_add_ref(event_list[i]);
        depend_events[i] = event_list[i];
      }

      if (depend_events)
        assert(total_events);

      e = cl_event_new(ctx, queue, type, total_events, depend_events);
      if (e == NULL) {
        err = CL_OUT_OF_HOST_MEMORY;
        break;
      }
      depend_events = NULL;
    }
  } while (0);

  if (err != CL_SUCCESS) {
    if (depend_events) {
      for (i = 0; i < total_events; i++) {
        cl_event_delete(depend_events[i]);
      }
      cl_free(depend_events);
    }

    // if set depend_events, must succeed.
    assert(e->depend_events == NULL);
    cl_event_delete(e);
  }

  if (errcode_ret)
    *errcode_ret = err;

  return e;
}

LOCAL cl_int
cl_event_set_callback(cl_event event, cl_int exec_type, cl_event_notify_cb pfn_notify, void *user_data)
{
  cl_int err = CL_SUCCESS;
  cl_event_user_callback cb;
  cl_bool exec_imm = CL_FALSE;

  assert(event);
  assert(pfn_notify);

  do {
    cb = cl_calloc(1, sizeof(_cl_event_user_callback));
    if (cb == NULL) {
      err = CL_OUT_OF_HOST_MEMORY;
      break;
    }

    list_node_init(&cb->node);
    cb->pfn_notify = pfn_notify;
    cb->user_data = user_data;
    cb->status = exec_type;
    cb->executed = CL_FALSE;

    CL_OBJECT_LOCK(event);
    if (event->status > exec_type) {
      list_add_tail(&event->callbacks, &cb->node);
      cb = NULL;
    } else {
      /* The state has already OK, call it immediately. */
      exec_imm = CL_TRUE;
    }
    CL_OBJECT_UNLOCK(event);

    if (exec_imm) {
      cb->pfn_notify(event, event->status, cb->user_data);
    }

  } while (0);

  if (cb)
    cl_free(cb);

  return err;
}

LOCAL cl_int
cl_event_set_status(cl_event event, cl_int status)
{
  list_head tmp_callbacks;
  list_node *n;
  list_node *pos;
  cl_bool notify_queue = CL_FALSE;
  cl_event_user_callback cb;

  assert(event);

  CL_OBJECT_LOCK(event);
  if (event->status <= CL_COMPLETE) { // Already set to error or completed
    CL_OBJECT_UNLOCK(event);
    return CL_INVALID_OPERATION;
  }

  if (CL_EVENT_IS_USER(event)) {
    assert(event->status != CL_RUNNING && event->status != CL_QUEUED);
  } else {
    assert(event->queue); // Must belong to some queue.
  }

  if (status >= event->status) { // Should never go back.
    CL_OBJECT_UNLOCK(event);
    return CL_INVALID_OPERATION;
  }

  event->status = status;

  /* Call all the callbacks. */
  if (!list_empty(&event->callbacks)) {
    do {
      status = event->status;
      list_init(&tmp_callbacks);
      list_move(&event->callbacks, &tmp_callbacks);
      /* Call all the callbacks without lock. */
      CL_OBJECT_UNLOCK(event);

      list_for_each_safe(pos, n, &tmp_callbacks)
      {
        cb = list_entry(pos, _cl_event_user_callback, node);

        assert(cb->executed == CL_FALSE);

        if (cb->status < status)
          continue;

        list_node_del(&cb->node);
        cb->executed = CL_TRUE;
        cb->pfn_notify(event, status, cb->user_data);
        cl_free(cb);
      }

      CL_OBJECT_LOCK(event);

      // Set back the uncalled callbacks.
      list_merge(&event->callbacks, &tmp_callbacks);

      /* Status may changed because we unlock. need to check again. */
    } while (status != event->status);
  }

  /*  Wakeup all the waiter for status change. */
  CL_OBJECT_NOTIFY_COND(event);

  if (event->status <= CL_COMPLETE) {
    notify_queue = CL_TRUE;
  }

  CL_OBJECT_UNLOCK(event);

  /* Need to notify all the command queue within the same context. */
  if (notify_queue) {
    cl_command_queue queue = NULL;

    /*First, we need to remove it from queue's barrier list. */
    if (CL_EVENT_IS_BARRIER(event)) {
      assert(event->queue);
      cl_command_queue_remove_barrier_event(event->queue, event);
    }

    /* Then, notify all the queues within the same context. */
    CL_OBJECT_LOCK(event->ctx);
    /* Disable remove and add queue to the context temporary. We need to
       make sure all the queues in the context currently are valid. */
    event->ctx->queue_modify_disable++;
    CL_OBJECT_UNLOCK(event->ctx);
    list_for_each(pos, &event->ctx->queues)
    {
      queue = (cl_command_queue)(list_entry(pos, _cl_base_object, node));
      assert(queue != NULL);
      cl_command_queue_notify(queue);
    }
    CL_OBJECT_LOCK(event->ctx);
    /* Disable remove and add queue to the context temporary. We need to
       make sure all the queues in the context currently are valid. */
    event->ctx->queue_modify_disable--;
    CL_OBJECT_NOTIFY_COND(event->ctx);
    CL_OBJECT_UNLOCK(event->ctx);
  }

  return CL_SUCCESS;
}

LOCAL cl_int
cl_event_wait_for_event_ready(const cl_event event)
{
  assert(CL_OBJECT_IS_EVENT(event));
  return cl_event_wait_for_events_list(event->depend_event_num, event->depend_events);
}

LOCAL cl_int
cl_event_wait_for_events_list(cl_uint num_events, const cl_event *event_list)
{
  int i;
  cl_event e;
  cl_int ret = CL_SUCCESS;

  for (i = 0; i < num_events; i++) {
    e = event_list[i];
    assert(e);
    assert(CL_OBJECT_IS_EVENT(e));

    CL_OBJECT_LOCK(e);
    while (e->status > CL_COMPLETE) {
      CL_OBJECT_WAIT_ON_COND(e);
    }

    assert(e->status <= CL_COMPLETE);
    /* Iff some error happened, return the error. */
    if (e->status < CL_COMPLETE) {
      ret = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
    }
    CL_OBJECT_UNLOCK(e);
  }

  return ret;
}

LOCAL cl_int
cl_event_check_waitlist(cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
                        cl_event *event, cl_context ctx)
{
  cl_int err = CL_SUCCESS;
  cl_int i;

  do {
    /* check the event_wait_list and num_events_in_wait_list */
    if ((event_wait_list == NULL) && (num_events_in_wait_list > 0)) {
      err = CL_INVALID_EVENT_WAIT_LIST;
      break;
    }

    if ((event_wait_list != NULL) && (num_events_in_wait_list == 0)) {
      err = CL_INVALID_EVENT_WAIT_LIST;
      break;
    }

    /* check the event and context */
    for (i = 0; i < num_events_in_wait_list; i++) {
      if (!CL_OBJECT_IS_EVENT(event_wait_list[i])) {
        err = CL_INVALID_EVENT_WAIT_LIST;
        break;
      }

      if (event == event_wait_list + i) { /* Pointer of element of the wait list */
        err = CL_INVALID_EVENT_WAIT_LIST;
        break;
      }

      /* check all belong to same context. */
      if (ctx == NULL) {
        ctx = event_wait_list[i]->ctx;
      }
      if (event_wait_list[i]->ctx != ctx) {
        err = CL_INVALID_CONTEXT;
        break;
      }
    }

    if (err != CL_SUCCESS)
      break;

  } while (0);

  return err;
}

/* When we call this function, all the events it depends
   on should already be ready, unless ignore_depends is set. */
LOCAL cl_uint
cl_event_exec(cl_event event, cl_int exec_to_status, cl_bool ignore_depends)
{
  /* We are MT safe here, no one should call this
     at the same time. No need to lock */
  cl_int ret = CL_SUCCESS;
  cl_int cur_status = cl_event_get_status(event);
  cl_int depend_status;
  cl_int s;

  assert(exec_to_status >= CL_COMPLETE);
  assert(exec_to_status <= CL_QUEUED);
  if (cur_status < CL_COMPLETE) {
    return cur_status;
  }

  depend_status = cl_event_is_ready(event);
  assert(depend_status <= CL_COMPLETE || ignore_depends || exec_to_status == CL_QUEUED);
  if (depend_status < CL_COMPLETE) { // Error happend, cancel exec.
    ret = cl_event_set_status(event, depend_status);
    cl_event_delete_depslist(event);
    return depend_status;
  }
  if (depend_status == CL_COMPLETE) { // Avoid memory leak
    cl_event_delete_depslist(event);
  }

  if (cur_status <= exec_to_status) {
    return ret;
  }

  /* Exec to the target status. */
  for (s = cur_status - 1; s >= exec_to_status; s--) {
    assert(s >= CL_COMPLETE);
    ret = cl_enqueue_handle(&event->exec_data, s);

    if (ret != CL_SUCCESS) {
      assert(ret < 0);
      DEBUGP(DL_WARNING, "Exec event %p error, type is %d, error status is %d",
             event, event->event_type, ret);
      ret = cl_event_set_status(event, ret);
      assert(ret == CL_SUCCESS);
      return ret; // Failed and we never do further.
    } else {
      assert(!CL_EVENT_IS_USER(event));
      if ((event->queue->props & CL_QUEUE_PROFILING_ENABLE) != 0) {
        /* record the timestamp before actually doing something. */
        cl_event_update_timestamp(event, s);
      }

      ret = cl_event_set_status(event, s);
      assert(ret == CL_SUCCESS);
    }
  }

  return ret;
}

/* 0 means ready, >0 means not ready, <0 means error. */
LOCAL cl_int
cl_event_is_ready(cl_event event)
{
  int i;
  int status;
  int ret_status = CL_COMPLETE;

  for (i = 0; i < event->depend_event_num; i++) {
    status = cl_event_get_status(event->depend_events[i]);

    if (status > CL_COMPLETE) { // Find some not ready, just OK
      return status;
    }

    if (status < CL_COMPLETE) { // Record some error.
      ret_status = status;
    }
  }

  return ret_status;
}

LOCAL cl_event
cl_event_create_marker_or_barrier(cl_command_queue queue, cl_uint num_events_in_wait_list,
                                  const cl_event *event_wait_list, cl_bool is_barrier, cl_int *error)
{
  cl_event e = NULL;
  cl_int err = CL_SUCCESS;
  cl_command_type type = CL_COMMAND_MARKER;
  enqueue_type eq_type = EnqueueMarker;

  if (is_barrier) {
    type = CL_COMMAND_BARRIER;
    eq_type = EnqueueBarrier;
  }

  if (event_wait_list) {
    assert(num_events_in_wait_list > 0);

    e = cl_event_create(queue->ctx, queue, num_events_in_wait_list,
                        event_wait_list, type, &err);
    if (err != CL_SUCCESS) {
      *error = err;
      return NULL;
    }
  } else { /* The marker depends on all events in the queue now. */
    cl_command_queue_enqueue_worker worker = &queue->worker;
    cl_uint i;
    cl_uint event_num;
    cl_event *depend_events;

    CL_OBJECT_LOCK(queue);

    /* First, wait for the command queue retire all in executing event. */
    while (1) {
      if (worker->quit) { // already destroy the queue?
        CL_OBJECT_UNLOCK(queue);
        *error = CL_INVALID_COMMAND_QUEUE;
        return NULL;
      }

      if (worker->in_exec_status != CL_COMPLETE) {
        CL_OBJECT_WAIT_ON_COND(queue);
        continue;
      }

      break;
    }

    event_num = 0;
    depend_events = NULL;
    if (!list_empty(&worker->enqueued_events)) {
      depend_events = cl_command_queue_record_in_queue_events(queue, &event_num);
    }

    CL_OBJECT_UNLOCK(queue);

    e = cl_event_create(queue->ctx, queue, event_num, depend_events, type, &err);

    for (i = 0; i < event_num; i++) { //unref the temp
      cl_event_delete(depend_events[i]);
    }
    if (depend_events)
      cl_free(depend_events);

    if (err != CL_SUCCESS) {
      *error = err;
      return NULL;
    }
  }

  e->exec_data.type = eq_type;
  *error = CL_SUCCESS;
  return e;
}

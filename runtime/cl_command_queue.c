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
 * Author: He Junyan <junyan.he@intel.com>
 */

#include "cl_command_queue.h"
#include "cl_alloc.h"
#include "cl_device_id.h"
#include "cl_event.h"

static cl_command_queue
cl_command_queue_new(cl_context ctx)
{
  cl_command_queue queue = NULL;

  assert(ctx);
  queue = CL_CALLOC(1, sizeof(_cl_command_queue));
  if (queue == NULL)
    return NULL;

  CL_OBJECT_INIT_BASE(queue, CL_OBJECT_COMMAND_QUEUE_MAGIC);
  if (cl_command_queue_init_enqueue(queue) != CL_SUCCESS) {
    CL_FREE(queue);
    return NULL;
  }

  /* Append the command queue in the list */
  cl_context_add_queue(ctx, queue);
  return queue;
}

LOCAL cl_command_queue
cl_command_queue_create(cl_context ctx, cl_device_id device, cl_command_queue_properties properties,
                        cl_uint queue_size, cl_int *errcode_ret)
{
  cl_command_queue queue = cl_command_queue_new(ctx);
  if (queue == NULL) {
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
  }

  queue->props = properties;
  queue->device = device;
  queue->size = queue_size;

  *errcode_ret = device->api.command_queue_create(device, queue);
  if (*errcode_ret != CL_SUCCESS) {
    cl_command_queue_delete(queue);
    return NULL;
  }

  return queue;
}

LOCAL void
cl_command_queue_delete(cl_command_queue queue)
{
  assert(queue);
  if (CL_OBJECT_DEC_REF(queue) > 1)
    return;

  cl_command_queue_destroy_enqueue(queue);

  queue->device->api.command_queue_create(queue->device, queue);

  cl_context_remove_queue(queue->ctx, queue);
  if (queue->barrier_events) {
    CL_FREE(queue->barrier_events);
  }
  CL_OBJECT_DESTROY_BASE(queue);
  CL_FREE(queue);
}

LOCAL void
cl_command_queue_add_ref(cl_command_queue queue)
{
  CL_OBJECT_INC_REF(queue);
}

LOCAL void
cl_command_queue_insert_barrier_event(cl_command_queue queue, cl_event event)
{
  cl_int i = 0;

  cl_event_add_ref(event);

  assert(queue != NULL);
  CL_OBJECT_LOCK(queue);

  if (queue->barrier_events == NULL) {
    queue->barrier_events_size = 4;
    queue->barrier_events = CL_CALLOC(queue->barrier_events_size, sizeof(cl_event));
    assert(queue->barrier_events);
  }

  for (i = 0; i < queue->barrier_events_num; i++) {
    assert(queue->barrier_events[i] != event);
  }

  if (queue->barrier_events_num < queue->barrier_events_size) {
    queue->barrier_events[queue->barrier_events_num++] = event;
    CL_OBJECT_UNLOCK(queue);
    return;
  }

  /* Array is full, double expand. */
  queue->barrier_events_size *= 2;
  queue->barrier_events = CL_REALLOC(queue->barrier_events,
                                     queue->barrier_events_size * sizeof(cl_event));
  assert(queue->barrier_events);

  queue->barrier_events[queue->barrier_events_num++] = event;
  CL_OBJECT_UNLOCK(queue);
  return;
}

LOCAL void
cl_command_queue_remove_barrier_event(cl_command_queue queue, cl_event event)
{
  cl_int i = 0;
  assert(queue != NULL);

  CL_OBJECT_LOCK(queue);

  assert(queue->barrier_events_num > 0);
  assert(queue->barrier_events);

  for (i = 0; i < queue->barrier_events_num; i++) {
    if (queue->barrier_events[i] == event)
      break;
  }
  assert(i < queue->barrier_events_num); // Must find it.

  if (i == queue->barrier_events_num - 1) { // The last one.
    queue->barrier_events[i] = NULL;
  } else {
    for (; i < queue->barrier_events_num - 1; i++) { // Move forward.
      queue->barrier_events[i] = queue->barrier_events[i + 1];
    }
  }
  queue->barrier_events_num -= 1;
  CL_OBJECT_UNLOCK(queue);

  cl_event_delete(event);
}

static void *
worker_thread_function(void *Arg)
{
  cl_command_queue_enqueue_worker worker = (cl_command_queue_enqueue_worker)Arg;
  cl_command_queue queue = worker->queue;
  cl_event e;
  cl_uint cookie = -1;
  list_node *pos;
  list_node *n;
  list_head ready_list;
  cl_int exec_status;

  CL_OBJECT_LOCK(queue);

  while (1) {
    /* Must have locked here. */

    if (worker->quit == CL_TRUE) {
      CL_OBJECT_UNLOCK(queue);
      return NULL;
    }

    if (list_empty(&worker->enqueued_events)) {
      CL_OBJECT_WAIT_ON_COND(queue);
      continue;
    }

    /* The cookie will change when event status change or something happend to
       this command queue. If we already checked the event list and do not find
       anything to exec, we need to wait the cookie update, to avoid loop for ever. */
    if (cookie == worker->cookie) {
      CL_OBJECT_WAIT_ON_COND(queue);
      continue;
    }

    /* Here we hold lock to check event status, to avoid missing the status notify*/
    list_init(&ready_list);
    list_for_each_safe(pos, n, &worker->enqueued_events)
    {
      e = list_entry(pos, _cl_event, enqueue_node);
      if (cl_event_is_ready(e) <= CL_COMPLETE) {
        list_node_del(&e->enqueue_node);
        list_add_tail(&ready_list, &e->enqueue_node);
      }
    }

    if (list_empty(&ready_list)) { /* Nothing to do, just wait. */
      cookie = worker->cookie;
      continue;
    }

    /* Notify waiters, we change the event list. */
    CL_OBJECT_NOTIFY_COND(queue);

    worker->in_exec_status = CL_QUEUED;
    CL_OBJECT_UNLOCK(queue);

    /* Do the really job without lock.*/
    exec_status = CL_SUBMITTED;
    list_for_each_safe(pos, n, &ready_list)
    {
      e = list_entry(pos, _cl_event, enqueue_node);
      cl_event_exec(e, exec_status, CL_FALSE);
    }

    /* Notify all waiting for flush. */
    CL_OBJECT_LOCK(queue);
    worker->in_exec_status = CL_SUBMITTED;
    CL_OBJECT_NOTIFY_COND(queue);
    CL_OBJECT_UNLOCK(queue);

    list_for_each_safe(pos, n, &ready_list)
    {
      e = list_entry(pos, _cl_event, enqueue_node);
      cl_event_exec(e, CL_COMPLETE, CL_FALSE);
    }

    /* Clear and delete all the events. */
    list_for_each_safe(pos, n, &ready_list)
    {
      e = list_entry(pos, _cl_event, enqueue_node);
      list_node_del(&e->enqueue_node);
      cl_event_delete(e);
    }

    CL_OBJECT_LOCK(queue);
    worker->in_exec_status = CL_COMPLETE;

    /* Notify finish waiters, we have done all the ready event. */
    CL_OBJECT_NOTIFY_COND(queue);
  }
}

LOCAL void
cl_command_queue_notify(cl_command_queue queue)
{
  if (CL_OBJECT_GET_REF(queue) < 1) {
    return;
  }

  assert(queue && (((cl_base_object)queue)->magic == CL_OBJECT_COMMAND_QUEUE_MAGIC));
  CL_OBJECT_LOCK(queue);
  queue->worker.cookie++;
  CL_OBJECT_NOTIFY_COND(queue);
  CL_OBJECT_UNLOCK(queue);
}

LOCAL void
cl_command_queue_enqueue_event(cl_command_queue queue, cl_event event)
{
  CL_OBJECT_INC_REF(event);
  assert(CL_OBJECT_IS_COMMAND_QUEUE(queue));
  CL_OBJECT_LOCK(queue);
  assert(queue->worker.quit == CL_FALSE);
  assert(list_node_out_of_list(&event->enqueue_node));
  list_add_tail(&queue->worker.enqueued_events, &event->enqueue_node);
  queue->worker.cookie++;
  CL_OBJECT_NOTIFY_COND(queue);
  CL_OBJECT_UNLOCK(queue);
}

LOCAL cl_int
cl_command_queue_init_enqueue(cl_command_queue queue)
{
  cl_command_queue_enqueue_worker worker = &queue->worker;
  worker->queue = queue;
  worker->quit = CL_FALSE;
  worker->in_exec_status = CL_COMPLETE;
  worker->cookie = 8;
  list_init(&worker->enqueued_events);

  if (pthread_create(&worker->tid, NULL, worker_thread_function, worker)) {
    CL_LOG_ERROR("Can not create worker thread for queue %p...\n", queue);
    return CL_OUT_OF_RESOURCES;
  }

  return CL_SUCCESS;
}

LOCAL void
cl_command_queue_destroy_enqueue(cl_command_queue queue)
{
  cl_command_queue_enqueue_worker worker = &queue->worker;
  list_node *pos;
  list_node *n;
  cl_event e;

  assert(worker->queue == queue);
  assert(worker->quit == CL_FALSE);

  CL_OBJECT_LOCK(queue);
  worker->quit = 1;
  CL_OBJECT_NOTIFY_COND(queue);
  CL_OBJECT_UNLOCK(queue);

  pthread_join(worker->tid, NULL);

  /* We will wait for finish before destroy the command queue. */
  if (!list_empty(&worker->enqueued_events)) {
    CL_LOG_WARNING("There are still some enqueued works in the queue %p when this"
                   " queue is destroyed, this may cause very serious problems.\n",
                   queue);

    list_for_each_safe(pos, n, &worker->enqueued_events)
    {
      e = list_entry(pos, _cl_event, enqueue_node);
      list_node_del(&e->enqueue_node);
      cl_event_set_status(e, -1); // Give waiters a chance to wakeup.
      cl_event_delete(e);
    }
  }
}

/* Note: Must call this function with queue's lock. */
LOCAL cl_event *
cl_command_queue_record_in_queue_events(cl_command_queue queue, cl_uint *list_num)
{
  int event_num = 0;
  list_node *pos;
  cl_command_queue_enqueue_worker worker = &queue->worker;
  cl_event *enqueued_list = NULL;
  int i;
  cl_event tmp_e = NULL;

  list_for_each(pos, &worker->enqueued_events)
  {
    event_num++;
  }
  assert(event_num > 0);

  enqueued_list = CL_CALLOC(event_num, sizeof(cl_event));
  assert(enqueued_list);

  i = 0;
  list_for_each(pos, &worker->enqueued_events)
  {
    tmp_e = list_entry(pos, _cl_event, enqueue_node);
    cl_event_add_ref(tmp_e); // Add ref temp avoid delete.
    enqueued_list[i] = tmp_e;
    i++;
  }
  assert(i == event_num);

  *list_num = event_num;
  return enqueued_list;
}

LOCAL cl_int
cl_command_queue_wait_flush(cl_command_queue queue)
{
  cl_command_queue_enqueue_worker worker = &queue->worker;
  cl_event *enqueued_list = NULL;
  cl_uint enqueued_num = 0;
  int i;

  CL_OBJECT_LOCK(queue);

  if (worker->quit) { // already destroy the queue?
    CL_OBJECT_UNLOCK(queue);
    return CL_INVALID_COMMAND_QUEUE;
  }

  if (!list_empty(&worker->enqueued_events)) {
    enqueued_list = cl_command_queue_record_in_queue_events(queue, &enqueued_num);
    assert(enqueued_num > 0);
    assert(enqueued_list);
  }

  while (worker->in_exec_status == CL_QUEUED) {
    CL_OBJECT_WAIT_ON_COND(queue);

    if (worker->quit) { // already destroy the queue?
      CL_OBJECT_UNLOCK(queue);
      return CL_INVALID_COMMAND_QUEUE;
    }
  }

  CL_OBJECT_UNLOCK(queue);

  /* Wait all event enter submitted status. */
  for (i = 0; i < enqueued_num; i++) {
    CL_OBJECT_LOCK(enqueued_list[i]);
    while (enqueued_list[i]->status > CL_SUBMITTED) {
      CL_OBJECT_WAIT_ON_COND(enqueued_list[i]);
    }
    CL_OBJECT_UNLOCK(enqueued_list[i]);
  }

  for (i = 0; i < enqueued_num; i++) {
    cl_event_delete(enqueued_list[i]);
  }
  if (enqueued_list)
    CL_FREE(enqueued_list);

  return CL_SUCCESS;
}

LOCAL cl_int
cl_command_queue_wait_finish(cl_command_queue queue)
{
  cl_command_queue_enqueue_worker worker = &queue->worker;
  cl_event *enqueued_list = NULL;
  cl_uint enqueued_num = 0;
  int i;

  CL_OBJECT_LOCK(queue);

  if (worker->quit) { // already destroy the queue?
    CL_OBJECT_UNLOCK(queue);
    return CL_INVALID_COMMAND_QUEUE;
  }

  if (!list_empty(&worker->enqueued_events)) {
    enqueued_list = cl_command_queue_record_in_queue_events(queue, &enqueued_num);
    assert(enqueued_num > 0);
    assert(enqueued_list);
  }

  while (worker->in_exec_status > CL_COMPLETE) {
    CL_OBJECT_WAIT_ON_COND(queue);

    if (worker->quit) { // already destroy the queue?
      CL_OBJECT_UNLOCK(queue);
      return CL_INVALID_COMMAND_QUEUE;
    }
  }

  CL_OBJECT_UNLOCK(queue);

  /* Wait all event enter submitted status. */
  for (i = 0; i < enqueued_num; i++) {
    CL_OBJECT_LOCK(enqueued_list[i]);
    while (enqueued_list[i]->status > CL_COMPLETE) {
      CL_OBJECT_WAIT_ON_COND(enqueued_list[i]);
    }
    CL_OBJECT_UNLOCK(enqueued_list[i]);
  }

  for (i = 0; i < enqueued_num; i++) {
    cl_event_delete(enqueued_list[i]);
  }
  if (enqueued_list)
    CL_FREE(enqueued_list);

  return CL_SUCCESS;
}

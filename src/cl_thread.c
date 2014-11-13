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
#include <string.h>
#include <stdio.h>

#include "cl_thread.h"
#include "cl_alloc.h"
#include "cl_utils.h"

/* Because the cl_command_queue can be used in several threads simultaneously but
   without add ref to it, we now handle it like this:
   Keep one threads_slot_array, every time the thread get gpgpu or batch buffer, if it
   does not have a slot, assign it.
   The resources are keeped in queue private, and resize it if needed.
   When the thread exit, the slot will be set invalid.
   When queue released, all the resources will be released. If user still enqueue, flush
   or finish the queue after it has been released, the behavior is undefined.
   TODO: Need to shrink the slot map.
   */

static int thread_array_num = 1;
static int *thread_slot_map = NULL;
static int thread_magic_num = 1;
static pthread_mutex_t thread_queue_map_lock = PTHREAD_MUTEX_INITIALIZER;

static __thread int thread_id = -1;
static __thread int thread_magic = -1;

typedef struct _thread_spec_data {
  cl_gpgpu gpgpu ;
  int valid;
  void* thread_batch_buf;
  int thread_magic;
} thread_spec_data;

typedef struct _queue_thread_private {
  thread_spec_data**  threads_data;
  int threads_data_num;
  pthread_mutex_t thread_data_lock;
} queue_thread_private;

static thread_spec_data * __create_thread_spec_data(cl_command_queue queue, int create)
{
  queue_thread_private *thread_private = ((queue_thread_private *)(queue->thread_data));
  thread_spec_data* spec = NULL;
  int i = 0;

  if (thread_id == -1) {

    pthread_mutex_lock(&thread_queue_map_lock);
    for (i = 0; i < thread_array_num; i++) {
      if (thread_slot_map[i] == 0) {
        thread_id = i;
        break;
      }
    }

    if (i == thread_array_num) {
      thread_array_num *= 2;
      thread_slot_map = realloc(thread_slot_map, sizeof(int) * thread_array_num);
      memset(thread_slot_map + thread_array_num/2, 0, sizeof(int) * (thread_array_num/2));
      thread_id = thread_array_num/2;
    }

    thread_slot_map[thread_id] = 1;

    thread_magic = thread_magic_num++;
    pthread_mutex_unlock(&thread_queue_map_lock);
  }

  pthread_mutex_lock(&thread_private->thread_data_lock);
  if (thread_array_num > thread_private->threads_data_num) {// just enlarge
    int old_num = thread_private->threads_data_num;
    thread_private->threads_data_num = thread_array_num;
    thread_private->threads_data = realloc(thread_private->threads_data,
                thread_private->threads_data_num * sizeof(void *));
    memset(thread_private->threads_data + old_num, 0,
           sizeof(void*) * (thread_private->threads_data_num - old_num));
  }

  assert(thread_id != -1 && thread_id < thread_array_num);
  spec = thread_private->threads_data[thread_id];
  if (!spec && create) {
       spec = CALLOC(thread_spec_data);
       spec->thread_magic = thread_magic;
       thread_private->threads_data[thread_id] = spec;
  }

  pthread_mutex_unlock(&thread_private->thread_data_lock);

  return spec;
}

void* cl_thread_data_create(void)
{
  queue_thread_private* thread_private = CALLOC(queue_thread_private);

  if (thread_private == NULL)
    return NULL;

  if (thread_slot_map == NULL) {
    pthread_mutex_lock(&thread_queue_map_lock);
    thread_slot_map = calloc(thread_array_num, sizeof(int));
    pthread_mutex_unlock(&thread_queue_map_lock);

  }

  pthread_mutex_init(&thread_private->thread_data_lock, NULL);

  pthread_mutex_lock(&thread_private->thread_data_lock);
  thread_private->threads_data = malloc(thread_array_num * sizeof(void *));
  memset(thread_private->threads_data, 0, sizeof(void*) * thread_array_num);
  thread_private->threads_data_num = thread_array_num;
  pthread_mutex_unlock(&thread_private->thread_data_lock);

  return thread_private;
}

cl_gpgpu cl_get_thread_gpgpu(cl_command_queue queue)
{
  thread_spec_data* spec = __create_thread_spec_data(queue, 1);

  if (!spec->thread_magic && spec->thread_magic != thread_magic) {
    //We may get the slot from last thread. So free the resource.
    spec->valid = 0;
  }

  if (!spec->valid) {
    if (spec->thread_batch_buf) {
      cl_gpgpu_unref_batch_buf(spec->thread_batch_buf);
      spec->thread_batch_buf = NULL;
    }
    if (spec->gpgpu) {
      cl_gpgpu_delete(spec->gpgpu);
      spec->gpgpu = NULL;
    }
    TRY_ALLOC_NO_ERR(spec->gpgpu, cl_gpgpu_new(queue->ctx->drv));
    spec->valid = 1;
  }

 error:
  return spec->gpgpu;
}

void cl_set_thread_batch_buf(cl_command_queue queue, void* buf)
{
  thread_spec_data* spec = __create_thread_spec_data(queue, 1);

  assert(spec && spec->thread_magic == thread_magic);

  if (spec->thread_batch_buf) {
    cl_gpgpu_unref_batch_buf(spec->thread_batch_buf);
  }
  spec->thread_batch_buf = buf;
}

void* cl_get_thread_batch_buf(cl_command_queue queue) {
  thread_spec_data* spec = __create_thread_spec_data(queue, 1);

  assert(spec && spec->thread_magic == thread_magic);

  return spec->thread_batch_buf;
}

void cl_invalid_thread_gpgpu(cl_command_queue queue)
{
  queue_thread_private *thread_private = ((queue_thread_private *)(queue->thread_data));
  thread_spec_data* spec = NULL;

  pthread_mutex_lock(&thread_private->thread_data_lock);
  spec = thread_private->threads_data[thread_id];
  assert(spec);
  pthread_mutex_unlock(&thread_private->thread_data_lock);

  if (!spec->valid) {
    return;
  }

  assert(spec->gpgpu);
  cl_gpgpu_delete(spec->gpgpu);
  spec->gpgpu = NULL;
  spec->valid = 0;
}

cl_gpgpu cl_thread_gpgpu_take(cl_command_queue queue)
{
  queue_thread_private *thread_private = ((queue_thread_private *)(queue->thread_data));
  thread_spec_data* spec = NULL;

  pthread_mutex_lock(&thread_private->thread_data_lock);
  spec = thread_private->threads_data[thread_id];
  assert(spec);
  pthread_mutex_unlock(&thread_private->thread_data_lock);

  if (!spec->valid)
    return NULL;

  assert(spec->gpgpu);
  cl_gpgpu gpgpu = spec->gpgpu;
  spec->gpgpu = NULL;
  spec->valid = 0;
  return gpgpu;
}

/* The destructor for clean the thread specific data. */
void cl_thread_data_destroy(cl_command_queue queue)
{
  int i = 0;
  queue_thread_private *thread_private = ((queue_thread_private *)(queue->thread_data));
  int threads_data_num;
  thread_spec_data** threads_data;

  pthread_mutex_lock(&thread_private->thread_data_lock);
  threads_data_num = thread_private->threads_data_num;
  threads_data = thread_private->threads_data;
  thread_private->threads_data_num = 0;
  thread_private->threads_data = NULL;
  pthread_mutex_unlock(&thread_private->thread_data_lock);
  cl_free(thread_private);
  queue->thread_data = NULL;

  for (i = 0; i < threads_data_num; i++) {
    if (threads_data[i] != NULL && threads_data[i]->thread_batch_buf) {
      cl_gpgpu_unref_batch_buf(threads_data[i]->thread_batch_buf);
      threads_data[i]->thread_batch_buf = NULL;
    }

    if (threads_data[i] != NULL && threads_data[i]->valid) {
      cl_gpgpu_delete(threads_data[i]->gpgpu);
      threads_data[i]->gpgpu = NULL;
      threads_data[i]->valid = 0;
    }
    cl_free(threads_data[i]);
  }

  cl_free(threads_data);
}

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
 */

#include "cl_thread.h"
#include "cl_alloc.h"
#include "cl_utils.h"

static __thread void* thread_batch_buf = NULL;

typedef struct _cl_thread_spec_data {
  cl_gpgpu gpgpu ;
  int valid;
}cl_thread_spec_data;

void cl_set_thread_batch_buf(void* buf) {
  if (thread_batch_buf) {
    cl_gpgpu_unref_batch_buf(thread_batch_buf);
  }
  thread_batch_buf = buf;
}

void* cl_get_thread_batch_buf(void) {
  return thread_batch_buf;
}

cl_gpgpu cl_get_thread_gpgpu(cl_command_queue queue)
{
  pthread_key_t* key = queue->thread_data;
  cl_thread_spec_data* thread_spec_data = pthread_getspecific(*key);

  if (!thread_spec_data) {
    TRY_ALLOC_NO_ERR(thread_spec_data, CALLOC(struct _cl_thread_spec_data));
    if (pthread_setspecific(*key, thread_spec_data)) {
      cl_free(thread_spec_data);
      return NULL;
    }
  }

  if (!thread_spec_data->valid) {
    TRY_ALLOC_NO_ERR(thread_spec_data->gpgpu, cl_gpgpu_new(queue->ctx->drv));
    thread_spec_data->valid = 1;
  }

error:
  return thread_spec_data->gpgpu;
}

void cl_invalid_thread_gpgpu(cl_command_queue queue)
{
  pthread_key_t* key = queue->thread_data;
  cl_thread_spec_data* thread_spec_data = pthread_getspecific(*key);

  if (!thread_spec_data) {
    return;
  }

  if (!thread_spec_data->valid) {
    return;
  }

  assert(thread_spec_data->gpgpu);
  cl_gpgpu_delete(thread_spec_data->gpgpu);
  thread_spec_data->valid = 0;
}

static void thread_data_destructor(void *data) {
  cl_thread_spec_data* thread_spec_data = (cl_thread_spec_data *)data;

  if (thread_batch_buf) {
    cl_gpgpu_unref_batch_buf(thread_batch_buf);
    thread_batch_buf = NULL;
  }

  if (thread_spec_data->valid)
    cl_gpgpu_delete(thread_spec_data->gpgpu);
  cl_free(thread_spec_data);
}

/* Create the thread specific data. */
void* cl_thread_data_create(void)
{
  int rc = 0;

  pthread_key_t *thread_specific_key = CALLOC(pthread_key_t);
  if (thread_specific_key == NULL)
    return NULL;

  rc = pthread_key_create(thread_specific_key, thread_data_destructor);

  if (rc != 0)
    return NULL;

  return thread_specific_key;
}

/* The destructor for clean the thread specific data. */
void cl_thread_data_destroy(void * data)
{
  pthread_key_t *thread_specific_key = (pthread_key_t *)data;

  /* First release self spec data. */
  cl_thread_spec_data* thread_spec_data =
         pthread_getspecific(*thread_specific_key);
  if (thread_spec_data && thread_spec_data->valid) {
    cl_gpgpu_delete(thread_spec_data->gpgpu);
    if (thread_spec_data)
      cl_free(thread_spec_data);
  }

  pthread_key_delete(*thread_specific_key);
  cl_free(thread_specific_key);
}

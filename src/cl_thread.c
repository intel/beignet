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

cl_gpgpu cl_get_thread_gpgpu(cl_command_queue queue)
{
  pthread_key_t* key = queue->thread_data;
  cl_gpgpu gpgpu = pthread_getspecific(*key);

  if (!gpgpu) {
    TRY_ALLOC_NO_ERR (gpgpu, cl_gpgpu_new(queue->ctx->drv));
  }

  if (pthread_setspecific(*key, gpgpu)) {
    cl_gpgpu_delete(gpgpu);
    goto error;
  }

exit:
  return gpgpu;
error:
  pthread_setspecific(*key, NULL);
  goto exit;
}

static void thread_data_destructor(void *data) {
  cl_gpgpu gpgpu = (cl_gpgpu)data;
  cl_gpgpu_delete(gpgpu);
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
  pthread_key_delete(*thread_specific_key);
  cl_free(thread_specific_key);
}

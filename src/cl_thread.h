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

#ifndef __CL_THREAD_H__
#define __CL_THREAD_H__

#include <pthread.h>
#include "cl_internals.h"
#include "cl_command_queue.h"

/* Create the thread specific data. */
void* cl_thread_data_create(void);

/* The destructor for clean the thread specific data. */
void cl_thread_data_destroy(cl_command_queue queue);

/* Used to get the gpgpu struct of each thread. */
cl_gpgpu cl_get_thread_gpgpu(cl_command_queue queue);

/* Used to release the gpgpu struct of each thread. */
void cl_invalid_thread_gpgpu(cl_command_queue queue);

/* Used to set the batch buffer of each thread. */
void cl_set_thread_batch_buf(cl_command_queue queue, void* buf);

/* Used to get the batch buffer of each thread. */
void* cl_get_thread_batch_buf(cl_command_queue queue);

/* take current gpgpu from the thread gpgpu pool. */
cl_gpgpu cl_thread_gpgpu_take(cl_command_queue queue);

#endif /* __CL_THREAD_H__ */

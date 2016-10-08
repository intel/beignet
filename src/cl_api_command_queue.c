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
#include "cl_command_queue.h"
#include "CL/cl.h"
#include <stdio.h>

cl_int
clGetCommandQueueInfo(cl_command_queue command_queue,
                      cl_command_queue_info param_name,
                      size_t param_value_size,
                      void *param_value,
                      size_t *param_value_size_ret)
{
  const void *src_ptr = NULL;
  size_t src_size = 0;
  cl_int ref;

  if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
    return CL_INVALID_COMMAND_QUEUE;
  }

  if (param_name == CL_QUEUE_CONTEXT) {
    src_ptr = &command_queue->ctx;
    src_size = sizeof(cl_context);
  } else if (param_name == CL_QUEUE_DEVICE) {
    src_ptr = &command_queue->ctx->device;
    src_size = sizeof(cl_device_id);
  } else if (param_name == CL_QUEUE_REFERENCE_COUNT) {
    ref = CL_OBJECT_GET_REF(command_queue);
    src_ptr = &ref;
    src_size = sizeof(cl_int);
  } else if (param_name == CL_QUEUE_PROPERTIES) {
    src_ptr = &command_queue->props;
    src_size = sizeof(cl_command_queue_properties);
  } else if (param_name == CL_QUEUE_SIZE) {
    src_ptr = &command_queue->size;
    src_size = sizeof(command_queue->size);
  } else {
    return CL_INVALID_VALUE;
  }

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
}

cl_int
clFlush(cl_command_queue command_queue)
{
  if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
    return CL_INVALID_COMMAND_QUEUE;
  }

  return cl_command_queue_wait_flush(command_queue);
}

cl_int
clFinish(cl_command_queue command_queue)
{
  if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
    return CL_INVALID_COMMAND_QUEUE;
  }

  return cl_command_queue_wait_finish(command_queue);
}

cl_int
clReleaseCommandQueue(cl_command_queue command_queue)
{
  if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
    return CL_INVALID_COMMAND_QUEUE;
  }

  cl_command_queue_wait_flush(command_queue);

  cl_command_queue_delete(command_queue);
  return CL_SUCCESS;
}

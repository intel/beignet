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
#include "cl_device_id.h"
#include "CL/cl.h"
#include <stdio.h>

/* Depreciated in 2.0 later */
cl_command_queue
clCreateCommandQueue(cl_context context,
                     cl_device_id device,
                     cl_command_queue_properties properties,
                     cl_int *errcode_ret)
{
  cl_queue_properties props[3];
  props[0] = CL_QUEUE_PROPERTIES;
  props[1] = properties;
  props[2] = 0;
  return clCreateCommandQueueWithProperties(context, device, props, errcode_ret);
}

/* 2.0 new API for create command queue. */
cl_command_queue
clCreateCommandQueueWithProperties(cl_context context,
                                   cl_device_id device,
                                   const cl_queue_properties *properties,
                                   cl_int *errcode_ret)
{
  cl_command_queue queue = NULL;
  cl_int err = CL_SUCCESS;
  cl_command_queue_properties prop = 0xFFFFFFFF;
  cl_uint queue_sz = 0xFFFFFFFF;

  do {
    if (!CL_OBJECT_IS_CONTEXT(context)) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    err = cl_devices_list_include_check(context->device_num, context->devices, 1, &device);
    if (err)
      break;

    if (properties) {
      cl_ulong que_type;
      cl_ulong que_val;
      cl_uint i;
      for (i = 0; (que_type = properties[i++]) != 0; i++) {
        que_val = properties[i];
        switch (que_type) {
        case CL_QUEUE_PROPERTIES:
          if (prop != 0xFFFFFFFF)
            err = CL_INVALID_VALUE;
          else {
            switch (que_val) {
            case 0:
            case CL_QUEUE_PROFILING_ENABLE:
            case CL_QUEUE_PROFILING_ENABLE |
              CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE:
            case CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE:
            case CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_ON_DEVICE:
            case CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_ON_DEVICE |
              CL_QUEUE_ON_DEVICE_DEFAULT:
            case CL_QUEUE_PROFILING_ENABLE |
              CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_ON_DEVICE:
            case CL_QUEUE_PROFILING_ENABLE |
              CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_ON_DEVICE |
              CL_QUEUE_ON_DEVICE_DEFAULT:
              prop = que_val;
              break;
            default:
              err = CL_INVALID_VALUE;
              break;
            }
          }
          break;
        case CL_QUEUE_SIZE:
          queue_sz = que_val;
          break;
        default:
          err = CL_INVALID_VALUE;
          break;
        }
      }

      if (err) /* break the while and return some err. */
        break;
    }

    /* Set some paramters to default val. */
    if (prop == 0xFFFFFFFF)
      prop = 0;
    if (queue_sz != 0xFFFFFFFF)
      if (!(prop & CL_QUEUE_ON_DEVICE)) {
        err = CL_INVALID_VALUE;
        break;
      }
    if (queue_sz == 0xFFFFFFFF)
      queue_sz = device->queue_on_device_preferred_size;

    if (queue_sz > device->queue_on_device_max_size) {
      err = CL_INVALID_VALUE;
      break;
    }

    queue = cl_create_command_queue(context, device, prop, queue_sz, &err);
  } while (0);

  if (errcode_ret)
    *errcode_ret = err;
  return queue;
}

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
    src_ptr = &command_queue->device;
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
clRetainCommandQueue(cl_command_queue command_queue)
{
  if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
    return CL_INVALID_COMMAND_QUEUE;
  }
  cl_command_queue_add_ref(command_queue);
  return CL_SUCCESS;
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

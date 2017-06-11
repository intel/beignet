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
#include "cl_kernel.h"
#include "cl_command_queue.h"
#include "cl_event.h"
#include "cl_program.h"
#include "cl_mem.h"
#include "cl_device_id.h"
#include "cl_alloc.h"
#include <string.h>

cl_kernel
clCreateKernel(cl_program program,
               const char *kernel_name,
               cl_int *errcode_ret)
{
  cl_kernel kernel = NULL;
  cl_int err = CL_SUCCESS;

  do {
    if (!CL_OBJECT_IS_PROGRAM(program)) {
      err = CL_INVALID_PROGRAM;
      break;
    }

    if (kernel_name == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    kernel = cl_kernel_create(program, kernel_name, &err);
  } while (0);

  if (errcode_ret)
    *errcode_ret = err;
  return kernel;
}

cl_int
clSetKernelArg(cl_kernel kernel,
               cl_uint arg_index,
               size_t arg_size,
               const void *arg_value)
{
  cl_int err = CL_SUCCESS;
  if (!CL_OBJECT_IS_KERNEL(kernel)) {
    return CL_INVALID_KERNEL;
  }

  err = cl_kernel_set_arg(kernel, arg_index, arg_size, arg_value);
  return err;
}

cl_int
clSetKernelArgSVMPointer(cl_kernel kernel,
                         cl_uint arg_index,
                         const void *arg_value)
{
  cl_int err = CL_SUCCESS;

  if (!CL_OBJECT_IS_KERNEL(kernel)) {
    return CL_INVALID_KERNEL;
  }

  err = cl_kernel_set_arg_svm_pointer(kernel, arg_index, arg_value);
  return err;
}

cl_int
clGetKernelInfo(cl_kernel kernel,
                cl_kernel_info param_name,
                size_t param_value_size,
                void *param_value,
                size_t *param_value_size_ret)
{
  const void *src_ptr = NULL;
  size_t src_size = 0;
  const char *str = NULL;
  cl_int ref;
  cl_uint n;
  char null_attr = 0;

  if (!CL_OBJECT_IS_KERNEL(kernel)) {
    return CL_INVALID_KERNEL;
  }

  if (param_name == CL_KERNEL_CONTEXT) {
    src_ptr = &kernel->program->ctx;
    src_size = sizeof(cl_context);
  } else if (param_name == CL_KERNEL_PROGRAM) {
    src_ptr = &kernel->program;
    src_size = sizeof(cl_program);
  } else if (param_name == CL_KERNEL_NUM_ARGS) {
    n = kernel->arg_n;
    src_ptr = &n;
    src_size = sizeof(cl_uint);
  } else if (param_name == CL_KERNEL_REFERENCE_COUNT) {
    ref = CL_OBJECT_GET_REF(kernel);
    src_ptr = &ref;
    src_size = sizeof(cl_int);
  } else if (param_name == CL_KERNEL_FUNCTION_NAME) {
    str = kernel->name;
    src_ptr = str;
    src_size = strlen(str) + 1;
  } else if (param_name == CL_KERNEL_ATTRIBUTES) {
    str = kernel->kernel_attr;
    if (str == NULL)
      str = &null_attr;
    src_ptr = str;
    src_size = strlen(str) + 1;
  } else {
    return CL_INVALID_VALUE;
  }

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
}

cl_int
clEnqueueNDRangeKernel(cl_command_queue command_queue,
                       cl_kernel kernel,
                       cl_uint work_dim,
                       const size_t *global_work_offset,
                       const size_t *global_work_size,
                       const size_t *local_work_size,
                       cl_uint num_events_in_wait_list,
                       const cl_event *event_wait_list,
                       cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_uint i;
  cl_event e = NULL;
  cl_int event_status;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_KERNEL(kernel)) {
      err = CL_INVALID_KERNEL;
      break;
    }

    /* Check number of dimensions we have */
    if (work_dim == 0 || work_dim > 3) {
      err = CL_INVALID_WORK_DIMENSION;
      break;
    }

    /* We need a work size per dimension */
    if (global_work_size == NULL) {
      err = CL_INVALID_GLOBAL_WORK_SIZE;
      break;
    }

    if (global_work_offset != NULL) {
      for (i = 0; i < work_dim; ++i) {
        if (global_work_offset[i] + global_work_size[i] > (size_t)-1) {
          err = CL_INVALID_GLOBAL_OFFSET;
          break;
        }
      }
    }

    /* Queue and kernel must share the same context */
    assert(kernel->program);
    if (command_queue->ctx != kernel->program->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_NDRANGE_KERNEL, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueNDRangeKernel;
    e->exec_data.nd_range.kernel = kernel;
    e->exec_data.nd_range.work_dim = work_dim;
    if (global_work_offset) {
      for (i = 0; i < work_dim; ++i) {
        e->exec_data.nd_range.global_wk_off[i] = global_work_offset[i];
      }
    }
    for (i = 0; i < work_dim; ++i) {
      e->exec_data.nd_range.global_wk_sz[i] = global_work_size[i];
    }
    if (local_work_size) {
      for (i = 0; i < work_dim; ++i) {
        e->exec_data.nd_range.local_wk_sz[i] = local_work_size[i];
      }
    }

    event_status = cl_event_is_ready(e);
    if (event_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    err = cl_event_exec(e, (event_status == CL_COMPLETE ? CL_SUBMITTED : CL_QUEUED), CL_FALSE);
    if (err != CL_SUCCESS) {
      break;
    }

    cl_command_queue_enqueue_event(command_queue, e);
  } while (0);

  if (err == CL_SUCCESS && event) {
    *event = e;
  } else {
    cl_event_delete(e);
  }

  return err;
}

cl_int
clEnqueueTask(cl_command_queue command_queue,
              cl_kernel kernel,
              cl_uint num_events_in_wait_list,
              const cl_event *event_wait_list,
              cl_event *event)
{
  const size_t global_size[3] = {1, 0, 0};
  const size_t local_size[3] = {1, 0, 0};

  return clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
                                global_size, local_size,
                                num_events_in_wait_list, event_wait_list, event);
}

cl_int
clEnqueueNativeKernel(cl_command_queue command_queue,
                      void (*user_func)(void *),
                      void *args,
                      size_t cb_args,
                      cl_uint num_mem_objects,
                      const cl_mem *mem_list,
                      const void **args_mem_loc,
                      cl_uint num_events_in_wait_list,
                      const cl_event *event_wait_list,
                      cl_event *event)
{
  cl_int err = CL_SUCCESS;
  void *new_args = NULL;
  void **new_args_mem_loc = NULL;
  cl_mem *new_mem_list = NULL;
  cl_int i;
  cl_int e_status;
  cl_event e = NULL;

  do {
    if (user_func == NULL ||
        (args == NULL && cb_args > 0) ||
        (args == NULL && num_mem_objects > 0) ||
        (args != NULL && cb_args == 0) ||
        (num_mem_objects > 0 && (mem_list == NULL || args_mem_loc == NULL)) ||
        (num_mem_objects == 0 && (mem_list != NULL || args_mem_loc != NULL))) {
      err = CL_INVALID_VALUE;
      break;
    }

    //Per spec, need copy args
    if (cb_args) {
      new_args = CL_MALLOC(cb_args);
      if (num_mem_objects) {
        new_args_mem_loc = CL_MALLOC(sizeof(void *) * num_mem_objects);
        new_mem_list = CL_MALLOC(sizeof(cl_mem) * num_mem_objects);
        memcpy(new_mem_list, mem_list, sizeof(cl_mem) * num_mem_objects);
      }

      if (new_args == NULL || new_args_mem_loc == NULL) {
        err = CL_OUT_OF_HOST_MEMORY;
        break;
      }
      memcpy(new_args, args, cb_args);

      for (i = 0; i < num_mem_objects; ++i) {
        if (!CL_OBJECT_IS_MEM(mem_list[i])) {
          err = CL_INVALID_MEM_OBJECT;
          break;
        }

        new_args_mem_loc[i] = new_args + (args_mem_loc[i] - args); //change to new args
      }
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_NATIVE_KERNEL, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueNativeKernel;
    e->exec_data.native_kernel.mem_list = new_mem_list;
    e->exec_data.native_kernel.args = new_args;
    e->exec_data.native_kernel.cb_args = cb_args;
    e->exec_data.native_kernel.mem_num = num_mem_objects;
    e->exec_data.native_kernel.mem_arg_loc = new_args_mem_loc;
    e->exec_data.native_kernel.user_func = user_func;
    new_args = NULL;
    new_mem_list = NULL;
    new_args_mem_loc = NULL; // Event delete will free them.

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    err = cl_event_exec(e, (e_status == CL_COMPLETE ? CL_COMPLETE : CL_QUEUED), CL_FALSE);
    if (err != CL_SUCCESS) {
      break;
    }

    if (e_status != CL_COMPLETE)
      cl_command_queue_enqueue_event(command_queue, e);
  } while (0);

  if (err != CL_SUCCESS) {
    if (new_args)
      CL_FREE(new_args);
    if (new_mem_list)
      CL_FREE(new_mem_list);
    if (new_args_mem_loc)
      CL_FREE(new_args_mem_loc);
  }

  if (err == CL_SUCCESS && event) {
    *event = e;
  } else {
    cl_event_delete(e);
  }

  return err;
}

cl_int
clRetainKernel(cl_kernel kernel)
{
  if (!CL_OBJECT_IS_KERNEL(kernel)) {
    return CL_INVALID_KERNEL;
  }

  cl_kernel_add_ref(kernel);
  return CL_SUCCESS;
}

cl_int
clReleaseKernel(cl_kernel kernel)
{
  if (!CL_OBJECT_IS_KERNEL(kernel)) {
    return CL_INVALID_KERNEL;
  }

  cl_kernel_delete(kernel);
  return CL_SUCCESS;
}

cl_int clGetKernelArgInfo(cl_kernel kernel,
                          cl_uint arg_index,
                          cl_kernel_arg_info param_name,
                          size_t param_value_size,
                          void *param_value,
                          size_t *param_value_size_ret)
{
  cl_int err = CL_SUCCESS;

  if (!CL_OBJECT_IS_KERNEL(kernel)) {
    return CL_INVALID_KERNEL;
  }

  if (kernel->program->build_opts == NULL ||
      strstr(kernel->program->build_opts, "-cl-kernel-arg-info") == NULL) {
    return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
  }
  if (param_name != CL_KERNEL_ARG_ADDRESS_QUALIFIER && param_name != CL_KERNEL_ARG_ACCESS_QUALIFIER &&
      param_name != CL_KERNEL_ARG_TYPE_NAME && param_name != CL_KERNEL_ARG_TYPE_QUALIFIER &&
      param_name != CL_KERNEL_ARG_NAME) {
    return CL_INVALID_VALUE;
  }

  if (arg_index >= kernel->arg_n) {
    return CL_INVALID_ARG_INDEX;
  }

  CL_OBJECT_TAKE_OWNERSHIP(kernel, CL_TRUE);
  err = cl_kernel_get_argument_info(kernel, arg_index, param_name, param_value_size,
                                    param_value, param_value_size_ret);
  CL_OBJECT_RELEASE_OWNERSHIP(kernel);
  return err;
}

cl_int
clGetKernelWorkGroupInfo(cl_kernel kernel,
                         cl_device_id device,
                         cl_kernel_work_group_info param_name,
                         size_t param_value_size,
                         void *param_value,
                         size_t *param_value_size_ret)
{
  cl_int err = CL_SUCCESS;
  if (!CL_OBJECT_IS_KERNEL(kernel)) {
    return CL_INVALID_KERNEL;
  }

  if (device) {
    err = cl_devices_list_check(1, (const cl_device_id *)&device);
    if (err != CL_SUCCESS)
      return err;

    err = cl_devices_list_include_check(kernel->program->ctx->device_num,
                                        kernel->program->ctx->devices, 1,
                                        (const cl_device_id *)&device);
    if (err != CL_SUCCESS)
      return err;
  } else {
    if (kernel->each_device_num != 1)
      return CL_INVALID_DEVICE;
  }

  return cl_kernel_get_workgroup_info(kernel, device, param_name, param_value_size,
                                      param_value, param_value_size_ret);
}

cl_int
clGetKernelSubGroupInfoKHR(cl_kernel kernel,
                           cl_device_id device,
                           cl_kernel_work_group_info param_name,
                           size_t input_value_size,
                           const void *input_value,
                           size_t param_value_size,
                           void *param_value,
                           size_t *param_value_size_ret)
{
  cl_int err = CL_SUCCESS;
  if (!CL_OBJECT_IS_KERNEL(kernel)) {
    return CL_INVALID_KERNEL;
  }

  if (device == NULL) {
    if (kernel->program->ctx->device_num > 1)
      return CL_INVALID_DEVICE;

    device = kernel->program->ctx->devices[0];
  } else {
    err = cl_devices_list_check(1, (const cl_device_id *)&device);
    if (err != CL_SUCCESS)
      return err;

    err = cl_devices_list_include_check(kernel->program->ctx->device_num,
                                        kernel->program->ctx->devices, 1,
                                        (const cl_device_id *)&device);
    if (err != CL_SUCCESS)
      return err;
  }

  return cl_kernel_get_subgroup_info(kernel, device, param_name, input_value_size, input_value,
                                     param_value_size, param_value, param_value_size_ret);
}

cl_int
clSetKernelExecInfo(cl_kernel kernel,
                    cl_kernel_exec_info param_name,
                    size_t param_value_size,
                    const void *param_value)
{
  cl_int err = CL_SUCCESS;

  if (!CL_OBJECT_IS_KERNEL(kernel)) {
    return CL_INVALID_KERNEL;
  }

  if ((param_name != CL_KERNEL_EXEC_INFO_SVM_PTRS &&
       param_name != CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM) ||
      param_value == NULL || param_value_size == 0) {
    return CL_INVALID_VALUE;
  }

  if (param_name == CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM &&
      *(cl_bool *)param_value == CL_TRUE) { /* Not support fine grain yet */
    return CL_INVALID_OPERATION;
  }

  err = cl_kernel_set_exec_info(kernel, param_value_size, param_value);
  return err;
}

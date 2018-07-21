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
#include "cl_mem.h"
#include "cl_kernel.h"
#include "cl_enqueue.h"
#include "cl_command_queue.h"
#include "cl_event.h"
#include "cl_context.h"
#include "cl_program.h"
#include "cl_alloc.h"
#include "CL/cl.h"
#include <stdio.h>
#include <string.h>

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
    str = cl_kernel_get_name(kernel);
    src_ptr = str;
    src_size = strlen(str) + 1;
  } else if (param_name == CL_KERNEL_ATTRIBUTES) {
    str = cl_kernel_get_attributes(kernel);
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
  size_t fixed_global_off[] = {0, 0, 0};
  size_t fixed_global_sz[] = {1, 1, 1};
  size_t fixed_local_sz[] = {1, 1, 1};
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
    if (UNLIKELY(work_dim == 0 || work_dim > 3)) {
      err = CL_INVALID_WORK_DIMENSION;
      break;
    }

    /* We need a work size per dimension */
    if (UNLIKELY(global_work_size == NULL)) {
      err = CL_INVALID_GLOBAL_WORK_SIZE;
      break;
    }

    if (kernel->vme) {
      if (work_dim != 2) {
        err = CL_INVALID_WORK_DIMENSION;
        break;
      }
      if (local_work_size != NULL) {
        err = CL_INVALID_WORK_GROUP_SIZE;
        break;
      }
    }

    if (global_work_offset != NULL) {
      for (i = 0; i < work_dim; ++i) {
        if (UNLIKELY(global_work_offset[i] + global_work_size[i] > (size_t)-1)) {
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

    if (local_work_size != NULL) {
      for (i = 0; i < work_dim; ++i)
        fixed_local_sz[i] = local_work_size[i];
    } else {
      if (kernel->vme) {
        fixed_local_sz[0] = 16;
        fixed_local_sz[1] = 1;
      } else {
        uint j, maxDimSize = 64 /* from 64? */, maxGroupSize = 256; //MAX_WORK_GROUP_SIZE may too large
        size_t realGroupSize = 1;
        for (i = 0; i < work_dim; i++) {
          for (j = maxDimSize; j > 1; j--) {
            if (global_work_size[i] % j == 0 && j <= maxGroupSize) {
              fixed_local_sz[i] = j;
              maxGroupSize = maxGroupSize / j;
              maxDimSize = maxGroupSize > maxDimSize ? maxDimSize : maxGroupSize;
              break; //choose next work_dim
            }
          }
          realGroupSize *= fixed_local_sz[i];
        }

        //in a loop of conformance test (such as test_api repeated_setup_cleanup), in each loop:
        //create a new context, a new command queue, and uses 'globalsize[0]=1000, localsize=NULL' to enqueu kernel
        //it triggers the following message for many times.
        //to avoid too many messages, only print it for the first time of the process.
        //just use static variable since it doesn't matter to print a few times at multi-thread case.
        static int warn_no_good_localsize = 1;
        if (realGroupSize % 8 != 0 && warn_no_good_localsize) {
          warn_no_good_localsize = 0;
          DEBUGP(DL_WARNING, "unable to find good values for local_work_size[i], please provide\n"
                             " local_work_size[] explicitly, you can find good values with\n"
                             " trial-and-error method.");
        }
      }
    }

    if (kernel->vme) {
      fixed_global_sz[0] = (global_work_size[0] + 15) / 16 * 16;
      fixed_global_sz[1] = (global_work_size[1] + 15) / 16;
    } else {
      for (i = 0; i < work_dim; ++i)
        fixed_global_sz[i] = global_work_size[i];
    }

    if (global_work_offset != NULL)
      for (i = 0; i < work_dim; ++i)
        fixed_global_off[i] = global_work_offset[i];

    if (kernel->compile_wg_sz[0] || kernel->compile_wg_sz[1] || kernel->compile_wg_sz[2]) {
      if (fixed_local_sz[0] != kernel->compile_wg_sz[0] ||
          fixed_local_sz[1] != kernel->compile_wg_sz[1] ||
          fixed_local_sz[2] != kernel->compile_wg_sz[2]) {
        err = CL_INVALID_WORK_GROUP_SIZE;
        break;
      }
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    int i, j, k;
    const size_t global_wk_sz_div[3] = {
      fixed_global_sz[0] / fixed_local_sz[0] * fixed_local_sz[0],
      fixed_global_sz[1] / fixed_local_sz[1] * fixed_local_sz[1],
      fixed_global_sz[2] / fixed_local_sz[2] * fixed_local_sz[2]};

    const size_t global_wk_sz_rem[3] = {
      fixed_global_sz[0] % fixed_local_sz[0],
      fixed_global_sz[1] % fixed_local_sz[1],
      fixed_global_sz[2] % fixed_local_sz[2]};
    cl_uint count;
    count = global_wk_sz_rem[0] ? 2 : 1;
    count *= global_wk_sz_rem[1] ? 2 : 1;
    count *= global_wk_sz_rem[2] ? 2 : 1;

    const size_t *global_wk_all[2] = {global_wk_sz_div, global_wk_sz_rem};
    cl_bool allow_immediate_submit = cl_command_queue_allow_bypass_submit(command_queue);
    /* Go through the at most 8 cases and euque if there is work items left */
    for (i = 0; i < 2; i++) {
      for (j = 0; j < 2; j++) {
        for (k = 0; k < 2; k++) {
          size_t global_wk_sz_use[3] = {global_wk_all[k][0], global_wk_all[j][1], global_wk_all[i][2]};
          size_t global_dim_off[3] = {
            k * global_wk_sz_div[0] / fixed_local_sz[0],
            j * global_wk_sz_div[1] / fixed_local_sz[1],
            i * global_wk_sz_div[2] / fixed_local_sz[2]};
          size_t local_wk_sz_use[3] = {
            k ? global_wk_sz_rem[0] : fixed_local_sz[0],
            j ? global_wk_sz_rem[1] : fixed_local_sz[1],
            i ? global_wk_sz_rem[2] : fixed_local_sz[2]};
          if (local_wk_sz_use[0] == 0 || local_wk_sz_use[1] == 0 || local_wk_sz_use[2] == 0)
            continue;

          e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                              event_wait_list, CL_COMMAND_NDRANGE_KERNEL, &err);
          if (err != CL_SUCCESS) {
            break;
          }

          /* Do device specific checks are enqueue the kernel */
          err = cl_command_queue_ND_range(command_queue, kernel, e, work_dim,
                                          fixed_global_off, global_dim_off, fixed_global_sz,
                                          global_wk_sz_use, fixed_local_sz, local_wk_sz_use);
          if (err != CL_SUCCESS) {
            break;
          }
          e->exec_data.mid_event_of_enq = (count > 1);
          count--;

          /* We will flush the ndrange if no event depend. Else we will add it to queue list.
             The finish or Complete status will always be done in queue list. */
          event_status = cl_event_is_ready(e);
          if (event_status < CL_COMPLETE) { // Error happend, cancel.
            err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
            break;
          }

          err = cl_event_exec(e, ((allow_immediate_submit && event_status == CL_COMPLETE) ? CL_SUBMITTED : CL_QUEUED), CL_FALSE);
          if (err != CL_SUCCESS) {
            break;
          }

          cl_command_queue_enqueue_event(command_queue, e);

          if (e->exec_data.mid_event_of_enq)
            cl_event_delete(e);
        }
        if (err != CL_SUCCESS) {
          break;
        }
      }
      if (err != CL_SUCCESS) {
        break;
      }
    }
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
  enqueue_data *data = NULL;

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
      new_args = cl_malloc(cb_args);
      if (num_mem_objects) {
        new_args_mem_loc = cl_malloc(sizeof(void *) * num_mem_objects);
        new_mem_list = cl_malloc(sizeof(cl_mem) * num_mem_objects);
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

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    data = &e->exec_data;
    data->type = EnqueueNativeKernel;
    data->mem_list = new_mem_list;
    data->ptr = new_args;
    data->size = cb_args;
    data->offset = (size_t)num_mem_objects;
    data->const_ptr = new_args_mem_loc;
    data->user_func = user_func;
    new_args = NULL;
    new_mem_list = NULL;
    new_args_mem_loc = NULL; // Event delete will free them.

    err = cl_event_exec(e, (e_status == CL_COMPLETE ? CL_COMPLETE : CL_QUEUED), CL_FALSE);
    if (err != CL_SUCCESS) {
      break;
    }

    if (e_status != CL_COMPLETE)
      cl_command_queue_enqueue_event(command_queue, e);
  } while (0);

  if (err != CL_SUCCESS) {
    if (new_args)
      cl_free(new_args);
    if (new_mem_list)
      cl_free(new_mem_list);
    if (new_args_mem_loc)
      cl_free(new_args_mem_loc);
  }

  if (err == CL_SUCCESS && event) {
    *event = e;
  } else {
    cl_event_delete(e);
  }

  return err;
}

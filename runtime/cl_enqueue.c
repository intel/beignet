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
 * Author: Rong Yang <rong.r.yang@intel.com>
 */

//#include "cl_image.h"
#include "cl_enqueue.h"
#include "cl_event.h"
#include "cl_kernel.h"
#include "cl_command_queue.h"
#include "cl_utils.h"
#include "cl_alloc.h"
#include "cl_mem.h"
#include "cl_device_id.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

LOCAL void
cl_enqueue_delete_native_kernel(cl_event e)
{
  assert(e->exec_data.type == EnqueueNativeKernel);

  if (e->exec_data.native_kernel.mem_list) {
    CL_FREE(e->exec_data.native_kernel.mem_list);
    e->exec_data.native_kernel.mem_list = NULL;
  }
  if (e->exec_data.native_kernel.args) {
    CL_FREE(e->exec_data.native_kernel.args);
    e->exec_data.native_kernel.args = NULL;
  }
  if (e->exec_data.native_kernel.mem_arg_loc) {
    CL_FREE(e->exec_data.native_kernel.mem_arg_loc);
    e->exec_data.native_kernel.mem_arg_loc = NULL;
  }
}

static cl_int
cl_enqueue_handle_native_kernel(cl_event e, cl_int status)
{
  cl_mem *mem_list = e->exec_data.native_kernel.mem_list;
  cl_uint mem_n = e->exec_data.native_kernel.mem_num;
  cl_uint i;
  cl_command_queue queue = e->queue;
  cl_int err = CL_SUCCESS;

  if (status == CL_QUEUED) {
    for (i = 0; i < mem_n; i++) {
      assert(CL_OBJECT_IS_MEM(mem_list[i]));
      err = cl_mem_assure_allocated(queue->device, mem_list[i]);
      if (err != CL_SUCCESS) {
        return err;
      }
    }
  }

  err = queue->device->api.native_kernel(e, status);
  return err;
}

static cl_int
cl_enqueue_handle_marker_or_barrier(cl_event e, cl_int status)
{
  return CL_COMPLETE;
}

LOCAL cl_int
cl_enqueue_handle(cl_event e, cl_int status)
{
  switch (e->exec_data.type) {
  case EnqueueReturnSuccesss:
    return CL_SUCCESS;
  case EnqueueReadBuffer:
  case EnqueueReadBufferRect:
  case EnqueueWriteBuffer:
  case EnqueueWriteBufferRect:
  case EnqueueReadImage:
  case EnqueueWriteImage:
    return cl_enqueue_handle_read_write_mem(e, status);
  case EnqueueMapBuffer:
  case EnqueueMapImage:
    return cl_enqueue_handle_map_mem(e, status);
  case EnqueueUnmapMemObject:
    return cl_enqueue_handle_unmap_mem(e, status);
  case EnqueueSVMMemFree:
    return cl_enqueue_handle_svm_free(e, status);
  case EnqueueSVMMemCopy:
    return cl_enqueue_handle_svm_copy(e, status);
  case EnqueueSVMMemFill:
    return cl_enqueue_handle_svm_fill(e, status);
  case EnqueueMarker:
  case EnqueueBarrier:
    return cl_enqueue_handle_marker_or_barrier(e, status);
  case EnqueueCopyBufferRect:
  case EnqueueCopyBuffer:
  case EnqueueCopyImage:
  case EnqueueCopyBufferToImage:
  case EnqueueCopyImageToBuffer:
    return cl_enqueue_handle_copy_mem(e, status);
  case EnqueueNDRangeKernel:
    return cl_enqueue_handle_kernel_ndrange(e, status);
  case EnqueueFillBuffer:
  case EnqueueFillImage:
    return cl_enqueue_handle_fill_mem(e, status);
  case EnqueueNativeKernel:
    return cl_enqueue_handle_native_kernel(e, status);
  case EnqueueMigrateMemObj:
  default:
    return CL_SUCCESS;
  }
}

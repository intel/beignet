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

#include "cl_gen.h"

struct gen_gpgpu;
extern void gen_gpgpu_event_get_exec_timestamp(void *gpgpu_ctx, int index, uint64_t *ret_ts);
extern void gen_gpgpu_event_get_gpu_cur_timestamp(cl_device_id device, intel_driver_t *drv, uint64_t *ret_ts);

LOCAL void
cl_event_update_timestamp_gen(cl_event event, cl_int status)
{
  cl_ulong ts = 0;
  cl_context_gen ctx_gen = NULL;

  if ((event->exec_data.type == EnqueueCopyBufferRect) ||
      (event->exec_data.type == EnqueueCopyBuffer) ||
      (event->exec_data.type == EnqueueCopyImage) ||
      (event->exec_data.type == EnqueueCopyBufferToImage) ||
      (event->exec_data.type == EnqueueCopyImageToBuffer) ||
      (event->exec_data.type == EnqueueNDRangeKernel) ||
      (event->exec_data.type == EnqueueFillBuffer) ||
      (event->exec_data.type == EnqueueFillImage)) {

    if (status == CL_QUEUED || status == CL_SUBMITTED) {
      DEV_PRIVATE_DATA(event->ctx, event->queue->device, ctx_gen);
      gen_gpgpu_event_get_gpu_cur_timestamp(event->queue->device, ctx_gen->drv, &ts);

      if (ts == CL_EVENT_INVALID_TIMESTAMP)
        ts++;
      event->timestamp[CL_QUEUED - status] = ts;
      return;
    } else if (status == CL_RUNNING) {
      assert(event->exec_data.exec_ctx);
      return; // Wait for the event complete and get run and complete then.
    } else {
      assert(event->exec_data.exec_ctx);
      gen_gpgpu_event_get_exec_timestamp(event->exec_data.exec_ctx, 0, &ts);
      if (ts == CL_EVENT_INVALID_TIMESTAMP)
        ts++;
      event->timestamp[2] = ts;
      gen_gpgpu_event_get_exec_timestamp(event->exec_data.exec_ctx, 1, &ts);
      if (ts == CL_EVENT_INVALID_TIMESTAMP)
        ts++;
      event->timestamp[3] = ts;

      /* Set the submit time the same as running time if it is later. */
      if (event->timestamp[1] > event->timestamp[2] ||
          event->timestamp[2] - event->timestamp[1] > 0x0FFFFFFFFFF /*Overflowed */)
        event->timestamp[1] = event->timestamp[2];

      return;
    }
  } else {
    DEV_PRIVATE_DATA(event->ctx, event->queue->device, ctx_gen);
    gen_gpgpu_event_get_gpu_cur_timestamp(event->queue->device, ctx_gen->drv, &ts);
    if (ts == CL_EVENT_INVALID_TIMESTAMP)
      ts++;
    event->timestamp[CL_QUEUED - status] = ts;
    return;
  }
}

LOCAL cl_int
cl_event_create_gen(cl_device_id device, cl_event event)
{
  assert(event);
  assert(event->queue); // Can not be user event

  return CL_SUCCESS;
}

LOCAL void
cl_event_delete_gen(cl_device_id device, cl_event event)
{
  if (event->exec_data.type == EnqueueNDRangeKernel ||
      event->exec_data.type == EnqueueFillImage ||
      event->exec_data.type == EnqueueCopyImage ||
      event->exec_data.type == EnqueueCopyImageToBuffer ||
      event->exec_data.type == EnqueueCopyBufferToImage ||
      event->exec_data.type == EnqueueCopyBuffer ||
      event->exec_data.type == EnqueueCopyBufferRect ||
      event->exec_data.type == EnqueueFillBuffer) {
    cl_enqueue_nd_range_delete_gen(event);
  } else if (event->exec_data.type == EnqueueNativeKernel) {
    cl_enqueue_delete_native_kernel(event);
  } else if (event->exec_data.type == EnqueueSVMMemFree) {
    cl_svm_free_delete_func(event);
  }
}

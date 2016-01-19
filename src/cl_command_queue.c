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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "program.h" // for BTI_MAX_IMAGE_NUM
#include "cl_command_queue.h"
#include "cl_context.h"
#include "cl_program.h"
#include "cl_kernel.h"
#include "cl_device_id.h"
#include "cl_mem.h"
#include "cl_utils.h"
#include "cl_alloc.h"
#include "cl_driver.h"
#include "cl_khr_icd.h"
#include "cl_event.h"
#include "performance.h"
#include "cl_cmrt.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

LOCAL cl_command_queue
cl_command_queue_new(cl_context ctx)
{
  cl_command_queue queue = NULL;

  assert(ctx);
  TRY_ALLOC_NO_ERR (queue, CALLOC(struct _cl_command_queue));
  CL_OBJECT_INIT_BASE(queue, CL_OBJECT_COMMAND_QUEUE_MAGIC);
  cl_command_queue_init_enqueue(queue);

  /* Append the command queue in the list */
  cl_context_add_queue(ctx, queue);
  queue->ctx = ctx;
  queue->cmrt_event = NULL;

exit:
  return queue;
error:
  cl_command_queue_delete(queue);
  queue = NULL;
  goto exit;
}

LOCAL void
cl_command_queue_delete(cl_command_queue queue)
{
  assert(queue);
  if (CL_OBJECT_DEC_REF(queue) > 1)
    return;

  cl_command_queue_destroy_enqueue(queue);

#ifdef HAS_CMRT
  if (queue->cmrt_event != NULL)
    cmrt_destroy_event(queue);
#endif

  // If there is a list of valid events, we need to give them
  // a chance to call the call-back function.
  //cl_event_update_last_events(queue,1);

  cl_mem_delete(queue->perf);
  cl_context_remove_queue(queue->ctx, queue);
  if (queue->barrier_events) {
    cl_free(queue->barrier_events);
  }
  CL_OBJECT_DESTROY_BASE(queue);
  cl_free(queue);
}

LOCAL void
cl_command_queue_add_ref(cl_command_queue queue)
{
  CL_OBJECT_INC_REF(queue);
}

static void
set_image_info(char *curbe,
               struct ImageInfo * image_info,
               struct _cl_mem_image *image)
{
  if (image_info->wSlot >= 0)
    *(uint32_t*)(curbe + image_info->wSlot) = image->w;
  if (image_info->hSlot >= 0)
    *(uint32_t*)(curbe + image_info->hSlot) = image->h;
  if (image_info->depthSlot >= 0)
    *(uint32_t*)(curbe + image_info->depthSlot) = image->depth;
  if (image_info->channelOrderSlot >= 0)
    *(uint32_t*)(curbe + image_info->channelOrderSlot) = image->fmt.image_channel_order;
  if (image_info->dataTypeSlot >= 0)
    *(uint32_t*)(curbe + image_info->dataTypeSlot) = image->fmt.image_channel_data_type;
}

LOCAL cl_int
cl_command_queue_bind_image(cl_command_queue queue, cl_kernel k, cl_gpgpu gpgpu)
{
  uint32_t i;

  for (i = 0; i < k->image_sz; i++) {
    int id = k->images[i].arg_idx;
    struct _cl_mem_image *image;
    assert(interp_kernel_get_arg_type(k->opaque, id) == GBE_ARG_IMAGE);

    image = cl_mem_image(k->args[id].mem);
    set_image_info(k->curbe, &k->images[i], image);
    if(k->vme){
      if( (image->fmt.image_channel_order != CL_R) || (image->fmt.image_channel_data_type != CL_UNORM_INT8) )
        return CL_IMAGE_FORMAT_NOT_SUPPORTED;
      cl_gpgpu_bind_image_for_vme(gpgpu, k->images[i].idx, image->base.bo, image->offset + k->args[id].mem->offset,
                          image->intel_fmt, image->image_type, image->bpp,
                          image->w, image->h, image->depth,
                          image->row_pitch, image->slice_pitch, (cl_gpgpu_tiling)image->tiling);
    }
    else
      cl_gpgpu_bind_image(gpgpu, k->images[i].idx, image->base.bo, image->offset + k->args[id].mem->offset,
                          image->intel_fmt, image->image_type, image->bpp,
                          image->w, image->h, image->depth,
                          image->row_pitch, image->slice_pitch, (cl_gpgpu_tiling)image->tiling);
    // TODO, this workaround is for GEN7/GEN75 only, we may need to do it in the driver layer
    // on demand.
    if (image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
      cl_gpgpu_bind_image(gpgpu, k->images[i].idx + BTI_WORKAROUND_IMAGE_OFFSET, image->base.bo, image->offset + k->args[id].mem->offset,
                          image->intel_fmt, image->image_type, image->bpp,
                          image->w, image->h, image->depth,
                          image->row_pitch, image->slice_pitch, (cl_gpgpu_tiling)image->tiling);
  }
  return CL_SUCCESS;
}

LOCAL cl_int
cl_command_queue_bind_surface(cl_command_queue queue, cl_kernel k, cl_gpgpu gpgpu)
{
  /* Bind all user buffers (given by clSetKernelArg) */
  uint32_t i;
  uint32_t ocl_version = interp_kernel_get_ocl_version(k->opaque);
  enum gbe_arg_type arg_type; /* kind of argument */
  for (i = 0; i < k->arg_n; ++i) {
    int32_t offset; // location of the address in the curbe
    arg_type = interp_kernel_get_arg_type(k->opaque, i);
    if (!(arg_type == GBE_ARG_GLOBAL_PTR || (arg_type == GBE_ARG_CONSTANT_PTR && ocl_version >= 200)) || !k->args[i].mem)
      continue;
    offset = interp_kernel_get_curbe_offset(k->opaque, GBE_CURBE_KERNEL_ARGUMENT, i);
    if (offset < 0)
      continue;
    if (k->args[i].mem->type == CL_MEM_SUBBUFFER_TYPE) {
      struct _cl_mem_buffer* buffer = (struct _cl_mem_buffer*)k->args[i].mem;
      cl_gpgpu_bind_buf(gpgpu, k->args[i].mem->bo, offset, k->args[i].mem->offset + buffer->sub_offset, k->args[i].mem->size, interp_kernel_get_arg_bti(k->opaque, i));
    } else {
      cl_gpgpu_bind_buf(gpgpu, k->args[i].mem->bo, offset, k->args[i].mem->offset, k->args[i].mem->size, interp_kernel_get_arg_bti(k->opaque, i));
    }
  }

  return CL_SUCCESS;
}

extern cl_int cl_command_queue_ND_range_gen7(cl_command_queue, cl_kernel, cl_event, 
                                             uint32_t, const size_t *, const size_t *, const size_t *);

static cl_int
cl_kernel_check_args(cl_kernel k)
{
  uint32_t i;
  for (i = 0; i < k->arg_n; ++i)
    if (k->args[i].is_set == CL_FALSE)
      return CL_INVALID_KERNEL_ARGS;
  return CL_SUCCESS;
}

LOCAL cl_int
cl_command_queue_ND_range(cl_command_queue queue,
                          cl_kernel k,
                          cl_event event,
                          const uint32_t work_dim,
                          const size_t *global_wk_off,
                          const size_t *global_wk_sz,
                          const size_t *local_wk_sz)
{
  if(b_output_kernel_perf)
    time_start(queue->ctx, cl_kernel_get_name(k), queue);
  const int32_t ver = cl_driver_get_ver(queue->ctx->drv);
  cl_int err = CL_SUCCESS;

  /* Check that the user did not forget any argument */
  TRY (cl_kernel_check_args, k);


  if (ver == 7 || ver == 75 || ver == 8 || ver == 9)
    TRY (cl_command_queue_ND_range_gen7, queue, k, event,
         work_dim, global_wk_off, global_wk_sz, local_wk_sz);
  else
    FATAL ("Unknown Gen Device");

error:
  return err;
}

LOCAL int
cl_command_queue_flush_gpgpu(cl_gpgpu gpgpu)
{
  void* printf_info = cl_gpgpu_get_printf_info(gpgpu);
  void* profiling_info;

  if (cl_gpgpu_flush(gpgpu) < 0)
    return CL_OUT_OF_RESOURCES;

  if (printf_info && interp_get_printf_num(printf_info)) {
    void *addr = cl_gpgpu_map_printf_buffer(gpgpu);
    interp_output_printf(printf_info, addr);
    cl_gpgpu_unmap_printf_buffer(gpgpu);
  }

  if (printf_info) {
    interp_release_printf_info(printf_info);
    cl_gpgpu_set_printf_info(gpgpu, NULL);
  }

  /* If have profiling info, output it. */
  profiling_info = cl_gpgpu_get_profiling_info(gpgpu);
  if (profiling_info) {
    interp_output_profiling(profiling_info, cl_gpgpu_map_profiling_buffer(gpgpu));
    cl_gpgpu_unmap_profiling_buffer(gpgpu);
  }
  return CL_SUCCESS;
}

LOCAL void
cl_command_queue_insert_barrier_event(cl_command_queue queue, cl_event event)
{
  cl_int i = 0;

  cl_event_add_ref(event);

  assert(queue != NULL);
  CL_OBJECT_LOCK(queue);

  if (queue->barrier_events == NULL) {
    queue->barrier_events_size = 4;
    queue->barrier_events = cl_calloc(queue->barrier_events_size, sizeof(cl_event));
    assert(queue->barrier_events);
  }

  for (i = 0; i<queue->barrier_events_num; i++) {
    assert(queue->barrier_events[i] != event);
  }

  if(queue->barrier_events_num < queue->barrier_events_size) {
    queue->barrier_events[queue->barrier_events_num++] = event;
    CL_OBJECT_UNLOCK(queue);
    return;
  }

  /* Array is full, double expand. */
  queue->barrier_events_size *= 2;
  queue->barrier_events = cl_realloc(queue->barrier_events,
                                     queue->barrier_events_size * sizeof(cl_event));
  assert(queue->barrier_events);

  queue->barrier_events[queue->barrier_events_num++] = event;
  CL_OBJECT_UNLOCK(queue);
  return;
}

LOCAL void
cl_command_queue_remove_barrier_event(cl_command_queue queue, cl_event event)
{
  cl_int i = 0;
  assert(queue != NULL);

  CL_OBJECT_LOCK(queue);

  assert(queue->barrier_events_num > 0);
  assert(queue->barrier_events);

  for(i = 0; i < queue->barrier_events_num; i++) {
    if(queue->barrier_events[i] == event)
      break;
  }
  assert(i < queue->barrier_events_num); // Must find it.

  if(i == queue->barrier_events_num - 1) { // The last one.
    queue->barrier_events[i] = NULL;
  } else {
    for(; i < queue->barrier_events_num - 1; i++) { // Move forward.
      queue->barrier_events[i] = queue->barrier_events[i+1];
    }
  }
  queue->barrier_events_num -= 1;
  CL_OBJECT_UNLOCK(queue);
  
  cl_event_delete(event);
}

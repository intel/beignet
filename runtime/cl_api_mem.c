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

#include "cl_alloc.h"
#include "cl_mem.h"
#include "cl_image.h"
#include "cl_enqueue.h"
#include "cl_command_queue.h"
#include "cl_event.h"
#include "cl_device_id.h"
#include "CL/cl.h"
#include <string.h>

cl_int
clSetMemObjectDestructorCallback(cl_mem memobj,
                                 void(CL_CALLBACK *pfn_notify)(cl_mem, void *),
                                 void *user_data)
{
  if (!CL_OBJECT_IS_MEM(memobj))
    return CL_INVALID_MEM_OBJECT;

  if (pfn_notify == NULL)
    return CL_INVALID_VALUE;

  return cl_mem_set_destructor_callback(memobj, pfn_notify, user_data);
}

cl_int
clGetMemObjectInfo(cl_mem memobj,
                   cl_mem_info param_name,
                   size_t param_value_size,
                   void *param_value,
                   size_t *param_value_size_ret)
{
  const void *src_ptr = NULL;
  size_t src_size = 0;
  cl_mem_object_type type;
  size_t offset;
  void *ptr;
  cl_int ref;
  cl_mem parent;
  cl_bool is_svm;

  if (!CL_OBJECT_IS_MEM(memobj)) {
    return CL_INVALID_MEM_OBJECT;
  }

  switch (param_name) {
  case CL_MEM_TYPE: {
    type = cl_mem_get_object_type(memobj);
    src_ptr = &type;
    src_size = sizeof(cl_mem_object_type);
    break;
  }
  case CL_MEM_FLAGS:
    src_ptr = &memobj->flags;
    src_size = sizeof(cl_mem_flags);
    break;
  case CL_MEM_SIZE:
    src_ptr = &memobj->size;
    src_size = sizeof(size_t);
    break;
  case CL_MEM_HOST_PTR: {
    ptr = memobj->host_ptr;
    src_ptr = &ptr;
    src_size = sizeof(void *);
    break;
  }
  case CL_MEM_USES_SVM_POINTER: {
    is_svm = CL_FALSE;
    if (CL_OBJECT_IS_BUFFER(memobj)) {
      is_svm = (cl_mem_to_buffer(memobj)->svm_buf != NULL);
    }
    src_ptr = &is_svm;
    src_size = sizeof(is_svm);
    break;
  }
  case CL_MEM_MAP_COUNT:
    ref = atomic_read(&memobj->map_ref);
    src_ptr = &ref;
    src_size = sizeof(cl_uint);
    break;
  case CL_MEM_REFERENCE_COUNT: {
    ref = CL_OBJECT_GET_REF(memobj);
    src_ptr = &ref;
    src_size = sizeof(cl_int);
    break;
  }
  case CL_MEM_CONTEXT:
    src_ptr = &memobj->ctx;
    src_size = sizeof(cl_context);
    break;
  case CL_MEM_ASSOCIATED_MEMOBJECT: {
    parent = NULL;
    if (CL_OBJECT_IS_SUB_BUFFER(memobj)) {
      cl_mem_buffer buf = cl_mem_to_buffer(memobj);
      parent = (cl_mem)(buf->parent);
    } else if (memobj->type == CL_MEM_IMAGE_TYPE) {
      parent = cl_mem_to_image(memobj)->mem_from;
    } else {
      parent = NULL;
    }
    src_ptr = &parent;
    src_size = sizeof(cl_mem);
    break;
  }
  case CL_MEM_OFFSET: {
    offset = 0;
    if (CL_OBJECT_IS_SUB_BUFFER(memobj)) {
      cl_mem_buffer buf = cl_mem_to_buffer(memobj);
      offset = buf->sub_offset;
    }
    src_ptr = &offset;
    src_size = sizeof(size_t);
    break;
  }
  default:
    return CL_INVALID_VALUE;
  }

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
}

cl_int
clGetImageInfo(cl_mem memobj,
               cl_image_info param_name,
               size_t param_value_size,
               void *param_value,
               size_t *param_value_size_ret)
{
  const void *src_ptr = NULL;
  size_t src_size = 0;
  struct _cl_mem_image *image;
  size_t height, depth, array_sz;
  cl_uint value;

  if (!CL_OBJECT_IS_IMAGE(memobj)) {
    return CL_INVALID_MEM_OBJECT;
  }
  image = cl_mem_to_image(memobj);

  switch (param_name) {
  case CL_IMAGE_FORMAT:
    src_ptr = &image->fmt;
    src_size = sizeof(cl_image_format);
    break;
  case CL_IMAGE_ELEMENT_SIZE:
    src_ptr = &image->bpp;
    src_size = sizeof(size_t);
    break;
  case CL_IMAGE_ROW_PITCH:
    src_ptr = &image->row_pitch;
    src_size = sizeof(size_t);
    break;
  case CL_IMAGE_SLICE_PITCH:
    src_ptr = &image->slice_pitch;
    src_size = sizeof(size_t);
    break;
  case CL_IMAGE_WIDTH:
    src_ptr = &image->w;
    src_size = sizeof(size_t);
    break;
  case CL_IMAGE_HEIGHT: {
    height = CL_OBJECT_IS_1D_IMAGE(image) ? 0 : image->h;
    src_ptr = &height;
    src_size = sizeof(size_t);
    break;
  }
  case CL_IMAGE_DEPTH: {
    depth = CL_OBJECT_IS_3D_IMAGE(image) ? image->depth : 0;
    src_ptr = &depth;
    src_size = sizeof(size_t);
    break;
  }
  case CL_IMAGE_ARRAY_SIZE: {
    array_sz = CL_OBJECT_IS_IMAGE_ARRAY(image) ? image->depth : 0;
    src_ptr = &array_sz;
    src_size = sizeof(size_t);
    break;
  }
  case CL_IMAGE_BUFFER:
    src_ptr = &image->mem_from;
    src_size = sizeof(cl_mem);
    break;
  case CL_IMAGE_NUM_MIP_LEVELS:
  case CL_IMAGE_NUM_SAMPLES: {
    value = 0;
    src_ptr = &value;
    src_size = sizeof(cl_uint);
    break;
  }
  default:
    return CL_INVALID_VALUE;
  }

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
}

cl_int
clGetPipeInfo(cl_mem mem,
              cl_pipe_info param_name,
              size_t param_value_size,
              void *param_value,
              size_t *param_value_size_ret)
{
  cl_mem_pipe pipe = NULL;
  const void *src_ptr = NULL;
  size_t src_size = 0;

  if (!CL_OBJECT_IS_PIPE(mem))
    return CL_INVALID_MEM_OBJECT;

  pipe = cl_mem_to_pipe(mem);

  switch (param_name) {
  case CL_PIPE_PACKET_SIZE:
    src_ptr = &pipe->packet_size;
    src_size = sizeof(cl_uint);
    break;
  case CL_PIPE_MAX_PACKETS:
    src_ptr = &pipe->max_packets;
    src_size = sizeof(cl_uint);
    break;
  default:
    return CL_INVALID_VALUE;
  }

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
}

void *
clEnqueueMapBuffer(cl_command_queue command_queue,
                   cl_mem buffer,
                   cl_bool blocking_map,
                   cl_map_flags map_flags,
                   size_t offset,
                   size_t size,
                   cl_uint num_events_in_wait_list,
                   const cl_event *event_wait_list,
                   cl_event *event,
                   cl_int *errcode_ret)
{
  cl_int err = CL_SUCCESS;
  void *ptr = NULL;
  cl_event e = NULL;
  cl_int e_status;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_BUFFER(buffer)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    if (command_queue->ctx != buffer->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (!size || offset + size > buffer->size) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (map_flags & (~(CL_MAP_READ | CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION))) {
      err = CL_INVALID_OPERATION;
      break;
    }

    if ((map_flags & CL_MAP_READ &&
         buffer->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) ||
        (map_flags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION) &&
         buffer->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))) {
      err = CL_INVALID_OPERATION;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_MAP_BUFFER, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueMapBuffer;
    e->exec_data.map_buffer.mem_obj = buffer;
    e->exec_data.map_buffer.offset = offset;
    e->exec_data.map_buffer.size = size;
    e->exec_data.map_buffer.ptr = NULL;
    e->exec_data.map_buffer.unsync_map = CL_FALSE;
    e->exec_data.map_buffer.write_map = CL_FALSE;
    if (map_flags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION))
      e->exec_data.map_buffer.write_map = CL_TRUE;

    if (blocking_map) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_finish(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    if (e_status == CL_COMPLETE) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      e->exec_data.map_buffer.unsync_map = CL_TRUE;
      err = cl_event_exec(e, CL_QUEUED, CL_TRUE);
      if (err != CL_SUCCESS) {
        break;
      }

      cl_command_queue_enqueue_event(command_queue, e);
    }

    ptr = e->exec_data.map_buffer.ptr;
    assert(ptr);
    assert(err == CL_SUCCESS);
  } while (0);

  if (err == CL_SUCCESS && event) {
    *event = e;
  } else {
    cl_event_delete(e);
  }

  if (errcode_ret)
    *errcode_ret = err;

  return ptr;
}

cl_int
clEnqueueUnmapMemObject(cl_command_queue command_queue,
                        cl_mem memobj,
                        void *mapped_ptr,
                        cl_uint num_events_in_wait_list,
                        const cl_event *event_wait_list,
                        cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_int e_status;
  cl_event e = NULL;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_MEM(memobj)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    if (command_queue->ctx != memobj->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_UNMAP_MEM_OBJECT, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueUnmapMemObject;
    e->exec_data.unmap.mem_obj = memobj;
    e->exec_data.unmap.ptr = mapped_ptr;

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    if (e_status == CL_COMPLETE) { // No need to wait
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else { // May need to wait some event to complete.
      err = cl_event_exec(e, CL_QUEUED, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
      cl_command_queue_enqueue_event(command_queue, e);
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
clEnqueueReadBuffer(cl_command_queue command_queue,
                    cl_mem buffer,
                    cl_bool blocking_read,
                    size_t offset,
                    size_t size,
                    void *ptr,
                    cl_uint num_events_in_wait_list,
                    const cl_event *event_wait_list,
                    cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_int e_status;
  cl_event e = NULL;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_BUFFER(buffer)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    if (command_queue->ctx != buffer->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (!ptr || !size || offset + size > buffer->size) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (buffer->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) {
      err = CL_INVALID_OPERATION;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_READ_BUFFER, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueReadBuffer;
    e->exec_data.read_write_buffer.buffer = buffer;
    e->exec_data.read_write_buffer.ptr = ptr;
    e->exec_data.read_write_buffer.offset = offset;
    e->exec_data.read_write_buffer.size = size;

    if (blocking_read) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_finish(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    if (e_status == CL_COMPLETE) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      err = cl_event_exec(e, CL_QUEUED, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
      cl_command_queue_enqueue_event(command_queue, e);
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
clEnqueueWriteBuffer(cl_command_queue command_queue,
                     cl_mem buffer,
                     cl_bool blocking_write,
                     size_t offset,
                     size_t size,
                     const void *ptr,
                     cl_uint num_events_in_wait_list,
                     const cl_event *event_wait_list,
                     cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_int e_status;
  cl_event e = NULL;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_BUFFER(buffer)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    if (command_queue->ctx != buffer->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (!ptr || !size || offset + size > buffer->size) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (buffer->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)) {
      err = CL_INVALID_OPERATION;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_WRITE_BUFFER, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueWriteBuffer;
    e->exec_data.read_write_buffer.buffer = buffer;
    e->exec_data.read_write_buffer.ptr = (void *)ptr;
    e->exec_data.read_write_buffer.offset = offset;
    e->exec_data.read_write_buffer.size = size;

    if (blocking_write) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_finish(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    if (e_status == CL_COMPLETE) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      err = cl_event_exec(e, CL_QUEUED, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
      cl_command_queue_enqueue_event(command_queue, e);
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
clEnqueueReadBufferRect(cl_command_queue command_queue,
                        cl_mem buffer,
                        cl_bool blocking_read,
                        const size_t *buffer_origin,
                        const size_t *host_origin,
                        const size_t *region,
                        size_t buffer_row_pitch,
                        size_t buffer_slice_pitch,
                        size_t host_row_pitch,
                        size_t host_slice_pitch,
                        void *ptr,
                        cl_uint num_events_in_wait_list,
                        const cl_event *event_wait_list,
                        cl_event *event)
{
  cl_int err = CL_SUCCESS;
  size_t total_size = 0;
  cl_int e_status;
  cl_event e = NULL;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_BUFFER(buffer)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    if (command_queue->ctx != buffer->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (buffer->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) {
      err = CL_INVALID_OPERATION;
      break;
    }

    if (!ptr || !region || region[0] == 0 || region[1] == 0 || region[2] == 0) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (buffer_row_pitch == 0)
      buffer_row_pitch = region[0];
    if (buffer_slice_pitch == 0)
      buffer_slice_pitch = region[1] * buffer_row_pitch;

    if (host_row_pitch == 0)
      host_row_pitch = region[0];
    if (host_slice_pitch == 0)
      host_slice_pitch = region[1] * host_row_pitch;

    if (buffer_row_pitch < region[0] ||
        host_row_pitch < region[0]) {
      err = CL_INVALID_VALUE;
      break;
    }

    if ((buffer_slice_pitch < region[1] * buffer_row_pitch || buffer_slice_pitch % buffer_row_pitch != 0) ||
        (host_slice_pitch < region[1] * host_row_pitch || host_slice_pitch % host_row_pitch != 0)) {
      err = CL_INVALID_VALUE;
      break;
    }

    total_size = (buffer_origin[2] + region[2] - 1) * buffer_slice_pitch +
                 (buffer_origin[1] + region[1] - 1) * buffer_row_pitch + buffer_origin[0] + region[0];
    if (total_size > buffer->size) {
      err = CL_INVALID_VALUE;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_READ_BUFFER_RECT, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueReadBufferRect;
    e->exec_data.read_write_buffer_rect.buffer = buffer;
    e->exec_data.read_write_buffer_rect.ptr = ptr;
    e->exec_data.read_write_buffer_rect.origin[0] = buffer_origin[0];
    e->exec_data.read_write_buffer_rect.origin[1] = buffer_origin[1];
    e->exec_data.read_write_buffer_rect.origin[2] = buffer_origin[2];
    e->exec_data.read_write_buffer_rect.host_origin[0] = host_origin[0];
    e->exec_data.read_write_buffer_rect.host_origin[1] = host_origin[1];
    e->exec_data.read_write_buffer_rect.host_origin[2] = host_origin[2];
    e->exec_data.read_write_buffer_rect.region[0] = region[0];
    e->exec_data.read_write_buffer_rect.region[1] = region[1];
    e->exec_data.read_write_buffer_rect.region[2] = region[2];
    e->exec_data.read_write_buffer_rect.row_pitch = buffer_row_pitch;
    e->exec_data.read_write_buffer_rect.slice_pitch = buffer_slice_pitch;
    e->exec_data.read_write_buffer_rect.host_row_pitch = host_row_pitch;
    e->exec_data.read_write_buffer_rect.host_slice_pitch = host_slice_pitch;

    if (blocking_read) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_finish(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    if (e_status == CL_COMPLETE) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      err = cl_event_exec(e, CL_QUEUED, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
      cl_command_queue_enqueue_event(command_queue, e);
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
clEnqueueWriteBufferRect(cl_command_queue command_queue,
                         cl_mem buffer,
                         cl_bool blocking_write,
                         const size_t *buffer_origin,
                         const size_t *host_origin,
                         const size_t *region,
                         size_t buffer_row_pitch,
                         size_t buffer_slice_pitch,
                         size_t host_row_pitch,
                         size_t host_slice_pitch,
                         const void *ptr,
                         cl_uint num_events_in_wait_list,
                         const cl_event *event_wait_list,
                         cl_event *event)
{
  cl_int err = CL_SUCCESS;
  size_t total_size = 0;
  cl_int e_status;
  cl_event e = NULL;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_BUFFER(buffer)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    if (command_queue->ctx != buffer->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (buffer->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)) {
      err = CL_INVALID_OPERATION;
      break;
    }

    if (!ptr || !region || region[0] == 0 || region[1] == 0 || region[2] == 0) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (buffer_row_pitch == 0)
      buffer_row_pitch = region[0];
    if (buffer_slice_pitch == 0)
      buffer_slice_pitch = region[1] * buffer_row_pitch;

    if (host_row_pitch == 0)
      host_row_pitch = region[0];
    if (host_slice_pitch == 0)
      host_slice_pitch = region[1] * host_row_pitch;

    if (buffer_row_pitch < region[0] ||
        host_row_pitch < region[0]) {
      err = CL_INVALID_VALUE;
      break;
    }

    if ((buffer_slice_pitch < region[1] * buffer_row_pitch || buffer_slice_pitch % buffer_row_pitch != 0) ||
        (host_slice_pitch < region[1] * host_row_pitch || host_slice_pitch % host_row_pitch != 0)) {
      err = CL_INVALID_VALUE;
      break;
    }

    total_size = (buffer_origin[2] + region[2] - 1) * buffer_slice_pitch +
                 (buffer_origin[1] + region[1] - 1) * buffer_row_pitch +
                 buffer_origin[0] + region[0];

    if (total_size > buffer->size) {
      err = CL_INVALID_VALUE;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_WRITE_BUFFER_RECT, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueWriteBufferRect;
    e->exec_data.read_write_buffer_rect.buffer = buffer;
    e->exec_data.read_write_buffer_rect.ptr = (void *)ptr;
    e->exec_data.read_write_buffer_rect.origin[0] = buffer_origin[0];
    e->exec_data.read_write_buffer_rect.origin[1] = buffer_origin[1];
    e->exec_data.read_write_buffer_rect.origin[2] = buffer_origin[2];
    e->exec_data.read_write_buffer_rect.host_origin[0] = host_origin[0];
    e->exec_data.read_write_buffer_rect.host_origin[1] = host_origin[1];
    e->exec_data.read_write_buffer_rect.host_origin[2] = host_origin[2];
    e->exec_data.read_write_buffer_rect.region[0] = region[0];
    e->exec_data.read_write_buffer_rect.region[1] = region[1];
    e->exec_data.read_write_buffer_rect.region[2] = region[2];
    e->exec_data.read_write_buffer_rect.row_pitch = buffer_row_pitch;
    e->exec_data.read_write_buffer_rect.slice_pitch = buffer_slice_pitch;
    e->exec_data.read_write_buffer_rect.host_row_pitch = host_row_pitch;
    e->exec_data.read_write_buffer_rect.host_slice_pitch = host_slice_pitch;

    if (blocking_write) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_finish(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    if (e_status == CL_COMPLETE) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      err = cl_event_exec(e, CL_QUEUED, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
      cl_command_queue_enqueue_event(command_queue, e);
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
clEnqueueCopyBuffer(cl_command_queue command_queue,
                    cl_mem src_buffer,
                    cl_mem dst_buffer,
                    size_t src_offset,
                    size_t dst_offset,
                    size_t cb,
                    cl_uint num_events_in_wait_list,
                    const cl_event *event_wait_list,
                    cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_event e = NULL;
  cl_int e_status;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_MEM(src_buffer)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }
    if (!CL_OBJECT_IS_MEM(dst_buffer)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    if (command_queue->ctx != src_buffer->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }
    if (command_queue->ctx != dst_buffer->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (src_offset + cb > src_buffer->size) {
      err = CL_INVALID_VALUE;
      break;
    }
    if (dst_offset + cb > dst_buffer->size) {
      err = CL_INVALID_VALUE;
      break;
    }

    /* Check overlap */
    if (src_buffer == dst_buffer && (src_offset <= dst_offset && dst_offset <= src_offset + cb - 1) &&
        (dst_offset <= src_offset && src_offset <= dst_offset + cb - 1)) {
      err = CL_MEM_COPY_OVERLAP;
      break;
    }

    /* Check sub overlap */
    if (src_buffer->type == CL_MEM_SUBBUFFER_TYPE && dst_buffer->type == CL_MEM_SUBBUFFER_TYPE) {
      struct _cl_mem_buffer *src_b = (struct _cl_mem_buffer *)src_buffer;
      struct _cl_mem_buffer *dst_b = (struct _cl_mem_buffer *)dst_buffer;
      size_t src_sub_offset = src_b->sub_offset;
      size_t dst_sub_offset = dst_b->sub_offset;
      if ((src_offset + src_sub_offset <= dst_offset + dst_sub_offset &&
           dst_offset + dst_sub_offset <= src_offset + src_sub_offset + cb - 1) &&
          (dst_offset + dst_sub_offset <= src_offset + src_sub_offset &&
           src_offset + src_sub_offset <= dst_offset + dst_sub_offset + cb - 1)) {
        err = CL_MEM_COPY_OVERLAP;
        break;
      }
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_COPY_BUFFER, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueCopyBuffer;
    e->exec_data.copy_buffer.src = src_buffer;
    e->exec_data.copy_buffer.dst = dst_buffer;
    e->exec_data.copy_buffer.src_offset = src_offset;
    e->exec_data.copy_buffer.dst_offset = dst_offset;
    e->exec_data.copy_buffer.cb = cb;

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    /* We will flush the ndrange if no event depend. Else we will add it to queue list.
       The finish or Complete status will always be done in queue list. */
    err = cl_event_exec(e, e_status == CL_COMPLETE ? CL_SUBMITTED : CL_QUEUED, CL_FALSE);
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

/* The following code checking overlap is from Appendix of openCL spec 1.1 */
static cl_bool
check_copy_overlap(const size_t src_offset[3],
                   const size_t dst_offset[3],
                   const size_t region[3],
                   size_t row_pitch, size_t slice_pitch)
{
  const size_t src_min[] = {src_offset[0], src_offset[1], src_offset[2]};
  const size_t src_max[] = {src_offset[0] + region[0],
                            src_offset[1] + region[1],
                            src_offset[2] + region[2]};
  const size_t dst_min[] = {dst_offset[0], dst_offset[1], dst_offset[2]};
  const size_t dst_max[] = {dst_offset[0] + region[0],
                            dst_offset[1] + region[1],
                            dst_offset[2] + region[2]};
  // Check for overlap
  cl_bool overlap = CL_TRUE;
  unsigned i;
  size_t dst_start = dst_offset[2] * slice_pitch +
                     dst_offset[1] * row_pitch + dst_offset[0];
  size_t dst_end = dst_start + (region[2] * slice_pitch +
                                region[1] * row_pitch + region[0]);
  size_t src_start = src_offset[2] * slice_pitch +
                     src_offset[1] * row_pitch + src_offset[0];
  size_t src_end = src_start + (region[2] * slice_pitch +
                                region[1] * row_pitch + region[0]);

  for (i = 0; i != 3; ++i) {
    overlap = overlap && (src_min[i] < dst_max[i]) && (src_max[i] > dst_min[i]);
  }

  if (!overlap) {
    size_t delta_src_x = (src_offset[0] + region[0] > row_pitch) ? src_offset[0] + region[0] - row_pitch : 0;
    size_t delta_dst_x = (dst_offset[0] + region[0] > row_pitch) ? dst_offset[0] + region[0] - row_pitch : 0;
    if ((delta_src_x > 0 && delta_src_x > dst_offset[0]) ||
        (delta_dst_x > 0 && delta_dst_x > src_offset[0])) {
      if ((src_start <= dst_start && dst_start < src_end) ||
          (dst_start <= src_start && src_start < dst_end))
        overlap = CL_TRUE;
    }
    if (region[2] > 1) {
      size_t src_height = slice_pitch / row_pitch;
      size_t dst_height = slice_pitch / row_pitch;
      size_t delta_src_y = (src_offset[1] + region[1] > src_height) ? src_offset[1] + region[1] - src_height : 0;
      size_t delta_dst_y = (dst_offset[1] + region[1] > dst_height) ? dst_offset[1] + region[1] - dst_height : 0;
      if ((delta_src_y > 0 && delta_src_y > dst_offset[1]) ||
          (delta_dst_y > 0 && delta_dst_y > src_offset[1])) {
        if ((src_start <= dst_start && dst_start < src_end) ||
            (dst_start <= src_start && src_start < dst_end))
          overlap = CL_TRUE;
      }
    }
  }
  return overlap;
}

cl_int
clEnqueueCopyBufferRect(cl_command_queue command_queue,
                        cl_mem src_buffer,
                        cl_mem dst_buffer,
                        const size_t *src_origin,
                        const size_t *dst_origin,
                        const size_t *region,
                        size_t src_row_pitch,
                        size_t src_slice_pitch,
                        size_t dst_row_pitch,
                        size_t dst_slice_pitch,
                        cl_uint num_events_in_wait_list,
                        const cl_event *event_wait_list,
                        cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_event e = NULL;
  size_t total_size = 0;
  cl_int e_status;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_MEM(src_buffer)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }
    if (!CL_OBJECT_IS_MEM(dst_buffer)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    if ((command_queue->ctx != src_buffer->ctx) ||
        (command_queue->ctx != dst_buffer->ctx)) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (!region || region[0] == 0 || region[1] == 0 || region[2] == 0) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (src_row_pitch == 0)
      src_row_pitch = region[0];
    if (src_slice_pitch == 0)
      src_slice_pitch = region[1] * src_row_pitch;

    if (dst_row_pitch == 0)
      dst_row_pitch = region[0];
    if (dst_slice_pitch == 0)
      dst_slice_pitch = region[1] * dst_row_pitch;

    if (src_row_pitch < region[0] ||
        dst_row_pitch < region[0]) {
      err = CL_INVALID_VALUE;
      break;
    }

    if ((src_slice_pitch < region[1] * src_row_pitch || src_slice_pitch % src_row_pitch != 0) ||
        (dst_slice_pitch < region[1] * dst_row_pitch || dst_slice_pitch % dst_row_pitch != 0)) {
      err = CL_INVALID_VALUE;
      break;
    }

    total_size = (src_origin[2] + region[2] - 1) * src_slice_pitch +
                 (src_origin[1] + region[1] - 1) * src_row_pitch + src_origin[0] + region[0];
    if (total_size > src_buffer->size) {
      err = CL_INVALID_VALUE;
      break;
    }
    total_size = (dst_origin[2] + region[2] - 1) * dst_slice_pitch +
                 (dst_origin[1] + region[1] - 1) * dst_row_pitch + dst_origin[0] + region[0];
    if (total_size > dst_buffer->size) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (src_buffer == dst_buffer &&
        (src_row_pitch != dst_row_pitch || src_slice_pitch != dst_slice_pitch)) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (src_buffer == dst_buffer &&
        check_copy_overlap(src_origin, dst_origin, region, src_row_pitch, src_slice_pitch)) {
      err = CL_MEM_COPY_OVERLAP;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_COPY_BUFFER_RECT, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueCopyBufferRect;
    e->exec_data.copy_buffer_rect.src_buf = src_buffer;
    e->exec_data.copy_buffer_rect.dst_buf = dst_buffer;
    e->exec_data.copy_buffer_rect.src_origin[0] = src_origin[0];
    e->exec_data.copy_buffer_rect.src_origin[1] = src_origin[1];
    e->exec_data.copy_buffer_rect.src_origin[2] = src_origin[2];
    e->exec_data.copy_buffer_rect.dst_origin[0] = dst_origin[0];
    e->exec_data.copy_buffer_rect.dst_origin[1] = dst_origin[1];
    e->exec_data.copy_buffer_rect.dst_origin[2] = dst_origin[2];
    e->exec_data.copy_buffer_rect.region[0] = region[0];
    e->exec_data.copy_buffer_rect.region[1] = region[1];
    e->exec_data.copy_buffer_rect.region[2] = region[2];
    e->exec_data.copy_buffer_rect.src_row_pitch = src_row_pitch;
    e->exec_data.copy_buffer_rect.src_slice_pitch = src_slice_pitch;
    e->exec_data.copy_buffer_rect.dst_row_pitch = dst_row_pitch;
    e->exec_data.copy_buffer_rect.dst_slice_pitch = dst_slice_pitch;

    /* We will flush the ndrange if no event depend. Else we will add it to queue list.
       The finish or Complete status will always be done in queue list. */
    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    } else if (e_status == CL_COMPLETE) {
      err = cl_event_exec(e, CL_SUBMITTED, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
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
clEnqueueFillBuffer(cl_command_queue command_queue,
                    cl_mem buffer,
                    const void *pattern,
                    size_t pattern_size,
                    size_t offset,
                    size_t size,
                    cl_uint num_events_in_wait_list,
                    const cl_event *event_wait_list,
                    cl_event *event)
{
  cl_int err = CL_SUCCESS;
  static size_t valid_sz[] = {1, 2, 4, 8, 16, 32, 64, 128};
  int i = 0;
  cl_event e = NULL;
  cl_int e_status;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_BUFFER(buffer)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    if (command_queue->ctx != buffer->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (offset + size > buffer->size) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (pattern == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    for (i = 0; i < sizeof(valid_sz) / sizeof(size_t); i++) {
      if (valid_sz[i] == pattern_size)
        break;
    }
    if (i == sizeof(valid_sz) / sizeof(size_t)) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (offset % pattern_size || size % pattern_size) {
      err = CL_INVALID_VALUE;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_FILL_BUFFER, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueFillBuffer;
    e->exec_data.fill_buffer.pattern = (void *)pattern;
    e->exec_data.fill_buffer.pattern_size = pattern_size;
    e->exec_data.fill_buffer.buffer = buffer;
    e->exec_data.fill_buffer.offset = offset;
    e->exec_data.fill_buffer.size = size;

    /* We will flush the ndrange if no event depend. Else we will add it to queue list.
       The finish or Complete status will always be done in queue list. */
    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    err = cl_event_exec(e, e_status == CL_COMPLETE ? CL_SUBMITTED : CL_QUEUED, CL_FALSE);
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
clEnqueueMigrateMemObjects(cl_command_queue command_queue,
                           cl_uint num_mem_objects,
                           const cl_mem *mem_objects,
                           cl_mem_migration_flags flags,
                           cl_uint num_events_in_wait_list,
                           const cl_event *event_wait_list,
                           cl_event *event)
{
  /* So far, we just support 1 device and no subdevice. So all the command queues
     belong to the small context. There is no need to migrate the mem objects by now. */
  cl_int err = CL_SUCCESS;
  cl_event e = NULL;
  cl_int e_status;
  cl_uint i = 0;

  do {
    if (!flags & CL_MIGRATE_MEM_OBJECT_HOST) {
      if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
        err = CL_INVALID_COMMAND_QUEUE;
        break;
      }
    }

    if (num_mem_objects == 0 || mem_objects == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (flags && flags & ~(CL_MIGRATE_MEM_OBJECT_HOST | CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED)) {
      err = CL_INVALID_VALUE;
      break;
    }

    for (i = 0; i < num_mem_objects; i++) {
      if (!CL_OBJECT_IS_MEM(mem_objects[i])) {
        err = CL_INVALID_MEM_OBJECT;
        break;
      }
      if (mem_objects[i]->ctx != command_queue->ctx) {
        err = CL_INVALID_CONTEXT;
        break;
      }
    }
    if (err != CL_SUCCESS) {
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_MIGRATE_MEM_OBJECTS, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    /* Noting to do now, just enqueue a event. */
    e->exec_data.type = EnqueueMigrateMemObj;
    /* We will flush the ndrange if no event depend. Else we will add it to queue list.
       The finish or Complete status will always be done in queue list. */
    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    err = cl_event_exec(e, e_status == CL_COMPLETE ? CL_SUBMITTED : CL_QUEUED, CL_FALSE);
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

/************************************ Images *********************************************/
static cl_int
check_image_region(struct _cl_mem_image *image, const size_t *pregion, size_t *region)
{
  if (pregion == NULL) {
    return CL_INVALID_VALUE;
  }

  if (image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
    region[0] = pregion[0];
    region[1] = 1;
    region[2] = pregion[1];
  } else {
    region[0] = pregion[0];
    region[1] = pregion[1];
    region[2] = pregion[2];
  }

  if ((region[0] == 0) || (region[1] == 0) || (region[2] == 0)) {
    return CL_INVALID_VALUE;
  }

  return CL_SUCCESS;
}

static cl_int
check_image_origin(struct _cl_mem_image *image, const size_t *porigin, size_t *origin)
{
  if (porigin == NULL) {
    return CL_INVALID_VALUE;
  }

  if (image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
    origin[0] = porigin[0];
    origin[1] = 0;
    origin[2] = porigin[1];
  } else {
    origin[0] = porigin[0];
    origin[1] = porigin[1];
    origin[2] = porigin[2];
  }

  return CL_SUCCESS;
}

void *
clEnqueueMapImage(cl_command_queue command_queue,
                  cl_mem mem,
                  cl_bool blocking_map,
                  cl_map_flags map_flags,
                  const size_t *porigin,
                  const size_t *pregion,
                  size_t *image_row_pitch,
                  size_t *image_slice_pitch,
                  cl_uint num_events_in_wait_list,
                  const cl_event *event_wait_list,
                  cl_event *event,
                  cl_int *errcode_ret)
{
  cl_int err = CL_SUCCESS;
  void *ptr = NULL;
  cl_mem_image image = NULL;
  cl_int e_status;
  size_t region[3];
  size_t origin[3];
  cl_event e = NULL;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_IMAGE(mem)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    image = cl_mem_to_image(mem);

    err = check_image_region(image, pregion, region);
    if (err != CL_SUCCESS) {
      break;
    }

    err = check_image_origin(image, porigin, origin);
    if (err != CL_SUCCESS) {
      break;
    }

    if (command_queue->ctx != mem->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (origin[0] + region[0] > image->w ||
        origin[1] + region[1] > image->h ||
        origin[2] + region[2] > image->depth) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (!image_row_pitch || (image->slice_pitch && !image_slice_pitch)) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (map_flags & (~(CL_MAP_READ | CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION))) {
      err = CL_INVALID_OPERATION;
      break;
    }

    if ((map_flags & CL_MAP_READ &&
         mem->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) ||
        (map_flags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION) &&
         mem->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))) {
      err = CL_INVALID_OPERATION;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_MAP_IMAGE, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueMapImage;
    e->exec_data.map_image.mem_obj = mem;
    e->exec_data.map_image.origin[0] = origin[0];
    e->exec_data.map_image.origin[1] = origin[1];
    e->exec_data.map_image.origin[2] = origin[2];
    e->exec_data.map_image.region[0] = region[0];
    e->exec_data.map_image.region[1] = region[1];
    e->exec_data.map_image.region[2] = region[2];
    e->exec_data.map_image.ptr = NULL;
    e->exec_data.map_image.unsync_map = CL_FALSE;
    if (map_flags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION))
      e->exec_data.map_image.write_map = 1;

    if (blocking_map) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_finish(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    if (e_status == CL_COMPLETE) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      e->exec_data.map_image.unsync_map = CL_TRUE;
      err = cl_event_exec(e, CL_QUEUED, CL_TRUE);
      if (err != CL_SUCCESS) {
        break;
      }

      cl_command_queue_enqueue_event(command_queue, e);
    }

    ptr = e->exec_data.map_image.ptr;
    assert(ptr);

    if (image_slice_pitch)
      *image_slice_pitch = e->exec_data.map_image.slice_pitch;
    if (image_row_pitch)
      *image_row_pitch = e->exec_data.map_image.row_pitch;
  } while (0);

  if (err != CL_SUCCESS) {
    if (e) {
      cl_event_delete(e);
      e = NULL;
    }

    assert(ptr == NULL);
  }

  if (err == CL_SUCCESS && event) {
    *event = e;
  } else {
    cl_event_delete(e);
  }

  if (errcode_ret)
    *errcode_ret = err;

  return ptr;
}

cl_int
clEnqueueReadImage(cl_command_queue command_queue,
                   cl_mem mem,
                   cl_bool blocking_read,
                   const size_t *porigin,
                   const size_t *pregion,
                   size_t row_pitch,
                   size_t slice_pitch,
                   void *ptr,
                   cl_uint num_events_in_wait_list,
                   const cl_event *event_wait_list,
                   cl_event *event)
{
  cl_int err = CL_SUCCESS;
  struct _cl_mem_image *image = NULL;
  cl_int e_status;
  size_t region[3];
  size_t origin[3];
  cl_event e = NULL;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_IMAGE(mem)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    image = cl_mem_to_image(mem);

    err = check_image_region(image, pregion, region);
    if (err != CL_SUCCESS) {
      break;
    }

    err = check_image_origin(image, porigin, origin);
    if (err != CL_SUCCESS) {
      break;
    }

    if (command_queue->ctx != mem->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (origin[0] + region[0] > image->w ||
        origin[1] + region[1] > image->h ||
        origin[2] + region[2] > image->depth) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (!row_pitch) {
      row_pitch = image->bpp * region[0];
    } else if (row_pitch < image->bpp * region[0]) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (image->slice_pitch) {
      if (!slice_pitch) {
        slice_pitch = row_pitch * region[1];
      } else if (slice_pitch < row_pitch * region[1]) {
        err = CL_INVALID_VALUE;
        break;
      }
    } else if (slice_pitch) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (!ptr) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (mem->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) {
      err = CL_INVALID_OPERATION;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_READ_IMAGE, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueReadImage;
    e->exec_data.read_write_image.image = mem;
    e->exec_data.read_write_image.ptr = ptr;
    e->exec_data.read_write_image.origin[0] = origin[0];
    e->exec_data.read_write_image.origin[1] = origin[1];
    e->exec_data.read_write_image.origin[2] = origin[2];
    e->exec_data.read_write_image.region[0] = region[0];
    e->exec_data.read_write_image.region[1] = region[1];
    e->exec_data.read_write_image.region[2] = region[2];
    e->exec_data.read_write_image.row_pitch = row_pitch;
    e->exec_data.read_write_image.slice_pitch = slice_pitch;

    if (blocking_read) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_finish(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    if (e_status == CL_COMPLETE) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      err = cl_event_exec(e, CL_QUEUED, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
      cl_command_queue_enqueue_event(command_queue, e);
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
clEnqueueWriteImage(cl_command_queue command_queue,
                    cl_mem mem,
                    cl_bool blocking_write,
                    const size_t *porigin,
                    const size_t *pregion,
                    size_t row_pitch,
                    size_t slice_pitch,
                    const void *ptr,
                    cl_uint num_events_in_wait_list,
                    const cl_event *event_wait_list,
                    cl_event *event)
{
  cl_int err = CL_SUCCESS;
  struct _cl_mem_image *image = NULL;
  cl_int e_status;
  size_t region[3];
  size_t origin[3];
  cl_event e = NULL;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_IMAGE(mem)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    image = cl_mem_to_image(mem);

    err = check_image_region(image, pregion, region);
    if (err != CL_SUCCESS) {
      break;
    }

    err = check_image_origin(image, porigin, origin);
    if (err != CL_SUCCESS) {
      break;
    }

    if (command_queue->ctx != mem->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (origin[0] + region[0] > image->w ||
        origin[1] + region[1] > image->h ||
        origin[2] + region[2] > image->depth) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (!row_pitch) {
      row_pitch = image->bpp * region[0];
    } else if (row_pitch < image->bpp * region[0]) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (image->slice_pitch) {
      if (!slice_pitch) {
        slice_pitch = row_pitch * region[1];
      } else if (slice_pitch < row_pitch * region[1]) {
        err = CL_INVALID_VALUE;
        break;
      }
    } else if (slice_pitch) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (!ptr) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (mem->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)) {
      err = CL_INVALID_OPERATION;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_WRITE_IMAGE, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueWriteImage;
    e->exec_data.read_write_image.image = mem;
    e->exec_data.read_write_image.ptr = (void *)ptr;
    e->exec_data.read_write_image.origin[0] = origin[0];
    e->exec_data.read_write_image.origin[1] = origin[1];
    e->exec_data.read_write_image.origin[2] = origin[2];
    e->exec_data.read_write_image.region[0] = region[0];
    e->exec_data.read_write_image.region[1] = region[1];
    e->exec_data.read_write_image.region[2] = region[2];
    e->exec_data.read_write_image.row_pitch = row_pitch;
    e->exec_data.read_write_image.slice_pitch = slice_pitch;

    if (blocking_write) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_finish(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    if (e_status == CL_COMPLETE) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      err = cl_event_exec(e, CL_QUEUED, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
      cl_command_queue_enqueue_event(command_queue, e);
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
clEnqueueCopyImage(cl_command_queue command_queue,
                   cl_mem src_mem,
                   cl_mem dst_mem,
                   const size_t *psrc_origin,
                   const size_t *pdst_origin,
                   const size_t *pregion,
                   cl_uint num_events_in_wait_list,
                   const cl_event *event_wait_list,
                   cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_bool overlap = CL_TRUE;
  cl_int i = 0;
  cl_event e = NULL;
  struct _cl_mem_image *src_image = NULL;
  struct _cl_mem_image *dst_image = NULL;
  size_t region[3];
  size_t src_origin[3];
  size_t dst_origin[3];
  cl_int e_status;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_IMAGE(src_mem)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }
    if (!CL_OBJECT_IS_IMAGE(dst_mem)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    src_image = cl_mem_to_image(src_mem);
    dst_image = cl_mem_to_image(dst_mem);

    err = check_image_region(src_image, pregion, region);
    if (err != CL_SUCCESS) {
      break;
    }

    err = check_image_origin(src_image, psrc_origin, src_origin);
    if (err != CL_SUCCESS) {
      break;
    }
    err = check_image_origin(dst_image, pdst_origin, dst_origin);
    if (err != CL_SUCCESS) {
      break;
    }

    if (command_queue->ctx != src_mem->ctx ||
        command_queue->ctx != dst_mem->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (src_image->fmt.image_channel_order != dst_image->fmt.image_channel_order ||
        src_image->fmt.image_channel_data_type != dst_image->fmt.image_channel_data_type) {
      err = CL_IMAGE_FORMAT_MISMATCH;
      break;
    }

    if (src_origin[0] + region[0] > src_image->w ||
        src_origin[1] + region[1] > src_image->h ||
        src_origin[2] + region[2] > src_image->depth) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (dst_origin[0] + region[0] > dst_image->w ||
        dst_origin[1] + region[1] > dst_image->h ||
        dst_origin[2] + region[2] > dst_image->depth) {
      err = CL_INVALID_VALUE;
      break;
    }

    if ((src_image->image_type == CL_MEM_OBJECT_IMAGE2D && (src_origin[2] != 0 || region[2] != 1)) ||
        (dst_image->image_type == CL_MEM_OBJECT_IMAGE2D && (dst_origin[2] != 0 || region[2] != 1))) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (src_image == dst_image) {
      for (i = 0; i < 3; i++) {
        overlap = overlap && (src_origin[i] < dst_origin[i] + region[i]) &&
                  (dst_origin[i] < src_origin[i] + region[i]);
      }
      if (overlap == CL_TRUE) {
        err = CL_MEM_COPY_OVERLAP;
        break;
      }
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_COPY_IMAGE, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueCopyImage;
    e->exec_data.copy_image.src_image = src_mem;
    e->exec_data.copy_image.dst_image = dst_mem;
    e->exec_data.copy_image.src_origin[0] = src_origin[0];
    e->exec_data.copy_image.src_origin[1] = src_origin[1];
    e->exec_data.copy_image.src_origin[2] = src_origin[2];
    e->exec_data.copy_image.dst_origin[0] = dst_origin[0];
    e->exec_data.copy_image.dst_origin[1] = dst_origin[1];
    e->exec_data.copy_image.dst_origin[2] = dst_origin[2];
    e->exec_data.copy_image.region[0] = region[0];
    e->exec_data.copy_image.region[1] = region[1];
    e->exec_data.copy_image.region[2] = region[2];

    /* We will flush the ndrange if no event depend. Else we will add it to queue list.
       The finish or Complete status will always be done in queue list. */
    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    err = cl_event_exec(e, e_status == CL_COMPLETE ? CL_SUBMITTED : CL_QUEUED, CL_FALSE);
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
clEnqueueCopyImageToBuffer(cl_command_queue command_queue,
                           cl_mem src_mem,
                           cl_mem dst_buffer,
                           const size_t *psrc_origin,
                           const size_t *pregion,
                           size_t dst_offset,
                           cl_uint num_events_in_wait_list,
                           const cl_event *event_wait_list,
                           cl_event *event)
{
  cl_int err = CL_SUCCESS;
  struct _cl_mem_image *src_image = NULL;
  size_t region[3];
  size_t src_origin[3];
  cl_event e = NULL;
  cl_int e_status;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_IMAGE(src_mem)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }
    if (!CL_OBJECT_IS_BUFFER(dst_buffer)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    src_image = cl_mem_to_image(src_mem);

    err = check_image_region(src_image, pregion, region);
    if (err != CL_SUCCESS) {
      break;
    }

    err = check_image_origin(src_image, psrc_origin, src_origin);
    if (err != CL_SUCCESS) {
      break;
    }

    if (command_queue->ctx != src_mem->ctx ||
        command_queue->ctx != dst_buffer->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (dst_offset + region[0] * region[1] * region[2] * src_image->bpp > dst_buffer->size) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (src_origin[0] + region[0] > src_image->w ||
        src_origin[1] + region[1] > src_image->h ||
        src_origin[2] + region[2] > src_image->depth) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (src_image->image_type == CL_MEM_OBJECT_IMAGE2D && (src_origin[2] != 0 || region[2] != 1)) {
      err = CL_INVALID_VALUE;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_COPY_IMAGE_TO_BUFFER, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueCopyImageToBuffer;
    e->exec_data.copy_image_and_buffer.image = src_mem;
    e->exec_data.copy_image_and_buffer.buffer = dst_buffer;
    e->exec_data.copy_image_and_buffer.offset = dst_offset;
    e->exec_data.copy_image_and_buffer.origin[0] = src_origin[0];
    e->exec_data.copy_image_and_buffer.origin[1] = src_origin[1];
    e->exec_data.copy_image_and_buffer.origin[2] = src_origin[2];
    e->exec_data.copy_image_and_buffer.region[0] = region[0];
    e->exec_data.copy_image_and_buffer.region[1] = region[1];
    e->exec_data.copy_image_and_buffer.region[2] = region[2];

    /* We will flush the ndrange if no event depend. Else we will add it to queue list.
       The finish or Complete status will always be done in queue list. */
    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    err = cl_event_exec(e, e_status == CL_COMPLETE ? CL_SUBMITTED : CL_QUEUED, CL_FALSE);
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
clEnqueueCopyBufferToImage(cl_command_queue command_queue,
                           cl_mem src_buffer,
                           cl_mem dst_mem,
                           size_t src_offset,
                           const size_t *pdst_origin,
                           const size_t *pregion,
                           cl_uint num_events_in_wait_list,
                           const cl_event *event_wait_list,
                           cl_event *event)
{
  cl_int err = CL_SUCCESS;
  struct _cl_mem_image *dst_image = NULL;
  size_t region[3];
  size_t dst_origin[3];
  cl_event e = NULL;
  cl_int e_status;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_BUFFER(src_buffer)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }
    if (!CL_OBJECT_IS_IMAGE(dst_mem)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    dst_image = cl_mem_to_image(dst_mem);

    err = check_image_region(dst_image, pregion, region);
    if (err != CL_SUCCESS) {
      break;
    }

    err = check_image_origin(dst_image, pdst_origin, dst_origin);
    if (err != CL_SUCCESS) {
      break;
    }

    if (command_queue->ctx != src_buffer->ctx ||
        command_queue->ctx != dst_mem->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (src_offset + region[0] * region[1] * region[2] * dst_image->bpp > src_buffer->size) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (dst_origin[0] + region[0] > dst_image->w ||
        dst_origin[1] + region[1] > dst_image->h ||
        dst_origin[2] + region[2] > dst_image->depth) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (dst_image->image_type == CL_MEM_OBJECT_IMAGE2D && (dst_origin[2] != 0 || region[2] != 1)) {
      err = CL_INVALID_VALUE;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_COPY_BUFFER_TO_IMAGE, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueCopyBufferToImage;
    e->exec_data.copy_image_and_buffer.image = dst_mem;
    e->exec_data.copy_image_and_buffer.buffer = src_buffer;
    e->exec_data.copy_image_and_buffer.offset = src_offset;
    e->exec_data.copy_image_and_buffer.origin[0] = dst_origin[0];
    e->exec_data.copy_image_and_buffer.origin[1] = dst_origin[1];
    e->exec_data.copy_image_and_buffer.origin[2] = dst_origin[2];
    e->exec_data.copy_image_and_buffer.region[0] = region[0];
    e->exec_data.copy_image_and_buffer.region[1] = region[1];
    e->exec_data.copy_image_and_buffer.region[2] = region[2];

    /* We will flush the ndrange if no event depend. Else we will add it to queue list.
       The finish or Complete status will always be done in queue list. */
    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    err = cl_event_exec(e, e_status == CL_COMPLETE ? CL_SUBMITTED : CL_QUEUED, CL_FALSE);
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
clEnqueueFillImage(cl_command_queue command_queue,
                   cl_mem mem,
                   const void *fill_color,
                   const size_t *porigin,
                   const size_t *pregion,
                   cl_uint num_events_in_wait_list,
                   const cl_event *event_wait_list,
                   cl_event *event)
{
  cl_int err = CL_SUCCESS;
  size_t region[3];
  size_t origin[3];
  cl_event e = NULL;
  struct _cl_mem_image *image = NULL;
  cl_int e_status;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (!CL_OBJECT_IS_IMAGE(mem)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    image = cl_mem_to_image(mem);

    err = check_image_region(image, pregion, region);
    if (err != CL_SUCCESS) {
      break;
    }

    err = check_image_origin(image, porigin, origin);
    if (err != CL_SUCCESS) {
      break;
    }

    if (command_queue->ctx != mem->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (fill_color == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (origin[0] + region[0] > image->w ||
        origin[1] + region[1] > image->h ||
        origin[2] + region[2] > image->depth) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (image->image_type == CL_MEM_OBJECT_IMAGE2D && (origin[2] != 0 || region[2] != 1)) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (image->image_type == CL_MEM_OBJECT_IMAGE1D && (origin[2] != 0 || origin[1] != 0 ||
                                                       region[2] != 1 || region[1] != 1)) {
      err = CL_INVALID_VALUE;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_FILL_IMAGE, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueFillImage;
    e->exec_data.fill_image.image = mem;
    e->exec_data.fill_image.origin[0] = origin[0];
    e->exec_data.fill_image.origin[1] = origin[1];
    e->exec_data.fill_image.origin[2] = origin[2];
    e->exec_data.fill_image.region[0] = region[0];
    e->exec_data.fill_image.region[1] = region[1];
    e->exec_data.fill_image.region[2] = region[2];
    e->exec_data.fill_image.pattern = (void *)fill_color;

    /* We will flush the ndrange if no event depend. Else we will add it to queue list.
       The finish or Complete status will always be done in queue list. */
    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    err = cl_event_exec(e, e_status == CL_COMPLETE ? CL_SUBMITTED : CL_QUEUED, CL_FALSE);
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
clRetainMemObject(cl_mem memobj)
{
  if (!CL_OBJECT_IS_MEM(memobj)) {
    return CL_INVALID_MEM_OBJECT;
  }

  cl_mem_add_ref(memobj);
  return CL_SUCCESS;
}

cl_int
clReleaseMemObject(cl_mem memobj)
{
  if (!CL_OBJECT_IS_MEM(memobj)) {
    return CL_INVALID_MEM_OBJECT;
  }

  cl_mem_delete(memobj);
  return CL_SUCCESS;
}

cl_mem
clCreatePipe(cl_context context,
             cl_mem_flags flags,
             cl_uint pipe_packet_size,
             cl_uint pipe_max_packets,
             const cl_pipe_properties *properties,
             cl_int *errcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  cl_uint device_max_size = 0;
  cl_int i;

  do {
    if (!CL_OBJECT_IS_CONTEXT(context)) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if ((flags & ~(CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS)) != 0) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (properties != NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (pipe_packet_size == 0 || pipe_max_packets == 0) {
      err = CL_INVALID_PIPE_SIZE;
      break;
    }

    for (i = 0; i < context->device_num; i++) {
      if ((err = cl_device_get_info(context->devices[i], CL_DEVICE_PIPE_MAX_PACKET_SIZE,
                                    sizeof(device_max_size), &device_max_size, NULL)) != CL_SUCCESS) {
        break;
      }

      if (pipe_packet_size > device_max_size) {
        err = CL_INVALID_PIPE_SIZE;
        break;
      }
    }
    if (err != CL_SUCCESS)
      break;

    if (flags == 0)
      flags = CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS;

    mem = cl_mem_create_pipe(context, flags, pipe_packet_size, pipe_max_packets, &err);
  } while (0);

  if (errcode_ret)
    *errcode_ret = err;
  return mem;
}

cl_mem
clCreateBuffer(cl_context context,
               cl_mem_flags flags,
               size_t size,
               void *host_ptr,
               cl_int *errcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  cl_ulong max_mem_size;
  cl_int i;

  do {
    if (!CL_OBJECT_IS_CONTEXT(context)) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (UNLIKELY(size == 0)) {
      err = CL_INVALID_BUFFER_SIZE;
      break;
    }

    /* Possible mem flag combination:
         CL_MEM_ALLOC_HOST_PTR
         CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR
         CL_MEM_USE_HOST_PTR
         CL_MEM_COPY_HOST_PTR */
    if (((flags & CL_MEM_READ_WRITE) && (flags & (CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY))) ||
        ((flags & CL_MEM_READ_ONLY) && (flags & (CL_MEM_WRITE_ONLY))) ||
        ((flags & CL_MEM_ALLOC_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR)) ||
        ((flags & CL_MEM_COPY_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR)) ||
        ((flags & CL_MEM_HOST_READ_ONLY) && (flags & CL_MEM_HOST_NO_ACCESS)) ||
        ((flags & CL_MEM_HOST_READ_ONLY) && (flags & CL_MEM_HOST_WRITE_ONLY)) ||
        ((flags & CL_MEM_HOST_WRITE_ONLY) && (flags & CL_MEM_HOST_NO_ACCESS)) ||
        ((flags & (~(CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY |
                     CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR |
                     CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))) != 0)) {
      err = CL_INVALID_VALUE;
      break;
    }

    /* This flag is valid only if host_ptr is not NULL */
    if ((((flags & CL_MEM_COPY_HOST_PTR) || (flags & CL_MEM_USE_HOST_PTR)) && host_ptr == NULL) ||
        (!(flags & (CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR)) && (host_ptr != NULL))) {
      err = CL_INVALID_HOST_PTR;
      break;
    }

    for (i = 0; i < context->device_num; i++) {
      if ((err = cl_device_get_info(context->devices[i], CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                                    sizeof(max_mem_size), &max_mem_size, NULL)) != CL_SUCCESS) {
        break;
      }
    }
    if (err != CL_SUCCESS)
      break;

    if (size > max_mem_size) {
      err = CL_INVALID_BUFFER_SIZE;
      break;
    }

    mem = cl_mem_create_buffer(context, flags, size, host_ptr, &err);
  } while (0);

  if (errcode_ret)
    *errcode_ret = err;

  return mem;
}

cl_mem
clCreateSubBuffer(cl_mem buffer,
                  cl_mem_flags flags,
                  cl_buffer_create_type buffer_create_type,
                  const void *buffer_create_info,
                  cl_int *errcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  cl_buffer_region *info = NULL;
  cl_int i;

  do {
    if (!CL_OBJECT_IS_BUFFER(buffer)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    // Can not create sub buffer for sub buffer.
    if (buffer->type != CL_MEM_BUFFER_TYPE) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    if (flags &&
        (((buffer->flags & CL_MEM_WRITE_ONLY) && (flags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY))) ||
         ((buffer->flags & CL_MEM_READ_ONLY) && (flags & (CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY))) ||
         (flags & (CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR)) ||
         ((flags & CL_MEM_HOST_READ_ONLY) && (flags & CL_MEM_HOST_NO_ACCESS)) ||
         ((flags & CL_MEM_HOST_READ_ONLY) && (flags & CL_MEM_HOST_WRITE_ONLY)) ||
         ((flags & CL_MEM_HOST_WRITE_ONLY) && (flags & CL_MEM_HOST_NO_ACCESS)))) {
      err = CL_INVALID_VALUE;
      break;
    }

    /* Inherit some flags from parent. */
    if ((flags & (CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY | CL_MEM_READ_WRITE)) == 0) {
      flags |= buffer->flags & (CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY | CL_MEM_READ_WRITE);
    }

    flags |= buffer->flags & (CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR);
    if ((flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)) == 0) {
      flags |= buffer->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS);
    }

    if (buffer_create_type != CL_BUFFER_CREATE_TYPE_REGION) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (buffer_create_info == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }
    info = (cl_buffer_region *)buffer_create_info;

    if (!info->size) {
      err = CL_INVALID_BUFFER_SIZE;
      break;
    }

    if (info->origin > buffer->size || info->origin + info->size > buffer->size) {
      err = CL_INVALID_VALUE;
      break;
    }

    for (i = 0; i < buffer->ctx->device_num; i++) {
      if (info->origin & (buffer->ctx->devices[0]->mem_base_addr_align / 8 - 1)) {
        err = CL_MISALIGNED_SUB_BUFFER_OFFSET;
        break;
      }
    }
    if (err != CL_SUCCESS)
      break;

    mem = cl_mem_create_sub_buffer(buffer, flags, buffer_create_type, buffer_create_info, &err);
  } while (0);

  if (errcode_ret)
    *errcode_ret = err;
  return mem;
}

cl_mem
clCreateImage(cl_context context,
              cl_mem_flags flags,
              const cl_image_format *image_format,
              const cl_image_desc *image_desc,
              void *host_ptr,
              cl_int *errcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;

  do {
    if (!CL_OBJECT_IS_CONTEXT(context)) {
      err = CL_INVALID_MEM_OBJECT;
      break;
    }

    /* This flag is valid only if host_ptr is not NULL */
    if ((((flags & CL_MEM_COPY_HOST_PTR) || (flags & CL_MEM_USE_HOST_PTR)) && host_ptr == NULL) ||
        (!(flags & (CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR)) && (host_ptr != NULL))) {
      err = CL_INVALID_HOST_PTR;
      break;
    }

    /* Possible mem flag combination:
         CL_MEM_ALLOC_HOST_PTR
         CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR
         CL_MEM_USE_HOST_PTR
         CL_MEM_COPY_HOST_PTR */
    if (((flags & CL_MEM_READ_WRITE) && (flags & (CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY))) ||
        ((flags & CL_MEM_READ_ONLY) && (flags & (CL_MEM_WRITE_ONLY))) ||
        ((flags & CL_MEM_ALLOC_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR)) ||
        ((flags & CL_MEM_COPY_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR)) ||
        ((flags & CL_MEM_HOST_READ_ONLY) && (flags & CL_MEM_HOST_NO_ACCESS)) ||
        ((flags & CL_MEM_HOST_READ_ONLY) && (flags & CL_MEM_HOST_WRITE_ONLY)) ||
        ((flags & CL_MEM_HOST_WRITE_ONLY) && (flags & CL_MEM_HOST_NO_ACCESS)) ||
        ((flags & (~(CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY |
                     CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR |
                     CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))) != 0)) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (image_format == NULL) {
      err = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
      break;
    }
    if (image_format->image_channel_order < CL_R ||
        (image_format->image_channel_order > CL_sBGRA && image_format->image_channel_order != CL_NV12_INTEL)) {
      err = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
      break;
    }
    if (image_format->image_channel_data_type < CL_SNORM_INT8 ||
        image_format->image_channel_data_type > CL_FLOAT) {
      err = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
      break;
    }

    if (image_desc == NULL) {
      err = CL_INVALID_IMAGE_DESCRIPTOR;
      break;
    }
    if (image_desc->image_type <= CL_MEM_OBJECT_BUFFER ||
        image_desc->image_type > CL_MEM_OBJECT_IMAGE1D_BUFFER) {
      err = CL_INVALID_IMAGE_DESCRIPTOR;
      break;
    }

    /* buffer refers to a valid buffer memory object if image_type is
     CL_MEM_OBJECT_IMAGE1D_BUFFER or CL_MEM_OBJECT_IMAGE2D. Otherwise it must be NULL. */
    if (image_desc->image_type != CL_MEM_OBJECT_IMAGE1D_BUFFER &&
        image_desc->image_type != CL_MEM_OBJECT_IMAGE2D &&
        image_desc->buffer) {
      err = CL_INVALID_IMAGE_DESCRIPTOR;
      break;
    }

    if (image_desc->num_mip_levels || image_desc->num_samples) {
      err = CL_INVALID_IMAGE_DESCRIPTOR;
      break;
    }

    if (image_desc->buffer) {
      if (!CL_OBJECT_IS_BUFFER(image_desc->buffer) && !CL_OBJECT_IS_IMAGE(image_desc->buffer)) {
        err = CL_INVALID_IMAGE_DESCRIPTOR;
        break;
      }

      if (image_desc->buffer->ctx != context) {
        err = CL_INVALID_IMAGE_DESCRIPTOR;
        break;
      }

      if (flags & (CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR)) {
        err = CL_INVALID_VALUE;
        break;
      }

      /* access check. */
      if ((image_desc->buffer->flags & CL_MEM_WRITE_ONLY) &&
          (flags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY))) {
        err = CL_INVALID_VALUE;
        break;
      }
      if ((image_desc->buffer->flags & CL_MEM_READ_ONLY) &&
          (flags & (CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY))) {
        err = CL_INVALID_VALUE;
        break;
      }
      if ((image_desc->buffer->flags & CL_MEM_HOST_WRITE_ONLY) &&
          (flags & CL_MEM_HOST_READ_ONLY)) {
        err = CL_INVALID_VALUE;
        break;
      }
      if ((image_desc->buffer->flags & CL_MEM_HOST_READ_ONLY) &&
          (flags & CL_MEM_HOST_WRITE_ONLY)) {
        err = CL_INVALID_VALUE;
        break;
      }
      if ((image_desc->buffer->flags & CL_MEM_HOST_NO_ACCESS) &&
          (flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY))) {
        err = CL_INVALID_VALUE;
        break;
      }
    }

    /* Sepcial case for NV12 image */
    if (image_format->image_channel_order == CL_NV12_INTEL &&
        (image_format->image_channel_data_type != CL_UNORM_INT8 ||
         image_desc->image_width % 4 || image_desc->image_height % 4)) {
      err = CL_INVALID_IMAGE_DESCRIPTOR;
      break;
    }

    mem = cl_mem_create_image(context, flags, image_format, image_desc, host_ptr, &err);
  } while (0);

  /* Other details check for image_desc will leave to image create. */
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
}

cl_mem
clCreateImage2D(cl_context context,
                cl_mem_flags flags,
                const cl_image_format *image_format,
                size_t image_width,
                size_t image_height,
                size_t image_row_pitch,
                void *host_ptr,
                cl_int *errcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  cl_image_desc image_desc;

  if (!CL_OBJECT_IS_CONTEXT(context)) {
    if (errcode_ret)
      *errcode_ret = CL_INVALID_CONTEXT;
    return NULL;
  }

  memset(&image_desc, 0, sizeof(image_desc));
  image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  image_desc.image_width = image_width;
  image_desc.image_height = image_height;
  image_desc.image_row_pitch = image_row_pitch;

  mem = cl_mem_create_image(context, flags, image_format, &image_desc, host_ptr, &err);

  if (errcode_ret)
    *errcode_ret = err;
  return mem;
}

cl_mem
clCreateImage3D(cl_context context,
                cl_mem_flags flags,
                const cl_image_format *image_format,
                size_t image_width,
                size_t image_height,
                size_t image_depth,
                size_t image_row_pitch,
                size_t image_slice_pitch,
                void *host_ptr,
                cl_int *errcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  cl_image_desc image_desc;

  if (!CL_OBJECT_IS_CONTEXT(context)) {
    if (errcode_ret)
      *errcode_ret = CL_INVALID_CONTEXT;
    return NULL;
  }

  memset(&image_desc, 0, sizeof(image_desc));
  image_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
  image_desc.image_width = image_width;
  image_desc.image_height = image_height;
  image_desc.image_depth = image_depth;
  image_desc.image_row_pitch = image_row_pitch;
  image_desc.image_slice_pitch = image_slice_pitch;

  mem = cl_mem_create_image(context, flags, image_format, &image_desc, host_ptr, &err);

  if (errcode_ret)
    *errcode_ret = err;
  return mem;
}

cl_int
clGetSupportedImageFormats(cl_context ctx,
                           cl_mem_flags flags,
                           cl_mem_object_type image_type,
                           cl_uint num_entries,
                           cl_image_format *image_formats,
                           cl_uint *num_image_formats)
{
  cl_int err = CL_SUCCESS;

  if (!CL_OBJECT_IS_CONTEXT(ctx)) {
    return CL_INVALID_CONTEXT;
  }

  if (num_entries == 0 && image_formats != NULL) {
    return CL_INVALID_VALUE;
  }

  if (image_type != CL_MEM_OBJECT_IMAGE1D &&
      image_type != CL_MEM_OBJECT_IMAGE1D_ARRAY &&
      image_type != CL_MEM_OBJECT_IMAGE1D_BUFFER &&
      image_type != CL_MEM_OBJECT_IMAGE2D_ARRAY &&
      image_type != CL_MEM_OBJECT_IMAGE2D &&
      image_type != CL_MEM_OBJECT_IMAGE3D) {
    return CL_INVALID_VALUE;
  }

  err = cl_image_get_supported_fmt(ctx, flags, image_type, num_entries,
                                   image_formats, num_image_formats);

  return err;
}

void *
clSVMAlloc(cl_context context,
           cl_svm_mem_flags flags,
           size_t size,
           unsigned int alignment)
{
  cl_int err = CL_SUCCESS;
  size_t max_mem_size;
  cl_int i;

  if (!CL_OBJECT_IS_CONTEXT(context))
    return NULL;

  if (size == 0)
    return NULL;

  if (alignment & (alignment - 1)) // not the power of 2
    return NULL;

  for (i = 0; i < context->device_num; i++) {
    if ((err = cl_device_get_info(context->devices[i], CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                                  sizeof(max_mem_size), &max_mem_size, NULL)) != CL_SUCCESS)
      return NULL;

    if (size > max_mem_size)
      return NULL;
  }

  if (flags & (~(CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY |
                 CL_MEM_SVM_FINE_GRAIN_BUFFER | CL_MEM_SVM_ATOMICS)))
    return NULL; // Unrecognized flag

  if (flags && (((flags & CL_MEM_WRITE_ONLY) && (flags & CL_MEM_READ_ONLY)) ||
                ((flags & CL_MEM_WRITE_ONLY) && (flags & CL_MEM_READ_WRITE)) ||
                ((flags & CL_MEM_READ_ONLY) && (flags & CL_MEM_READ_WRITE))))
    return NULL; // Conflict flag

  if ((flags & CL_MEM_SVM_ATOMICS) && ((flags & CL_MEM_SVM_FINE_GRAIN_BUFFER) == 0))
    return NULL; // Conflict flag

  return cl_mem_svm_allocate(context, flags, size, alignment);
}

void clSVMFree(cl_context context, void *svm_pointer)
{
  cl_mem mem = NULL;

  if (!CL_OBJECT_IS_CONTEXT(context))
    return;

  if (svm_pointer == NULL)
    return;

  mem = cl_context_get_svm_by_ptr(context, svm_pointer, CL_TRUE);
  if (mem == NULL)
    return;

  return cl_mem_svm_delete(context, mem);
}

cl_int
clEnqueueSVMFree(cl_command_queue command_queue,
                 cl_uint num_svm_pointers,
                 void *svm_pointers[],
                 void(CL_CALLBACK *pfn_free_func)(cl_command_queue queue,
                                                  cl_uint num_svm_pointers,
                                                  void *svm_pointers[],
                                                  void *user_data),
                 void *user_data,
                 cl_uint num_events_in_wait_list,
                 const cl_event *event_wait_list,
                 cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_int i = 0;
  cl_mem *mem_pointers = NULL;
  void **new_pointers = NULL;
  cl_event e = NULL;
  cl_int e_status;
  cl_mem mem;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (num_svm_pointers == 0 || svm_pointers == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    mem_pointers = CL_CALLOC(num_svm_pointers, sizeof(cl_mem));
    if (mem_pointers == NULL) {
      err = CL_OUT_OF_HOST_MEMORY;
      break;
    }
    new_pointers = CL_CALLOC(num_svm_pointers, sizeof(void *));
    if (new_pointers == NULL) {
      err = CL_OUT_OF_HOST_MEMORY;
      break;
    }
    memcpy(new_pointers, svm_pointers, sizeof(void *) * num_svm_pointers);

    for (i = 0; i < num_svm_pointers; i++) {
      if (svm_pointers[i] == NULL) {
        err = CL_INVALID_VALUE;
        break;
      }
      mem = cl_context_get_svm_by_ptr(command_queue->ctx, svm_pointers[i], CL_TRUE);
      if (mem) {
        mem_pointers[i] = mem;
      }
    }
    if (err != CL_SUCCESS)
      break;

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_SVM_FREE, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    e->exec_data.type = EnqueueSVMMemFree;
    e->exec_data.svm_free.queue = command_queue;
    e->exec_data.svm_free.ptrs = new_pointers;
    e->exec_data.svm_free.mem_ptrs = mem_pointers;
    e->exec_data.svm_free.mem_num = num_svm_pointers;
    e->exec_data.svm_free.free_func = pfn_free_func;
    e->exec_data.svm_free.user_data = user_data;

    if (e_status == CL_COMPLETE) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      err = cl_event_exec(e, CL_QUEUED, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
      cl_command_queue_enqueue_event(command_queue, e);
    }
  } while (0);

  if (err != CL_SUCCESS) {
    if (mem_pointers)
      CL_FREE(mem_pointers);
    if (new_pointers)
      CL_FREE(new_pointers);
  }

  if (err == CL_SUCCESS && event) {
    *event = e;
  } else {
    cl_event_delete(e);
  }

  return err;
}

cl_int
clEnqueueSVMMap(cl_command_queue command_queue,
                cl_bool blocking_map,
                cl_map_flags map_flags,
                void *svm_ptr,
                size_t size,
                cl_uint num_events_in_wait_list,
                const cl_event *event_wait_list,
                cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_mem svm_buffer;
  cl_event e = NULL;
  cl_int e_status;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (svm_ptr == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }
    svm_buffer = cl_context_get_svm_by_ptr(command_queue->ctx, svm_ptr, CL_FALSE);
    if (svm_buffer == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    assert(svm_buffer->host_ptr);
    if (size == 0 || svm_ptr - svm_buffer->host_ptr + size > svm_buffer->size) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (map_flags & (~(CL_MAP_READ | CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION))) {
      err = CL_INVALID_OPERATION;
      break;
    }

    if (command_queue->ctx != svm_buffer->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_SVM_MAP, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueSVMMemMap;
    e->exec_data.svm_map.ptr = svm_ptr;
    e->exec_data.svm_map.svm = svm_buffer;
    e->exec_data.svm_map.size = size;
    e->exec_data.svm_map.unsync_map = CL_FALSE;
    e->exec_data.svm_map.write_map = CL_FALSE;
    if (map_flags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION))
      e->exec_data.svm_map.write_map = CL_TRUE;

    if (blocking_map) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_finish(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    if (e_status == CL_COMPLETE) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      e->exec_data.svm_map.unsync_map = CL_TRUE;
      err = cl_event_exec(e, CL_QUEUED, CL_TRUE);
      if (err != CL_SUCCESS) {
        break;
      }

      cl_command_queue_enqueue_event(command_queue, e);
    }
    assert(err == CL_SUCCESS);
  } while (0);

  if (err == CL_SUCCESS && event) {
    *event = e;
  } else {
    cl_event_delete(e);
  }

  return err;
}

cl_int
clEnqueueSVMUnmap(cl_command_queue command_queue,
                  void *svm_ptr,
                  cl_uint num_events_in_wait_list,
                  const cl_event *event_wait_list,
                  cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_mem svm_buffer;
  cl_event e = NULL;
  cl_int e_status;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (svm_ptr == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }
    svm_buffer = cl_context_get_svm_by_ptr(command_queue->ctx, svm_ptr, CL_FALSE);
    if (svm_buffer == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (command_queue->ctx != svm_buffer->ctx) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_SVM_UNMAP, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueSVMMemUnMap;
    e->exec_data.svm_unmap.svm = svm_buffer;
    e->exec_data.svm_unmap.ptr = svm_ptr;

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    if (e_status == CL_COMPLETE) { // No need to wait
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else { // May need to wait some event to complete.
      err = cl_event_exec(e, CL_QUEUED, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
      cl_command_queue_enqueue_event(command_queue, e);
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
clEnqueueSVMMemcpy(cl_command_queue command_queue,
                   cl_bool blocking_copy,
                   void *dst_ptr,
                   const void *src_ptr,
                   size_t size,
                   cl_uint num_events_in_wait_list,
                   const cl_event *event_wait_list,
                   cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_int e_status;
  cl_event e = NULL;
  cl_mem src_mem = NULL;
  cl_mem dst_mem = NULL;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (UNLIKELY(dst_ptr == NULL || src_ptr == NULL || size == 0)) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (((size_t)src_ptr < (size_t)dst_ptr && ((size_t)src_ptr + size > (size_t)dst_ptr)) ||
        ((size_t)dst_ptr < (size_t)src_ptr && ((size_t)dst_ptr + size > (size_t)src_ptr))) {
      err = CL_MEM_COPY_OVERLAP;
      break;
    }

    src_mem = cl_context_get_svm_by_ptr(command_queue->ctx, src_ptr, CL_FALSE);
    if (src_mem == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }
    dst_mem = cl_context_get_svm_by_ptr(command_queue->ctx, dst_ptr, CL_FALSE);
    if (dst_mem == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (src_ptr - src_mem->host_ptr + size > src_mem->size ||
        dst_ptr - dst_mem->host_ptr + size > dst_mem->size) {
      err = CL_INVALID_VALUE;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_SVM_MEMCPY, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueSVMMemCopy;
    e->exec_data.svm_copy.src = src_mem;
    e->exec_data.svm_copy.dst = dst_mem;
    e->exec_data.svm_copy.size = size;
    e->exec_data.svm_copy.src_ptr = (void *)src_ptr;
    e->exec_data.svm_copy.dst_ptr = dst_ptr;

    if (blocking_copy) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_finish(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    if (e_status == CL_COMPLETE) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      err = cl_event_exec(e, CL_QUEUED, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
      cl_command_queue_enqueue_event(command_queue, e);
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
clEnqueueSVMMemFill(cl_command_queue command_queue,
                    void *svm_ptr,
                    const void *pattern,
                    size_t pattern_size,
                    size_t size,
                    cl_uint num_events_in_wait_list,
                    const cl_event *event_wait_list,
                    cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_int e_status;
  cl_event e = NULL;
  cl_mem svm_mem = NULL;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if (svm_ptr == NULL || ((size_t)svm_ptr & (pattern_size - 1)) != 0) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (pattern == NULL || (pattern_size & (pattern_size - 1)) != 0 || pattern_size > 128) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (size == 0 || (size % pattern_size) != 0) {
      err = CL_INVALID_VALUE;
      break;
    }

    svm_mem = cl_context_get_svm_by_ptr(command_queue->ctx, svm_ptr, CL_FALSE);
    if (svm_mem == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_SVM_MEMFILL, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e->exec_data.type = EnqueueSVMMemFill;
    e->exec_data.svm_fill.svm = svm_mem;
    e->exec_data.svm_fill.pattern = (void *)pattern;
    e->exec_data.svm_fill.pattern_size = pattern_size;
    e->exec_data.svm_fill.size = size;
    e->exec_data.svm_fill.ptr = svm_ptr;

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    if (e_status == CL_COMPLETE) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      err = cl_event_exec(e, CL_QUEUED, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
      cl_command_queue_enqueue_event(command_queue, e);
    }
  } while (0);

  if (err == CL_SUCCESS && event) {
    *event = e;
  } else {
    cl_event_delete(e);
  }

  return err;
}

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

#include "cl_mem.h"
#include "cl_enqueue.h"
#include "cl_command_queue.h"
#include "cl_event.h"
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
  size_t ptr, offset;
  cl_int ref;
  cl_mem parent;

  if (!CL_OBJECT_IS_MEM(memobj)) {
    return CL_INVALID_MEM_OBJECT;
  }

  switch (param_name) {
  case CL_MEM_TYPE: {
    type = cl_get_mem_object_type(memobj);
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
    ptr = 0;
    if (memobj->type == CL_MEM_IMAGE_TYPE) {
      ptr = (size_t)memobj->host_ptr;
    } else {
      struct _cl_mem_buffer *buf = (struct _cl_mem_buffer *)memobj;
      ptr = (size_t)memobj->host_ptr + buf->sub_offset;
    }
    src_ptr = &ptr;
    src_size = sizeof(size_t);
    break;
  }
  case CL_MEM_USES_SVM_POINTER: {
    src_ptr = &memobj->is_svm;
    src_size = sizeof(memobj->is_svm);
    break;
  }
  case CL_MEM_MAP_COUNT:
    src_ptr = &memobj->map_ref;
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
    if (memobj->type == CL_MEM_SUBBUFFER_TYPE) {
      struct _cl_mem_buffer *buf = (struct _cl_mem_buffer *)memobj;
      parent = (cl_mem)(buf->parent);
    } else if (memobj->type == CL_MEM_IMAGE_TYPE) {
      parent = memobj;
    } else if (memobj->type == CL_MEM_BUFFER1D_IMAGE_TYPE) {
      struct _cl_mem_buffer1d_image *image_buffer = (struct _cl_mem_buffer1d_image *)memobj;
      parent = image_buffer->descbuffer;
    } else
      parent = NULL;
    src_ptr = &parent;
    src_size = sizeof(cl_mem);
    break;
  }
  case CL_MEM_OFFSET: {
    offset = 0;
    if (memobj->type == CL_MEM_SUBBUFFER_TYPE) {
      struct _cl_mem_buffer *buf = (struct _cl_mem_buffer *)memobj;
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

  if (!CL_OBJECT_IS_MEM(memobj)) {
    return CL_INVALID_MEM_OBJECT;
  }
  image = cl_mem_image(memobj);

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
    if (memobj->type == CL_MEM_BUFFER1D_IMAGE_TYPE) {
      struct _cl_mem_buffer1d_image *buffer1d_image = (struct _cl_mem_buffer1d_image *)image;
      src_ptr = &buffer1d_image->size;
    } else {
      src_ptr = &image->w;
    }
    src_size = sizeof(size_t);
    break;
  case CL_IMAGE_HEIGHT: {
    height = 0;
    if (memobj->type != CL_MEM_BUFFER1D_IMAGE_TYPE) {
      height = IS_1D_IMAGE(image) ? 0 : image->h;
    }
    src_ptr = &height;
    src_size = sizeof(size_t);
    break;
  }
  case CL_IMAGE_DEPTH: {
    depth = 0;
    depth = IS_3D_IMAGE(image) ? image->depth : 0;
    src_ptr = &depth;
    src_size = sizeof(size_t);
    break;
  }
  case CL_IMAGE_ARRAY_SIZE: {
    array_sz = 0;
    array_sz = IS_IMAGE_ARRAY(image) ? image->depth : 0;
    src_ptr = &array_sz;
    src_size = sizeof(size_t);
    break;
  }
  case CL_IMAGE_BUFFER:
    src_ptr = &image->buffer_1d;
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
  void *mem_ptr = NULL;
  cl_event e = NULL;
  cl_int e_status;
  enqueue_data *data = NULL;

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

    if (blocking_map) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_flush(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    data = &e->exec_data;
    data->type = EnqueueMapBuffer;
    data->mem_obj = buffer;
    data->offset = offset;
    data->size = size;
    data->ptr = NULL;
    data->unsync_map = 0;
    if (map_flags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION))
      data->write_map = 1;

    if (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      err = cl_event_exec(e, CL_SUBMITTED, CL_TRUE); // Submit to get the address.
      if (err != CL_SUCCESS) {
        break;
      }

      cl_command_queue_enqueue_event(command_queue, e);
      if (blocking_map) {
        cl_event_wait_for_events_list(1, &e);
      }
    }

    ptr = data->ptr;
    assert(ptr);
    err = cl_mem_record_map_mem(buffer, ptr, &mem_ptr, offset, size, NULL, NULL);
    assert(err == CL_SUCCESS);
  } while (0);

  if (err == CL_SUCCESS && event) {
    *event = e;
  } else {
    cl_event_delete(e);
  }

  if (errcode_ret)
    *errcode_ret = err;

  return mem_ptr;
}

static cl_int
clEnqueueUnmapMemObjectForKernel(cl_command_queue command_queue,
                        cl_mem memobj,
                        void *mapped_ptr,
                        cl_uint num_events_in_wait_list,
                        const cl_event *event_wait_list,
                        cl_event *event)
{
  cl_int err = CL_SUCCESS;
  int i, j;
  uint8_t write_map = 0;
  cl_mem tmp_ker_buf = NULL;
  size_t origin[3], region[3];
  void *v_ptr = NULL;

  assert(memobj->mapped_ptr_sz >= memobj->map_ref);
  for (i = 0; i < memobj->mapped_ptr_sz; i++) {
    if (memobj->mapped_ptr[i].ptr == mapped_ptr) {
      memobj->mapped_ptr[i].ptr = NULL;
      v_ptr = memobj->mapped_ptr[i].v_ptr;
      write_map = memobj->mapped_ptr[i].ker_write_map;
      tmp_ker_buf = memobj->mapped_ptr[i].tmp_ker_buf;
      for (j = 0; j < 3; j++) {
        region[j] = memobj->mapped_ptr[i].region[j];
        origin[j] = memobj->mapped_ptr[i].origin[j];
        memobj->mapped_ptr[i].region[j] = 0;
        memobj->mapped_ptr[i].origin[j] = 0;
      }
      memobj->mapped_ptr[i].size = 0;
      memobj->mapped_ptr[i].ker_write_map = 0;
      memobj->mapped_ptr[i].tmp_ker_buf = 0;
      memobj->mapped_ptr[i].v_ptr = NULL;
      memobj->map_ref--;
      break;
    }
  }

  if (!tmp_ker_buf)
    return CL_INVALID_MEM_OBJECT;

  cl_event e;
  err = clEnqueueUnmapMemObject(command_queue, tmp_ker_buf, v_ptr,
    num_events_in_wait_list, event_wait_list, &e);
  if (err != CL_SUCCESS) {
    clReleaseEvent(e);
    return err;
  }

  if (write_map) {
    err = clEnqueueCopyBufferToImage(command_queue, tmp_ker_buf, memobj, 0, origin, region,
        1, &e, event);
    if (err != CL_SUCCESS) {
      clReleaseEvent(e);
      return err;
    }

    if (event == NULL) {
      err = clFinish(command_queue);
      if (err != CL_SUCCESS) {
        clReleaseEvent(e);
        return err;
      }
    }
  }

  clReleaseEvent(e);
  clReleaseMemObject(tmp_ker_buf);
  return err;
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
  enqueue_data *data = NULL;
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

    if (CL_OBJECT_IS_IMAGE(memobj) && cl_mem_image(memobj)->is_ker_copy) {
      return clEnqueueUnmapMemObjectForKernel(command_queue, memobj, mapped_ptr,
        num_events_in_wait_list, event_wait_list, event);
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

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    data = &e->exec_data;
    data->type = EnqueueUnmapMemObject;
    data->mem_obj = memobj;
    data->ptr = mapped_ptr;

    if (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) { // No need to wait
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
  enqueue_data *data = NULL;
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

    if (blocking_read) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_flush(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    data = &e->exec_data;
    data->type = EnqueueReadBuffer;
    data->mem_obj = buffer;
    data->ptr = ptr;
    data->offset = offset;
    data->size = size;

    if (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) {
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
      if (blocking_read) {
        cl_event_wait_for_events_list(1, &e);
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
  enqueue_data *data = NULL;
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

    if (blocking_write) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_flush(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    data = &e->exec_data;
    data->type = EnqueueWriteBuffer;
    data->mem_obj = buffer;
    data->const_ptr = ptr;
    data->offset = offset;
    data->size = size;

    if (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) {
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
      if (blocking_write) {
        cl_event_wait_for_events_list(1, &e);
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
  enqueue_data *data = NULL;
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

    if (blocking_read) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_flush(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    data = &e->exec_data;
    data->type = EnqueueReadBufferRect;
    data->mem_obj = buffer;
    data->ptr = ptr;
    data->origin[0] = buffer_origin[0];
    data->origin[1] = buffer_origin[1];
    data->origin[2] = buffer_origin[2];
    data->host_origin[0] = host_origin[0];
    data->host_origin[1] = host_origin[1];
    data->host_origin[2] = host_origin[2];
    data->region[0] = region[0];
    data->region[1] = region[1];
    data->region[2] = region[2];
    data->row_pitch = buffer_row_pitch;
    data->slice_pitch = buffer_slice_pitch;
    data->host_row_pitch = host_row_pitch;
    data->host_slice_pitch = host_slice_pitch;

    if (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) {
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
      if (blocking_read) {
        cl_event_wait_for_events_list(1, &e);
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
  enqueue_data *data = NULL;
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

    if (blocking_write) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_flush(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    data = &e->exec_data;
    data->type = EnqueueWriteBufferRect;
    data->mem_obj = buffer;
    data->const_ptr = ptr;
    data->origin[0] = buffer_origin[0];
    data->origin[1] = buffer_origin[1];
    data->origin[2] = buffer_origin[2];
    data->host_origin[0] = host_origin[0];
    data->host_origin[1] = host_origin[1];
    data->host_origin[2] = host_origin[2];
    data->region[0] = region[0];
    data->region[1] = region[1];
    data->region[2] = region[2];
    data->row_pitch = buffer_row_pitch;
    data->slice_pitch = buffer_slice_pitch;
    data->host_row_pitch = host_row_pitch;
    data->host_slice_pitch = host_slice_pitch;

    if (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) {
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
      if (blocking_write) {
        cl_event_wait_for_events_list(1, &e);
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

    err = cl_mem_copy(command_queue, e, src_buffer, dst_buffer, src_offset, dst_offset, cb);
    if (err != CL_SUCCESS) {
      break;
    }

    /* We will flush the ndrange if no event depend. Else we will add it to queue list.
       The finish or Complete status will always be done in queue list. */
    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    err = cl_event_exec(e, (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) ? CL_SUBMITTED : CL_QUEUED, CL_FALSE);
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

    err = cl_mem_copy_buffer_rect(command_queue, e, src_buffer, dst_buffer, src_origin, dst_origin, region,
                                  src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch);
    if (err != CL_SUCCESS) {
      break;
    }

    /* We will flush the ndrange if no event depend. Else we will add it to queue list.
       The finish or Complete status will always be done in queue list. */
    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    } else if (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) {
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

    err = cl_mem_fill(command_queue, e, pattern, pattern_size, buffer, offset, size);
    if (err) {
      break;
    }

    /* We will flush the ndrange if no event depend. Else we will add it to queue list.
       The finish or Complete status will always be done in queue list. */
    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    err = cl_event_exec(e, (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) ? CL_SUBMITTED : CL_QUEUED, CL_FALSE);
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

    err = cl_event_exec(e, (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) ? CL_SUBMITTED : CL_QUEUED, CL_FALSE);
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

static void *
clEnqueueMapImageByKernel(cl_command_queue command_queue,
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
  void *mem_ptr = NULL;
  struct _cl_mem_image *image = NULL;
  size_t region[3], copy_origin[3];
  size_t origin[3], copy_region[3];
  size_t offset = 0;
  size_t buf_size = 0;

  image = cl_mem_image(mem);

  err = check_image_region(image, pregion, region);
  if (err != CL_SUCCESS && errcode_ret) {
    *errcode_ret = err;
    return NULL;
  }

  err = check_image_origin(image, porigin, origin);
  if (err != CL_SUCCESS && errcode_ret) {
    *errcode_ret = err;
    return NULL;
  }

  if (image->tmp_ker_buf)
    clReleaseMemObject(image->tmp_ker_buf);

  if (mem->flags & CL_MEM_USE_HOST_PTR) {
    buf_size = image->w * image->h * image->depth * image->bpp;
    memset(copy_origin, 0, sizeof(size_t) * 3);
    copy_region[0] = image->w;
    copy_region[1] = image->h;
    copy_region[2] = image->depth;
    image->tmp_ker_buf =
      clCreateBuffer(command_queue->ctx, CL_MEM_USE_HOST_PTR, mem->size, mem->host_ptr, &err);
  } else {
    buf_size = region[0] * region[1] * region[2] * image->bpp;
    memcpy(copy_origin, origin, sizeof(size_t) * 3);
    memcpy(copy_region, region, sizeof(size_t) * 3);
    image->tmp_ker_buf =
      clCreateBuffer(command_queue->ctx, CL_MEM_ALLOC_HOST_PTR, buf_size, NULL, &err);
  }
  if ((image->tmp_ker_buf == NULL || err != CL_SUCCESS) && errcode_ret) {
    image->tmp_ker_buf = NULL;
    *errcode_ret = err;
    return NULL;
  }

  cl_event e;
  err = clEnqueueCopyImageToBuffer(command_queue, mem, image->tmp_ker_buf, copy_origin,
    copy_region, 0, num_events_in_wait_list, event_wait_list, &e);
  if (err != CL_SUCCESS && errcode_ret) {
    clReleaseMemObject(image->tmp_ker_buf);
    clReleaseEvent(e);
    image->tmp_ker_buf = NULL;
    *errcode_ret = err;
    return NULL;
  }

  if (mem->flags & CL_MEM_USE_HOST_PTR) {
      if (image_slice_pitch)
        *image_slice_pitch = image->host_slice_pitch;
      if (image_row_pitch)
        *image_row_pitch = image->host_row_pitch;

      offset = image->bpp * origin[0] + image->host_row_pitch * origin[1] +
               image->host_slice_pitch * origin[2];
  } else {
    if (image_slice_pitch)
      *image_slice_pitch = (image->image_type == CL_MEM_OBJECT_IMAGE2D) ?
        image->slice_pitch : region[0] * region[1] * image->bpp;
    if (image_row_pitch)
      *image_row_pitch = region[0] * image->bpp;
  }

  ptr = clEnqueueMapBuffer(command_queue, image->tmp_ker_buf, blocking_map, map_flags, 0,
      buf_size, 1, &e, event, &err);
  if (err != CL_SUCCESS && errcode_ret) {
    clReleaseMemObject(image->tmp_ker_buf);
    clReleaseEvent(e);
    image->tmp_ker_buf = NULL;
    *errcode_ret = err;
    return NULL;
  }

  err = cl_mem_record_map_mem_for_kernel(mem, ptr, &mem_ptr, offset, 0, origin, region,
    image->tmp_ker_buf, (map_flags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION)) ? 1 : 0);
  image->tmp_ker_buf = NULL;
  assert(err == CL_SUCCESS); // Easy way, do not use unmap to handle error.
  if (errcode_ret)
    *errcode_ret = err;
  clReleaseEvent(e);
  return mem_ptr;
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
  void *mem_ptr = NULL;
  size_t offset = 0;
  struct _cl_mem_image *image = NULL;
  cl_int e_status;
  enqueue_data *data = NULL;
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

    image = cl_mem_image(mem);

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

    if ((map_flags & CL_MAP_READ &&
         mem->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) ||
        (map_flags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION) &&
         mem->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))) {
      err = CL_INVALID_OPERATION;
      break;
    }

    if (CL_OBJECT_IS_IMAGE(mem) && cl_mem_image(mem)->is_ker_copy) {
      return clEnqueueMapImageByKernel(command_queue, mem, blocking_map, map_flags, origin, region,
        image_row_pitch, image_slice_pitch, num_events_in_wait_list, event_wait_list,
        event, errcode_ret);
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

    if (blocking_map) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_flush(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    data = &e->exec_data;
    data->type = EnqueueMapImage;
    data->mem_obj = mem;
    data->origin[0] = origin[0];
    data->origin[1] = origin[1];
    data->origin[2] = origin[2];
    data->region[0] = region[0];
    data->region[1] = region[1];
    data->region[2] = region[2];
    data->ptr = ptr;
    data->unsync_map = 1;
    if (map_flags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION))
      data->write_map = 1;

    if (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      err = cl_event_exec(e, CL_SUBMITTED, CL_TRUE); // Submit to get the address.
      if (err != CL_SUCCESS) {
        break;
      }

      cl_command_queue_enqueue_event(command_queue, e);
      if (blocking_map) {
        cl_event_wait_for_events_list(1, &e);
      }
    }

    ptr = data->ptr;
    assert(ptr);

    /* Store and write back map info. */
    if (mem->flags & CL_MEM_USE_HOST_PTR) {
      if (image_slice_pitch)
        *image_slice_pitch = image->host_slice_pitch;
      *image_row_pitch = image->host_row_pitch;

      offset = image->bpp * origin[0] + image->host_row_pitch * origin[1] +
               image->host_slice_pitch * origin[2];
    } else {
      if (image_slice_pitch)
        *image_slice_pitch = image->slice_pitch;
      if (image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
        *image_row_pitch = image->slice_pitch;
      else
        *image_row_pitch = image->row_pitch;

      offset = image->bpp * origin[0] + image->row_pitch * origin[1] + image->slice_pitch * origin[2];
    }

    err = cl_mem_record_map_mem(mem, ptr, &mem_ptr, offset, 0, origin, region);
    assert(err == CL_SUCCESS); // Easy way, do not use unmap to handle error.
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

  return mem_ptr;
}

static cl_int
clEnqueueReadImageByKernel(cl_command_queue command_queue,
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
  size_t region[3];
  size_t origin[3];

  image = cl_mem_image(mem);

  err = check_image_region(image, pregion, region);
  if (err != CL_SUCCESS)
    return err;

  err = check_image_origin(image, porigin, origin);
  if (err != CL_SUCCESS)
    return err;

  if (image->tmp_ker_buf)
    clReleaseMemObject(image->tmp_ker_buf);

  size_t buf_size = region[0] * region[1] * region[2] * image->bpp;
  image->tmp_ker_buf = clCreateBuffer(command_queue->ctx, CL_MEM_USE_HOST_PTR,
    buf_size, ptr, &err);
  if (image->tmp_ker_buf == NULL || err != CL_SUCCESS) {
    image->tmp_ker_buf = NULL;
    return err;
  }

  cl_event e;
  err = clEnqueueCopyImageToBuffer(command_queue, mem, image->tmp_ker_buf, origin,
    region, 0, num_events_in_wait_list, event_wait_list, &e);
  if (err != CL_SUCCESS) {
    clReleaseMemObject(image->tmp_ker_buf);
    clReleaseEvent(e);
    image->tmp_ker_buf = NULL;
    return err;
  }

  err = clEnqueueReadBuffer(command_queue, image->tmp_ker_buf, blocking_read, 0,
    buf_size, ptr, 1, &e, event);
  clReleaseEvent(e);
  return err;
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
  enqueue_data *data = NULL;
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

    image = cl_mem_image(mem);

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

    if (image->is_ker_copy) {
      return clEnqueueReadImageByKernel(command_queue, mem, blocking_read, origin,
        region, row_pitch, slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
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

    if (blocking_read) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_flush(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    data = &e->exec_data;
    data->type = EnqueueReadImage;
    data->mem_obj = mem;
    data->ptr = ptr;
    data->origin[0] = origin[0];
    data->origin[1] = origin[1];
    data->origin[2] = origin[2];
    data->region[0] = region[0];
    data->region[1] = region[1];
    data->region[2] = region[2];
    data->row_pitch = row_pitch;
    data->slice_pitch = slice_pitch;

    if (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) {
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
      if (blocking_read) {
        cl_event_wait_for_events_list(1, &e);
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

static cl_int
clEnqueueWriteImageByKernel(cl_command_queue command_queue,
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
  size_t region[3];
  size_t origin[3];

  image = cl_mem_image(mem);

  err = check_image_region(image, pregion, region);
  if (err != CL_SUCCESS)
    return err;

  err = check_image_origin(image, porigin, origin);
  if (err != CL_SUCCESS)
    return err;

  if (image->tmp_ker_buf)
    clReleaseMemObject(image->tmp_ker_buf);

  size_t buf_size = region[0] * region[1] * region[2] * image->bpp;
  image->tmp_ker_buf = clCreateBuffer(command_queue->ctx, CL_MEM_USE_HOST_PTR, buf_size, (void*)ptr, &err);
  if (image->tmp_ker_buf == NULL || err != CL_SUCCESS) {
    image->tmp_ker_buf = NULL;
    return err;
  }

  err = clEnqueueCopyBufferToImage(command_queue, image->tmp_ker_buf, mem, 0, origin, region,
    num_events_in_wait_list, event_wait_list, event);

  if (blocking_write)
    err = clFinish(command_queue);

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
  enqueue_data *data = NULL;
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

    image = cl_mem_image(mem);

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

    if (image->is_ker_copy) {
      return clEnqueueWriteImageByKernel(command_queue, mem, blocking_write, origin,
        region, row_pitch, slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
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

    if (blocking_write) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_flush(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    data = &e->exec_data;
    data->type = EnqueueWriteImage;
    data->mem_obj = mem;
    data->const_ptr = ptr;
    data->origin[0] = origin[0];
    data->origin[1] = origin[1];
    data->origin[2] = origin[2];
    data->region[0] = region[0];
    data->region[1] = region[1];
    data->region[2] = region[2];
    data->row_pitch = row_pitch;
    data->slice_pitch = slice_pitch;

    if (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) {
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
      if (blocking_write) {
        cl_event_wait_for_events_list(1, &e);
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

    src_image = cl_mem_image(src_mem);
    dst_image = cl_mem_image(dst_mem);

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

    err = cl_mem_kernel_copy_image(command_queue, e, src_image, dst_image,
                                   src_origin, dst_origin, region);
    if (err != CL_SUCCESS) {
      break;
    }
    /* We will flush the ndrange if no event depend. Else we will add it to queue list.
       The finish or Complete status will always be done in queue list. */
    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    err = cl_event_exec(e, (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) ? CL_SUBMITTED : CL_QUEUED, CL_FALSE);
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

    src_image = cl_mem_image(src_mem);

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

    err = cl_mem_copy_image_to_buffer(command_queue, e, src_image, dst_buffer,
                                      src_origin, dst_offset, region);
    if (err != CL_SUCCESS) {
      break;
    }

    /* We will flush the ndrange if no event depend. Else we will add it to queue list.
       The finish or Complete status will always be done in queue list. */
    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    err = cl_event_exec(e, (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) ? CL_SUBMITTED : CL_QUEUED, CL_FALSE);
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

    dst_image = cl_mem_image(dst_mem);

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

    err = cl_mem_copy_buffer_to_image(command_queue, e, src_buffer, dst_image,
                                      src_offset, dst_origin, region);

    if (err != CL_SUCCESS) {
      break;
    }

    /* We will flush the ndrange if no event depend. Else we will add it to queue list.
       The finish or Complete status will always be done in queue list. */
    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    err = cl_event_exec(e, (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) ? CL_SUBMITTED : CL_QUEUED, CL_FALSE);
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

    image = cl_mem_image(mem);

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

    err = cl_image_fill(command_queue, e, fill_color, image, origin, region);
    if (err != CL_SUCCESS) {
      break;
    }

    /* We will flush the ndrange if no event depend. Else we will add it to queue list.
       The finish or Complete status will always be done in queue list. */
    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) { // Error happend, cancel.
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    err = cl_event_exec(e, (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) ? CL_SUBMITTED : CL_QUEUED, CL_FALSE);
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

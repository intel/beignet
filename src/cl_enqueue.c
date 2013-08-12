/*
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#include "cl_enqueue.h"
#include "cl_image.h"
#include "cl_driver.h"
#include "cl_utils.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

cl_int cl_enqueue_read_buffer(enqueue_data* data)
{
  cl_int err = CL_SUCCESS;
  /*void* src_ptr;

  CHECK_QUEUE(command_queue);
  CHECK_MEM(buffer);
  if (command_queue->ctx != buffer->ctx) {
     err = CL_INVALID_CONTEXT;
     goto error;
  }

  if (blocking_read != CL_TRUE)
     NOT_IMPLEMENTED;

  if (!ptr || !size || offset + size > buffer->size) {
     err = CL_INVALID_VALUE;
     goto error;
  }

  if (buffer->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) {
     err = CL_INVALID_OPERATION;
     goto error;
  }

  if (!(src_ptr = cl_mem_map_auto(buffer))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  memcpy(ptr, (char*)src_ptr + offset, size);

  err = cl_mem_unmap_auto(buffer);

error: */
  return err;
}

cl_int cl_enqueue_write_buffer(enqueue_data *data)
{
  cl_int err = CL_SUCCESS;
  /*void* dst_ptr;

  CHECK_QUEUE(command_queue);
  CHECK_MEM(buffer);
  if (command_queue->ctx != buffer->ctx) {
    err = CL_INVALID_CONTEXT;
    goto error;
  }

  if (blocking_write != CL_TRUE)
    NOT_IMPLEMENTED;

  if (!ptr || !size || offset + size > buffer->size) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if (buffer->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)) {
    err = CL_INVALID_OPERATION;
    goto error;
  }

  if (!(dst_ptr = cl_mem_map_auto(buffer))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  memcpy((char*)dst_ptr + offset, ptr, size);

  err = cl_mem_unmap_auto(buffer);

error: */
  return err;
}

cl_int cl_enqueue_read_image(enqueue_data *data)
{
  cl_int err = CL_SUCCESS;
  /*  void* src_ptr;

  CHECK_QUEUE(command_queue);
  CHECK_IMAGE(image);
  if (command_queue->ctx != image->ctx) {
     err = CL_INVALID_CONTEXT;
     goto error;
  }

  if (blocking_read != CL_TRUE)
     NOT_IMPLEMENTED;

  if (!origin || !region || origin[0] + region[0] > image->w || origin[1] + region[1] > image->h || origin[2] + region[2] > image->depth) {
     err = CL_INVALID_VALUE;
     goto error;
  }

  if (!row_pitch)
    row_pitch = image->bpp*region[0];
  else if (row_pitch < image->bpp*region[0]) {
     err = CL_INVALID_VALUE;
     goto error;
  }

  if (image->slice_pitch) {
    if (!slice_pitch)
      slice_pitch = row_pitch*region[1];
    else if (slice_pitch < row_pitch*region[1]) {
      err = CL_INVALID_VALUE;
      goto error;
    }
  }
  else if (slice_pitch) {
     err = CL_INVALID_VALUE;
     goto error;
  }

  if (!ptr) {
     err = CL_INVALID_VALUE;
     goto error;
  }

  if (image->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) {
     err = CL_INVALID_OPERATION;
     goto error;
  }

  if (!(src_ptr = cl_mem_map_auto(image))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  size_t offset = image->bpp*origin[0] + image->row_pitch*origin[1] + image->slice_pitch*origin[2];
  src_ptr = (char*)src_ptr + offset;

  if (!origin[0] && region[0] == image->w && row_pitch == image->row_pitch &&
      (region[2] == 1 || (!origin[1] && region[1] == image->h && slice_pitch == image->slice_pitch)))
  {
    memcpy(ptr, src_ptr, region[2] == 1 ? row_pitch*region[1] : slice_pitch*region[2]);
  }
  else {
    cl_uint y, z;
    for (z = 0; z < region[2]; z++) {
      const char* src = src_ptr;
      char* dst = ptr;
      for (y = 0; y < region[1]; y++) {
	memcpy(dst, src, image->bpp*region[0]);
	src += image->row_pitch;
	dst += row_pitch;
      }
      src_ptr = (char*)src_ptr + image->slice_pitch;
      ptr = (char*)ptr + slice_pitch;
    }
  }

  err = cl_mem_unmap_auto(image);

error: */
  return err;

}

cl_int cl_enqueue_write_image(enqueue_data *data)
{
  cl_int err = CL_SUCCESS;
  /*void* dst_ptr;

  CHECK_QUEUE(command_queue);
  CHECK_IMAGE(image);
  if (command_queue->ctx != image->ctx) {
    err = CL_INVALID_CONTEXT;
    goto error;
  }

  if (blocking_write != CL_TRUE)
    NOT_IMPLEMENTED;

  if (!origin || !region || origin[0] + region[0] > image->w || origin[1] + region[1] > image->h || origin[2] + region[2] > image->depth) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if (!row_pitch)
    row_pitch = image->bpp*region[0];
  else if (row_pitch < image->bpp*region[0]) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if (image->slice_pitch) {
    if (!slice_pitch)
      slice_pitch = row_pitch*region[1];
    else if (slice_pitch < row_pitch*region[1]) {
      err = CL_INVALID_VALUE;
      goto error;
    }
  }
  else if (slice_pitch) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if (!ptr) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if (image->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)) {
    err = CL_INVALID_OPERATION;
    goto error;
  }

  if (!(dst_ptr = cl_mem_map_auto(image))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  size_t offset = image->bpp*origin[0] + image->row_pitch*origin[1] + image->slice_pitch*origin[2];
  dst_ptr = (char*)dst_ptr + offset;

  if (!origin[0] && region[0] == image->w && row_pitch == image->row_pitch &&
      (region[2] == 1 || (!origin[1] && region[1] == image->h && slice_pitch == image->slice_pitch)))
  {
    memcpy(dst_ptr, ptr, region[2] == 1 ? row_pitch*region[1] : slice_pitch*region[2]);
  }
  else {
    cl_uint y, z;
    for (z = 0; z < region[2]; z++) {
      const char* src = ptr;
      char* dst = dst_ptr;
      for (y = 0; y < region[1]; y++) {
	memcpy(dst, src, image->bpp*region[0]);
	src += row_pitch;
	dst += image->row_pitch;
      }
      ptr = (char*)ptr + slice_pitch;
      dst_ptr = (char*)dst_ptr + image->slice_pitch;
    }
  }

  err = cl_mem_unmap_auto(image);

error: */
  return err;

}

cl_int cl_enqueue_map_buffer(enqueue_data *data)
{
  cl_int err = CL_SUCCESS;
  /*  void *ptr = NULL;
  void *mem_ptr = NULL;
  cl_int err = CL_SUCCESS;
  int slot = -1;

  CHECK_QUEUE(command_queue);
  CHECK_MEM(buffer);
  if (command_queue->ctx != buffer->ctx) {
    err = CL_INVALID_CONTEXT;
    goto error;
  }

  if (blocking_map != CL_TRUE)
    NOT_IMPLEMENTED;

  if (!size || offset + size > buffer->size) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if ((map_flags & CL_MAP_READ &&
       buffer->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) ||
      (map_flags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION) &&
       buffer->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)))
  {
    err = CL_INVALID_OPERATION;
    goto error;
  }

  if (!(ptr = cl_mem_map_auto(buffer))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  ptr = (char*)ptr + offset;

  if(buffer->flags & CL_MEM_USE_HOST_PTR) {
    assert(buffer->host_ptr);
    memcpy(buffer->host_ptr + offset, ptr, size);
    mem_ptr = buffer->host_ptr + offset;
  } else {
    mem_ptr = ptr;
  }

  //Record the mapped address.
  if (!buffer->mapped_ptr_sz) {
    buffer->mapped_ptr_sz = 16;
    buffer->mapped_ptr = (cl_mapped_ptr *)malloc(
          sizeof(cl_mapped_ptr) * buffer->mapped_ptr_sz);
    if (!buffer->mapped_ptr) {
      cl_mem_unmap_auto (buffer);
      err = CL_OUT_OF_HOST_MEMORY;
      ptr = NULL;
      goto error;
    }

    memset(buffer->mapped_ptr, 0, buffer->mapped_ptr_sz * sizeof(cl_mapped_ptr));
    slot = 0;
  } else {
    int i = 0;
    for (; i < buffer->mapped_ptr_sz; i++) {
      if (buffer->mapped_ptr[i].ptr == NULL) {
        slot = i;
        break;
      }
    }

    if (i == buffer->mapped_ptr_sz) {
      cl_mapped_ptr *new_ptr = (cl_mapped_ptr *)malloc(
          sizeof(cl_mapped_ptr) * buffer->mapped_ptr_sz * 2);
      if (!new_ptr) {
        cl_mem_unmap_auto (buffer);
        err = CL_OUT_OF_HOST_MEMORY;
        ptr = NULL;
        goto error;
      }
      memset(new_ptr, 0, 2 * buffer->mapped_ptr_sz * sizeof(cl_mapped_ptr));
      memcpy(new_ptr, buffer->mapped_ptr,
             buffer->mapped_ptr_sz * sizeof(cl_mapped_ptr));
      slot = buffer->mapped_ptr_sz;
      buffer->mapped_ptr_sz *= 2;
      free(buffer->mapped_ptr);
      buffer->mapped_ptr = new_ptr;
    }
  }

  assert(slot != -1);
  buffer->mapped_ptr[slot].ptr = mem_ptr;
  buffer->mapped_ptr[slot].v_ptr = ptr;
  buffer->mapped_ptr[slot].size = size;
  buffer->map_ref++;

error:
  if (errcode_ret)
    *errcode_ret = err;
  return mem_ptr;

error: */
  return err;
}

cl_int cl_enqueue_map_image(enqueue_data *data)
{
  cl_int err = CL_SUCCESS;
  /*void *ptr = NULL;
  cl_int err = CL_SUCCESS;

  CHECK_QUEUE(command_queue);
  CHECK_IMAGE(image);
  if (command_queue->ctx != image->ctx) {
    err = CL_INVALID_CONTEXT;
    goto error;
  }

  if (blocking_map != CL_TRUE)
    NOT_IMPLEMENTED;

  if (!origin || !region || origin[0] + region[0] > image->w || origin[1] + region[1] > image->h || origin[2] + region[2] > image->depth) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if (!image_row_pitch || (image->slice_pitch && !image_slice_pitch)) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  *image_row_pitch = image->row_pitch;
  if (image_slice_pitch)
    *image_slice_pitch = image->slice_pitch;

  if ((map_flags & CL_MAP_READ &&
       image->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) ||
      (map_flags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION) &&
       image->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)))
  {
    err = CL_INVALID_OPERATION;
    goto error;
  }

  if (!(ptr = cl_mem_map_auto(image))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  size_t offset = image->bpp*origin[0] + image->row_pitch*origin[1] + image->slice_pitch*origin[2];
  ptr = (char*)ptr + offset;

error:
  if (errcode_ret)
    *errcode_ret = err;
  return ptr;

error: */
  return err;
}

cl_int cl_enqueue_unmap_mem_object(enqueue_data *data)
{
  cl_int err = CL_SUCCESS;
  /*int i;
  size_t mapped_size = 0;
  void * v_ptr = NULL;

  CHECK_QUEUE(command_queue);
  CHECK_MEM(memobj);
  if (command_queue->ctx != memobj->ctx) {
    err = CL_INVALID_CONTEXT;
    goto error;
  }

  assert(memobj->mapped_ptr_sz >= memobj->map_ref);
  INVALID_VALUE_IF(!mapped_ptr);
  for (i = 0; i < memobj->mapped_ptr_sz; i++) {
    if (memobj->mapped_ptr[i].ptr == mapped_ptr) {
      memobj->mapped_ptr[i].ptr = NULL;
      mapped_size = memobj->mapped_ptr[i].size;
      v_ptr = memobj->mapped_ptr[i].v_ptr;
      memobj->mapped_ptr[i].size = 0;
      memobj->mapped_ptr[i].v_ptr = NULL;
      memobj->map_ref--;
      break;
    }
  }
  // can not find a mapped address?
  INVALID_VALUE_IF(i == memobj->mapped_ptr_sz);

  if (memobj->flags & CL_MEM_USE_HOST_PTR) {
    assert(mapped_ptr >= memobj->host_ptr &&
      mapped_ptr + mapped_size <= memobj->host_ptr + memobj->size);
    //Sync the data.
    memcpy(v_ptr, mapped_ptr, mapped_size);
  } else {
    assert(v_ptr == mapped_ptr);
  }

  cl_mem_unmap_auto(memobj);

  //shrink the mapped slot.
  if (memobj->mapped_ptr_sz/2 > memobj->map_ref) {
    int j = 0;
    cl_mapped_ptr *new_ptr = (cl_mapped_ptr *)malloc(
	sizeof(cl_mapped_ptr) * (memobj->mapped_ptr_sz/2));
    if (!new_ptr) {
      //Just do nothing.
      goto error;
    }
    memset(new_ptr, 0, (memobj->mapped_ptr_sz/2) * sizeof(cl_mapped_ptr));

    for (i = 0; i < memobj->mapped_ptr_sz; i++) {
      if (memobj->mapped_ptr[i].ptr) {
        new_ptr[j] = memobj->mapped_ptr[i];
        j++;
        assert(j < memobj->mapped_ptr_sz/2);
      }
    }
    memobj->mapped_ptr_sz = memobj->mapped_ptr_sz/2;
    free(memobj->mapped_ptr);
    memobj->mapped_ptr = new_ptr;
  }

error: */
  return err;
}

cl_int cl_enqueue_handle(enqueue_data* data)
{
  switch(data->type) {
    case EnqueueReadBuffer:
      return cl_enqueue_read_buffer(data);
    case EnqueueWriteBuffer:
      return cl_enqueue_write_buffer(data);
    case EnqueueReadImage:
      return cl_enqueue_read_image(data);
    case EnqueueWriteImage:
      return cl_enqueue_write_image(data);
    case EnqueueMapBuffer:
      return cl_enqueue_map_buffer(data);
    case EnqueueMapImage:
      return cl_enqueue_map_image(data);
    case EnqueueUnmapMemObject:
      return cl_enqueue_unmap_mem_object(data);
    case EnqueueNDRangeKernel:
      //cl_gpgpu_event_resume((cl_gpgpu_event)data->ptr);   //goto default
    default:
      return CL_SUCCESS;
  }
}

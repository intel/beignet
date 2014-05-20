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
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include "cl_enqueue.h"
#include "cl_image.h"
#include "cl_driver.h"
#include "cl_event.h"
#include "cl_command_queue.h"
#include "cl_utils.h"


cl_int cl_enqueue_read_buffer(enqueue_data* data)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = data->mem_obj;
  assert(mem->type == CL_MEM_BUFFER_TYPE ||
         mem->type == CL_MEM_SUBBUFFER_TYPE);
  void* src_ptr;
  struct _cl_mem_buffer* buffer = (struct _cl_mem_buffer*)mem;

  if (!(src_ptr = cl_mem_map_auto(data->mem_obj))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  memcpy(data->ptr, (char*)src_ptr + data->offset + buffer->sub_offset, data->size);

  err = cl_mem_unmap_auto(data->mem_obj);

error:
  return err;
}

cl_int cl_enqueue_read_buffer_rect(enqueue_data* data)
{
  cl_int err = CL_SUCCESS;
  void* src_ptr;
  void* dst_ptr;

  const size_t* origin = data->origin;
  const size_t* host_origin = data->host_origin;
  const size_t* region = data->region;

  if (!(src_ptr = cl_mem_map_auto(data->mem_obj))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

   size_t offset = origin[0] + data->row_pitch*origin[1] + data->slice_pitch*origin[2];
   src_ptr = (char*)src_ptr + offset;

   offset = host_origin[0] + data->host_row_pitch*host_origin[1] + data->host_slice_pitch*host_origin[2];
   dst_ptr = (char *)data->ptr + offset;

   if (data->row_pitch == region[0] && data->row_pitch == data->host_row_pitch &&
       (region[2] == 1 || (data->slice_pitch == region[0]*region[1] && data->slice_pitch == data->host_slice_pitch)))
   {
     memcpy(dst_ptr, src_ptr, region[2] == 1 ? data->row_pitch*region[1] : data->slice_pitch*region[2]);
   }
   else {
     cl_uint y, z;
     for (z = 0; z < region[2]; z++) {
       const char* src = src_ptr;
       char* dst = dst_ptr;
       for (y = 0; y < region[1]; y++) {
         memcpy(dst, src, region[0]);
         src += data->row_pitch;
         dst += data->host_row_pitch;
       }
       src_ptr = (char*)src_ptr + data->slice_pitch;
       dst_ptr = (char*)dst_ptr + data->host_slice_pitch;
     }
   }

  err = cl_mem_unmap_auto(data->mem_obj);

error:
  return err;
}

cl_int cl_enqueue_write_buffer(enqueue_data *data)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = data->mem_obj;
  assert(mem->type == CL_MEM_BUFFER_TYPE ||
         mem->type == CL_MEM_SUBBUFFER_TYPE);
  struct _cl_mem_buffer* buffer = (struct _cl_mem_buffer*)mem;
  void* dst_ptr;

  if (!(dst_ptr = cl_mem_map_auto(data->mem_obj))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  memcpy((char*)dst_ptr + data->offset + buffer->sub_offset, data->const_ptr, data->size);

  err = cl_mem_unmap_auto(data->mem_obj);

error:
  return err;
}

cl_int cl_enqueue_write_buffer_rect(enqueue_data *data)
{
  cl_int err = CL_SUCCESS;
  void* src_ptr;
  void* dst_ptr;

  const size_t* origin = data->origin;
  const size_t* host_origin = data->host_origin;
  const size_t* region = data->region;

  if (!(dst_ptr = cl_mem_map_auto(data->mem_obj))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  size_t offset = origin[0] + data->row_pitch*origin[1] + data->slice_pitch*origin[2];
  dst_ptr = (char *)dst_ptr + offset;

  offset = host_origin[0] + data->host_row_pitch*host_origin[1] + data->host_slice_pitch*host_origin[2];
  src_ptr = (char*)data->const_ptr + offset;

  if (data->row_pitch == region[0] && data->row_pitch == data->host_row_pitch &&
      (region[2] == 1 || (data->slice_pitch == region[0]*region[1] && data->slice_pitch == data->host_slice_pitch)))
  {
    memcpy(dst_ptr, src_ptr, region[2] == 1 ? data->row_pitch*region[1] : data->slice_pitch*region[2]);
  }
  else {
    cl_uint y, z;
    for (z = 0; z < region[2]; z++) {
      const char* src = src_ptr;
      char* dst = dst_ptr;
      for (y = 0; y < region[1]; y++) {
        memcpy(dst, src, region[0]);
        src += data->host_row_pitch;
        dst += data->row_pitch;
      }
      src_ptr = (char*)src_ptr + data->host_slice_pitch;
      dst_ptr = (char*)dst_ptr + data->slice_pitch;
    }
  }

  err = cl_mem_unmap_auto(data->mem_obj);

error:
  return err;
}


cl_int cl_enqueue_read_image(enqueue_data *data)
{
  cl_int err = CL_SUCCESS;
  void* src_ptr;

  cl_mem mem = data->mem_obj;
  CHECK_IMAGE(mem, image);
  const size_t* origin = data->origin;
  const size_t* region = data->region;

  if (!(src_ptr = cl_mem_map_auto(mem))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  size_t offset = image->bpp*origin[0] + image->row_pitch*origin[1] + image->slice_pitch*origin[2];
  src_ptr = (char*)src_ptr + offset;

  if (!origin[0] && region[0] == image->w && data->row_pitch == image->row_pitch &&
      (region[2] == 1 || (!origin[1] && region[1] == image->h && data->slice_pitch == image->slice_pitch)))
  {
    memcpy(data->ptr, src_ptr, region[2] == 1 ? data->row_pitch*region[1] : data->slice_pitch*region[2]);
  }
  else {
    cl_uint y, z;
    for (z = 0; z < region[2]; z++) {
      const char* src = src_ptr;
      char* dst = data->ptr;
      for (y = 0; y < region[1]; y++) {
        memcpy(dst, src, image->bpp*region[0]);
        src += image->row_pitch;
        dst += data->row_pitch;
      }
      src_ptr = (char*)src_ptr + image->slice_pitch;
      data->ptr = (char*)data->ptr + data->slice_pitch;
    }
  }

 err = cl_mem_unmap_auto(mem);

error:
  return err;

}

cl_int cl_enqueue_write_image(enqueue_data *data)
{
  cl_int err = CL_SUCCESS;
  void* dst_ptr;

  cl_mem mem = data->mem_obj;
  CHECK_IMAGE(mem, image);

  if (!(dst_ptr = cl_mem_map_auto(mem))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  cl_mem_copy_image_region(data->origin, data->region, dst_ptr,
                           image->row_pitch, image->slice_pitch,
                           data->const_ptr, data->row_pitch,
                           data->slice_pitch, image);
  err = cl_mem_unmap_auto(mem);

error:
  return err;

}

cl_int cl_enqueue_map_buffer(enqueue_data *data)
{
  void *ptr = NULL;
  cl_int err = CL_SUCCESS;
  cl_mem mem = data->mem_obj;
  assert(mem->type == CL_MEM_BUFFER_TYPE ||
         mem->type == CL_MEM_SUBBUFFER_TYPE);
  struct _cl_mem_buffer* buffer = (struct _cl_mem_buffer*)mem;

  if(data->unsync_map == 1)
    //because using unsync map in clEnqueueMapBuffer, so force use map_gtt here
    ptr = cl_mem_map_gtt(mem);
  else
    ptr = cl_mem_map_auto(mem);

  if (ptr == NULL) {
    err = CL_MAP_FAILURE;
    goto error;
  }
  data->ptr = ptr;

  if(mem->flags & CL_MEM_USE_HOST_PTR) {
    assert(mem->host_ptr);
    ptr = (char*)ptr + data->offset + buffer->sub_offset;
    memcpy(mem->host_ptr + data->offset, ptr, data->size);
  }

error:
  return err;
}

cl_int cl_enqueue_map_image(enqueue_data *data)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = data->mem_obj;
  void *ptr = NULL;
  CHECK_IMAGE(mem, image);

  if(data->unsync_map == 1)
    //because using unsync map in clEnqueueMapBuffer, so force use map_gtt here
    ptr = cl_mem_map_gtt(mem);
  else
    ptr = cl_mem_map_auto(mem);

  if (ptr == NULL) {
    err = CL_MAP_FAILURE;
    goto error;
  }
  data->ptr = ptr;

  if(mem->flags & CL_MEM_USE_HOST_PTR) {
    assert(mem->host_ptr);
    cl_mem_copy_image_region(data->origin, data->region,
                             mem->host_ptr, image->host_row_pitch, image->host_slice_pitch,
                             data->ptr, data->row_pitch, data->slice_pitch, image);
  }

error:
  return err;
}

cl_int cl_enqueue_unmap_mem_object(enqueue_data *data)
{
  cl_int err = CL_SUCCESS;
  int i;
  size_t mapped_size = 0;
  void * v_ptr = NULL;
  void * mapped_ptr = data->ptr;
  cl_mem memobj = data->mem_obj;

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
  /* can not find a mapped address? */
  INVALID_VALUE_IF(i == memobj->mapped_ptr_sz);

  if (memobj->flags & CL_MEM_USE_HOST_PTR) {
    assert(mapped_ptr >= memobj->host_ptr &&
      mapped_ptr + mapped_size <= memobj->host_ptr + memobj->size);
    /* Sync the data. */
    memcpy(v_ptr, mapped_ptr, mapped_size);
  } else {
    assert(v_ptr == mapped_ptr);
  }

  cl_mem_unmap_auto(memobj);

  /* shrink the mapped slot. */
  if (memobj->mapped_ptr_sz/2 > memobj->map_ref) {
    int j = 0;
    cl_mapped_ptr *new_ptr = (cl_mapped_ptr *)malloc(
                             sizeof(cl_mapped_ptr) * (memobj->mapped_ptr_sz/2));
    if (!new_ptr) {
      /* Just do nothing. */
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

error:
  return err;
}

cl_int cl_enqueue_native_kernel(enqueue_data *data)
{
  cl_int err = CL_SUCCESS;
  cl_uint num_mem_objects = (cl_uint)data->offset;
  const cl_mem *mem_list = data->mem_list;
  const void **args_mem_loc = (const void **)data->const_ptr;
  cl_uint i;

  for (i=0; i<num_mem_objects; ++i)
  {
      const cl_mem buffer = mem_list[i];
      CHECK_MEM(buffer);

      *((void **)args_mem_loc[i]) = cl_mem_map_auto(buffer);
  }
  data->user_func(data->ptr);

  for (i=0; i<num_mem_objects; ++i)
  {
      cl_mem_unmap_auto(mem_list[i]);
  }

  free(data->ptr);
error:
  return err;
}

cl_int cl_enqueue_handle(cl_event event, enqueue_data* data)
{
  /* if need profiling, add the submit timestamp here. */
  if (event && event->type != CL_COMMAND_USER
           && event->queue->props & CL_QUEUE_PROFILING_ENABLE) {
    cl_event_get_timestamp(event, CL_PROFILING_COMMAND_SUBMIT);
  }

  switch(data->type) {
    case EnqueueReadBuffer:
      return cl_enqueue_read_buffer(data);
    case EnqueueReadBufferRect:
      return cl_enqueue_read_buffer_rect(data);
    case EnqueueWriteBuffer:
      return cl_enqueue_write_buffer(data);
    case EnqueueWriteBufferRect:
      return cl_enqueue_write_buffer_rect(data);
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
    case EnqueueCopyBufferRect:
    case EnqueueCopyBuffer:
    case EnqueueCopyImage:
    case EnqueueCopyBufferToImage:
    case EnqueueCopyImageToBuffer:
    case EnqueueNDRangeKernel:
      cl_gpgpu_event_resume((cl_gpgpu_event)data->ptr);
      return CL_SUCCESS;
    case EnqueueNativeKernel:
      return cl_enqueue_native_kernel(data);
    default:
      return CL_SUCCESS;
  }
}

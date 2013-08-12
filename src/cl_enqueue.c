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
  void* src_ptr;

  if (!(src_ptr = cl_mem_map_auto(data->mem_obj))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  memcpy(data->ptr, (char*)src_ptr + data->offset, data->size);

  err = cl_mem_unmap_auto(data->mem_obj);

error:
  return err;
}

cl_int cl_enqueue_write_buffer(enqueue_data *data)
{
  cl_int err = CL_SUCCESS;
  void* dst_ptr;

  if (!(dst_ptr = cl_mem_map_auto(data->mem_obj))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  memcpy((char*)dst_ptr + data->offset, data->const_ptr, data->size);

  err = cl_mem_unmap_auto(data->mem_obj);

error:
  return err;
}

cl_int cl_enqueue_read_image(enqueue_data *data)
{
  cl_int err = CL_SUCCESS;
  void* src_ptr;

  cl_mem image = data->mem_obj;
  const size_t* origin = data->origin;
  const size_t* region = data->region;

  if (!(src_ptr = cl_mem_map_auto(image))) {
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

 err = cl_mem_unmap_auto(image);

error:
  return err;

}

cl_int cl_enqueue_write_image(enqueue_data *data)
{
  cl_int err = CL_SUCCESS;
  void* dst_ptr;

  cl_mem image = data->mem_obj;
  const size_t *origin = data->origin;
  const size_t *region = data->region;

  if (!(dst_ptr = cl_mem_map_auto(image))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  size_t offset = image->bpp*origin[0] + image->row_pitch*origin[1] + image->slice_pitch*origin[2];
  dst_ptr = (char*)dst_ptr + offset;

  if (!origin[0] && region[0] == image->w && data->row_pitch == image->row_pitch &&
      (region[2] == 1 || (!origin[1] && region[1] == image->h && data->slice_pitch == image->slice_pitch)))
  {
    memcpy(dst_ptr, data->ptr, region[2] == 1 ? data->row_pitch*region[1] : data->slice_pitch*region[2]);
  }
  else {
    cl_uint y, z;
    for (z = 0; z < region[2]; z++) {
      const char* src = data->const_ptr;
      char* dst = dst_ptr;
      for (y = 0; y < region[1]; y++) {
        memcpy(dst, src, image->bpp*region[0]);
        src += data->row_pitch;
        dst += image->row_pitch;
      }
      data->ptr = (char*)data->ptr + data->slice_pitch;
      dst_ptr = (char*)dst_ptr + image->slice_pitch;
    }
  }

  err = cl_mem_unmap_auto(image);

error:
  return err;

}

cl_int cl_enqueue_map_buffer(enqueue_data *data)
{

  void *ptr = NULL;
  cl_int err = CL_SUCCESS;
  void *mem_ptr = NULL;
  cl_int slot = -1;
  cl_mem buffer = data->mem_obj;

  if (!(ptr = cl_mem_map_auto(buffer))) {
    err = CL_MAP_FAILURE;
  }

  ptr = (char*)ptr + data->offset;

  if(buffer->flags & CL_MEM_USE_HOST_PTR) {
    assert(buffer->host_ptr);
    memcpy(buffer->host_ptr + data->offset, ptr, data->size);
    mem_ptr = buffer->host_ptr + data->offset;
  } else {
    mem_ptr = ptr;
  }

  /* Record the mapped address. */
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
  buffer->mapped_ptr[slot].size = data->size;
  buffer->map_ref++;

  data->ptr = mem_ptr;

error:
  return err;
}

cl_int cl_enqueue_map_image(enqueue_data *data)
{
  void *ptr = NULL;
  cl_int err = CL_SUCCESS;

  cl_mem image = data->mem_obj;
  const size_t *origin = data->origin;

  if (!(ptr = cl_mem_map_auto(image))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  size_t offset = image->bpp*origin[0] + image->row_pitch*origin[1] + image->slice_pitch*origin[2];
  data->ptr = (char*)ptr + offset;

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

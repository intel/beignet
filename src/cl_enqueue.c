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
#include "cl_driver.h"
#include "cl_event.h"
#include "cl_command_queue.h"
#include "cl_utils.h"
#include "cl_alloc.h"
#include "cl_device_enqueue.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

static cl_int
cl_enqueue_read_buffer(enqueue_data *data, cl_int status)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = data->mem_obj;

  if (status != CL_COMPLETE)
    return err;

  assert(mem->type == CL_MEM_BUFFER_TYPE ||
         mem->type == CL_MEM_SUBBUFFER_TYPE);
  struct _cl_mem_buffer *buffer = (struct _cl_mem_buffer *)mem;
  //cl_buffer_get_subdata sometime is very very very slow in linux kernel, in skl and chv,
  //and it is randomly. So temporary disable it, use map/copy/unmap to read.
  //Should re-enable it after find root cause.
  if (0 && !mem->is_userptr) {
    if (cl_buffer_get_subdata(mem->bo, data->offset + buffer->sub_offset,
                              data->size, data->ptr) != 0)
      err = CL_MAP_FAILURE;
  } else {
    void *src_ptr = cl_mem_map_auto(mem, 0);
    if (src_ptr == NULL)
      err = CL_MAP_FAILURE;
    else {
      //sometimes, application invokes read buffer, instead of map buffer, even if userptr is enabled
      //memcpy is not necessary for this case
      if (data->ptr != (char *)src_ptr + data->offset + buffer->sub_offset)
        memcpy(data->ptr, (char *)src_ptr + data->offset + buffer->sub_offset, data->size);
      cl_mem_unmap_auto(mem);
    }
  }
  return err;
}

static cl_int
cl_enqueue_read_buffer_rect(enqueue_data *data, cl_int status)
{
  cl_int err = CL_SUCCESS;
  void *src_ptr;
  void *dst_ptr;

  const size_t *origin = data->origin;
  const size_t *host_origin = data->host_origin;
  const size_t *region = data->region;

  cl_mem mem = data->mem_obj;

  if (status != CL_COMPLETE)
    return err;

  assert(mem->type == CL_MEM_BUFFER_TYPE ||
         mem->type == CL_MEM_SUBBUFFER_TYPE);
  struct _cl_mem_buffer *buffer = (struct _cl_mem_buffer *)mem;

  if (!(src_ptr = cl_mem_map_auto(mem, 0))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  size_t offset = origin[0] + data->row_pitch * origin[1] + data->slice_pitch * origin[2];
  src_ptr = (char *)src_ptr + offset + buffer->sub_offset;

  offset = host_origin[0] + data->host_row_pitch * host_origin[1] + data->host_slice_pitch * host_origin[2];
  dst_ptr = (char *)data->ptr + offset;

  if (data->row_pitch == region[0] && data->row_pitch == data->host_row_pitch &&
      (region[2] == 1 || (data->slice_pitch == region[0] * region[1] && data->slice_pitch == data->host_slice_pitch))) {
    memcpy(dst_ptr, src_ptr, region[2] == 1 ? data->row_pitch * region[1] : data->slice_pitch * region[2]);
  } else {
    cl_uint y, z;
    for (z = 0; z < region[2]; z++) {
      const char *src = src_ptr;
      char *dst = dst_ptr;
      for (y = 0; y < region[1]; y++) {
        memcpy(dst, src, region[0]);
        src += data->row_pitch;
        dst += data->host_row_pitch;
      }
      src_ptr = (char *)src_ptr + data->slice_pitch;
      dst_ptr = (char *)dst_ptr + data->host_slice_pitch;
    }
  }

  err = cl_mem_unmap_auto(mem);

error:
  return err;
}

static cl_int
cl_enqueue_write_buffer(enqueue_data *data, cl_int status)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = data->mem_obj;
  assert(mem->type == CL_MEM_BUFFER_TYPE ||
         mem->type == CL_MEM_SUBBUFFER_TYPE);
  struct _cl_mem_buffer *buffer = (struct _cl_mem_buffer *)mem;

  if (status != CL_COMPLETE)
    return err;

  if (mem->is_userptr) {
    void *dst_ptr = cl_mem_map_auto(mem, 1);
    if (dst_ptr == NULL)
      err = CL_MAP_FAILURE;
    else {
      memcpy((char *)dst_ptr + data->offset + buffer->sub_offset, data->const_ptr, data->size);
      cl_mem_unmap_auto(mem);
    }
  } else {
    if (cl_buffer_subdata(mem->bo, data->offset + buffer->sub_offset,
                          data->size, data->const_ptr) != 0)
      err = CL_MAP_FAILURE;
  }

  return err;
}

static cl_int
cl_enqueue_write_buffer_rect(enqueue_data *data, cl_int status)
{
  cl_int err = CL_SUCCESS;
  void *src_ptr;
  void *dst_ptr;

  const size_t *origin = data->origin;
  const size_t *host_origin = data->host_origin;
  const size_t *region = data->region;

  cl_mem mem = data->mem_obj;
  assert(mem->type == CL_MEM_BUFFER_TYPE ||
         mem->type == CL_MEM_SUBBUFFER_TYPE);
  struct _cl_mem_buffer *buffer = (struct _cl_mem_buffer *)mem;

  if (status != CL_COMPLETE)
    return err;

  if (!(dst_ptr = cl_mem_map_auto(mem, 1))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  size_t offset = origin[0] + data->row_pitch * origin[1] + data->slice_pitch * origin[2];
  dst_ptr = (char *)dst_ptr + offset + buffer->sub_offset;

  offset = host_origin[0] + data->host_row_pitch * host_origin[1] + data->host_slice_pitch * host_origin[2];
  src_ptr = (char *)data->const_ptr + offset;

  if (data->row_pitch == region[0] && data->row_pitch == data->host_row_pitch &&
      (region[2] == 1 || (data->slice_pitch == region[0] * region[1] && data->slice_pitch == data->host_slice_pitch))) {
    memcpy(dst_ptr, src_ptr, region[2] == 1 ? data->row_pitch * region[1] : data->slice_pitch * region[2]);
  } else {
    cl_uint y, z;
    for (z = 0; z < region[2]; z++) {
      const char *src = src_ptr;
      char *dst = dst_ptr;
      for (y = 0; y < region[1]; y++) {
        memcpy(dst, src, region[0]);
        src += data->host_row_pitch;
        dst += data->row_pitch;
      }
      src_ptr = (char *)src_ptr + data->host_slice_pitch;
      dst_ptr = (char *)dst_ptr + data->slice_pitch;
    }
  }

  err = cl_mem_unmap_auto(mem);

error:
  return err;
}

static cl_int
cl_enqueue_read_image(enqueue_data *data, cl_int status)
{
  cl_int err = CL_SUCCESS;
  void *src_ptr;

  cl_mem mem = data->mem_obj;
  CHECK_IMAGE(mem, image);
  const size_t *origin = data->origin;
  const size_t *region = data->region;

  if (status != CL_COMPLETE)
    return err;

  if (!(src_ptr = cl_mem_map_auto(mem, 0))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  size_t offset = image->offset + image->bpp*origin[0] + image->row_pitch*origin[1] + image->slice_pitch*origin[2];
  src_ptr = (char*)src_ptr + offset;

  if (!origin[0] && region[0] == image->w && data->row_pitch == image->row_pitch &&
      (region[2] == 1 || (!origin[1] && region[1] == image->h && data->slice_pitch == image->slice_pitch))) {
    memcpy(data->ptr, src_ptr, region[2] == 1 ? data->row_pitch * region[1] : data->slice_pitch * region[2]);
  } else {
    cl_uint y, z;
    for (z = 0; z < region[2]; z++) {
      const char *src = src_ptr;
      char *dst = data->ptr;
      for (y = 0; y < region[1]; y++) {
        memcpy(dst, src, image->bpp * region[0]);
        src += image->row_pitch;
        dst += data->row_pitch;
      }
      src_ptr = (char *)src_ptr + image->slice_pitch;
      data->ptr = (char *)data->ptr + data->slice_pitch;
    }
  }

  err = cl_mem_unmap_auto(mem);

error:
  return err;
}

static cl_int
cl_enqueue_write_image(enqueue_data *data, cl_int status)
{
  cl_int err = CL_SUCCESS;
  void *dst_ptr;

  cl_mem mem = data->mem_obj;

  CHECK_IMAGE(mem, image);

  if (status != CL_COMPLETE)
    return err;

  if (!(dst_ptr = cl_mem_map_auto(mem, 1))) {
    err = CL_MAP_FAILURE;
    goto error;
  }
  cl_mem_copy_image_region(data->origin, data->region,
                           dst_ptr + image->offset,
                           image->row_pitch, image->slice_pitch,
                           data->const_ptr, data->row_pitch,
                           data->slice_pitch, image, CL_TRUE, CL_FALSE);
  err = cl_mem_unmap_auto(mem);

error:
  return err;
}

static cl_int
cl_enqueue_map_buffer(enqueue_data *data, cl_int status)
{
  void *ptr = NULL;
  cl_int err = CL_SUCCESS;
  cl_mem mem = data->mem_obj;
  assert(mem->type == CL_MEM_BUFFER_TYPE ||
         mem->type == CL_MEM_SUBBUFFER_TYPE ||
         mem->type == CL_MEM_SVM_TYPE);
  struct _cl_mem_buffer* buffer = (struct _cl_mem_buffer *)mem;

  if (status == CL_SUBMITTED) {
    if (buffer->base.is_userptr) {
      ptr = buffer->base.host_ptr;
    } else {
      if ((ptr = cl_mem_map_gtt_unsync(&buffer->base)) == NULL) {
        err = CL_MAP_FAILURE;
        return err;
      }
    }
    data->ptr = ptr;
  } else if (status == CL_COMPLETE) {
    if (mem->is_userptr)
      ptr = cl_mem_map_auto(mem, data->write_map ? 1 : 0);
    else {
      if (data->unsync_map == 1)
        //because using unsync map in clEnqueueMapBuffer, so force use map_gtt here
        ptr = cl_mem_map_gtt(mem);
      else
        ptr = cl_mem_map_auto(mem, data->write_map ? 1 : 0);
    }

    if (ptr == NULL) {
      err = CL_MAP_FAILURE;
      return err;
    }
    data->ptr = ptr;

    if ((mem->flags & CL_MEM_USE_HOST_PTR) && !mem->is_userptr) {
      assert(mem->host_ptr);
      ptr = (char *)ptr + data->offset + buffer->sub_offset;
      memcpy(mem->host_ptr + data->offset + buffer->sub_offset, ptr, data->size);
    }
  }

  return err;
}

static cl_int
cl_enqueue_map_image(enqueue_data *data, cl_int status)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = data->mem_obj;
  void *ptr = NULL;
  size_t row_pitch = 0;
  CHECK_IMAGE(mem, image);

  if (status == CL_SUBMITTED) {
    if ((ptr = cl_mem_map_gtt_unsync(mem)) == NULL) {
      err = CL_MAP_FAILURE;
      goto error;
    }
    data->ptr = ptr;
  } else if (status == CL_COMPLETE) {
    if (data->unsync_map == 1)
      //because using unsync map in clEnqueueMapBuffer, so force use map_gtt here
      ptr = cl_mem_map_gtt(mem);
    else
      ptr = cl_mem_map_auto(mem, data->write_map ? 1 : 0);

    if (ptr == NULL) {
      err = CL_MAP_FAILURE;
      goto error;
    }

    data->ptr = (char*)ptr + image->offset;
    if (image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
      row_pitch = image->slice_pitch;
    else
      row_pitch = image->row_pitch;

    if(mem->flags & CL_MEM_USE_HOST_PTR) {
      assert(mem->host_ptr);
      if (!mem->is_userptr)
        //src and dst need add offset in function cl_mem_copy_image_region
        cl_mem_copy_image_region(data->origin, data->region,
                                 mem->host_ptr, image->host_row_pitch, image->host_slice_pitch,
                                 data->ptr, row_pitch, image->slice_pitch, image, CL_TRUE, CL_TRUE);
    }
  }

error:
  return err;
}

static cl_int
cl_enqueue_unmap_mem_object(enqueue_data *data, cl_int status)
{
  cl_int err = CL_SUCCESS;
  int i, j;
  size_t mapped_size = 0;
  size_t origin[3], region[3];
  void *v_ptr = NULL;
  void *mapped_ptr = data->ptr;
  cl_mem memobj = data->mem_obj;
  size_t row_pitch = 0;

  if (status != CL_COMPLETE)
    return err;

  assert(memobj->mapped_ptr_sz >= memobj->map_ref);
  INVALID_VALUE_IF(!mapped_ptr);
  for (i = 0; i < memobj->mapped_ptr_sz; i++) {
    if (memobj->mapped_ptr[i].ptr == mapped_ptr) {
      memobj->mapped_ptr[i].ptr = NULL;
      mapped_size = memobj->mapped_ptr[i].size;
      v_ptr = memobj->mapped_ptr[i].v_ptr;
      for (j = 0; j < 3; j++) {
        region[j] = memobj->mapped_ptr[i].region[j];
        origin[j] = memobj->mapped_ptr[i].origin[j];
        memobj->mapped_ptr[i].region[j] = 0;
        memobj->mapped_ptr[i].origin[j] = 0;
      }
      memobj->mapped_ptr[i].size = 0;
      memobj->mapped_ptr[i].v_ptr = NULL;
      memobj->map_ref--;
      break;
    }
  }
  /* can not find a mapped address? */
  INVALID_VALUE_IF(i == memobj->mapped_ptr_sz);

  if (memobj->flags & CL_MEM_USE_HOST_PTR) {
    if (memobj->type == CL_MEM_BUFFER_TYPE ||
        memobj->type == CL_MEM_SUBBUFFER_TYPE ||
        memobj->type == CL_MEM_SVM_TYPE) {
      assert(mapped_ptr >= memobj->host_ptr &&
             mapped_ptr + mapped_size <= memobj->host_ptr + memobj->size);
      /* Sync the data. */
      if (!memobj->is_userptr)
        memcpy(v_ptr, mapped_ptr, mapped_size);
    } else {
      CHECK_IMAGE(memobj, image);

      if (image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY)
        row_pitch = image->slice_pitch;
      else
        row_pitch = image->row_pitch;
      if (!memobj->is_userptr)
        //v_ptr have added offset, host_ptr have not added offset.
        cl_mem_copy_image_region(origin, region, v_ptr, row_pitch, image->slice_pitch,
                                 memobj->host_ptr, image->host_row_pitch, image->host_slice_pitch,
                                 image, CL_FALSE, CL_TRUE);
    }
  } else {
    assert(v_ptr == mapped_ptr);
  }

  cl_mem_unmap_auto(memobj);

  /* shrink the mapped slot. */
  if (memobj->mapped_ptr_sz / 2 > memobj->map_ref) {
    int j = 0;
    cl_mapped_ptr *new_ptr = (cl_mapped_ptr *)malloc(
      sizeof(cl_mapped_ptr) * (memobj->mapped_ptr_sz / 2));
    if (!new_ptr) {
      /* Just do nothing. */
      goto error;
    }
    memset(new_ptr, 0, (memobj->mapped_ptr_sz / 2) * sizeof(cl_mapped_ptr));

    for (i = 0; i < memobj->mapped_ptr_sz; i++) {
      if (memobj->mapped_ptr[i].ptr) {
        new_ptr[j] = memobj->mapped_ptr[i];
        j++;
        assert(j < memobj->mapped_ptr_sz / 2);
      }
    }
    memobj->mapped_ptr_sz = memobj->mapped_ptr_sz / 2;
    free(memobj->mapped_ptr);
    memobj->mapped_ptr = new_ptr;
  }

error:
  return err;
}

static cl_int
cl_enqueue_native_kernel(enqueue_data *data, cl_int status)
{
  cl_int err = CL_SUCCESS;
  cl_uint num_mem_objects = (cl_uint)data->offset;
  const cl_mem *mem_list = data->mem_list;
  const void **args_mem_loc = (const void **)data->const_ptr;
  cl_uint i;

  if (status != CL_COMPLETE)
    return err;

  for (i = 0; i < num_mem_objects; ++i) {
    const cl_mem buffer = mem_list[i];
    CHECK_MEM(buffer);

    *((void **)args_mem_loc[i]) = cl_mem_map_auto(buffer, 0);
  }
  data->user_func(data->ptr);

  for (i = 0; i < num_mem_objects; ++i) {
    cl_mem_unmap_auto(mem_list[i]);
  }

error:
  return err;
}

cl_int cl_enqueue_svm_free(enqueue_data *data, cl_int status) {
  int i;
  void **pointers = data->pointers;
  uint num_svm_ptrs = data->size;
  cl_int err = CL_SUCCESS;

  if (status != CL_COMPLETE)
    return err;

  if(data->free_func) {
    data->free_func(data->queue, num_svm_ptrs, pointers, data->ptr);
  } else {
    for(i=0; i<num_svm_ptrs; i++)
      cl_mem_svm_delete(data->queue->ctx, pointers[i]);
  }

  free(pointers);
  return CL_SUCCESS;
}

cl_int cl_enqueue_svm_mem_copy(enqueue_data *data, cl_int status) {
  cl_mem mem;
  size_t size = data->size;
  const char* src_ptr = (const char *)data->const_ptr;
  char *dst_ptr = (char *)data->ptr;
  cl_int err = CL_SUCCESS;
  int i;

  if (status != CL_COMPLETE)
    return err;

  if((mem = cl_context_get_svm_from_ptr(data->queue->ctx, data->ptr)) != NULL) {
      dst_ptr = (char *)cl_mem_map_auto(mem, 1);
  }

  if((mem = cl_context_get_svm_from_ptr(data->queue->ctx, data->const_ptr)) != NULL) {
      src_ptr = (const char *)cl_mem_map_auto(mem, 0);
  }

  for(i=0; i<size; i++) {
    dst_ptr[i] = src_ptr[i];
  }

  return CL_SUCCESS;
}

cl_int cl_enqueue_svm_mem_fill(enqueue_data *data, cl_int status) {
  cl_mem mem;
  size_t size = data->size;
  size_t pattern_size = data->pattern_size;
  const char* pattern = (const char *)data->const_ptr;
  char *ptr = (char *)data->ptr;
  cl_int err = CL_SUCCESS;
  int i, j;

  if (status != CL_COMPLETE)
    return err;

  if((mem = cl_context_get_svm_from_ptr(data->queue->ctx, data->ptr)) != NULL) {
      ptr = (char *)cl_mem_map_auto(mem, 1);
  }

  for(i=0; i<size; ) {
    for(j=0; j<pattern_size; j++) {
      ptr[i++] = pattern[j];
    }
  }

  return CL_SUCCESS;
}

static cl_int
cl_enqueue_ndrange(enqueue_data *data, cl_int status)
{
  cl_int err = CL_SUCCESS;

  if (status == CL_SUBMITTED) {
    err = cl_command_queue_flush_gpgpu(data->gpgpu);
    //if it is the last ndrange of an cl enqueue api,
    //check the device enqueue information.
    if (data->mid_event_of_enq == 0) {
      assert(data->queue);
      cl_device_enqueue_parse_result(data->queue, data->gpgpu);
    }
  } else if (status == CL_COMPLETE) {
    void *batch_buf = cl_gpgpu_ref_batch_buf(data->gpgpu);
    cl_gpgpu_sync(batch_buf);
    cl_gpgpu_unref_batch_buf(batch_buf);
  }

  return err;
}

static cl_int
cl_enqueue_marker_or_barrier(enqueue_data *data, cl_int status)
{
  return CL_COMPLETE;
}

LOCAL void
cl_enqueue_delete(enqueue_data *data)
{
  if (data == NULL)
    return;

  if (data->type == EnqueueCopyBufferRect ||
      data->type == EnqueueCopyBuffer ||
      data->type == EnqueueCopyImage ||
      data->type == EnqueueCopyBufferToImage ||
      data->type == EnqueueCopyImageToBuffer ||
      data->type == EnqueueNDRangeKernel ||
      data->type == EnqueueFillBuffer ||
      data->type == EnqueueFillImage) {
    if (data->gpgpu) {
      cl_gpgpu_delete(data->gpgpu);
      data->gpgpu = NULL;
    }
    return;
  }

  if (data->type == EnqueueNativeKernel) {
    if (data->mem_list) {
      cl_free((void*)data->mem_list);
      data->mem_list = NULL;
    }
    if (data->ptr) {
      cl_free((void*)data->ptr);
      data->ptr = NULL;
    }
    if (data->const_ptr) {
      cl_free((void*)data->const_ptr);
      data->const_ptr = NULL;
    }
  }
}

LOCAL cl_int
cl_enqueue_handle(enqueue_data *data, cl_int status)
{
  switch (data->type) {
  case EnqueueReturnSuccesss:
    return CL_SUCCESS;
  case EnqueueReadBuffer:
    return cl_enqueue_read_buffer(data, status);
  case EnqueueReadBufferRect:
    return cl_enqueue_read_buffer_rect(data, status);
  case EnqueueWriteBuffer:
    return cl_enqueue_write_buffer(data, status);
  case EnqueueWriteBufferRect:
    return cl_enqueue_write_buffer_rect(data, status);
  case EnqueueReadImage:
    return cl_enqueue_read_image(data, status);
  case EnqueueWriteImage:
    return cl_enqueue_write_image(data, status);
  case EnqueueMapBuffer:
    return cl_enqueue_map_buffer(data, status);
  case EnqueueMapImage:
    return cl_enqueue_map_image(data, status);
  case EnqueueUnmapMemObject:
    return cl_enqueue_unmap_mem_object(data, status);
  case EnqueueSVMFree:
    return cl_enqueue_svm_free(data, status);
  case EnqueueSVMMemCopy:
    return cl_enqueue_svm_mem_copy(data, status);
  case EnqueueSVMMemFill:
    return cl_enqueue_svm_mem_fill(data, status);
  case EnqueueMarker:
  case EnqueueBarrier:
    return cl_enqueue_marker_or_barrier(data, status);
  case EnqueueCopyBufferRect:
  case EnqueueCopyBuffer:
  case EnqueueCopyImage:
  case EnqueueCopyBufferToImage:
  case EnqueueCopyImageToBuffer:
  case EnqueueNDRangeKernel:
  case EnqueueFillBuffer:
  case EnqueueFillImage:
    //return cl_event_flush(event);
    return cl_enqueue_ndrange(data, status);
  case EnqueueNativeKernel:
    return cl_enqueue_native_kernel(data, status);
  case EnqueueMigrateMemObj:
  default:
    return CL_SUCCESS;
  }
}

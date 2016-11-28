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

#include "cl_mem.h"
#include "cl_image.h"
#include "cl_context.h"
#include "cl_utils.h"
#include "cl_alloc.h"
#include "cl_device_id.h"
#include "cl_driver.h"
#include "cl_khr_icd.h"
#include "cl_kernel.h"
#include "cl_command_queue.h"
#include "cl_cmrt.h"
#include "cl_enqueue.h"

#include "CL/cl.h"
#include "CL/cl_intel.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#define FIELD_SIZE(CASE,TYPE)               \
  case JOIN(CL_,CASE):                      \
    if(param_value_size_ret)                \
      *param_value_size_ret = sizeof(TYPE); \
    if(!param_value)                        \
      return CL_SUCCESS;                    \
    if(param_value_size < sizeof(TYPE))     \
      return CL_INVALID_VALUE;              \
    break;

#define MAX_TILING_SIZE                             128 * MB

LOCAL cl_mem_object_type
cl_get_mem_object_type(cl_mem mem)
{
  switch (mem->type) {
    case CL_MEM_BUFFER_TYPE:
    case CL_MEM_SUBBUFFER_TYPE:
      return CL_MEM_OBJECT_BUFFER;
    case CL_MEM_IMAGE_TYPE:
    case CL_MEM_GL_IMAGE_TYPE:
    {
      struct _cl_mem_image *image = cl_mem_image(mem);
      return image->image_type;
    }
    default:
      return CL_MEM_OBJECT_BUFFER;
  }
}

LOCAL cl_int
cl_get_pipe_info(cl_mem mem,
                    cl_mem_info param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret)
{
  _cl_mem_pipe *pipe;
  switch(param_name)
  {
    FIELD_SIZE(PIPE_PACKET_SIZE, cl_uint);
    FIELD_SIZE(PIPE_MAX_PACKETS, cl_uint);
  default:
    return CL_INVALID_VALUE;
  }

  if(mem->type != CL_MEM_PIPE_TYPE)
    return CL_INVALID_MEM_OBJECT;

  pipe = cl_mem_pipe(mem);

  switch(param_name)
  {
  case CL_PIPE_PACKET_SIZE:
    *((cl_uint *)param_value) = pipe->packet_size;
    break;
  case CL_PIPE_MAX_PACKETS:
    *((cl_uint *)param_value) = pipe->max_packets;
    break;
  }

  return CL_SUCCESS;
}

LOCAL cl_mem
cl_mem_allocate(enum cl_mem_type type,
                cl_context ctx,
                cl_mem_flags flags,
                size_t sz,
                cl_int is_tiled,
                void *host_ptr,         //pointer from application
                cl_mem buffer,          //image2D from buffer
                cl_int *errcode)
{
  cl_buffer_mgr bufmgr = NULL;
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  size_t alignment = 64;

  assert(ctx);

  /* Allocate and inialize the structure itself */
  if (type == CL_MEM_IMAGE_TYPE) {
    struct _cl_mem_image *image = NULL;
    TRY_ALLOC (image, CALLOC(struct _cl_mem_image));
    mem = &image->base;
  } else if (type == CL_MEM_GL_IMAGE_TYPE ) {
    struct _cl_mem_gl_image *gl_image = NULL;
    TRY_ALLOC (gl_image, CALLOC(struct _cl_mem_gl_image));
    mem = &gl_image->base.base;
  } else if (type == CL_MEM_BUFFER1D_IMAGE_TYPE) {
    struct _cl_mem_buffer1d_image *buffer1d_image = NULL;
    TRY_ALLOC(buffer1d_image, CALLOC(struct _cl_mem_buffer1d_image));
    mem = &buffer1d_image->base.base;
  } else if (type == CL_MEM_PIPE_TYPE) {
    _cl_mem_pipe *pipe = NULL;
    TRY_ALLOC(pipe, CALLOC(struct _cl_mem_pipe));
    mem = &pipe->base;
  } else {
    struct _cl_mem_buffer *buffer = NULL;
    TRY_ALLOC (buffer, CALLOC(struct _cl_mem_buffer));
    mem = &buffer->base;
  }

  CL_OBJECT_INIT_BASE(mem, CL_OBJECT_MEM_MAGIC);
  list_init(&mem->dstr_cb_head);
  mem->type = type;
  mem->flags = flags;
  mem->is_userptr = 0;
  mem->offset = 0;
  mem->is_svm = 0;
  mem->cmrt_mem = NULL;
  if (mem->type == CL_MEM_IMAGE_TYPE) {
    cl_mem_image(mem)->is_image_from_buffer = 0;
  }

  if (sz != 0) {
    /* Pinning will require stricter alignment rules */
    if ((flags & CL_MEM_PINNABLE) || is_tiled)
      alignment = 4096;

    /* Allocate space in memory */
    bufmgr = cl_context_get_bufmgr(ctx);
    assert(bufmgr);

#ifdef HAS_USERPTR
    uint8_t bufCreated = 0;
    if (ctx->devices[0]->host_unified_memory) {
      int page_size = getpagesize();
      int cacheline_size = 0;
      cl_get_device_info(ctx->devices[0], CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(cacheline_size), &cacheline_size, NULL);

      if (type == CL_MEM_BUFFER_TYPE) {
        if (flags & CL_MEM_USE_HOST_PTR) {
          assert(host_ptr != NULL);
          cl_mem svm_mem = NULL;
          if((svm_mem = cl_context_get_svm_from_ptr(ctx, host_ptr)) != NULL)
            mem->is_svm = 1;
          /* userptr not support tiling */
          if (!is_tiled) {
            if(svm_mem != NULL) {  //SVM always paged alignment
              mem->offset = 0;
              mem->is_userptr = 1;
              mem->bo = svm_mem->bo;
              cl_mem_add_ref(svm_mem);
              bufCreated = 1;
            } else if ((ALIGN((unsigned long)host_ptr, cacheline_size) == (unsigned long)host_ptr) &&
                (ALIGN((unsigned long)sz, cacheline_size) == (unsigned long)sz)) {
              void* aligned_host_ptr = (void*)(((unsigned long)host_ptr) & (~(page_size - 1)));
              mem->offset = host_ptr - aligned_host_ptr;
              mem->is_userptr = 1;
              size_t aligned_sz = ALIGN((mem->offset + sz), page_size);
              mem->bo = cl_buffer_alloc_userptr(bufmgr, "CL userptr memory object", aligned_host_ptr, aligned_sz, 0);
              bufCreated = 1;
            }
          }
        }
        else if (flags & CL_MEM_ALLOC_HOST_PTR) {
          const size_t alignedSZ = ALIGN(sz, page_size);
          void* internal_host_ptr = cl_aligned_malloc(alignedSZ, page_size);
          mem->host_ptr = internal_host_ptr;
          mem->is_userptr = 1;
          mem->bo = cl_buffer_alloc_userptr(bufmgr, "CL userptr memory object", internal_host_ptr, alignedSZ, 0);
          bufCreated = 1;
        }
      } else if (type == CL_MEM_IMAGE_TYPE) {
        if (host_ptr != NULL) {
          assert(flags & CL_MEM_USE_HOST_PTR);
          assert(!is_tiled);
          assert(ALIGN((unsigned long)host_ptr, cacheline_size) == (unsigned long)host_ptr);
          void* aligned_host_ptr = (void*)(((unsigned long)host_ptr) & (~(page_size - 1)));
          mem->offset = host_ptr - aligned_host_ptr;
          mem->is_userptr = 1;
          size_t aligned_sz = ALIGN((mem->offset + sz), page_size);
          mem->bo = cl_buffer_alloc_userptr(bufmgr, "CL userptr memory object", aligned_host_ptr, aligned_sz, 0);
          bufCreated = 1;
        }
      }
    }

    if(type == CL_MEM_IMAGE_TYPE && buffer != NULL) {
      // if create image from USE_HOST_PTR buffer, the buffer's base address need be aligned.
      if(buffer->is_userptr) {
        int base_alignement = 0;
        cl_get_device_info(ctx->devices[0], CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT, sizeof(base_alignement), &base_alignement, NULL);
        if(ALIGN((unsigned long)buffer->host_ptr, base_alignement) != (unsigned long)buffer->host_ptr) {
          err = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
          goto error;
        }
      }
      // if the image if created from buffer, should use the bo directly to share same bo.
      mem->bo = buffer->bo;
      cl_mem_image(mem)->is_image_from_buffer = 1;
      bufCreated = 1;
    }

    if (!bufCreated)
      mem->bo = cl_buffer_alloc(bufmgr, "CL memory object", sz, alignment);
#else
    if(type == CL_MEM_IMAGE_TYPE && buffer != NULL) {
      // if the image if created from buffer, should use the bo directly to share same bo.
      mem->bo = buffer->bo;
      cl_mem_image(mem)->is_image_from_buffer = 1;
    } else
      mem->bo = cl_buffer_alloc(bufmgr, "CL memory object", sz, alignment);
#endif

    if (UNLIKELY(mem->bo == NULL)) {
      err = CL_MEM_OBJECT_ALLOCATION_FAILURE;
      goto error;
    }
    mem->size = sz;
  }

  /* Append the buffer in the context buffer list */
  cl_context_add_mem(ctx, mem);

exit:
  if (errcode)
    *errcode = err;
  return mem;
error:
  cl_mem_delete(mem);
  mem = NULL;
  goto exit;

}

LOCAL cl_int
cl_mem_is_valid(cl_mem mem, cl_context ctx)
{
  struct list_node *pos;
  cl_base_object pbase_object;

  CL_OBJECT_LOCK(ctx);
  list_for_each (pos, (&ctx->mem_objects)) {
    pbase_object = list_entry(pos, _cl_base_object, node);
    if (pbase_object == (cl_base_object)mem) {
      if (UNLIKELY(!CL_OBJECT_IS_MEM(mem))) {
        CL_OBJECT_UNLOCK(ctx);
        return CL_INVALID_MEM_OBJECT;
      }

      CL_OBJECT_UNLOCK(ctx);
      return CL_SUCCESS;
    }
  }

  CL_OBJECT_UNLOCK(ctx);
  return CL_INVALID_MEM_OBJECT;
}

LOCAL cl_mem
cl_mem_new_buffer(cl_context ctx,
                  cl_mem_flags flags,
                  size_t sz,
                  void *data,
                  cl_int *errcode_ret)
{
  /* Possible mem type combination:
       CL_MEM_ALLOC_HOST_PTR
       CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR
       CL_MEM_USE_HOST_PTR
       CL_MEM_COPY_HOST_PTR   */

  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;
  cl_ulong max_mem_size;

  if (UNLIKELY(sz == 0)) {
    err = CL_INVALID_BUFFER_SIZE;
    goto error;
  }

  if (UNLIKELY(((flags & CL_MEM_READ_WRITE)
                  && (flags & (CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY)))
		      || ((flags & CL_MEM_READ_ONLY) && (flags & (CL_MEM_WRITE_ONLY)))
              || ((flags & CL_MEM_ALLOC_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR))
              || ((flags & CL_MEM_COPY_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR))
              || ((flags & CL_MEM_HOST_READ_ONLY) && (flags & CL_MEM_HOST_NO_ACCESS))
              || ((flags & CL_MEM_HOST_READ_ONLY) && (flags & CL_MEM_HOST_WRITE_ONLY))
              || ((flags & CL_MEM_HOST_WRITE_ONLY) && (flags & CL_MEM_HOST_NO_ACCESS))
              || ((flags & (~(CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY
                        | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR
                        | CL_MEM_USE_HOST_PTR | CL_MEM_HOST_WRITE_ONLY
                        | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))) != 0))) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  /* This flag is valid only if host_ptr is not NULL */
  if (UNLIKELY((((flags & CL_MEM_COPY_HOST_PTR) ||
                (flags & CL_MEM_USE_HOST_PTR)) &&
                data == NULL))
               || (!(flags & (CL_MEM_COPY_HOST_PTR
                            |CL_MEM_USE_HOST_PTR))
                    && (data != NULL))) {
    err = CL_INVALID_HOST_PTR;
    goto error;
  }

  if ((err = cl_get_device_info(ctx->devices[0],
                                CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                                sizeof(max_mem_size),
                                &max_mem_size,
                                NULL)) != CL_SUCCESS) {
    goto error;
  }

  if (UNLIKELY(sz > max_mem_size)) {
    err = CL_INVALID_BUFFER_SIZE;
    goto error;
  }

  /* HSW: Byte scattered Read/Write has limitation that
     the buffer size must be a multiple of 4 bytes. */
  sz = ALIGN(sz, 4);

  /* Create the buffer in video memory */
  mem = cl_mem_allocate(CL_MEM_BUFFER_TYPE, ctx, flags, sz, CL_FALSE, data, NULL, &err);
  if (mem == NULL || err != CL_SUCCESS)
    goto error;

  /* Copy the data if required */
  if (flags & CL_MEM_COPY_HOST_PTR) {
    if (mem->is_userptr)
      memcpy(mem->host_ptr, data, sz);
    else
      cl_buffer_subdata(mem->bo, 0, sz, data);
  }

  if ((flags & CL_MEM_USE_HOST_PTR) && !mem->is_userptr)
    cl_buffer_subdata(mem->bo, 0, sz, data);

  if (flags & CL_MEM_USE_HOST_PTR)
    mem->host_ptr = data;

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
error:
  cl_mem_delete(mem);
  mem = NULL;
  goto exit;
}

LOCAL cl_mem
cl_mem_new_sub_buffer(cl_mem buffer,
                      cl_mem_flags flags,
                      cl_buffer_create_type create_type,
                      const void *create_info,
                      cl_int *errcode_ret)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;
  struct _cl_mem_buffer *sub_buf = NULL;

  if (buffer->type != CL_MEM_BUFFER_TYPE) {
    err = CL_INVALID_MEM_OBJECT;
    goto error;
  }

  if (flags && (((buffer->flags & CL_MEM_WRITE_ONLY) && (flags & (CL_MEM_READ_WRITE|CL_MEM_READ_ONLY)))
          || ((buffer->flags & CL_MEM_READ_ONLY) && (flags & (CL_MEM_READ_WRITE|CL_MEM_WRITE_ONLY)))
          || (flags & (CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR))
          || ((flags & CL_MEM_HOST_READ_ONLY) && (flags & CL_MEM_HOST_NO_ACCESS))
          || ((flags & CL_MEM_HOST_READ_ONLY) && (flags & CL_MEM_HOST_WRITE_ONLY))
          || ((flags & CL_MEM_HOST_WRITE_ONLY) && (flags & CL_MEM_HOST_NO_ACCESS)))) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if((flags & (CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY | CL_MEM_READ_WRITE)) == 0) {
    flags |= buffer->flags & (CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY | CL_MEM_READ_WRITE);
  }
  flags |= buffer->flags & (CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR);
  if((flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)) == 0) {
    flags |= buffer->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS);
  }

  if (create_type != CL_BUFFER_CREATE_TYPE_REGION) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if (!create_info) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  cl_buffer_region *info = (cl_buffer_region *)create_info;

  if (!info->size) {
    err = CL_INVALID_BUFFER_SIZE;
    goto error;
  }

  if (info->origin > buffer->size || info->origin + info->size > buffer->size) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if (info->origin & (buffer->ctx->devices[0]->mem_base_addr_align / 8 - 1)) {
    err = CL_MISALIGNED_SUB_BUFFER_OFFSET;
    goto error;
  }

  /* Now create the sub buffer and link it to the buffer. */
  TRY_ALLOC (sub_buf, CALLOC(struct _cl_mem_buffer));
  mem = &sub_buf->base;

  CL_OBJECT_INIT_BASE(mem, CL_OBJECT_MEM_MAGIC);
  list_init(&mem->dstr_cb_head);
  mem->type = CL_MEM_SUBBUFFER_TYPE;
  mem->flags = flags;
  mem->offset = buffer->offset;
  mem->is_userptr = buffer->is_userptr;
  sub_buf->parent = (struct _cl_mem_buffer*)buffer;

  cl_mem_add_ref(buffer);
  /* Append the buffer in the parent buffer list */
  pthread_mutex_lock(&((struct _cl_mem_buffer*)buffer)->sub_lock);
  sub_buf->sub_next = ((struct _cl_mem_buffer*)buffer)->subs;
  if (((struct _cl_mem_buffer*)buffer)->subs != NULL)
    ((struct _cl_mem_buffer*)buffer)->subs->sub_prev = sub_buf;
  ((struct _cl_mem_buffer*)buffer)->subs = sub_buf;
  pthread_mutex_unlock(&((struct _cl_mem_buffer*)buffer)->sub_lock);

  mem->bo = buffer->bo;
  mem->size = info->size;
  sub_buf->sub_offset = info->origin;
  if (buffer->flags & CL_MEM_USE_HOST_PTR || buffer->flags & CL_MEM_COPY_HOST_PTR || buffer->flags & CL_MEM_ALLOC_HOST_PTR) {
    mem->host_ptr = buffer->host_ptr;
  }

  /* Append the buffer in the context buffer list */
  cl_context_add_mem(buffer->ctx, mem);

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
error:
  cl_mem_delete(mem);
  mem = NULL;
  goto exit;
}

cl_mem cl_mem_new_pipe(cl_context ctx,
                             cl_mem_flags flags,
                             cl_uint packet_size,
                             cl_uint max_packets,
                             cl_int *errcode_ret)
{
  _cl_mem_pipe* pipe = NULL;
  cl_uint *ptr = NULL;
  cl_mem mem = NULL;
  cl_int err;
  cl_uint sz;
  if(UNLIKELY((pipe = CALLOC(_cl_mem_pipe)) == NULL)) {
    err = CL_OUT_OF_RESOURCES;
    goto error;
  }

  sz = packet_size * max_packets;
  assert(sz != 0);

  /* HSW: Byte scattered Read/Write has limitation that
     the buffer size must be a multiple of 4 bytes. */
  sz = ALIGN(sz, 4);

  sz += 128;   //The head of pipe is for data struct, and alignment to 128 byte for max data type double16

  mem = cl_mem_allocate(CL_MEM_PIPE_TYPE, ctx, flags, sz, CL_FALSE,NULL , NULL, &err);

  if (mem == NULL || err != CL_SUCCESS)
    goto error;

  ptr = cl_mem_map_auto(mem, 1);
  if(ptr == NULL){
    err = CL_OUT_OF_RESOURCES;
    goto error;
  }
  ptr[0] = max_packets;
  ptr[1] = packet_size;
  ptr[2] = 0;              //write ptr
  ptr[3] = 0;              //read ptr
  ptr[4] = 0;              //reservation read ptr
  ptr[5] = 0;              //reservation write ptr
  ptr[6] = 0;              //packet num
  cl_mem_unmap(mem);

  pipe = cl_mem_pipe(mem);
  pipe->flags = flags;
  pipe->packet_size = packet_size;
  pipe->max_packets = max_packets;

  return mem;

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
error:
  cl_mem_delete(mem);
  mem = NULL;
  goto exit;

}

void cl_mem_replace_buffer(cl_mem buffer, cl_buffer new_bo)
{
  cl_buffer_unreference(buffer->bo);
  buffer->bo = new_bo;
  cl_buffer_reference(new_bo);
  if (buffer->type != CL_MEM_SUBBUFFER_TYPE)
    return;

  struct _cl_mem_buffer *it = ((struct _cl_mem_buffer*)buffer)->sub_next;
  for( ; it != (struct _cl_mem_buffer*)buffer; it = it->sub_next)
  {
    cl_buffer_unreference(it->base.bo);
    it->base.bo = new_bo;
    cl_buffer_reference(new_bo);
  }
}

void* cl_mem_svm_allocate(cl_context ctx, cl_svm_mem_flags flags,
                                 size_t size, unsigned int alignment)
{
  cl_int err = CL_SUCCESS;
  size_t max_mem_size;

  if(UNLIKELY(alignment & (alignment - 1)))
    return NULL;

  if ((err = cl_get_device_info(ctx->devices[0],
                                 CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                                 sizeof(max_mem_size),
                                 &max_mem_size,
                                 NULL)) != CL_SUCCESS) {
      return NULL;
  }

  if(UNLIKELY(size == 0 || size > max_mem_size)) {
    return NULL;
  }

  if (flags & (CL_MEM_SVM_FINE_GRAIN_BUFFER | CL_MEM_SVM_ATOMICS)) {
    return NULL;
  }
  if (flags && ((flags & (CL_MEM_SVM_FINE_GRAIN_BUFFER | CL_MEM_SVM_FINE_GRAIN_BUFFER))
          || ((flags & CL_MEM_WRITE_ONLY) && (flags & CL_MEM_READ_ONLY))
          || ((flags & CL_MEM_WRITE_ONLY) && (flags & CL_MEM_READ_WRITE))
          || ((flags & CL_MEM_READ_ONLY) && (flags & CL_MEM_READ_WRITE)))) {
    return NULL;
  }

  void * ptr = NULL;
#ifdef HAS_BO_SET_SOFTPIN
  cl_buffer_mgr bufmgr = NULL;
  cl_mem mem;
  _cl_mem_svm* svm;
  if(UNLIKELY((svm = CALLOC(_cl_mem_svm)) == NULL))
    return NULL;
  mem = &svm->base;

  mem->type = CL_MEM_SVM_TYPE;
  CL_OBJECT_INIT_BASE(mem, CL_OBJECT_MEM_MAGIC);
  list_init(&mem->dstr_cb_head);
  mem->flags = flags | CL_MEM_USE_HOST_PTR;
  mem->is_userptr = 0;
  mem->is_svm = 0;
  mem->offset = 0;

  bufmgr = cl_context_get_bufmgr(ctx);
  assert(bufmgr);

  int page_size = getpagesize();
  const size_t alignedSZ = ALIGN(size, page_size);
  if(alignment == 0)
    alignment = page_size;
  else
    alignment = ALIGN(alignment, page_size);
  ptr = cl_aligned_malloc(alignedSZ, alignment);
  if(ptr == NULL) return NULL;

  mem->host_ptr = ptr;
  mem->is_svm = 1;
  mem->is_userptr = 1;
  mem->bo = cl_buffer_alloc_userptr(bufmgr, "CL SVM memory object", ptr, alignedSZ, 0);
  mem->size = size;
  cl_buffer_set_softpin_offset(mem->bo, (size_t)ptr);
  cl_buffer_set_bo_use_full_range(mem->bo, 1);

  /* Append the svm in the context buffer list */
  cl_context_add_mem(ctx, mem);
#endif

  return ptr;
}

void
cl_mem_copy_image_region(const size_t *origin, const size_t *region,
                         void *dst, size_t dst_row_pitch, size_t dst_slice_pitch,
                         const void *src, size_t src_row_pitch, size_t src_slice_pitch,
                         const struct _cl_mem_image *image, cl_bool offset_dst, cl_bool offset_src)
{
  if(offset_dst) {
    size_t dst_offset = image->bpp * origin[0] + dst_row_pitch * origin[1] + dst_slice_pitch * origin[2];
    dst = (char*)dst + dst_offset;
  }
  if(offset_src) {
    size_t src_offset = image->bpp * origin[0] + src_row_pitch * origin[1] + src_slice_pitch * origin[2];
    src = (char*)src + src_offset;
  }
  if (!origin[0] && region[0] == image->w && dst_row_pitch == src_row_pitch &&
      (region[2] == 1 || (!origin[1] && region[1] == image->h && dst_slice_pitch == src_slice_pitch)))
  {
    memcpy(dst, src, region[2] == 1 ? src_row_pitch*region[1] : src_slice_pitch*region[2]);
  }
  else {
    cl_uint y, z;
    for (z = 0; z < region[2]; z++) {
      const char* src_ptr = src;
      char* dst_ptr = dst;
      for (y = 0; y < region[1]; y++) {
        memcpy(dst_ptr, src_ptr, image->bpp*region[0]);
        src_ptr += src_row_pitch;
        dst_ptr += dst_row_pitch;
      }
      src = (char*)src + src_slice_pitch;
      dst = (char*)dst + dst_slice_pitch;
    }
  }
}

void
cl_mem_copy_image_to_image(const size_t *dst_origin,const size_t *src_origin, const size_t *region,
                           const struct _cl_mem_image *dst_image, const struct _cl_mem_image *src_image)
{
  char* dst= cl_mem_map_auto((cl_mem)dst_image, 1);
  char* src= cl_mem_map_auto((cl_mem)src_image, 0);
  size_t dst_offset = dst_image->bpp * dst_origin[0] + dst_image->row_pitch * dst_origin[1] + dst_image->slice_pitch * dst_origin[2];
  size_t src_offset = src_image->bpp * src_origin[0] + src_image->row_pitch * src_origin[1] + src_image->slice_pitch * src_origin[2];
  dst= (char*)dst+ dst_offset;
  src= (char*)src+ src_offset;
  cl_uint y, z;
  for (z = 0; z < region[2]; z++) {
    const char* src_ptr = src;
    char* dst_ptr = dst;
    for (y = 0; y < region[1]; y++) {
      memcpy(dst_ptr, src_ptr, src_image->bpp*region[0]);
      src_ptr += src_image->row_pitch;
      dst_ptr += dst_image->row_pitch;
    }
    src = (char*)src + src_image->slice_pitch;
    dst = (char*)dst + dst_image->slice_pitch;
  }

  cl_mem_unmap_auto((cl_mem)src_image);
  cl_mem_unmap_auto((cl_mem)dst_image);

}

static void
cl_mem_copy_image(struct _cl_mem_image *image,
		  size_t row_pitch,
		  size_t slice_pitch,
		  void* host_ptr)
{
  char* dst_ptr = cl_mem_map_auto((cl_mem)image, 1);
  size_t origin[3] = {0, 0, 0};
  size_t region[3] = {image->w, image->h, image->depth};

  cl_mem_copy_image_region(origin, region, dst_ptr, image->row_pitch, image->slice_pitch,
                           host_ptr, row_pitch, slice_pitch, image, CL_FALSE, CL_FALSE); //offset is 0
  cl_mem_unmap_auto((cl_mem)image);
}

cl_image_tiling_t cl_get_default_tiling(cl_driver drv)
{
  static int initialized = 0;
  static cl_image_tiling_t tiling = CL_TILE_X;

  if (!initialized) {
    // FIXME, need to find out the performance diff's root cause on BDW.
    // SKL's 3D Image can't use TILE_X, so use TILE_Y as default
    if(cl_driver_get_ver(drv) == 8 || cl_driver_get_ver(drv) == 9)
      tiling = CL_TILE_Y;
    char *tilingStr = getenv("OCL_TILING");
    if (tilingStr != NULL) {
      switch (tilingStr[0]) {
        case '0': tiling = CL_NO_TILE; break;
        case '1': tiling = CL_TILE_X; break;
        case '2': tiling = CL_TILE_Y; break;
        default:
          break;
      }
    }
    initialized = 1;
  }

  return tiling;
}

static cl_mem
_cl_mem_new_image(cl_context ctx,
                  cl_mem_flags flags,
                  const cl_image_format *fmt,
                  const cl_mem_object_type orig_image_type,
                  size_t w,
                  size_t h,
                  size_t depth,
                  size_t pitch,
                  size_t slice_pitch,
                  void *data,           //pointer from application
                  cl_mem buffer,        //for image2D from buffer
                  cl_int *errcode_ret)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;
  cl_mem_object_type image_type = orig_image_type;
  uint32_t bpp = 0, intel_fmt = INTEL_UNSUPPORTED_FORMAT;
  size_t sz = 0, aligned_pitch = 0, aligned_slice_pitch = 0, aligned_h = 0;
  size_t origin_width = w;  // for image1d buffer work around.
  cl_image_tiling_t tiling = CL_NO_TILE;
  int enable_true_hostptr = 0;

  // can't use BVAR (backend/src/sys/cvar.hpp) here as it's C++
  const char *env = getenv("OCL_IMAGE_HOSTPTR");
  if (env != NULL) {
    sscanf(env, "%i", &enable_true_hostptr);
  }

  /* Check flags consistency */
  if (UNLIKELY((flags & (CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR)) && data == NULL)) {
    err = CL_INVALID_HOST_PTR;
    goto error;
  }

  /* Get the size of each pixel */
  if (UNLIKELY((err = cl_image_byte_per_pixel(fmt, &bpp)) != CL_SUCCESS))
    goto error;

  /* Only a sub-set of the formats are supported */
  intel_fmt = cl_image_get_intel_format(fmt);
  if (UNLIKELY(intel_fmt == INTEL_UNSUPPORTED_FORMAT)) {
    err = CL_IMAGE_FORMAT_NOT_SUPPORTED;
    goto error;
  }

  /* See if the user parameters match */
#define DO_IMAGE_ERROR            \
  do {                            \
    err = CL_INVALID_IMAGE_SIZE;  \
    goto error;                   \
  } while (0);

  if (UNLIKELY(w == 0)) DO_IMAGE_ERROR;
  if (UNLIKELY(h == 0 && (image_type != CL_MEM_OBJECT_IMAGE1D &&
      image_type != CL_MEM_OBJECT_IMAGE1D_ARRAY &&
      image_type != CL_MEM_OBJECT_IMAGE1D_BUFFER)))
    DO_IMAGE_ERROR;

  if (image_type == CL_MEM_OBJECT_IMAGE1D) {
    size_t min_pitch = bpp * w;
    if (data && pitch == 0)
      pitch = min_pitch;

    h = 1;
    depth = 1;
    if (UNLIKELY(w > ctx->devices[0]->image2d_max_width)) DO_IMAGE_ERROR;
    if (UNLIKELY(data && min_pitch > pitch)) DO_IMAGE_ERROR;
    if (UNLIKELY(data && (slice_pitch % pitch != 0))) DO_IMAGE_ERROR;
    if (UNLIKELY(!data && pitch != 0)) DO_IMAGE_ERROR;
    if (UNLIKELY(!data && slice_pitch != 0)) DO_IMAGE_ERROR;
    tiling = CL_NO_TILE;
  } else if (image_type == CL_MEM_OBJECT_IMAGE2D ||
             image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER) {

    if (image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER) {
      if (UNLIKELY(w > ctx->devices[0]->image_mem_size)) DO_IMAGE_ERROR;
      /* This is an image1d buffer which exceeds normal image size restrication
         We have to use a 2D image to simulate this 1D image. */
      h = (w + ctx->devices[0]->image2d_max_width - 1) / ctx->devices[0]->image2d_max_width;
      w = w > ctx->devices[0]->image2d_max_width ? ctx->devices[0]->image2d_max_width : w;
      tiling = CL_NO_TILE;
    } else if(image_type == CL_MEM_OBJECT_IMAGE2D && buffer != NULL) {
      tiling = CL_NO_TILE;
    } else if (cl_driver_get_ver(ctx->drv) != 6) {
      /* Pick up tiling mode (we do only linear on SNB) */
      tiling = cl_get_default_tiling(ctx->drv);
    }

    size_t min_pitch = bpp * w;
    if (data && pitch == 0)
      pitch = min_pitch;

    if (UNLIKELY(w > ctx->devices[0]->image2d_max_width)) DO_IMAGE_ERROR;
    if (UNLIKELY(h > ctx->devices[0]->image2d_max_height)) DO_IMAGE_ERROR;
    if (UNLIKELY(data && min_pitch > pitch)) DO_IMAGE_ERROR;
    if (UNLIKELY(!data && pitch != 0 && buffer == NULL)) DO_IMAGE_ERROR;

    depth = 1;
  } else if (image_type == CL_MEM_OBJECT_IMAGE3D ||
             image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY ||
             image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
    if (image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
      h = 1;
      tiling = CL_NO_TILE;
    } else if (cl_driver_get_ver(ctx->drv) != 6)
      tiling = cl_get_default_tiling(ctx->drv);

    size_t min_pitch = bpp * w;
    if (data && pitch == 0)
      pitch = min_pitch;
    size_t min_slice_pitch = pitch * h;
    if (data && slice_pitch == 0)
      slice_pitch = min_slice_pitch;
    if (UNLIKELY(w > ctx->devices[0]->image3d_max_width)) DO_IMAGE_ERROR;
    if (UNLIKELY(h > ctx->devices[0]->image3d_max_height)) DO_IMAGE_ERROR;
    if (image_type == CL_MEM_OBJECT_IMAGE3D &&
       (UNLIKELY(depth > ctx->devices[0]->image3d_max_depth))) DO_IMAGE_ERROR
    else if (UNLIKELY(depth > ctx->devices[0]->image_max_array_size)) DO_IMAGE_ERROR;
    if (UNLIKELY(data && min_pitch > pitch)) DO_IMAGE_ERROR;
    if (UNLIKELY(data && min_slice_pitch > slice_pitch)) DO_IMAGE_ERROR;
    if (UNLIKELY(!data && pitch != 0)) DO_IMAGE_ERROR;
    if (UNLIKELY(!data && slice_pitch != 0)) DO_IMAGE_ERROR;

  } else
    assert(0);

#undef DO_IMAGE_ERROR

  uint8_t enableUserptr = 0;
  if (enable_true_hostptr && ctx->devices[0]->host_unified_memory && data != NULL && (flags & CL_MEM_USE_HOST_PTR)) {
    int cacheline_size = 0;
    cl_get_device_info(ctx->devices[0], CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(cacheline_size), &cacheline_size, NULL);
    if (ALIGN((unsigned long)data, cacheline_size) == (unsigned long)data &&
        ALIGN(h, cl_buffer_get_tiling_align(ctx, CL_NO_TILE, 1)) == h &&
        ALIGN(h * pitch * depth, cacheline_size) == h * pitch * depth && //h and pitch should same as aligned_h and aligned_pitch if enable userptr
        ((image_type != CL_MEM_OBJECT_IMAGE3D && image_type != CL_MEM_OBJECT_IMAGE1D_ARRAY && image_type != CL_MEM_OBJECT_IMAGE2D_ARRAY) || pitch * h == slice_pitch)) {
      tiling = CL_NO_TILE;
      enableUserptr = 1;
    }
  }

  /* Tiling requires to align both pitch and height */
  if (tiling == CL_NO_TILE) {
    aligned_pitch = w * bpp;
    if (aligned_pitch < pitch && enableUserptr)
      aligned_pitch = pitch;
    //no need align the height if 2d image from buffer.
    //the pitch should be same with buffer's pitch as they share same bo.
    if (image_type == CL_MEM_OBJECT_IMAGE2D && buffer != NULL) {
      if(aligned_pitch < pitch) {
        aligned_pitch = pitch;
      }
      aligned_h = h;
    }
    else
      aligned_h  = ALIGN(h, cl_buffer_get_tiling_align(ctx, CL_NO_TILE, 1));
  } else if (tiling == CL_TILE_X) {
    aligned_pitch = ALIGN(w * bpp, cl_buffer_get_tiling_align(ctx, CL_TILE_X, 0));
    aligned_h     = ALIGN(h, cl_buffer_get_tiling_align(ctx, CL_TILE_X, 1));
  } else if (tiling == CL_TILE_Y) {
    aligned_pitch = ALIGN(w * bpp, cl_buffer_get_tiling_align(ctx, CL_TILE_Y, 0));
    aligned_h     = ALIGN(h, cl_buffer_get_tiling_align(ctx, CL_TILE_Y, 1));
  }

  sz = aligned_pitch * aligned_h * depth;
  if (image_type == CL_MEM_OBJECT_IMAGE2D && buffer != NULL)  {
    //image 2d created from buffer: per spec, the buffer sz maybe larger than the image 2d.
    if (buffer->size >= sz)
      sz = buffer->size;
    else {
      err = CL_INVALID_IMAGE_SIZE;
      goto error;
    }
  }

  /* If sz is large than 128MB, map gtt may fail in some system.
     Because there is no obviours performance drop, disable tiling. */
  if(tiling != CL_NO_TILE && sz > MAX_TILING_SIZE) {
    tiling = CL_NO_TILE;
    aligned_pitch = w * bpp;
    aligned_h     = ALIGN(h, cl_buffer_get_tiling_align(ctx, CL_NO_TILE, 1));
    sz = aligned_pitch * aligned_h * depth;
  }

  if (image_type != CL_MEM_OBJECT_IMAGE1D_BUFFER) {
    if (image_type == CL_MEM_OBJECT_IMAGE2D && buffer != NULL)
      mem = cl_mem_allocate(CL_MEM_IMAGE_TYPE, ctx, flags, sz, tiling != CL_NO_TILE, NULL, buffer, &err);
    else {
      if (enableUserptr)
        mem = cl_mem_allocate(CL_MEM_IMAGE_TYPE, ctx, flags, sz, tiling != CL_NO_TILE, data, NULL, &err);
      else
        mem = cl_mem_allocate(CL_MEM_IMAGE_TYPE, ctx, flags, sz, tiling != CL_NO_TILE, NULL, NULL, &err);
    }
  } else {
    mem = cl_mem_allocate(CL_MEM_BUFFER1D_IMAGE_TYPE, ctx, flags, sz, tiling != CL_NO_TILE, NULL, NULL, &err);
    if (mem != NULL && err == CL_SUCCESS) {
      struct _cl_mem_buffer1d_image *buffer1d_image = (struct _cl_mem_buffer1d_image *)mem;
      buffer1d_image->size = origin_width;;
    }
  }

  if (mem == NULL || err != CL_SUCCESS)
    goto error;

  if(!(image_type == CL_MEM_OBJECT_IMAGE2D && buffer != NULL))  {
    //no need set tiling if image 2d created from buffer since share same bo.
    cl_buffer_set_tiling(mem->bo, tiling, aligned_pitch);
  }

  if (image_type == CL_MEM_OBJECT_IMAGE1D ||
      image_type == CL_MEM_OBJECT_IMAGE2D ||
      image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER)
    aligned_slice_pitch = 0;
  else
    //SKL need use tiling's aligned_h to calc slice_pitch and IVB to BDW need CL_NO_TILE's aligned_h to calc.
    aligned_slice_pitch = aligned_pitch * ALIGN(h, cl_buffer_get_tiling_align(ctx, tiling, 2));

  cl_mem_image_init(cl_mem_image(mem), w, h, image_type, depth, *fmt,
                    intel_fmt, bpp, aligned_pitch, aligned_slice_pitch, tiling,
                    0, 0, 0);

  /* Copy the data if required */
  if (flags & CL_MEM_COPY_HOST_PTR && data)
    cl_mem_copy_image(cl_mem_image(mem), pitch, slice_pitch, data);

  if (flags & CL_MEM_USE_HOST_PTR && data) {
    mem->host_ptr = data;
    cl_mem_image(mem)->host_row_pitch = pitch;
    cl_mem_image(mem)->host_slice_pitch = slice_pitch;
    if (!enableUserptr)
      cl_mem_copy_image(cl_mem_image(mem), pitch, slice_pitch, data);
  }

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
error:
  cl_mem_delete(mem);
  mem = NULL;
  goto exit;
}

static cl_mem
_cl_mem_new_image_from_buffer(cl_context ctx,
                              cl_mem_flags flags,
                              const cl_image_format* image_format,
                              const cl_image_desc *image_desc,
                              cl_int *errcode_ret)
{
  cl_mem image = NULL;
  cl_mem buffer = image_desc->buffer;
  cl_int err = CL_SUCCESS;
  *errcode_ret = err;
  cl_ulong max_size;
  cl_mem_flags merged_flags;
  uint32_t bpp;
  uint32_t intel_fmt = INTEL_UNSUPPORTED_FORMAT;
  size_t offset = 0;

  /* Get the size of each pixel */
  if (UNLIKELY((err = cl_image_byte_per_pixel(image_format, &bpp)) != CL_SUCCESS))
    goto error;

  /* Only a sub-set of the formats are supported */
  intel_fmt = cl_image_get_intel_format(image_format);
  if (UNLIKELY(intel_fmt == INTEL_UNSUPPORTED_FORMAT)) {
    err = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    goto error;
  }

  if (!buffer) {
    err = CL_INVALID_IMAGE_DESCRIPTOR;
    goto error;
  }

  if (flags & (CL_MEM_USE_HOST_PTR|CL_MEM_ALLOC_HOST_PTR|CL_MEM_COPY_HOST_PTR)) {
    err = CL_INVALID_IMAGE_DESCRIPTOR;
    goto error;
  }

  /* access check. */
  if ((buffer->flags & CL_MEM_WRITE_ONLY) &&
      (flags & (CL_MEM_READ_WRITE|CL_MEM_READ_ONLY))) {
    err = CL_INVALID_VALUE;
    goto error;
  }
  if ((buffer->flags & CL_MEM_READ_ONLY) &&
      (flags & (CL_MEM_READ_WRITE|CL_MEM_WRITE_ONLY))) {
    err = CL_INVALID_VALUE;
    goto error;
  }
  if ((buffer->flags & CL_MEM_HOST_WRITE_ONLY) &&
      (flags & CL_MEM_HOST_READ_ONLY)) {
    err = CL_INVALID_VALUE;
    goto error;
  }
  if ((buffer->flags & CL_MEM_HOST_READ_ONLY) &&
      (flags & CL_MEM_HOST_WRITE_ONLY)) {
    err = CL_INVALID_VALUE;
    goto error;
  }
  if ((buffer->flags & CL_MEM_HOST_NO_ACCESS) &&
      (flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY))) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if ((err = cl_get_device_info(ctx->devices[0],
                                CL_DEVICE_IMAGE_MAX_BUFFER_SIZE,
                                sizeof(max_size),
                                &max_size,
                                NULL)) != CL_SUCCESS) {
    goto error;
  }

  if (image_desc->image_width > max_size) {
    err = CL_INVALID_IMAGE_DESCRIPTOR;
    goto error;
  }

  if (image_desc->image_width*bpp > buffer->size) {
    err = CL_INVALID_IMAGE_DESCRIPTOR;
    goto error;
  }

  merged_flags = buffer->flags;
  if (flags & (CL_MEM_READ_WRITE|CL_MEM_READ_WRITE|CL_MEM_WRITE_ONLY)) {
    merged_flags &= ~(CL_MEM_READ_WRITE|CL_MEM_READ_WRITE|CL_MEM_WRITE_ONLY);
    merged_flags |= flags & (CL_MEM_READ_WRITE|CL_MEM_READ_WRITE|CL_MEM_WRITE_ONLY);
  }
  if (flags & (CL_MEM_HOST_WRITE_ONLY|CL_MEM_HOST_READ_ONLY|CL_MEM_HOST_NO_ACCESS)) {
    merged_flags &= ~(CL_MEM_HOST_WRITE_ONLY|CL_MEM_HOST_READ_ONLY|CL_MEM_HOST_NO_ACCESS);
    merged_flags |= flags & (CL_MEM_HOST_WRITE_ONLY|CL_MEM_HOST_READ_ONLY|CL_MEM_HOST_NO_ACCESS);
  }
  struct _cl_mem_buffer *mem_buffer = (struct _cl_mem_buffer*)buffer;
  if (buffer->type == CL_MEM_SUBBUFFER_TYPE) {
    offset = ((struct _cl_mem_buffer *)buffer)->sub_offset;
    mem_buffer = mem_buffer->parent;
  }
  /* Get the size of each pixel */
  if (UNLIKELY((err = cl_image_byte_per_pixel(image_format, &bpp)) != CL_SUCCESS))
    goto error;

  if(image_desc->image_type == CL_MEM_OBJECT_IMAGE2D) {
    image = _cl_mem_new_image(ctx, flags, image_format, image_desc->image_type,
                             image_desc->image_width, image_desc->image_height, image_desc->image_depth,
                             image_desc->image_row_pitch, image_desc->image_slice_pitch,
                             NULL, image_desc->buffer, errcode_ret);
  } else if (image_desc->image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER) {
    // Per bspec, a image should has a at least 2 line vertical alignment,
    // thus we can't simply attach a buffer to a 1d image surface which has the same size.
    // We have to create a new image, and copy the buffer data to this new image.
    // And replace all the buffer object's reference to this image.
    image = _cl_mem_new_image(ctx, flags, image_format, image_desc->image_type,
                    mem_buffer->base.size / bpp, 0, 0, 0, 0, NULL, NULL, errcode_ret);
  }
  else
    assert(0);

  if (image == NULL)
    return NULL;

  if(image_desc->image_type == CL_MEM_OBJECT_IMAGE2D)  {
    //no need copy since the image 2d and buffer share same bo.
  }
  else if (image_desc->image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER)  {
    // FIXME, we could use copy buffer to image to do this on GPU latter.
    // currently the copy buffer to image function doesn't support 1D image.
    //
    // There is a potential risk that this buffer was mapped and the caller
    // still hold the pointer and want to access it again. This scenario is
    // not explicitly forbidden in the spec, although it should not be permitted.
    void *src = cl_mem_map(buffer, 0);
    void *dst = cl_mem_map(image, 1);
    memcpy(dst, src, mem_buffer->base.size);
    cl_mem_unmap(image);
    cl_mem_unmap(buffer);
    struct _cl_mem_buffer1d_image* image_buffer = (struct _cl_mem_buffer1d_image*)image;
    image_buffer->descbuffer = buffer;
  }
  else
    assert(0);

  if (err != 0)
    goto error;
 
  // Now replace buffer's bo to this new bo, need to take care of sub buffer
  // case. 
  if (image_desc->image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER)
    cl_mem_replace_buffer(buffer, image->bo);
  /* Now point to the right offset if buffer is a SUB_BUFFER. */
  if (buffer->flags & CL_MEM_USE_HOST_PTR)
    image->host_ptr = buffer->host_ptr + offset;
  cl_mem_image(image)->offset = offset;
  cl_mem_add_ref(buffer);
  cl_mem_image(image)->buffer_1d = buffer;
  return image;

error:
  if (image)
    cl_mem_delete(image);
  image = NULL;
  *errcode_ret = err;
  return image;
}

LOCAL cl_mem
cl_mem_new_image(cl_context context,
                 cl_mem_flags flags,
                 const cl_image_format *image_format,
                 const cl_image_desc *image_desc,
                 void *host_ptr,
                 cl_int *errcode_ret)
{
  switch (image_desc->image_type) {
  case CL_MEM_OBJECT_IMAGE1D:
  case CL_MEM_OBJECT_IMAGE3D:
    return _cl_mem_new_image(context, flags, image_format, image_desc->image_type,
                             image_desc->image_width, image_desc->image_height, image_desc->image_depth,
                             image_desc->image_row_pitch, image_desc->image_slice_pitch,
                             host_ptr, NULL, errcode_ret);
  case CL_MEM_OBJECT_IMAGE2D:
    if(image_desc->buffer)
      return _cl_mem_new_image_from_buffer(context, flags, image_format,
                             image_desc, errcode_ret);
    else
      return _cl_mem_new_image(context, flags, image_format, image_desc->image_type,
                             image_desc->image_width, image_desc->image_height, image_desc->image_depth,
                             image_desc->image_row_pitch, image_desc->image_slice_pitch,
                             host_ptr, NULL, errcode_ret);
  case CL_MEM_OBJECT_IMAGE1D_ARRAY:
  case CL_MEM_OBJECT_IMAGE2D_ARRAY:
    return _cl_mem_new_image(context, flags, image_format, image_desc->image_type,
                             image_desc->image_width, image_desc->image_height, image_desc->image_array_size,
                             image_desc->image_row_pitch, image_desc->image_slice_pitch,
                             host_ptr, NULL, errcode_ret);
  case CL_MEM_OBJECT_IMAGE1D_BUFFER:
    return _cl_mem_new_image_from_buffer(context, flags, image_format,
                                         image_desc, errcode_ret);
    break;
  case CL_MEM_OBJECT_BUFFER:
  default:
    assert(0);
  }
  return NULL;
}

LOCAL void
cl_mem_svm_delete(cl_context ctx, void *svm_pointer)
{
  cl_mem mem;
  if(UNLIKELY(svm_pointer == NULL))
    return;
  mem = cl_context_get_svm_from_ptr(ctx, svm_pointer);
  if(mem == NULL)
    return;
  cl_mem_delete(mem);
}

LOCAL void
cl_mem_delete(cl_mem mem)
{
  cl_int i;
  cl_mem_dstr_cb cb = NULL;

  if (UNLIKELY(mem == NULL))
    return;
  if (CL_OBJECT_DEC_REF(mem) > 1)
    return;
#ifdef HAS_GL_EGL
  if (UNLIKELY(IS_GL_IMAGE(mem))) {
     cl_mem_gl_delete(cl_mem_gl_image(mem));
  }
#endif

#ifdef HAS_CMRT
  if (mem->cmrt_mem != NULL)
    cmrt_destroy_memory(mem);
#endif

  /* First, call all the callbacks registered by user. */
  while (!list_empty(&mem->dstr_cb_head)) {
    cb = list_entry(mem->dstr_cb_head.head_node.n, _cl_mem_dstr_cb, node);
    list_node_del(&cb->node);
    cb->pfn_notify(mem, cb->user_data);
    cl_free(cb);
  }

  /* iff we are a image, delete the 1d buffer if has. */
  if (IS_IMAGE(mem)) {
    if (cl_mem_image(mem)->buffer_1d) {
      assert(cl_mem_image(mem)->image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER ||
          cl_mem_image(mem)->image_type == CL_MEM_OBJECT_IMAGE2D);
        cl_mem_delete(cl_mem_image(mem)->buffer_1d);
        if(cl_mem_image(mem)->image_type == CL_MEM_OBJECT_IMAGE2D && cl_mem_image(mem)->is_image_from_buffer == 1)
        {
          cl_mem_image(mem)->buffer_1d = NULL;
          mem->bo = NULL;
        }
    }
  }

  /* Someone still mapped, unmap */
  if(mem->map_ref > 0) {
    assert(mem->mapped_ptr);
    for(i=0; i<mem->mapped_ptr_sz; i++) {
      if(mem->mapped_ptr[i].ptr != NULL) {
        mem->map_ref--;
        cl_mem_unmap_auto(mem);
      }
    }
    assert(mem->map_ref == 0);
  }

  if (mem->mapped_ptr)
    free(mem->mapped_ptr);

  /* Iff we are sub, do nothing for bo release. */
  if (mem->type == CL_MEM_SUBBUFFER_TYPE) {
    struct _cl_mem_buffer* buffer = (struct _cl_mem_buffer*)mem;
    /* Remove it from the parent's list */
    assert(buffer->parent);
    pthread_mutex_lock(&buffer->parent->sub_lock);
    if (buffer->sub_prev)
      buffer->sub_prev->sub_next = buffer->sub_next;
    if (buffer->sub_next)
      buffer->sub_next->sub_prev = buffer->sub_prev;
    if (buffer->parent->subs == buffer)
      buffer->parent->subs = buffer->sub_next;
    pthread_mutex_unlock(&buffer->parent->sub_lock);
    cl_mem_delete((cl_mem )(buffer->parent));
  } else if (mem->is_svm && mem->type != CL_MEM_SVM_TYPE) {
    cl_mem svm_mem = cl_context_get_svm_from_ptr(mem->ctx, mem->host_ptr);
    if (svm_mem != NULL)
      cl_mem_delete(svm_mem);
  } else if (LIKELY(mem->bo != NULL)) {
    cl_buffer_unreference(mem->bo);
  }

  /* Remove it from the list */
  cl_context_remove_mem(mem->ctx, mem);

  if ((mem->is_userptr &&
      (mem->flags & CL_MEM_ALLOC_HOST_PTR) &&
      (mem->type != CL_MEM_SUBBUFFER_TYPE)) ||
      (mem->is_svm && mem->type == CL_MEM_SVM_TYPE))
    cl_free(mem->host_ptr);

  CL_OBJECT_DESTROY_BASE(mem);
  cl_free(mem);
}

LOCAL void
cl_mem_add_ref(cl_mem mem)
{
  assert(mem);
  CL_OBJECT_INC_REF(mem);
}

#define LOCAL_SZ_0   16
#define LOCAL_SZ_1   4
#define LOCAL_SZ_2   4

LOCAL cl_int
cl_mem_copy(cl_command_queue queue, cl_event event, cl_mem src_buf, cl_mem dst_buf,
            size_t src_offset, size_t dst_offset, size_t cb)
{
  cl_int ret = CL_SUCCESS;
  cl_kernel ker = NULL;
  size_t global_off[] = {0,0,0};
  size_t global_sz[] = {1,1,1};
  size_t local_sz[] = {1,1,1};
  const unsigned int masks[4] = {0xffffffff, 0x0ff, 0x0ffff, 0x0ffffff};
  int aligned = 0;
  int dw_src_offset = src_offset/4;
  int dw_dst_offset = dst_offset/4;

  if (!cb)
    return ret;

  /* We use one kernel to copy the data. The kernel is lazily created. */
  assert(src_buf->ctx == dst_buf->ctx);

  /* All 16 bytes aligned, fast and easy one. */
  if((cb % 16 == 0) && (src_offset % 16 == 0) && (dst_offset % 16 == 0)) {
    extern char cl_internal_copy_buf_align16_str[];
    extern size_t cl_internal_copy_buf_align16_str_size;

    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_BUFFER_ALIGN16,
             cl_internal_copy_buf_align16_str, (size_t)cl_internal_copy_buf_align16_str_size, NULL);
    cb = cb/16;
    aligned = 1;
  } else if ((cb % 4 == 0) && (src_offset % 4 == 0) && (dst_offset % 4 == 0)) { /* all Dword aligned.*/
    extern char cl_internal_copy_buf_align4_str[];
    extern size_t cl_internal_copy_buf_align4_str_size;

    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_BUFFER_ALIGN4,
             cl_internal_copy_buf_align4_str, (size_t)cl_internal_copy_buf_align4_str_size, NULL);
    cb = cb/4;
    aligned = 1;
  }

  if (aligned) {
    if (!ker)
      return CL_OUT_OF_RESOURCES;

    if (cb < LOCAL_SZ_0) {
      local_sz[0] = 1;
    } else {
      local_sz[0] = LOCAL_SZ_0;
    }
    global_sz[0] = ((cb + LOCAL_SZ_0 - 1)/LOCAL_SZ_0)*LOCAL_SZ_0;
    cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &src_buf);
    cl_kernel_set_arg(ker, 1, sizeof(int), &dw_src_offset);
    cl_kernel_set_arg(ker, 2, sizeof(cl_mem), &dst_buf);
    cl_kernel_set_arg(ker, 3, sizeof(int), &dw_dst_offset);
    cl_kernel_set_arg(ker, 4, sizeof(int), &cb);
    ret = cl_command_queue_ND_range(queue, ker, event, 1, global_off,
                                    global_off, global_sz, global_sz, local_sz, local_sz);
    cl_kernel_delete(ker);
    return ret;
  }

  /* Now handle the unaligned cases. */
  int dw_num = ((dst_offset % 4 + cb) + 3) / 4;
  unsigned int first_mask = dst_offset % 4 == 0 ? 0x0 : masks[dst_offset % 4];
  unsigned int last_mask = masks[(dst_offset + cb) % 4];
  /* handle the very small range copy. */
  if (cb < 4 && dw_num == 1) {
    first_mask = first_mask | ~last_mask;
  }

  if (cb < LOCAL_SZ_0) {
    local_sz[0] = 1;
  } else {
    local_sz[0] = LOCAL_SZ_0;
  }
  global_sz[0] = ((dw_num + LOCAL_SZ_0 - 1)/LOCAL_SZ_0)*LOCAL_SZ_0;

  if (src_offset % 4 == dst_offset % 4) {
    /* Src and dst has the same unaligned offset, just handle the
       header and tail. */
    extern char cl_internal_copy_buf_unalign_same_offset_str[];
    extern size_t cl_internal_copy_buf_unalign_same_offset_str_size;

    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_BUFFER_UNALIGN_SAME_OFFSET,
             cl_internal_copy_buf_unalign_same_offset_str,
             (size_t)cl_internal_copy_buf_unalign_same_offset_str_size, NULL);

    if (!ker)
      return CL_OUT_OF_RESOURCES;

    cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &src_buf);
    cl_kernel_set_arg(ker, 1, sizeof(int), &dw_src_offset);
    cl_kernel_set_arg(ker, 2, sizeof(cl_mem), &dst_buf);
    cl_kernel_set_arg(ker, 3, sizeof(int), &dw_dst_offset);
    cl_kernel_set_arg(ker, 4, sizeof(int), &dw_num);
    cl_kernel_set_arg(ker, 5, sizeof(int), &first_mask);
    cl_kernel_set_arg(ker, 6, sizeof(int), &last_mask);
    ret = cl_command_queue_ND_range(queue, ker, event, 1, global_off,
                                    global_off, global_sz, global_sz, local_sz, local_sz);
    cl_kernel_delete(ker);
    return ret;
  }

  /* Dst's offset < Src's offset, so one dst dword need two sequential src dwords to fill it. */
  if (dst_offset % 4 < src_offset % 4) {
    extern char cl_internal_copy_buf_unalign_dst_offset_str[];
    extern size_t cl_internal_copy_buf_unalign_dst_offset_str_size;

    int align_diff = src_offset % 4 - dst_offset % 4;
    unsigned int dw_mask = masks[align_diff];
    int shift = align_diff * 8;

    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_BUFFER_UNALIGN_DST_OFFSET,
             cl_internal_copy_buf_unalign_dst_offset_str,
             (size_t)cl_internal_copy_buf_unalign_dst_offset_str_size, NULL);

    if (!ker)
      return CL_OUT_OF_RESOURCES;

    cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &src_buf);
    cl_kernel_set_arg(ker, 1, sizeof(int), &dw_src_offset);
    cl_kernel_set_arg(ker, 2, sizeof(cl_mem), &dst_buf);
    cl_kernel_set_arg(ker, 3, sizeof(int), &dw_dst_offset);
    cl_kernel_set_arg(ker, 4, sizeof(int), &dw_num);
    cl_kernel_set_arg(ker, 5, sizeof(int), &first_mask);
    cl_kernel_set_arg(ker, 6, sizeof(int), &last_mask);
    cl_kernel_set_arg(ker, 7, sizeof(int), &shift);
    cl_kernel_set_arg(ker, 8, sizeof(int), &dw_mask);
    ret = cl_command_queue_ND_range(queue, ker, event, 1, global_off,
                                    global_off, global_sz, global_sz, local_sz, local_sz);
    cl_kernel_delete(ker);
    return ret;
  }

  /* Dst's offset > Src's offset, so one dst dword need two sequential src - and src to fill it. */
  if (dst_offset % 4 > src_offset % 4) {
    extern char cl_internal_copy_buf_unalign_src_offset_str[];
    extern size_t cl_internal_copy_buf_unalign_src_offset_str_size;

    int align_diff = dst_offset % 4 - src_offset % 4;
    unsigned int dw_mask = masks[4 - align_diff];
    int shift = align_diff * 8;
    int src_less = !(src_offset % 4) && !((src_offset + cb) % 4);

    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_BUFFER_UNALIGN_SRC_OFFSET,
             cl_internal_copy_buf_unalign_src_offset_str,
             (size_t)cl_internal_copy_buf_unalign_src_offset_str_size, NULL);

    if (!ker)
      return CL_OUT_OF_RESOURCES;

    cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &src_buf);
    cl_kernel_set_arg(ker, 1, sizeof(int), &dw_src_offset);
    cl_kernel_set_arg(ker, 2, sizeof(cl_mem), &dst_buf);
    cl_kernel_set_arg(ker, 3, sizeof(int), &dw_dst_offset);
    cl_kernel_set_arg(ker, 4, sizeof(int), &dw_num);
    cl_kernel_set_arg(ker, 5, sizeof(int), &first_mask);
    cl_kernel_set_arg(ker, 6, sizeof(int), &last_mask);
    cl_kernel_set_arg(ker, 7, sizeof(int), &shift);
    cl_kernel_set_arg(ker, 8, sizeof(int), &dw_mask);
    cl_kernel_set_arg(ker, 9, sizeof(int), &src_less);
    ret = cl_command_queue_ND_range(queue, ker, event, 1, global_off,
                                    global_off, global_sz, global_sz, local_sz, local_sz);
    cl_kernel_delete(ker);
    return ret;
  }

  /* no case can hanldle? */
  assert(0);

  return ret;
}

LOCAL cl_int
cl_image_fill(cl_command_queue queue, cl_event e, const void * pattern, struct _cl_mem_image* src_image,
           const size_t * origin, const size_t * region)
{
  cl_int ret = CL_SUCCESS;
  cl_kernel ker = NULL;
  size_t global_off[] = {0,0,0};
  size_t global_sz[] = {1,1,1};
  size_t local_sz[] = {LOCAL_SZ_0,LOCAL_SZ_1,LOCAL_SZ_2};
  uint32_t savedIntelFmt = src_image->intel_fmt;


  if(region[1] == 1) local_sz[1] = 1;
  if(region[2] == 1) local_sz[2] = 1;
  global_sz[0] = ((region[0] + local_sz[0] - 1) / local_sz[0]) * local_sz[0];
  global_sz[1] = ((region[1] + local_sz[1] - 1) / local_sz[1]) * local_sz[1];
  global_sz[2] = ((region[2] + local_sz[2] - 1) / local_sz[2]) * local_sz[2];

  if(src_image->image_type == CL_MEM_OBJECT_IMAGE1D) {
    extern char cl_internal_fill_image_1d_str[];
    extern size_t cl_internal_fill_image_1d_str_size;

    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_FILL_IMAGE_1D,
        cl_internal_fill_image_1d_str, (size_t)cl_internal_fill_image_1d_str_size, NULL);
  }else if(src_image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
    extern char cl_internal_fill_image_1d_array_str[];
    extern size_t cl_internal_fill_image_1d_array_str_size;

    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_FILL_IMAGE_1D_ARRAY,
        cl_internal_fill_image_1d_array_str, (size_t)cl_internal_fill_image_1d_array_str_size, NULL);
  }else if(src_image->image_type == CL_MEM_OBJECT_IMAGE2D) {
    extern char cl_internal_fill_image_2d_str[];
    extern size_t cl_internal_fill_image_2d_str_size;

    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_FILL_IMAGE_2D,
        cl_internal_fill_image_2d_str, (size_t)cl_internal_fill_image_2d_str_size, NULL);
  }else if(src_image->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
    extern char cl_internal_fill_image_2d_array_str[];
    extern size_t cl_internal_fill_image_2d_array_str_size;

    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_FILL_IMAGE_2D_ARRAY,
        cl_internal_fill_image_2d_array_str, (size_t)cl_internal_fill_image_2d_array_str_size, NULL);
  }else if(src_image->image_type == CL_MEM_OBJECT_IMAGE3D) {
    extern char cl_internal_fill_image_3d_str[];
    extern size_t cl_internal_fill_image_3d_str_size;

    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_FILL_IMAGE_3D,
        cl_internal_fill_image_3d_str, (size_t)cl_internal_fill_image_3d_str_size, NULL);
  }else{
    return CL_IMAGE_FORMAT_NOT_SUPPORTED;
  }

  if (!ker)
    return CL_OUT_OF_RESOURCES;

  cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &src_image);
  if(src_image->fmt.image_channel_order >= CL_sRGBA) {
#define RGB2sRGB(linear)  ( linear <= 0.0031308f )? ( 12.92f * linear ):( 1.055f * powf( linear, 1.0f/2.4f ) - 0.055f);
    cl_image_format fmt;
    float newpattern[4] = {0.0,0.0,0.0,((float*)pattern)[3]};
    int i;
    for(i = 0;i < 3; i++){
      if(src_image->fmt.image_channel_order == CL_sRGBA) {
        newpattern[i] = RGB2sRGB(((float*)pattern)[i]);
      } else
        newpattern[2-i] = RGB2sRGB(((float*)pattern)[i]);
    }
    cl_kernel_set_arg(ker, 1, sizeof(float)*4, newpattern);
    fmt.image_channel_order = CL_RGBA;
    fmt.image_channel_data_type = CL_UNORM_INT8;
    src_image->intel_fmt = cl_image_get_intel_format(&fmt);
#undef RGB2sRGB
  } else
    cl_kernel_set_arg(ker, 1, sizeof(float)*4, pattern);
  cl_kernel_set_arg(ker, 2, sizeof(cl_int), &region[0]);
  cl_kernel_set_arg(ker, 3, sizeof(cl_int), &region[1]);
  cl_kernel_set_arg(ker, 4, sizeof(cl_int), &region[2]);
  cl_kernel_set_arg(ker, 5, sizeof(cl_int), &origin[0]);
  cl_kernel_set_arg(ker, 6, sizeof(cl_int), &origin[1]);
  cl_kernel_set_arg(ker, 7, sizeof(cl_int), &origin[2]);

  ret = cl_command_queue_ND_range(queue, ker, e, 3, global_off,
                                  global_off, global_sz, global_sz, local_sz, local_sz);
  cl_kernel_delete(ker);
  src_image->intel_fmt = savedIntelFmt;
  return ret;
}

LOCAL cl_int
cl_mem_fill(cl_command_queue queue, cl_event e, const void * pattern, size_t pattern_size,
            cl_mem buffer, size_t offset, size_t size)
{
  cl_int ret = CL_SUCCESS;
  cl_kernel ker = NULL;
  size_t global_off[] = {0,0,0};
  size_t global_sz[] = {1,1,1};
  size_t local_sz[] = {1,1,1};
  char pattern_comb[4];
  int is_128 = 0;
  const void * pattern1 = NULL;

  assert(offset % pattern_size == 0);
  assert(size % pattern_size == 0);

  if (!size)
    return ret;

  if (pattern_size == 128) {
    /* 128 is according to pattern of double16, but double works not very
       well on some platform. We use two float16 to handle this. */
    extern char cl_internal_fill_buf_align128_str[];
    extern size_t cl_internal_fill_buf_align128_str_size;

    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_FILL_BUFFER_ALIGN128,
               cl_internal_fill_buf_align128_str, (size_t)cl_internal_fill_buf_align128_str_size, NULL);
    is_128 = 1;
    pattern_size = pattern_size / 2;
    pattern1 = pattern + pattern_size;
    size = size / 2;
  } else if (pattern_size % 8 == 0) { /* Handle the 8 16 32 64 cases here. */
    extern char cl_internal_fill_buf_align8_str[];
    extern size_t cl_internal_fill_buf_align8_str_size;
    int order = ffs(pattern_size / 8) - 1;

    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_FILL_BUFFER_ALIGN8_8 + order,
               cl_internal_fill_buf_align8_str, (size_t)cl_internal_fill_buf_align8_str_size, NULL);
  } else if (pattern_size == 4) {
    extern char cl_internal_fill_buf_align4_str[];
    extern size_t cl_internal_fill_buf_align4_str_size;

    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_FILL_BUFFER_ALIGN4,
               cl_internal_fill_buf_align4_str, (size_t)cl_internal_fill_buf_align4_str_size, NULL);
  } else if (size >= 4 && size % 4 == 0 && offset % 4 == 0) {
    /* The unaligned case. But if copy size and offset are aligned to 4, we can fake
       the pattern with the pattern duplication fill in. */
    assert(pattern_size == 1 || pattern_size == 2);
    extern char cl_internal_fill_buf_align4_str[];
    extern size_t cl_internal_fill_buf_align4_str_size;

    if (pattern_size == 2) {
      memcpy(pattern_comb, pattern, sizeof(char)*2);
      memcpy(pattern_comb + 2, pattern, sizeof(char)*2);
    } else {
      pattern_comb[0] = pattern_comb[1] = pattern_comb[2]
        = pattern_comb[3] = *(char *)pattern;
    }

    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_FILL_BUFFER_ALIGN4,
               cl_internal_fill_buf_align4_str, (size_t)cl_internal_fill_buf_align4_str_size, NULL);
    pattern_size = 4;
    pattern = pattern_comb;
  }
  //TODO: Unaligned cases, we may need to optimize it as cl_mem_copy, using mask in kernel
  //functions. This depend on the usage but now we just use aligned 1 and 2.
  else if (pattern_size == 2) {
    extern char cl_internal_fill_buf_align2_str[];
    extern size_t cl_internal_fill_buf_align2_str_size;
    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_FILL_BUFFER_ALIGN2,
               cl_internal_fill_buf_align2_str, (size_t)cl_internal_fill_buf_align2_str_size, NULL);
  } else if (pattern_size == 1) {
    extern char cl_internal_fill_buf_unalign_str[];
    extern size_t cl_internal_fill_buf_unalign_str_size;
    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_FILL_BUFFER_UNALIGN,
               cl_internal_fill_buf_unalign_str, (size_t)cl_internal_fill_buf_unalign_str_size, NULL);
  } else
    assert(0);

  if (!ker)
    return CL_OUT_OF_RESOURCES;

  size = size / pattern_size;
  offset = offset / pattern_size;

  if (size < LOCAL_SZ_0) {
    local_sz[0] = 1;
  } else {
    local_sz[0] = LOCAL_SZ_0;
  }
  global_sz[0] = ((size + LOCAL_SZ_0 - 1) / LOCAL_SZ_0) * LOCAL_SZ_0;
  cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &buffer);
  cl_kernel_set_arg(ker, 1, pattern_size, pattern);
  cl_kernel_set_arg(ker, 2, sizeof(cl_uint), &offset);
  cl_kernel_set_arg(ker, 3, sizeof(cl_uint), &size);
  if (is_128)
    cl_kernel_set_arg(ker, 4, pattern_size, pattern1);

  ret = cl_command_queue_ND_range(queue, ker, e, 1, global_off,
                                  global_off, global_sz, global_sz, local_sz, local_sz);
  cl_kernel_delete(ker);
  return ret;
}

LOCAL cl_int
cl_mem_copy_buffer_rect(cl_command_queue queue, cl_event event, cl_mem src_buf, cl_mem dst_buf,
                       const size_t *src_origin, const size_t *dst_origin, const size_t *region,
                       size_t src_row_pitch, size_t src_slice_pitch,
                       size_t dst_row_pitch, size_t dst_slice_pitch) {
  cl_int ret;
  cl_kernel ker;
  size_t global_off[] = {0,0,0};
  size_t global_sz[] = {1,1,1};
  size_t local_sz[] = {LOCAL_SZ_0,LOCAL_SZ_1,LOCAL_SZ_1};
  // the src and dst mem rect is continuous, the copy is degraded to buf copy
  if((region[0] == dst_row_pitch) && (region[0] == src_row_pitch) &&
  (region[1] * src_row_pitch == src_slice_pitch) && (region[1] * dst_row_pitch == dst_slice_pitch)){
    cl_int src_offset = src_origin[2]*src_slice_pitch + src_origin[1]*src_row_pitch + src_origin[0];
    cl_int dst_offset = dst_origin[2]*dst_slice_pitch + dst_origin[1]*dst_row_pitch + dst_origin[0];
    cl_int size = region[0]*region[1]*region[2];
    ret = cl_mem_copy(queue, NULL, src_buf, dst_buf,src_offset, dst_offset, size);
    return ret;
  }

  if(region[1] == 1) local_sz[1] = 1;
  if(region[2] == 1) local_sz[2] = 1;
  global_sz[0] = ((region[0] + local_sz[0] - 1) / local_sz[0]) * local_sz[0];
  global_sz[1] = ((region[1] + local_sz[1] - 1) / local_sz[1]) * local_sz[1];
  global_sz[2] = ((region[2] + local_sz[2] - 1) / local_sz[2]) * local_sz[2];
  cl_int src_offset = src_origin[2]*src_slice_pitch + src_origin[1]*src_row_pitch + src_origin[0];
  cl_int dst_offset = dst_origin[2]*dst_slice_pitch + dst_origin[1]*dst_row_pitch + dst_origin[0];

  /* We use one kernel to copy the data. The kernel is lazily created. */
  assert(src_buf->ctx == dst_buf->ctx);

  /* setup the kernel and run. */
  size_t region0 = region[0];
  if( (src_offset % 4== 0) && (dst_offset % 4== 0) && (src_row_pitch % 4== 0) && (dst_row_pitch % 4== 0)
  && (src_slice_pitch % 4== 0) && (dst_slice_pitch % 4== 0) && (region0 % 4 == 0) ){
    extern char cl_internal_copy_buf_rect_align4_str[];
    extern size_t cl_internal_copy_buf_rect_align4_str_size;
    region0 /= 4;
    src_offset /= 4;
    dst_offset /= 4;
    src_row_pitch /= 4;
    dst_row_pitch /= 4;
    src_slice_pitch /= 4;
    dst_slice_pitch /= 4;
    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_BUFFER_RECT_ALIGN4,
    cl_internal_copy_buf_rect_align4_str, (size_t)cl_internal_copy_buf_rect_align4_str_size, NULL);
  }else{
    extern char cl_internal_copy_buf_rect_str[];
    extern size_t cl_internal_copy_buf_rect_str_size;
    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_BUFFER_RECT,
    cl_internal_copy_buf_rect_str, (size_t)cl_internal_copy_buf_rect_str_size, NULL);
  }

  if (!ker)
    return CL_OUT_OF_RESOURCES;

  cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &src_buf);
  cl_kernel_set_arg(ker, 1, sizeof(cl_mem), &dst_buf);
  cl_kernel_set_arg(ker, 2, sizeof(cl_int), &region0);
  cl_kernel_set_arg(ker, 3, sizeof(cl_int), &region[1]);
  cl_kernel_set_arg(ker, 4, sizeof(cl_int), &region[2]);
  cl_kernel_set_arg(ker, 5, sizeof(cl_int), &src_offset);
  cl_kernel_set_arg(ker, 6, sizeof(cl_int), &dst_offset);
  cl_kernel_set_arg(ker, 7, sizeof(cl_int), &src_row_pitch);
  cl_kernel_set_arg(ker, 8, sizeof(cl_int), &src_slice_pitch);
  cl_kernel_set_arg(ker, 9, sizeof(cl_int), &dst_row_pitch);
  cl_kernel_set_arg(ker, 10, sizeof(cl_int), &dst_slice_pitch);

  ret = cl_command_queue_ND_range(queue, ker, event, 1, global_off,
                                  global_off, global_sz, global_sz, local_sz, local_sz);
  cl_kernel_delete(ker);
  return ret;
}

LOCAL cl_int
cl_mem_kernel_copy_image(cl_command_queue queue, cl_event event, struct _cl_mem_image* src_image,
                         struct _cl_mem_image* dst_image, const size_t *src_origin,
                         const size_t *dst_origin, const size_t *region) {
  cl_int ret;
  cl_kernel ker = NULL;
  size_t global_off[] = {0,0,0};
  size_t global_sz[] = {1,1,1};
  size_t local_sz[] = {LOCAL_SZ_0,LOCAL_SZ_1,LOCAL_SZ_2};
  uint32_t fixupDataType;
  uint32_t savedIntelFmt;

  if(region[1] == 1) local_sz[1] = 1;
  if(region[2] == 1) local_sz[2] = 1;
  global_sz[0] = ((region[0] + local_sz[0] - 1) / local_sz[0]) * local_sz[0];
  global_sz[1] = ((region[1] + local_sz[1] - 1) / local_sz[1]) * local_sz[1];
  global_sz[2] = ((region[2] + local_sz[2] - 1) / local_sz[2]) * local_sz[2];

  switch (src_image->fmt.image_channel_data_type) {
    case CL_SNORM_INT8:
    case CL_UNORM_INT8:  fixupDataType = CL_UNSIGNED_INT8; break;
    case CL_HALF_FLOAT:
    case CL_SNORM_INT16:
    case CL_UNORM_INT16: fixupDataType = CL_UNSIGNED_INT16; break;
    case CL_FLOAT:       fixupDataType = CL_UNSIGNED_INT32; break;
    default:
      fixupDataType = 0;
  }

  if (fixupDataType) {
    cl_image_format fmt;
    if (src_image->fmt.image_channel_order != CL_BGRA &&
        src_image->fmt.image_channel_order != CL_sBGRA &&
        src_image->fmt.image_channel_order != CL_sRGBA)
      fmt.image_channel_order = src_image->fmt.image_channel_order;
    else
      fmt.image_channel_order = CL_RGBA;
    fmt.image_channel_data_type = fixupDataType;
    savedIntelFmt = src_image->intel_fmt;
    src_image->intel_fmt = cl_image_get_intel_format(&fmt);
    dst_image->intel_fmt = src_image->intel_fmt;
  }

  /* We use one kernel to copy the data. The kernel is lazily created. */
  assert(src_image->base.ctx == dst_image->base.ctx);

  /* setup the kernel and run. */
  if(src_image->image_type == CL_MEM_OBJECT_IMAGE1D) {
    if(dst_image->image_type == CL_MEM_OBJECT_IMAGE1D) {
      extern char cl_internal_copy_image_1d_to_1d_str[];
      extern size_t cl_internal_copy_image_1d_to_1d_str_size;

      ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_IMAGE_1D_TO_1D,
          cl_internal_copy_image_1d_to_1d_str, (size_t)cl_internal_copy_image_1d_to_1d_str_size, NULL);
    }
  } else if(src_image->image_type == CL_MEM_OBJECT_IMAGE2D) {
    if(dst_image->image_type == CL_MEM_OBJECT_IMAGE2D) {
      extern char cl_internal_copy_image_2d_to_2d_str[];
      extern size_t cl_internal_copy_image_2d_to_2d_str_size;

      ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_IMAGE_2D_TO_2D,
          cl_internal_copy_image_2d_to_2d_str, (size_t)cl_internal_copy_image_2d_to_2d_str_size, NULL);
    } else if(dst_image->image_type == CL_MEM_OBJECT_IMAGE3D) {
      extern char cl_internal_copy_image_2d_to_3d_str[];
      extern size_t cl_internal_copy_image_2d_to_3d_str_size;

      ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_IMAGE_2D_TO_3D,
          cl_internal_copy_image_2d_to_3d_str, (size_t)cl_internal_copy_image_2d_to_3d_str_size, NULL);
    } else if(dst_image->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
      extern char cl_internal_copy_image_2d_to_2d_array_str[];
      extern size_t cl_internal_copy_image_2d_to_2d_array_str_size;

      ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_IMAGE_2D_TO_2D_ARRAY,
          cl_internal_copy_image_2d_to_2d_array_str, (size_t)cl_internal_copy_image_2d_to_2d_array_str_size, NULL);
    }
  } else if(src_image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
    if(dst_image->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
      extern char cl_internal_copy_image_1d_array_to_1d_array_str[];
      extern size_t cl_internal_copy_image_1d_array_to_1d_array_str_size;

      ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_IMAGE_1D_ARRAY_TO_1D_ARRAY,
          cl_internal_copy_image_1d_array_to_1d_array_str,
          (size_t)cl_internal_copy_image_1d_array_to_1d_array_str_size, NULL);
    }
  } else if(src_image->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
    if(dst_image->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
      extern char cl_internal_copy_image_2d_array_to_2d_array_str[];
      extern size_t cl_internal_copy_image_2d_array_to_2d_array_str_size;

      ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_IMAGE_2D_ARRAY_TO_2D_ARRAY,
          cl_internal_copy_image_2d_array_to_2d_array_str,
          (size_t)cl_internal_copy_image_2d_array_to_2d_array_str_size, NULL);
    } else if(dst_image->image_type == CL_MEM_OBJECT_IMAGE2D) {
      extern char cl_internal_copy_image_2d_array_to_2d_str[];
      extern size_t cl_internal_copy_image_2d_array_to_2d_str_size;

      ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_IMAGE_2D_ARRAY_TO_2D,
          cl_internal_copy_image_2d_array_to_2d_str,
          (size_t)cl_internal_copy_image_2d_array_to_2d_str_size, NULL);
    } else if(dst_image->image_type == CL_MEM_OBJECT_IMAGE3D) {
      extern char cl_internal_copy_image_2d_array_to_3d_str[];
      extern size_t cl_internal_copy_image_2d_array_to_3d_str_size;

      ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_IMAGE_2D_ARRAY_TO_3D,
          cl_internal_copy_image_2d_array_to_3d_str,
          (size_t)cl_internal_copy_image_2d_array_to_3d_str_size, NULL);
    }
  } else if(src_image->image_type == CL_MEM_OBJECT_IMAGE3D) {
    if(dst_image->image_type == CL_MEM_OBJECT_IMAGE2D) {
      extern char cl_internal_copy_image_3d_to_2d_str[];
      extern size_t cl_internal_copy_image_3d_to_2d_str_size;

      ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_IMAGE_3D_TO_2D,
          cl_internal_copy_image_3d_to_2d_str, (size_t)cl_internal_copy_image_3d_to_2d_str_size, NULL);
    } else if(dst_image->image_type == CL_MEM_OBJECT_IMAGE3D) {
      extern char cl_internal_copy_image_3d_to_3d_str[];
      extern size_t cl_internal_copy_image_3d_to_3d_str_size;

      ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_IMAGE_3D_TO_3D,
          cl_internal_copy_image_3d_to_3d_str, (size_t)cl_internal_copy_image_3d_to_3d_str_size, NULL);
    } else if(dst_image->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
      extern char cl_internal_copy_image_3d_to_2d_array_str[];
      extern size_t cl_internal_copy_image_3d_to_2d_array_str_size;

      ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_IMAGE_3D_TO_2D_ARRAY,
          cl_internal_copy_image_3d_to_2d_array_str, (size_t)cl_internal_copy_image_3d_to_2d_array_str_size, NULL);
    }
  }

  if (!ker) {
    ret = CL_OUT_OF_RESOURCES;
    goto fail;
  }

  cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &src_image);
  cl_kernel_set_arg(ker, 1, sizeof(cl_mem), &dst_image);
  cl_kernel_set_arg(ker, 2, sizeof(cl_int), &region[0]);
  cl_kernel_set_arg(ker, 3, sizeof(cl_int), &region[1]);
  cl_kernel_set_arg(ker, 4, sizeof(cl_int), &region[2]);
  cl_kernel_set_arg(ker, 5, sizeof(cl_int), &src_origin[0]);
  cl_kernel_set_arg(ker, 6, sizeof(cl_int), &src_origin[1]);
  cl_kernel_set_arg(ker, 7, sizeof(cl_int), &src_origin[2]);
  cl_kernel_set_arg(ker, 8, sizeof(cl_int), &dst_origin[0]);
  cl_kernel_set_arg(ker, 9, sizeof(cl_int), &dst_origin[1]);
  cl_kernel_set_arg(ker, 10, sizeof(cl_int), &dst_origin[2]);

  ret = cl_command_queue_ND_range(queue, ker, event, 1, global_off,
                                  global_off, global_sz, global_sz, local_sz, local_sz);

fail:

  cl_kernel_delete(ker);
  if (fixupDataType) {
    src_image->intel_fmt = savedIntelFmt;
    dst_image->intel_fmt = savedIntelFmt;
  }
  return ret;
}

LOCAL cl_int
cl_mem_copy_image_to_buffer(cl_command_queue queue, cl_event event, struct _cl_mem_image* image, cl_mem buffer,
                         const size_t *src_origin, const size_t dst_offset, const size_t *region) {
  cl_int ret;
  cl_kernel ker = NULL;
  size_t global_off[] = {0,0,0};
  size_t global_sz[] = {1,1,1};
  size_t local_sz[] = {LOCAL_SZ_0,LOCAL_SZ_1,LOCAL_SZ_2};
  uint32_t intel_fmt, bpp;
  cl_image_format fmt;
  size_t origin0, region0;
  size_t kn_dst_offset;
  int align16 = 0;
  size_t align_size = 1;
  size_t w_saved;

  if(region[1] == 1) local_sz[1] = 1;
  if(region[2] == 1) local_sz[2] = 1;
  global_sz[0] = ((region[0] + local_sz[0] - 1) / local_sz[0]) * local_sz[0];
  global_sz[1] = ((region[1] + local_sz[1] - 1) / local_sz[1]) * local_sz[1];
  global_sz[2] = ((region[2] + local_sz[2] - 1) / local_sz[2]) * local_sz[2];

  /* We use one kernel to copy the data. The kernel is lazily created. */
  assert(image->base.ctx == buffer->ctx);

  intel_fmt = image->intel_fmt;
  bpp = image->bpp;
  w_saved = image->w;
  region0 = region[0] * bpp;
  kn_dst_offset = dst_offset;
  if((image->image_type == CL_MEM_OBJECT_IMAGE2D) && ((image->w * image->bpp) % 16 == 0) &&
      ((src_origin[0] * bpp) % 16 == 0) && (region0 % 16 == 0) && (dst_offset % 16 == 0)){
    fmt.image_channel_order = CL_RGBA;
    fmt.image_channel_data_type = CL_UNSIGNED_INT32;
    align16 = 1;
    align_size = 16;
  }
  else{
    fmt.image_channel_order = CL_R;
    fmt.image_channel_data_type = CL_UNSIGNED_INT8;
    align_size = 1;
  }
  image->intel_fmt = cl_image_get_intel_format(&fmt);
  image->w = (image->w * image->bpp) / align_size;
  image->bpp = align_size;
  region0 = (region[0] * bpp) / align_size;
  origin0 = (src_origin[0] * bpp) / align_size;
  kn_dst_offset /= align_size;
  global_sz[0] = ((region0 + local_sz[0] - 1) / local_sz[0]) * local_sz[0];

  /* setup the kernel and run. */
  if(image->image_type == CL_MEM_OBJECT_IMAGE2D) {
    if(align16){
      extern char cl_internal_copy_image_2d_to_buffer_align16_str[];
      extern size_t cl_internal_copy_image_2d_to_buffer_align16_str_size;

      ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_IMAGE_2D_TO_BUFFER_ALIGN16,
                cl_internal_copy_image_2d_to_buffer_align16_str,
                (size_t)cl_internal_copy_image_2d_to_buffer_align16_str_size, NULL);
    }
    else{
      extern char cl_internal_copy_image_2d_to_buffer_str[];
      extern size_t cl_internal_copy_image_2d_to_buffer_str_size;

      ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_IMAGE_2D_TO_BUFFER,
          cl_internal_copy_image_2d_to_buffer_str, (size_t)cl_internal_copy_image_2d_to_buffer_str_size, NULL);
    }
  }else if(image->image_type == CL_MEM_OBJECT_IMAGE3D) {
    extern char cl_internal_copy_image_3d_to_buffer_str[];
    extern size_t cl_internal_copy_image_3d_to_buffer_str_size;

    ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_IMAGE_3D_TO_BUFFER,
          cl_internal_copy_image_3d_to_buffer_str, (size_t)cl_internal_copy_image_3d_to_buffer_str_size, NULL);
  }

  if (!ker) {
    ret = CL_OUT_OF_RESOURCES;
    goto fail;
  }

  cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &image);
  cl_kernel_set_arg(ker, 1, sizeof(cl_mem), &buffer);
  cl_kernel_set_arg(ker, 2, sizeof(cl_int), &region0);
  cl_kernel_set_arg(ker, 3, sizeof(cl_int), &region[1]);
  cl_kernel_set_arg(ker, 4, sizeof(cl_int), &region[2]);
  cl_kernel_set_arg(ker, 5, sizeof(cl_int), &origin0);
  cl_kernel_set_arg(ker, 6, sizeof(cl_int), &src_origin[1]);
  cl_kernel_set_arg(ker, 7, sizeof(cl_int), &src_origin[2]);
  cl_kernel_set_arg(ker, 8, sizeof(cl_int), &kn_dst_offset);

  ret = cl_command_queue_ND_range(queue, ker, event, 1, global_off,
                                  global_off, global_sz, global_sz, local_sz, local_sz);

fail:

  cl_kernel_delete(ker);
  image->intel_fmt = intel_fmt;
  image->bpp = bpp;
  image->w = w_saved;

  return ret;
}


LOCAL cl_int
cl_mem_copy_buffer_to_image(cl_command_queue queue, cl_event event, cl_mem buffer, struct _cl_mem_image* image,
                         const size_t src_offset, const size_t *dst_origin, const size_t *region) {
  cl_int ret;
  cl_kernel ker = NULL;
  size_t global_off[] = {0,0,0};
  size_t global_sz[] = {1,1,1};
  size_t local_sz[] = {LOCAL_SZ_0,LOCAL_SZ_1,LOCAL_SZ_2};
  uint32_t intel_fmt, bpp;
  cl_image_format fmt;
  size_t origin0, region0;
  size_t kn_src_offset;
  int align16 = 0;
  size_t align_size = 1;
  size_t w_saved = 0;

  if(region[1] == 1) local_sz[1] = 1;
  if(region[2] == 1) local_sz[2] = 1;
  global_sz[0] = ((region[0] + local_sz[0] - 1) / local_sz[0]) * local_sz[0];
  global_sz[1] = ((region[1] + local_sz[1] - 1) / local_sz[1]) * local_sz[1];
  global_sz[2] = ((region[2] + local_sz[2] - 1) / local_sz[2]) * local_sz[2];

  /* We use one kernel to copy the data. The kernel is lazily created. */
  assert(image->base.ctx == buffer->ctx);

  intel_fmt = image->intel_fmt;
  bpp = image->bpp;
  w_saved = image->w;
  region0 = region[0] * bpp;
  kn_src_offset = src_offset;
  if((image->image_type == CL_MEM_OBJECT_IMAGE2D) && ((image->w * image->bpp) % 16 == 0) &&
      ((dst_origin[0] * bpp) % 16 == 0) && (region0 % 16 == 0) && (src_offset % 16 == 0)){
    fmt.image_channel_order = CL_RGBA;
    fmt.image_channel_data_type = CL_UNSIGNED_INT32;
    align16 = 1;
    align_size = 16;
  }
  else{
    fmt.image_channel_order = CL_R;
    fmt.image_channel_data_type = CL_UNSIGNED_INT8;
    align_size = 1;
  }
  image->intel_fmt = cl_image_get_intel_format(&fmt);
  image->w = (image->w * image->bpp) / align_size;
  image->bpp = align_size;
  region0 = (region[0] * bpp) / align_size;
  origin0 = (dst_origin[0] * bpp) / align_size;
  kn_src_offset /= align_size;
  global_sz[0] = ((region0 + local_sz[0] - 1) / local_sz[0]) * local_sz[0];

  /* setup the kernel and run. */
  if(image->image_type == CL_MEM_OBJECT_IMAGE2D) {
    if(align16){
      extern char cl_internal_copy_buffer_to_image_2d_align16_str[];
      extern size_t cl_internal_copy_buffer_to_image_2d_align16_str_size;

      ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_BUFFER_TO_IMAGE_2D_ALIGN16,
                cl_internal_copy_buffer_to_image_2d_align16_str,
                (size_t)cl_internal_copy_buffer_to_image_2d_align16_str_size, NULL);
    }
    else{
      extern char cl_internal_copy_buffer_to_image_2d_str[];
      extern size_t cl_internal_copy_buffer_to_image_2d_str_size;

      ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_BUFFER_TO_IMAGE_2D,
          cl_internal_copy_buffer_to_image_2d_str, (size_t)cl_internal_copy_buffer_to_image_2d_str_size, NULL);
    }
  }else if(image->image_type == CL_MEM_OBJECT_IMAGE3D) {
      extern char cl_internal_copy_buffer_to_image_3d_str[];
      extern size_t cl_internal_copy_buffer_to_image_3d_str_size;

      ker = cl_context_get_static_kernel_from_bin(queue->ctx, CL_ENQUEUE_COPY_BUFFER_TO_IMAGE_3D,
          cl_internal_copy_buffer_to_image_3d_str, (size_t)cl_internal_copy_buffer_to_image_3d_str_size, NULL);
  }
  if (!ker)
    return CL_OUT_OF_RESOURCES;

  cl_kernel_set_arg(ker, 0, sizeof(cl_mem), &image);
  cl_kernel_set_arg(ker, 1, sizeof(cl_mem), &buffer);
  cl_kernel_set_arg(ker, 2, sizeof(cl_int), &region0);
  cl_kernel_set_arg(ker, 3, sizeof(cl_int), &region[1]);
  cl_kernel_set_arg(ker, 4, sizeof(cl_int), &region[2]);
  cl_kernel_set_arg(ker, 5, sizeof(cl_int), &origin0);
  cl_kernel_set_arg(ker, 6, sizeof(cl_int), &dst_origin[1]);
  cl_kernel_set_arg(ker, 7, sizeof(cl_int), &dst_origin[2]);
  cl_kernel_set_arg(ker, 8, sizeof(cl_int), &kn_src_offset);

  ret = cl_command_queue_ND_range(queue, ker, event, 1, global_off,
                                  global_off, global_sz, global_sz, local_sz, local_sz);
  cl_kernel_delete(ker);

  image->intel_fmt = intel_fmt;
  image->bpp = bpp;
  image->w = w_saved;

  return ret;
}


LOCAL void*
cl_mem_map(cl_mem mem, int write)
{
  cl_buffer_map(mem->bo, write);
  assert(cl_buffer_get_virtual(mem->bo));
  return cl_buffer_get_virtual(mem->bo);
}

LOCAL cl_int
cl_mem_unmap(cl_mem mem)
{
  cl_buffer_unmap(mem->bo);
  return CL_SUCCESS;
}

LOCAL void*
cl_mem_map_gtt(cl_mem mem)
{
  cl_buffer_map_gtt(mem->bo);
  assert(cl_buffer_get_virtual(mem->bo));
  mem->mapped_gtt = 1;
  return cl_buffer_get_virtual(mem->bo);
}

LOCAL void *
cl_mem_map_gtt_unsync(cl_mem mem)
{
  cl_buffer_map_gtt_unsync(mem->bo);
  assert(cl_buffer_get_virtual(mem->bo));
  return cl_buffer_get_virtual(mem->bo);
}

LOCAL cl_int
cl_mem_unmap_gtt(cl_mem mem)
{
  cl_buffer_unmap_gtt(mem->bo);
  return CL_SUCCESS;
}

LOCAL void*
cl_mem_map_auto(cl_mem mem, int write)
{
  //if mem is not created from userptr, the offset should be always zero.
  if (!mem->is_userptr)
    assert(mem->offset == 0);

  if (IS_IMAGE(mem) && cl_mem_image(mem)->tiling != CL_NO_TILE)
    return cl_mem_map_gtt(mem);
  else {
    if (mem->is_userptr) {
      cl_buffer_wait_rendering(mem->bo);
      return mem->host_ptr;
    }else
      return cl_mem_map(mem, write);
  }
}

LOCAL cl_int
cl_mem_unmap_auto(cl_mem mem)
{
  if (mem->mapped_gtt == 1) {
    cl_buffer_unmap_gtt(mem->bo);
    mem->mapped_gtt = 0;
  }
  else if (!mem->is_userptr)
    cl_buffer_unmap(mem->bo);
  return CL_SUCCESS;
}

LOCAL cl_int
cl_mem_pin(cl_mem mem)
{
  assert(mem);
  if (UNLIKELY((mem->flags & CL_MEM_PINNABLE) == 0))
    return CL_INVALID_MEM_OBJECT;
  cl_buffer_pin(mem->bo, 4096);
  return CL_SUCCESS;
}

LOCAL cl_int
cl_mem_unpin(cl_mem mem)
{
  assert(mem);
  if (UNLIKELY((mem->flags & CL_MEM_PINNABLE) == 0))
    return CL_INVALID_MEM_OBJECT;
  cl_buffer_unpin(mem->bo);
  return CL_SUCCESS;
}

LOCAL cl_mem cl_mem_new_libva_buffer(cl_context ctx,
                                     unsigned int bo_name,
                                     cl_int* errcode)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;

  mem = cl_mem_allocate(CL_MEM_BUFFER_TYPE, ctx, 0, 0, CL_FALSE, NULL, NULL, &err);
  if (mem == NULL || err != CL_SUCCESS)
    goto error;

  size_t sz = 0;
  mem->bo = cl_buffer_get_buffer_from_libva(ctx, bo_name, &sz);
  if (mem->bo == NULL) {
    err = CL_MEM_OBJECT_ALLOCATION_FAILURE;
    goto error;
  }
  mem->size = sz;

exit:
  if (errcode)
    *errcode = err;
  return mem;

error:
  cl_mem_delete(mem);
  mem = NULL;
  goto exit;
}

LOCAL cl_mem cl_mem_new_libva_image(cl_context ctx,
                                    unsigned int bo_name, size_t offset,
                                    size_t width, size_t height,
                                    cl_image_format fmt,
                                    size_t row_pitch,
                                    cl_int *errcode)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;
  struct _cl_mem_image *image = NULL;
  uint32_t intel_fmt, bpp;

  /* Get the size of each pixel */
  if (UNLIKELY((err = cl_image_byte_per_pixel(&fmt, &bpp)) != CL_SUCCESS))
    goto error;

  intel_fmt = cl_image_get_intel_format(&fmt);
  if (intel_fmt == INTEL_UNSUPPORTED_FORMAT) {
    err = CL_IMAGE_FORMAT_NOT_SUPPORTED;
    goto error;
  }

  mem = cl_mem_allocate(CL_MEM_IMAGE_TYPE, ctx, 0, 0, 0, NULL, NULL, &err);
  if (mem == NULL || err != CL_SUCCESS)
    goto error;

  image = cl_mem_image(mem);

  mem->bo = cl_buffer_get_image_from_libva(ctx, bo_name, image);
  if (mem->bo == NULL) {
    err = CL_MEM_OBJECT_ALLOCATION_FAILURE;
    goto error;
  }

  image->w = width;
  image->h = height;
  image->image_type = CL_MEM_OBJECT_IMAGE2D;
  image->depth = 1;
  image->fmt = fmt;
  image->intel_fmt = intel_fmt;
  image->bpp = bpp;
  image->row_pitch = row_pitch;
  image->slice_pitch = 0;
  // NOTE: tiling of image is set in cl_buffer_get_image_from_libva().
  image->tile_x = 0;
  image->tile_y = 0;
  image->offset = offset;

exit:
  if (errcode)
    *errcode = err;
  return mem;

error:
  cl_mem_delete(mem);
  mem = NULL;
  goto exit;
}

LOCAL cl_int
cl_mem_get_fd(cl_mem mem,
              int* fd)
{
  cl_int err = CL_SUCCESS;
  if(cl_buffer_get_fd(mem->bo, fd))
	err = CL_INVALID_OPERATION;
  return err;
}

LOCAL cl_mem cl_mem_new_buffer_from_fd(cl_context ctx,
                                       int fd,
                                       int buffer_sz,
                                       cl_int* errcode)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;

  mem = cl_mem_allocate(CL_MEM_BUFFER_TYPE, ctx, 0, 0, CL_FALSE, NULL, NULL, &err);
  if (mem == NULL || err != CL_SUCCESS)
    goto error;

  mem->bo = cl_buffer_get_buffer_from_fd(ctx, fd, buffer_sz);
  if (mem->bo == NULL) {
    err = CL_MEM_OBJECT_ALLOCATION_FAILURE;
    goto error;
  }
  mem->size = buffer_sz;

exit:
  if (errcode)
    *errcode = err;
  return mem;

error:
  cl_mem_delete(mem);
  mem = NULL;
  goto exit;
}

LOCAL cl_mem cl_mem_new_image_from_fd(cl_context ctx,
                                      int fd, int image_sz,
                                      size_t offset,
                                      size_t width, size_t height,
                                      cl_image_format fmt,
                                      size_t row_pitch,
                                      cl_int *errcode)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;
  struct _cl_mem_image *image = NULL;
  uint32_t intel_fmt, bpp;

  /* Get the size of each pixel */
  if (UNLIKELY((err = cl_image_byte_per_pixel(&fmt, &bpp)) != CL_SUCCESS))
    goto error;

  intel_fmt = cl_image_get_intel_format(&fmt);
  if (intel_fmt == INTEL_UNSUPPORTED_FORMAT) {
    err = CL_IMAGE_FORMAT_NOT_SUPPORTED;
    goto error;
  }

  mem = cl_mem_allocate(CL_MEM_IMAGE_TYPE, ctx, 0, 0, 0, NULL, NULL, &err);
  if (mem == NULL || err != CL_SUCCESS)
    goto error;

  image = cl_mem_image(mem);

  mem->bo = cl_buffer_get_image_from_fd(ctx, fd, image_sz, image);
  if (mem->bo == NULL) {
    err = CL_MEM_OBJECT_ALLOCATION_FAILURE;
    goto error;
  }
  mem->size = image_sz;

  image->w = width;
  image->h = height;
  image->image_type = CL_MEM_OBJECT_IMAGE2D;
  image->depth = 1;
  image->fmt = fmt;
  image->intel_fmt = intel_fmt;
  image->bpp = bpp;
  image->row_pitch = row_pitch;
  image->slice_pitch = 0;
  // NOTE: tiling of image is set in cl_buffer_get_image_from_fd().
  image->tile_x = 0;
  image->tile_y = 0;
  image->offset = offset;

exit:
  if (errcode)
    *errcode = err;
  return mem;

error:
  cl_mem_delete(mem);
  mem = NULL;
  goto exit;
}

LOCAL cl_int
cl_mem_record_map_mem(cl_mem mem, void *ptr, void **mem_ptr, size_t offset,
                      size_t size, const size_t *origin, const size_t *region)
{
  // TODO: Need to add MT safe logic.

  cl_int slot = -1;
  int err = CL_SUCCESS;
  size_t sub_offset = 0;

  if(mem->type == CL_MEM_SUBBUFFER_TYPE) {
    struct _cl_mem_buffer* buffer = (struct _cl_mem_buffer*)mem;
    sub_offset = buffer->sub_offset;
  }

  ptr = (char*)ptr + offset + sub_offset;
  if(mem->flags & CL_MEM_USE_HOST_PTR) {
    assert(mem->host_ptr);
    //only calc ptr here, will do memcpy in enqueue
    *mem_ptr = (char *)mem->host_ptr + offset + sub_offset;
  } else {
    *mem_ptr = ptr;
  }
  /* Record the mapped address. */
  if (!mem->mapped_ptr_sz) {
    mem->mapped_ptr_sz = 16;
    mem->mapped_ptr = (cl_mapped_ptr *)malloc(
        sizeof(cl_mapped_ptr) * mem->mapped_ptr_sz);
    if (!mem->mapped_ptr) {
      cl_mem_unmap_auto(mem);
      err = CL_OUT_OF_HOST_MEMORY;
      goto error;
    }
    memset(mem->mapped_ptr, 0, mem->mapped_ptr_sz * sizeof(cl_mapped_ptr));
    slot = 0;
  } else {
    int i = 0;
    for (; i < mem->mapped_ptr_sz; i++) {
      if (mem->mapped_ptr[i].ptr == NULL) {
        slot = i;
        break;
      }
    }
    if (i == mem->mapped_ptr_sz) {
      cl_mapped_ptr *new_ptr = (cl_mapped_ptr *)malloc(
          sizeof(cl_mapped_ptr) * mem->mapped_ptr_sz * 2);
      if (!new_ptr) {
        cl_mem_unmap_auto(mem);
        err = CL_OUT_OF_HOST_MEMORY;
        goto error;
      }
      memset(new_ptr, 0, 2 * mem->mapped_ptr_sz * sizeof(cl_mapped_ptr));
      memcpy(new_ptr, mem->mapped_ptr,
          mem->mapped_ptr_sz * sizeof(cl_mapped_ptr));
      slot = mem->mapped_ptr_sz;
      mem->mapped_ptr_sz *= 2;
      free(mem->mapped_ptr);
      mem->mapped_ptr = new_ptr;
    }
  }
  assert(slot != -1);
  mem->mapped_ptr[slot].ptr = *mem_ptr;
  mem->mapped_ptr[slot].v_ptr = ptr;
  mem->mapped_ptr[slot].size = size;
  if(origin) {
    assert(region);
    mem->mapped_ptr[slot].origin[0] = origin[0];
    mem->mapped_ptr[slot].origin[1] = origin[1];
    mem->mapped_ptr[slot].origin[2] = origin[2];
    mem->mapped_ptr[slot].region[0] = region[0];
    mem->mapped_ptr[slot].region[1] = region[1];
    mem->mapped_ptr[slot].region[2] = region[2];
  }
  mem->map_ref++;
error:
  if (err != CL_SUCCESS)
    *mem_ptr = NULL;
  return err;
}

LOCAL cl_int
cl_mem_set_destructor_callback(cl_mem memobj,
                               void(CL_CALLBACK *pfn_notify)(cl_mem, void *), void *user_data)
{
  cl_mem_dstr_cb cb = cl_calloc(1, sizeof(_cl_mem_dstr_cb));
  if (cb == NULL) {
    return CL_OUT_OF_HOST_MEMORY;
  }

  memset(cb, 0, sizeof(_cl_mem_dstr_cb));
  list_node_init(&cb->node);
  cb->pfn_notify = pfn_notify;
  cb->user_data = user_data;

  CL_OBJECT_LOCK(memobj);
  list_add(&memobj->dstr_cb_head, &cb->node);
  CL_OBJECT_UNLOCK(memobj);
  return CL_SUCCESS;
}

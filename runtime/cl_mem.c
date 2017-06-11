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
#include "cl_image.h"
#include "cl_context.h"
#include "cl_event.h"
#include "cl_utils.h"
#include "cl_alloc.h"
#include "cl_device_id.h"
#include "cl_khr_icd.h"
#include "cl_kernel.h"
#include "cl_command_queue.h"
#include "cl_enqueue.h"

#include "CL/cl.h"
#include "CL/cl_intel.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

LOCAL cl_int
cl_mem_set_destructor_callback(cl_mem memobj,
                               void(CL_CALLBACK *pfn_notify)(cl_mem, void *), void *user_data)
{
  cl_mem_dstr_cb cb = CL_CALLOC(1, sizeof(_cl_mem_dstr_cb));
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

LOCAL cl_int
cl_mem_is_valid(cl_mem mem, cl_context ctx)
{
  struct list_node *pos;
  cl_base_object pbase_object;

  CL_OBJECT_LOCK(ctx);
  list_for_each(pos, (&ctx->mem_objects))
  {
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

LOCAL cl_mem_object_type
cl_mem_get_object_type(cl_mem mem)
{
  switch (mem->type) {
  case CL_MEM_BUFFER_TYPE:
  case CL_MEM_SUBBUFFER_TYPE:
    return CL_MEM_OBJECT_BUFFER;
  case CL_MEM_IMAGE_TYPE:
  case CL_MEM_GL_IMAGE_TYPE: {
    cl_mem_image image = cl_mem_to_image(mem);
    return image->image_type;
  }
  case CL_MEM_PIPE_TYPE:
    return CL_MEM_OBJECT_PIPE;
  default:
    assert(0);
  }
}

static cl_mem
cl_mem_new(cl_mem_type type, cl_context ctx, cl_mem_flags flags, size_t size)
{
  cl_mem mem = NULL;
  void *mem_ptr = NULL;

  /* Allocate and inialize the structure itself */
  if (type == CL_MEM_IMAGE_TYPE) {
    mem_ptr = CL_CALLOC(1, sizeof(_cl_mem_image));
  } else if (type == CL_MEM_GL_IMAGE_TYPE) {
    mem_ptr = CL_CALLOC(1, sizeof(_cl_mem_gl_image));
  } else if (type == CL_MEM_SVM_TYPE) {
    mem_ptr = CL_CALLOC(1, sizeof(_cl_mem_svm));
  } else if (type == CL_MEM_PIPE_TYPE) {
    mem_ptr = CL_CALLOC(1, sizeof(_cl_mem_pipe));
  } else if (type == CL_MEM_SUBBUFFER_TYPE || type == CL_MEM_BUFFER_TYPE) {
    mem_ptr = CL_CALLOC(1, sizeof(_cl_mem_buffer));
  } else {
    assert(0);
  }

  if (mem_ptr == NULL)
    return NULL;

  mem = (cl_mem)mem_ptr;
  CL_OBJECT_INIT_BASE(mem, CL_OBJECT_MEM_MAGIC);
  list_init(&mem->dstr_cb_head);
  list_init(&mem->mapped_ptr_head);
  mem->type = type;
  mem->flags = flags;
  mem->size = size;
  mem->each_device = CL_CALLOC(ctx->device_num, sizeof(cl_mem_for_device));
  mem->each_device_num = ctx->device_num;
  if (mem->each_device == NULL) {
    CL_FREE(mem_ptr);
    return NULL;
  }

  /* Append the buffer in the context buffer list */
  cl_context_add_mem(ctx, mem);
  return mem;
}

LOCAL void
cl_mem_add_ref(cl_mem mem)
{
  assert(mem);
  CL_OBJECT_INC_REF(mem);
}

LOCAL void
cl_mem_delete(cl_mem mem)
{
  cl_mem_dstr_cb cb = NULL;
  cl_uint map_ref;

  if (mem == NULL)
    return;

  if (CL_OBJECT_DEC_REF(mem) > 1)
    return;

  if (mem->in_enqueue_use) {
    CL_LOG_WARNING("Warning! Delete mem object: %p, while still in some enqueue API usage\n", mem);
  }
  map_ref = atomic_read(&mem->map_ref);
  if (map_ref > 0) {
    CL_LOG_WARNING("Warning! Delete mem object: %p, while still have map reference: %d\n", mem, map_ref);
  }

  if (!CL_OBJECT_IS_SVM(mem)) {
    /* First, call all the callbacks registered by user. */
    while (!list_empty(&mem->dstr_cb_head)) {
      cb = list_entry(mem->dstr_cb_head.head_node.n, _cl_mem_dstr_cb, node);
      list_node_del(&cb->node);
      cb->pfn_notify(mem, cb->user_data);
      CL_FREE(cb);
    }

    if (mem->each_device[0]) {
      mem->each_device[0]->device->api.mem_deallocate(mem->each_device[0]->device, mem);
    }
  }

  /* Can not use CL_OBJECT_IS_XXX macro, ref is already 0 */
  if (mem->type == CL_MEM_SUBBUFFER_TYPE) {
    cl_mem_buffer parent = ((cl_mem_buffer)mem)->parent;
    assert(parent);
    assert(CL_OBJECT_IS_BUFFER(parent));

    CL_OBJECT_LOCK(parent);
    list_node_del(&(((cl_mem_buffer)mem)->sub_node));
    parent->sub_buffer_num--;
    CL_OBJECT_UNLOCK(parent);
    cl_mem_delete((cl_mem)parent);
  } else if (mem->type >= CL_MEM_IMAGE_TYPE && ((cl_mem_image)mem)->mem_from) {
    cl_mem_delete(((cl_mem_image)mem)->mem_from);
    ((cl_mem_image)mem)->mem_from = NULL;
  } else if (mem->type == CL_MEM_SVM_TYPE) {
    cl_uint i;
    assert(mem->host_ptr);

    for (i = 0; i < mem->ctx->device_num; i++) {
      if (mem->each_device[i]) {
        mem->each_device[i]->device->api.svm_delete(mem->each_device[i]->device, mem);
        mem->each_device[i] = NULL;
      }
    }

    CL_FREE(mem->host_ptr);
    mem->host_ptr = NULL;
  }

  /* Remove it from the list */
  cl_context_remove_mem(mem->ctx, mem);

  CL_FREE(mem->each_device);
  CL_OBJECT_DESTROY_BASE(mem);
  CL_FREE(mem);
}

LOCAL cl_mem
cl_mem_create_buffer(cl_context ctx, cl_mem_flags flags, size_t sz, void *data, cl_int *errcode_ret)
{
  cl_mem mem = NULL;
  cl_mem_buffer buffer = NULL;
  cl_int err = CL_SUCCESS;

  mem = cl_mem_new(CL_MEM_BUFFER_TYPE, ctx, flags, sz);
  if (mem == NULL) {
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }
  buffer = cl_mem_to_buffer(mem);
  list_init(&buffer->sub_buffers);
  list_node_init(&buffer->sub_node);

  if (mem->flags & CL_MEM_USE_HOST_PTR) {
    mem->host_ptr = data;
    buffer->svm_buf = cl_context_get_svm_by_ptr(ctx, data, CL_FALSE);
    if (buffer->svm_buf) {
      buffer->svm_offset = buffer->svm_buf->host_ptr - data;
    }
  } else if (mem->flags & CL_MEM_COPY_HOST_PTR) {
    // Store the content temp in host_ptr
    mem->host_ptr = data;
  }

  if (err == CL_SUCCESS && ctx->device_num == 1) {
    // If just one device, allocate its real resource imm
    err = cl_mem_assure_allocated(ctx->devices[0], mem);
    if (err != CL_SUCCESS) {
      cl_mem_delete(mem);
      mem = NULL;
    }
  }

  *errcode_ret = err;
  return mem;
}

LOCAL cl_mem
cl_mem_create_sub_buffer(cl_mem parent_buffer, cl_mem_flags flags, cl_buffer_create_type create_type,
                         const void *create_info, cl_int *errcode_ret)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;
  cl_mem_buffer parent = NULL;
  cl_mem_buffer sub_buf = NULL;
  cl_buffer_region *info = (cl_buffer_region *)create_info;
  parent = cl_mem_to_buffer(parent_buffer);

  mem = cl_mem_new(CL_MEM_SUBBUFFER_TYPE, parent_buffer->ctx, flags, info->size);
  if (mem == NULL) {
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }
  sub_buf = cl_mem_to_buffer(mem);

  CL_OBJECT_LOCK(parent_buffer);
  list_add_tail(&parent->sub_buffers, &sub_buf->sub_node);
  parent->sub_buffer_num++;
  cl_mem_add_ref(parent_buffer);
  CL_OBJECT_UNLOCK(parent_buffer);
  sub_buf->parent = parent;
  sub_buf->sub_offset = info->origin;
  mem->size = info->size;

  if (parent_buffer->flags & CL_MEM_USE_HOST_PTR) {
    mem->host_ptr = parent_buffer->host_ptr + sub_buf->sub_offset;
  }
  sub_buf->svm_buf = parent->svm_buf;
  if (sub_buf->svm_buf) {
    assert(mem->host_ptr);
    sub_buf->svm_offset = sub_buf->svm_buf->host_ptr - mem->host_ptr;
  }

  if (err == CL_SUCCESS && parent_buffer->ctx->device_num == 1) {
    // If just one device, allocate its real resource imm
    err = cl_mem_assure_allocated(parent_buffer->ctx->devices[0], mem);
    if (err != CL_SUCCESS) {
      cl_mem_delete(mem);
      mem = NULL;
    }
  }

  *errcode_ret = err;
  return mem;
}

LOCAL cl_mem
cl_mem_create_pipe(cl_context ctx, cl_mem_flags flags, cl_uint pipe_packet_size,
                   cl_uint pipe_max_packets, cl_int *errcode_ret)
{
  cl_mem mem = NULL;
  cl_mem_pipe pipe = NULL;
  cl_int err = CL_SUCCESS;
  cl_uint sz = pipe_packet_size * pipe_max_packets;

  mem = cl_mem_new(CL_MEM_PIPE_TYPE, ctx, flags, sz);
  if (mem == NULL) {
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }

  pipe = cl_mem_to_pipe(mem);
  pipe->max_packets = pipe_max_packets;
  pipe->packet_size = pipe_packet_size;

  *errcode_ret = err;
  return mem;
}

LOCAL void *
cl_mem_svm_allocate(cl_context ctx, cl_svm_mem_flags flags, size_t size, unsigned int alignment)
{
  cl_mem mem = NULL;
  cl_mem_svm svm = NULL;
  int page_size;
  cl_uint i;
  cl_int err = CL_SUCCESS;

  mem = cl_mem_new(CL_MEM_SVM_TYPE, ctx,
                   /* Only store common flags */
                   (flags & (CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY)), size);
  if (mem == NULL) {
    return NULL;
  }

  svm = cl_mem_to_svm(mem);
  svm->flags = (flags & (CL_MEM_SVM_FINE_GRAIN_BUFFER | CL_MEM_SVM_ATOMICS));

  page_size = getpagesize();
  svm->real_size = ALIGN(size, page_size);

  if (alignment == 0)
    alignment = page_size;
  else
    alignment = ALIGN(alignment, page_size);

  mem->host_ptr = CL_MEMALIGN(alignment, svm->real_size);
  if (mem->host_ptr == NULL) {
    cl_mem_delete(mem);
    return NULL;
  }

  for (i = 0; i < ctx->device_num; i++) {
    err = (ctx->devices[i]->api.svm_create)(ctx->devices[i], mem);
    if (err != CL_SUCCESS) {
      cl_mem_delete(mem);
      return NULL;
    }
  }

  return mem->host_ptr;
}

LOCAL void
cl_mem_svm_delete(cl_context ctx, cl_mem mem_svm)
{
  assert(mem_svm->ctx == ctx);
  cl_mem_delete(mem_svm);
}

static void
cl_mem_image_init(cl_mem_image image, size_t w, size_t h, size_t depth, cl_mem_object_type image_type,
                  cl_image_format fmt, uint32_t bpp, size_t row_pitch, size_t slice_pitch, cl_mem mem_from)
{
  image->w = w;
  image->h = h;
  image->depth = depth;
  image->row_pitch = row_pitch;
  image->slice_pitch = slice_pitch;
  image->image_type = image_type;
  image->fmt = fmt;
  image->bpp = bpp;
  image->mem_from = mem_from;
  if (mem_from)
    cl_mem_add_ref(mem_from);
}

static cl_mem
cl_mem_create_2D_image(cl_context context, cl_mem_flags flags, const cl_image_format *image_format,
                       const cl_image_desc *image_desc, void *host_ptr, cl_int *errcode_ret)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;
  cl_mem_image image = NULL;
  uint32_t bpp = 0;
  size_t w = image_desc->image_width;
  size_t h = image_desc->image_height;
  size_t row_pitch = image_desc->image_row_pitch;
  size_t depth = 1;
  cl_mem mem_from = image_desc->buffer;
  cl_int i;

  if ((err = cl_image_byte_per_pixel(image_format, &bpp)) != CL_SUCCESS) {
    *errcode_ret = err;
    return NULL;
  }

  if (w == 0 || h == 0) {
    *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
    return NULL;
  }

  if (host_ptr == NULL && row_pitch != 0 && mem_from == NULL) {
    *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
    return NULL;
  }

  if (row_pitch == 0) { // Always calculate one
    row_pitch = bpp * w;
  } else {
    /* Must be multiple of bpp */
    if (row_pitch % bpp) {
      *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
      return NULL;
    }

    if (row_pitch < bpp * w) {
      *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
      return NULL;
    }
  }

  if (mem_from) {
    if (CL_OBJECT_IS_BUFFER(mem_from)) {
      if (row_pitch * h > mem_from->size) { /* The buffer is not big enough */
        *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
        return NULL;
      }
    } else {
      /* Create image from image, all are same except data fmt */
      assert(CL_OBJECT_IS_IMAGE(mem_from));
      if (w != cl_mem_to_image(mem_from)->w ||
          h != cl_mem_to_image(mem_from)->h ||
          depth != cl_mem_to_image(mem_from)->depth ||
          bpp != cl_mem_to_image(mem_from)->bpp ||
          row_pitch != cl_mem_to_image(mem_from)->row_pitch) {
        // TODO: Check the channel data type sRGB/RGB
        *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
        return NULL;
      }
    }
  }

  for (i = 0; i < context->device_num; i++) {
    if (context->devices[i]->image_support == CL_FALSE)
      continue;

    if (w > context->devices[i]->image2d_max_width) {
      *errcode_ret = CL_INVALID_IMAGE_SIZE;
      return NULL;
    }
    if (h > context->devices[i]->image2d_max_height) {
      *errcode_ret = CL_INVALID_IMAGE_SIZE;
      return NULL;
    }

    /* For a 2D image created from a buffer, the pitch must be a multiple of
       the maximum of the CL_DEVICE_IMAGE_PITCH_ALIGNMENT value */
    if (mem_from) {
      if (row_pitch % context->devices[i]->image_pitch_alignment) {
        *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
        return NULL;
      }

      if (CL_OBJECT_IS_BUFFER(mem_from)) {
        if (w > context->devices[i]->image_mem_size) {
          *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
          return NULL;
        }
      }

      if (mem_from->host_ptr &&
          ALIGN((unsigned long)(mem_from->host_ptr),
                context->devices[i]->image_base_address_alignment) !=
            (unsigned long)(mem_from->host_ptr)) {
        *errcode_ret = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
        return NULL;
      }
    }
  }

  mem = cl_mem_new(CL_MEM_IMAGE_TYPE, context, flags, row_pitch * h);
  if (mem == NULL) {
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }
  image = cl_mem_to_image(mem);

  if (mem->flags & CL_MEM_USE_HOST_PTR) {
    if (mem_from) {
      assert(mem_from->host_ptr);
      mem->host_ptr = mem_from->host_ptr;
    } else {
      assert(host_ptr);
      mem->host_ptr = host_ptr;
    }
  } else if (mem->flags & CL_MEM_COPY_HOST_PTR && mem_from == NULL) {
    // Store the content temp in host_ptr
    mem->host_ptr = host_ptr;
  }
  cl_mem_image_init(image, w, h, depth, CL_MEM_OBJECT_IMAGE2D, *image_format, bpp,
                    row_pitch, 0, mem_from);

  *errcode_ret = CL_SUCCESS;
  return mem;
}

static cl_mem
cl_mem_create_3D_image(cl_context context, cl_mem_flags flags, const cl_image_format *image_format,
                       const cl_image_desc *image_desc, void *host_ptr, cl_int *errcode_ret)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;
  cl_mem_image image = NULL;
  uint32_t bpp = 0;
  size_t w = image_desc->image_width;
  size_t h = 0;
  size_t depth = 0;
  size_t slice_pitch = image_desc->image_slice_pitch;
  size_t row_pitch = image_desc->image_row_pitch;
  cl_int i;

  assert(image_desc->buffer == NULL); // 3D never create from buffer

  if (image_desc->image_type == CL_MEM_OBJECT_IMAGE3D) {
    h = image_desc->image_height;
    depth = image_desc->image_depth;
  } else if (image_desc->image_type == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
    h = image_desc->image_height;
    depth = image_desc->image_array_size;
  } else if (image_desc->image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY) {
    h = 1;
    depth = image_desc->image_array_size;
  } else {
    assert(0);
  }

  if ((err = cl_image_byte_per_pixel(image_format, &bpp)) != CL_SUCCESS) {
    *errcode_ret = err;
    return NULL;
  }

  if (w == 0 || h == 0 || depth == 0) {
    *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
    return NULL;
  }

  if (host_ptr == NULL && (row_pitch != 0 || slice_pitch != 0)) {
    *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
    return NULL;
  }

  if (row_pitch == 0) {
    row_pitch = bpp * w;
  } else {
    /* Must be multiple of bpp */
    if (row_pitch % bpp) {
      *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
      return NULL;
    }
    if (row_pitch < bpp * w) {
      *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
      return NULL;
    }
  }

  if (slice_pitch == 0) {
    slice_pitch = row_pitch * h;
  } else {
    if (slice_pitch % row_pitch) {
      *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
      return NULL;
    }
    if (slice_pitch < row_pitch * h) {
      *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
      return NULL;
    }
  }

  for (i = 0; i < context->device_num; i++) {
    if (context->devices[i]->image_support == CL_FALSE)
      continue;

    if (w > context->devices[i]->image3d_max_width) {
      *errcode_ret = CL_INVALID_IMAGE_SIZE;
      return NULL;
    }
    if (h > context->devices[i]->image3d_max_height) {
      *errcode_ret = CL_INVALID_IMAGE_SIZE;
      return NULL;
    }
    if (image_desc->image_type == CL_MEM_OBJECT_IMAGE3D &&
        (depth > context->devices[i]->image3d_max_depth)) {
      *errcode_ret = CL_INVALID_IMAGE_SIZE;
      return NULL;
    } else if (depth > context->devices[i]->image_max_array_size) {
      *errcode_ret = CL_INVALID_IMAGE_SIZE;
      return NULL;
    }
  }

  mem = cl_mem_new(CL_MEM_IMAGE_TYPE, context, flags, slice_pitch * depth);
  if (mem == NULL) {
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }
  image = cl_mem_to_image(mem);

  if (mem->flags & CL_MEM_USE_HOST_PTR) {
    mem->host_ptr = host_ptr;
  } else if (mem->flags & CL_MEM_COPY_HOST_PTR) {
    // Store the content temp in host_ptr
    mem->host_ptr = host_ptr;
  }
  cl_mem_image_init(image, w, h, depth, image_desc->image_type,
                    *image_format, bpp, row_pitch, slice_pitch, NULL);

  *errcode_ret = CL_SUCCESS;
  return mem;
}

static cl_mem
cl_mem_create_1D_image(cl_context context, cl_mem_flags flags, const cl_image_format *image_format,
                       const cl_image_desc *image_desc, void *host_ptr, cl_int *errcode_ret)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;
  cl_mem_image image = NULL;
  uint32_t bpp = 0;
  size_t w = image_desc->image_width;
  size_t row_pitch = image_desc->image_row_pitch;
  size_t h = 1;
  size_t depth = 1;
  cl_mem mem_from = image_desc->buffer;
  cl_int i;

  if ((err = cl_image_byte_per_pixel(image_format, &bpp)) != CL_SUCCESS) {
    *errcode_ret = err;
    return NULL;
  }

  if (w == 0) {
    *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
    return NULL;
  }

  if (host_ptr == NULL && row_pitch != 0 && mem_from == NULL) {
    *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
    return NULL;
  }

  if (row_pitch == 0) { // Always calculate one
    row_pitch = bpp * w;
  } else {
    /* Must be multiple of bpp */
    if (row_pitch % bpp) {
      *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
      return NULL;
    }

    if (row_pitch < bpp * w) {
      *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
      return NULL;
    }
  }

  if (mem_from) {
    if (!CL_OBJECT_IS_BUFFER(mem_from)) {
      *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
      return NULL;
    }

    if (row_pitch > mem_from->size) { /* The buffer is not big enough */
      *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
      return NULL;
    }
  }

  for (i = 0; i < context->device_num; i++) {
    if (context->devices[i]->image_support == CL_FALSE)
      continue;

    if (w > context->devices[i]->image_mem_size) {
      *errcode_ret = CL_INVALID_IMAGE_SIZE;
      return NULL;
    }

    if (mem_from) {
      if (row_pitch % context->devices[i]->image_pitch_alignment) {
        *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
        return NULL;
      }
      if (w > context->devices[i]->image_mem_size) {
        *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
        return NULL;
      }

      if (ALIGN((unsigned long)(mem_from->host_ptr),
                context->devices[i]->image_base_address_alignment) !=
          (unsigned long)(mem_from->host_ptr)) {
        *errcode_ret = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
        return NULL;
      }
    }
  }

  mem = cl_mem_new(CL_MEM_IMAGE_TYPE, context, flags, row_pitch * h);
  if (mem == NULL) {
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }
  image = cl_mem_to_image(mem);

  if (mem->flags & CL_MEM_USE_HOST_PTR) {
    if (mem_from) {
      assert(mem_from->host_ptr);
      mem->host_ptr = mem_from->host_ptr;
    } else {
      assert(host_ptr);
      mem->host_ptr = host_ptr;
    }
  } else if (mem->flags & CL_MEM_COPY_HOST_PTR && mem_from == NULL) {
    // Store the content temp in host_ptr
    mem->host_ptr = host_ptr;
  }

  cl_mem_image_init(image, w, h, depth, image_desc->image_type, *image_format,
                    bpp, row_pitch, 0, mem_from);

  *errcode_ret = CL_SUCCESS;
  return mem;
}

LOCAL cl_mem
cl_mem_create_image(cl_context context, cl_mem_flags flags, const cl_image_format *image_format,
                    const cl_image_desc *image_desc, void *host_ptr, cl_int *errcode_ret)
{
  cl_mem_flags merged_flags;
  cl_mem ret_mem = NULL;

  merged_flags = flags;
  if (image_desc->buffer) {
    merged_flags = image_desc->buffer->flags;
    if (flags & (CL_MEM_READ_WRITE | CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY)) {
      merged_flags &= ~(CL_MEM_READ_WRITE | CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY);
      merged_flags |= flags & (CL_MEM_READ_WRITE | CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY);
    }
    if (flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)) {
      merged_flags &= ~(CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS);
      merged_flags |= flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS);
    }
  }

  switch (image_desc->image_type) {
  case CL_MEM_OBJECT_IMAGE2D:
    ret_mem = cl_mem_create_2D_image(context, merged_flags, image_format, image_desc, host_ptr, errcode_ret);
    break;
  case CL_MEM_OBJECT_IMAGE1D_ARRAY:
  case CL_MEM_OBJECT_IMAGE2D_ARRAY:
  case CL_MEM_OBJECT_IMAGE3D:
    ret_mem = cl_mem_create_3D_image(context, merged_flags, image_format, image_desc, host_ptr, errcode_ret);
    break;
  case CL_MEM_OBJECT_IMAGE1D:
  case CL_MEM_OBJECT_IMAGE1D_BUFFER:
    ret_mem = cl_mem_create_1D_image(context, merged_flags, image_format, image_desc, host_ptr, errcode_ret);
    break;
  default:
    assert(0);
  }

  if (*errcode_ret == CL_SUCCESS && context->device_num == 1) {
    // If just one device, allocate its real resource imm
    *errcode_ret = cl_mem_assure_allocated(context->devices[0], ret_mem);
    if (*errcode_ret != CL_SUCCESS) {
      cl_mem_delete(ret_mem);
      ret_mem = NULL;
    }
  }

  return ret_mem;
}

LOCAL void
cl_mem_copy_image_region_helper(const size_t *origin, const size_t *region,
                                void *dst, size_t dst_row_pitch, size_t dst_slice_pitch,
                                const void *src, size_t src_row_pitch, size_t src_slice_pitch,
                                size_t bpp, size_t image_w, size_t image_h,
                                cl_bool offset_dst, cl_bool offset_src)
{
  if (offset_dst) {
    size_t dst_offset = bpp * origin[0] + dst_row_pitch * origin[1] + dst_slice_pitch * origin[2];
    dst = (char *)dst + dst_offset;
  }
  if (offset_src) {
    size_t src_offset = bpp * origin[0] + src_row_pitch * origin[1] + src_slice_pitch * origin[2];
    src = (char *)src + src_offset;
  }
  if (!origin[0] && region[0] == image_w && dst_row_pitch == src_row_pitch &&
      (region[2] == 1 || (!origin[1] && region[1] == image_h && dst_slice_pitch == src_slice_pitch))) {
    memcpy(dst, src, region[2] == 1 ? src_row_pitch * region[1] : src_slice_pitch * region[2]);
  } else {
    cl_uint y, z;
    for (z = 0; z < region[2]; z++) {
      const char *src_ptr = src;
      char *dst_ptr = dst;
      for (y = 0; y < region[1]; y++) {
        memcpy(dst_ptr, src_ptr, bpp * region[0]);
        src_ptr += src_row_pitch;
        dst_ptr += dst_row_pitch;
      }
      src = (char *)src + src_slice_pitch;
      dst = (char *)dst + dst_slice_pitch;
    }
  }
}

/* Must call this with mem's ownership or first time when create */
LOCAL cl_int
cl_mem_assure_allocated(cl_device_id device, cl_mem mem)
{
  cl_int err = CL_SUCCESS;

  CL_OBJECT_TAKE_OWNERSHIP(mem, 1);
  if (mem->each_device[0]) {
    if (mem->each_device[0]->device == device) {
      CL_OBJECT_RELEASE_OWNERSHIP(mem);
      return CL_SUCCESS; // No need to do it again
    }

    CL_OBJECT_RELEASE_OWNERSHIP(mem);
    return CL_MEM_OBJECT_ALLOCATION_FAILURE; // Allocated by another device
  }

  if (CL_OBJECT_IS_SUB_BUFFER(mem)) {
    cl_mem parent = (cl_mem)(cl_mem_to_buffer(mem)->parent);
    assert(parent);

    err = cl_mem_assure_allocated(device, parent);

    if (err == CL_SUCCESS)
      err = device->api.mem_allocate(device, mem);
  } else if (CL_OBJECT_IS_IMAGE(mem)) {
    if (cl_mem_to_image(mem)->mem_from) {
      err = cl_mem_assure_allocated(device, cl_mem_to_image(mem)->mem_from);
    }

    if (err == CL_SUCCESS)
      err = device->api.mem_allocate(device, mem);
  } else if (CL_OBJECT_IS_SVM(mem)) {
    assert(0);
  } else {
    err = device->api.mem_allocate(device, mem);
  }

  CL_OBJECT_RELEASE_OWNERSHIP(mem);
  return err;
}

static void
cl_mem_insert_map_info(cl_mem memobj, cl_mem_map_info map_info)
{
  CL_OBJECT_LOCK(memobj);
  list_add_tail(&memobj->mapped_ptr_head, &map_info->node);
  atomic_inc(&memobj->map_ref);
  CL_OBJECT_UNLOCK(memobj);
}

static cl_mem_map_info
cl_mem_delete_map_info(cl_mem memobj, void *ptr)
{
  list_node *n;
  list_node *pos;
  cl_mem_map_info info;

  CL_OBJECT_LOCK(memobj);
  assert(list_empty(&memobj->mapped_ptr_head) || atomic_read(&memobj->map_ref) > 0);

  info = NULL;
  list_for_each_safe(pos, n, &memobj->mapped_ptr_head)
  {
    info = list_entry(pos, _cl_mem_map_info, node);
    if (info->map_ptr == ptr) {
      list_node_del(&info->node);
      atomic_dec(&memobj->map_ref);
      CL_OBJECT_UNLOCK(memobj);
      assert(atomic_read(&memobj->map_ref) >= 0);
      return info;
    }
  }
  CL_OBJECT_UNLOCK(memobj);
  return NULL;
}

LOCAL cl_int
cl_enqueue_handle_map_mem(cl_event event, cl_int status)
{
  cl_command_queue queue = event->queue;
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  cl_mem_map_info map_info = NULL;

  assert(queue);

  if (event->exec_data.type == EnqueueMapBuffer) {
    mem = event->exec_data.map_buffer.mem_obj;
    assert(CL_OBJECT_IS_BUFFER(mem));
  } else if (event->exec_data.type == EnqueueMapImage) {
    mem = event->exec_data.map_image.mem_obj;
    assert(CL_OBJECT_IS_IMAGE(mem));
  } else
    assert(0);

  if (status == CL_QUEUED) {
    err = cl_mem_assure_allocated(queue->device, mem);
    if (err != CL_SUCCESS) {
      CL_FREE(map_info);
      return err;
    }

    map_info = CL_CALLOC(1, sizeof(_cl_mem_map_info));
    if (map_info == NULL) {
      return CL_OUT_OF_HOST_MEMORY;
    }
  }

  /* Call the device's API to do the real thing */
  err = queue->device->api.mem_map(event, status);
  if (err != CL_SUCCESS) {
    if (map_info)
      CL_FREE(map_info);
    return err;
  }

  if (status == CL_QUEUED) {
    assert(map_info);
    list_node_init(&map_info->node);
    if (event->exec_data.type == EnqueueMapBuffer) {
      map_info->map_ptr = event->exec_data.map_buffer.ptr;
      assert(map_info->map_ptr);
      map_info->buffer.offset = event->exec_data.map_buffer.offset;
      map_info->buffer.size = event->exec_data.map_buffer.size;
    } else if (event->exec_data.type == EnqueueMapImage) {
      int i;
      map_info->map_ptr = event->exec_data.map_image.ptr;
      assert(map_info->map_ptr);
      for (i = 0; i < 3; i++) {
        map_info->image.origin[i] = event->exec_data.map_image.origin[i];
        map_info->image.region[i] = event->exec_data.map_image.region[i];
      }
      map_info->image.row_pitch = event->exec_data.map_image.row_pitch;
      map_info->image.slice_pitch = event->exec_data.map_image.slice_pitch;
    }
    cl_mem_insert_map_info(mem, map_info);
  }

  return CL_SUCCESS;
}

LOCAL cl_int
cl_enqueue_handle_unmap_mem(cl_event event, cl_int status)
{
  cl_command_queue queue = event->queue;
  cl_mem mem = event->exec_data.unmap.mem_obj;
  cl_int err = CL_SUCCESS;
  cl_mem_map_info map_info = NULL;
  assert(queue);
  assert(mem);
  assert(CL_OBJECT_IS_MEM(mem));

  if (status == CL_QUEUED) {
    map_info = cl_mem_delete_map_info(mem, event->exec_data.unmap.ptr);
    if (map_info == NULL) {
      return CL_INVALID_VALUE;
    }

    if (CL_OBJECT_IS_BUFFER(event->exec_data.unmap.mem_obj)) {
      assert(CL_OBJECT_IS_MEM(event->exec_data.unmap.mem_obj));
      event->exec_data.unmap.offset = map_info->buffer.offset;
      event->exec_data.unmap.size = map_info->buffer.size;
    } else {
      int i;
      assert(CL_OBJECT_IS_IMAGE(event->exec_data.unmap.mem_obj));
      for (i = 0; i < 3; i++) {
        event->exec_data.unmap.origin[i] = map_info->image.origin[i];
        event->exec_data.unmap.region[i] = map_info->image.region[i];
      }
      event->exec_data.unmap.row_pitch = map_info->image.row_pitch;
      event->exec_data.unmap.slice_pitch = map_info->image.slice_pitch;
    }

    CL_FREE(map_info);
  }

  /* Call the device's API to do the real thing */
  err = queue->device->api.mem_unmap(event, status);
  return err;
}

LOCAL cl_int
cl_enqueue_handle_read_write_mem(cl_event event, cl_int status)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;
  cl_command_queue queue = event->queue;
  assert(queue);

  if (event->exec_data.type == EnqueueWriteBuffer || event->exec_data.type == EnqueueReadBuffer) {
    mem = event->exec_data.read_write_buffer.buffer;
    assert(CL_OBJECT_IS_BUFFER(mem));
  } else if (event->exec_data.type == EnqueueWriteBufferRect || event->exec_data.type == EnqueueReadBufferRect) {
    mem = event->exec_data.read_write_buffer_rect.buffer;
    assert(CL_OBJECT_IS_BUFFER(mem));
  } else if (event->exec_data.type == EnqueueWriteImage || event->exec_data.type == EnqueueReadImage) {
    mem = event->exec_data.read_write_image.image;
    assert(CL_OBJECT_IS_IMAGE(mem));
  } else {
    assert(0);
  }

  if (status == CL_QUEUED) {
    err = cl_mem_assure_allocated(queue->device, mem);
    if (err != CL_SUCCESS) {
      return err;
    }
  }

  /* Call the device's API to do the real thing */
  if (event->exec_data.type == EnqueueReadBuffer)
    err = queue->device->api.buffer_read(event, status);
  else if (event->exec_data.type == EnqueueWriteBuffer)
    err = queue->device->api.buffer_write(event, status);
  else if (event->exec_data.type == EnqueueReadBufferRect)
    err = queue->device->api.buffer_read_rect(event, status);
  else if (event->exec_data.type == EnqueueWriteBufferRect)
    err = queue->device->api.buffer_write_rect(event, status);
  if (event->exec_data.type == EnqueueReadImage)
    err = queue->device->api.image_read(event, status);
  else if (event->exec_data.type == EnqueueWriteImage)
    err = queue->device->api.image_write(event, status);

  return err;
}

LOCAL cl_int
cl_enqueue_handle_copy_mem(cl_event event, cl_int status)
{
  cl_int err = CL_SUCCESS;
  cl_mem src_mem = NULL;
  cl_mem dst_mem = NULL;
  cl_command_queue queue = event->queue;

  assert(queue);

  if (event->exec_data.type == EnqueueCopyBuffer) {
    src_mem = event->exec_data.copy_buffer.src;
    dst_mem = event->exec_data.copy_buffer.dst;
    assert(CL_OBJECT_IS_BUFFER(src_mem));
    assert(CL_OBJECT_IS_BUFFER(dst_mem));
  } else if (event->exec_data.type == EnqueueCopyBufferRect) {
    src_mem = event->exec_data.copy_buffer_rect.src_buf;
    dst_mem = event->exec_data.copy_buffer_rect.dst_buf;
    assert(CL_OBJECT_IS_BUFFER(src_mem));
    assert(CL_OBJECT_IS_BUFFER(dst_mem));
  } else if (event->exec_data.type == EnqueueCopyImage) {
    src_mem = event->exec_data.copy_image.src_image;
    dst_mem = event->exec_data.copy_image.dst_image;
    assert(CL_OBJECT_IS_IMAGE(src_mem));
    assert(CL_OBJECT_IS_IMAGE(dst_mem));
  } else if (event->exec_data.type == EnqueueCopyImageToBuffer) {
    src_mem = event->exec_data.copy_image_and_buffer.image;
    dst_mem = event->exec_data.copy_image_and_buffer.buffer;
    assert(CL_OBJECT_IS_IMAGE(src_mem));
    assert(CL_OBJECT_IS_BUFFER(dst_mem));
  } else if (event->exec_data.type == EnqueueCopyBufferToImage) {
    src_mem = event->exec_data.copy_image_and_buffer.buffer;
    dst_mem = event->exec_data.copy_image_and_buffer.image;
    assert(CL_OBJECT_IS_BUFFER(src_mem));
    assert(CL_OBJECT_IS_IMAGE(dst_mem));
  } else {
    assert(0);
  }

  if (status == CL_QUEUED) {
    err = cl_mem_assure_allocated(queue->device, src_mem);
    if (err != CL_SUCCESS) {
      return err;
    }
    err = cl_mem_assure_allocated(queue->device, dst_mem);
    if (err != CL_SUCCESS) {
      return err;
    }
  }

  /* Call the device's API to do the real thing */
  if (event->exec_data.type == EnqueueCopyBufferRect)
    err = queue->device->api.buffer_copy_rect(event, status);
  else if (event->exec_data.type == EnqueueCopyBuffer)
    err = queue->device->api.buffer_copy(event, status);
  else if (event->exec_data.type == EnqueueCopyImage)
    err = queue->device->api.image_copy(event, status);
  else if (event->exec_data.type == EnqueueCopyImageToBuffer)
    err = queue->device->api.copy_image_to_buffer(event, status);
  else if (event->exec_data.type == EnqueueCopyBufferToImage)
    err = queue->device->api.copy_buffer_to_image(event, status);

  return err;
}

LOCAL cl_int
cl_enqueue_handle_fill_mem(cl_event event, cl_int status)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;
  cl_command_queue queue = event->queue;

  assert(queue);

  if (event->exec_data.type == EnqueueFillBuffer) {
    mem = event->exec_data.fill_buffer.buffer;
    assert(CL_OBJECT_IS_BUFFER(mem));
  } else if (event->exec_data.type == EnqueueFillImage) {
    mem = event->exec_data.fill_image.image;
    assert(CL_OBJECT_IS_IMAGE(mem));
  } else {
    assert(0);
  }

  if (status == CL_QUEUED) {
    err = cl_mem_assure_allocated(queue->device, mem);
    if (err != CL_SUCCESS) {
      return err;
    }
  }

  /* Call the device's API to do the real thing */
  if (event->exec_data.type == EnqueueFillBuffer)
    err = queue->device->api.buffer_fill(event, status);
  else if (event->exec_data.type == EnqueueFillImage)
    err = queue->device->api.image_fill(event, status);

  return err;
}

LOCAL void
cl_svm_free_delete_func(cl_event event)
{
  assert(event->exec_data.type == EnqueueSVMMemFree);
  if (event->exec_data.svm_free.ptrs) {
    CL_FREE(event->exec_data.svm_free.ptrs);
    event->exec_data.svm_free.ptrs = NULL;
  }

  if (event->exec_data.svm_free.mem_ptrs) {
    assert(event->exec_data.svm_free.mem_num > 0);
    CL_FREE(event->exec_data.svm_free.mem_ptrs);
    event->exec_data.svm_free.mem_ptrs = NULL;
  }
}

LOCAL cl_int
cl_enqueue_handle_svm_free(cl_event event, cl_int status)
{
  cl_command_queue queue = event->queue;
  cl_uint i;

  assert(event->exec_data.type == EnqueueSVMMemFree);
  assert(queue);
  assert(event->exec_data.svm_free.queue == queue);

  if (status != CL_COMPLETE)
    return CL_SUCCESS;

  if (event->exec_data.svm_free.free_func) {
    event->exec_data.svm_free.free_func(queue, event->exec_data.svm_free.mem_num,
                                        event->exec_data.svm_free.ptrs,
                                        event->exec_data.svm_free.user_data);
  } else {
    for (i = 0; i < event->exec_data.svm_free.mem_num; i++) {
      if (event->exec_data.svm_free.mem_ptrs[i] == NULL)
        continue;

      assert(CL_OBJECT_IS_SVM(event->exec_data.svm_free.mem_ptrs[i]));
      cl_mem_svm_delete(event->ctx, event->exec_data.svm_free.mem_ptrs[i]);
    }
  }
  return CL_SUCCESS;
}

LOCAL cl_int
cl_enqueue_handle_svm_map_unmap(cl_event event, cl_int status)
{
  cl_command_queue queue = event->queue;
  cl_int err = CL_SUCCESS;

  if (event->exec_data.type == EnqueueSVMMemMap) {
    assert(CL_OBJECT_IS_SVM(event->exec_data.svm_map.svm));
    err = queue->device->api.svm_map(event, status);
  } else if (event->exec_data.type == EnqueueSVMMemUnMap) {
    assert(CL_OBJECT_IS_SVM(event->exec_data.svm_unmap.svm));
    err = queue->device->api.svm_unmap(event, status);
  } else {
    assert(0);
  }

  return err;
}

LOCAL cl_int
cl_enqueue_handle_svm_copy(cl_event event, cl_int status)
{
  cl_command_queue queue = event->queue;
  cl_mem src, dst;

  assert(event->exec_data.type == EnqueueSVMMemCopy);
  src = event->exec_data.svm_copy.src;
  dst = event->exec_data.svm_copy.dst;
  assert(CL_OBJECT_IS_SVM(src));
  assert(CL_OBJECT_IS_SVM(dst));

  return queue->device->api.svm_copy(event, status);
}

LOCAL cl_int
cl_enqueue_handle_svm_fill(cl_event event, cl_int status)
{
  cl_command_queue queue = event->queue;

  assert(event->exec_data.type == EnqueueSVMMemFill);
  assert(CL_OBJECT_IS_SVM(event->exec_data.svm_fill.svm));

  return queue->device->api.svm_fill(event, status);
}

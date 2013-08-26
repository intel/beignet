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

#include "CL/cl.h"
#include "CL/cl_intel.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define FIELD_SIZE(CASE,TYPE)               \
  case JOIN(CL_,CASE):                      \
    if(param_value_size_ret)                \
      *param_value_size_ret = sizeof(TYPE); \
    if(!param_value)                        \
      return CL_SUCCESS;                    \
    if(param_value_size < sizeof(TYPE))     \
      return CL_INVALID_VALUE;              \
    break;

LOCAL cl_int
cl_get_mem_object_info(cl_mem mem,
                cl_mem_info param_name,
                size_t param_value_size,
                void *param_value,
                size_t *param_value_size_ret)
{
  switch(param_name)
  {
    FIELD_SIZE(MEM_TYPE, cl_mem_object_type);
    FIELD_SIZE(MEM_FLAGS, cl_mem_flags);
    FIELD_SIZE(MEM_SIZE, size_t);
    FIELD_SIZE(MEM_HOST_PTR, void *);
    FIELD_SIZE(MEM_MAP_COUNT, cl_uint);
    FIELD_SIZE(MEM_REFERENCE_COUNT, cl_uint);
    FIELD_SIZE(MEM_CONTEXT, cl_context);
    FIELD_SIZE(MEM_ASSOCIATED_MEMOBJECT, cl_mem);
    FIELD_SIZE(MEM_OFFSET, size_t);
  default:
    return CL_INVALID_VALUE;
  }

  switch(param_name)
  {
  case CL_MEM_TYPE:
    *((cl_mem_object_type *)param_value) = mem->type;
    break;
  case CL_MEM_FLAGS:
    *((cl_mem_flags *)param_value) = mem->flags;
    break;
  case CL_MEM_SIZE:
    *((size_t *)param_value) = mem->size;
    break;
  case CL_MEM_HOST_PTR:
    *((size_t *)param_value) = (size_t)mem->host_ptr;
    break;
  case CL_MEM_MAP_COUNT:
    *((cl_uint *)param_value) = mem->map_ref;
    break;
  case CL_MEM_REFERENCE_COUNT:
    *((cl_uint *)param_value) = mem->ref_n;
    break;
  case CL_MEM_CONTEXT:
    *((cl_context *)param_value) = mem->ctx;
    break;
  // TODO: Need to implement sub buffer first.
  case CL_MEM_ASSOCIATED_MEMOBJECT:
    NOT_IMPLEMENTED;
    break;
  case CL_MEM_OFFSET:
    NOT_IMPLEMENTED;
    break;
  }

  return CL_SUCCESS;
}

LOCAL cl_int
cl_get_image_info(cl_mem mem,
                  cl_image_info param_name,
                  size_t param_value_size,
                  void *param_value,
                  size_t *param_value_size_ret)
{
  int err;
  CHECK_IMAGE(mem);

  switch(param_name)
  {
    FIELD_SIZE(IMAGE_FORMAT, cl_image_format);
    FIELD_SIZE(IMAGE_ELEMENT_SIZE, size_t);
    FIELD_SIZE(IMAGE_ROW_PITCH, size_t);
    FIELD_SIZE(IMAGE_SLICE_PITCH, size_t);
    FIELD_SIZE(IMAGE_WIDTH, size_t);
    FIELD_SIZE(IMAGE_HEIGHT, size_t);
    FIELD_SIZE(IMAGE_DEPTH, size_t);
  default:
    return CL_INVALID_VALUE;
  }

  switch(param_name)
  {
  case CL_IMAGE_FORMAT:
    *(cl_image_format *)param_value = image->fmt;
    break;
  case CL_IMAGE_ELEMENT_SIZE:
    *(size_t *)param_value = image->bpp;
    break;
  case CL_IMAGE_ROW_PITCH:
    *(size_t *)param_value = image->row_pitch;
    break;
  case CL_IMAGE_SLICE_PITCH:
    *(size_t *)param_value = image->slice_pitch;
    break;
  case CL_IMAGE_WIDTH:
    *(size_t *)param_value = image->w;
    break;
  case CL_IMAGE_HEIGHT:
    *(size_t *)param_value = image->h;
    break;
  case CL_IMAGE_DEPTH:
    *(size_t *)param_value = image->depth;
    break;
  }

  return CL_SUCCESS;

error:
    return err;
}

#undef FIELD_SIZE

LOCAL cl_mem
cl_mem_allocate(enum cl_mem_type type,
                cl_context ctx,
                cl_mem_flags flags,
                size_t sz,
                cl_int is_tiled,
                cl_int *errcode)
{
  cl_buffer_mgr bufmgr = NULL;
  cl_mem mem = NULL;
  struct _cl_mem_image *image = NULL;
  struct _cl_mem_buffer *buffer = NULL;
  cl_int err = CL_SUCCESS;
  size_t alignment = 64;
  cl_ulong max_mem_size;

  assert(ctx);

  if ((err = cl_get_device_info(ctx->device,
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

  /* Allocate and inialize the structure itself */
  if (type == CL_MEM_IMAGE_TYPE) {
    TRY_ALLOC (image, CALLOC(struct _cl_mem_image));
    mem = &image->base;
  }
  else {
    TRY_ALLOC (buffer, CALLOC(struct _cl_mem_buffer));
    mem = &buffer->base;
  }
  mem->type = type;
  SET_ICD(mem->dispatch)
  mem->ref_n = 1;
  mem->magic = CL_MAGIC_MEM_HEADER;
  mem->flags = flags;

  if (sz != 0) {
    /* Pinning will require stricter alignment rules */
    if ((flags & CL_MEM_PINNABLE) || is_tiled)
      alignment = 4096;

    /* Allocate space in memory */
    bufmgr = cl_context_get_bufmgr(ctx);
    assert(bufmgr);
    mem->bo = cl_buffer_alloc(bufmgr, "CL memory object", sz, alignment);
    if (UNLIKELY(mem->bo == NULL)) {
      err = CL_MEM_OBJECT_ALLOCATION_FAILURE;
      goto error;
    }
    mem->size = sz;
  }

  cl_context_add_ref(ctx);
  mem->ctx = ctx;
    /* Append the buffer in the context buffer list */
  pthread_mutex_lock(&ctx->buffer_lock);
  mem->next = ctx->buffers;
  if (ctx->buffers != NULL)
    ctx->buffers->prev = mem;
  ctx->buffers = mem;
  pthread_mutex_unlock(&ctx->buffer_lock);

exit:
  if (errcode)
    *errcode = err;
  return mem;
error:
  cl_mem_delete(mem);
  mem = NULL;
  goto exit;

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

  /* This flag is valid only if host_ptr is not NULL */
  if (UNLIKELY((flags & CL_MEM_COPY_HOST_PTR ||
                flags & CL_MEM_USE_HOST_PTR) &&
                data == NULL)) {
    err = CL_INVALID_HOST_PTR;
    goto error;
  }

  /* CL_MEM_ALLOC_HOST_PTR and CL_MEM_USE_HOST_PTR
     are mutually exclusive. */
  if (UNLIKELY(flags & CL_MEM_ALLOC_HOST_PTR &&
               flags & CL_MEM_USE_HOST_PTR)) {
    err = CL_INVALID_HOST_PTR;
    goto error;
  }

  /* CL_MEM_COPY_HOST_PTR and CL_MEM_USE_HOST_PTR
     are mutually exclusive. */
  if (UNLIKELY(flags & CL_MEM_COPY_HOST_PTR &&
               flags & CL_MEM_USE_HOST_PTR)) {
    err = CL_INVALID_HOST_PTR;
    goto error;
  }

  /* Create the buffer in video memory */
  mem = cl_mem_allocate(CL_MEM_BUFFER_TYPE, ctx, flags, sz, CL_FALSE, &err);
  if (mem == NULL || err != CL_SUCCESS)
    goto error;

  mem->type = CL_MEM_OBJECT_BUFFER;

  /* Copy the data if required */
  if (flags & CL_MEM_COPY_HOST_PTR || flags & CL_MEM_USE_HOST_PTR)
    cl_buffer_subdata(mem->bo, 0, sz, data);

  if (flags & CL_MEM_USE_HOST_PTR || flags & CL_MEM_COPY_HOST_PTR)
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

static void
cl_mem_copy_image(struct _cl_mem_image *image,
		  size_t row_pitch,
		  size_t slice_pitch,
		  void* host_ptr)
{
  char* dst_ptr = cl_mem_map_auto((cl_mem)image);

  if (row_pitch == image->row_pitch &&
      (image->depth == 1 || slice_pitch == image->slice_pitch))
  {
    memcpy(dst_ptr, host_ptr, image->depth == 1 ? row_pitch*image->h : slice_pitch*image->depth);
  }
  else {
    size_t y, z;
    for (z = 0; z < image->depth; z++) {
      const char* src = host_ptr;
      char* dst = dst_ptr;
      for (y = 0; y < image->h; y++) {
	memcpy(dst, src, image->bpp*image->w);
	src += row_pitch;
	dst += image->row_pitch;
      }
      host_ptr = (char*)host_ptr + slice_pitch;
      dst_ptr = (char*)dst_ptr + image->slice_pitch;
    }
  }

  cl_mem_unmap_auto((cl_mem)image);
}

static const uint32_t tile_sz = 4096; /* 4KB per tile */
static const uint32_t tilex_w = 512;  /* tileX width in bytes */
static const uint32_t tilex_h = 8;    /* tileX height in number of rows */
static const uint32_t tiley_w = 128;  /* tileY width in bytes */
static const uint32_t tiley_h = 32;   /* tileY height in number of rows */

static cl_mem
_cl_mem_new_image(cl_context ctx,
                  cl_mem_flags flags,
                  const cl_image_format *fmt,
                  const cl_mem_object_type image_type,
                  size_t w,
                  size_t h,
                  size_t depth,
                  size_t pitch,
                  size_t slice_pitch,
                  void *data,
                  cl_int *errcode_ret)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;
  uint32_t bpp = 0, intel_fmt = INTEL_UNSUPPORTED_FORMAT;
  size_t sz = 0, aligned_pitch = 0, aligned_h;
  cl_image_tiling_t tiling = CL_NO_TILE;

  /* Check flags consistency */
  if (UNLIKELY((flags & CL_MEM_COPY_HOST_PTR) && data == NULL)) {
    err = CL_INVALID_HOST_PTR;
    goto error;
  }

  /* Get the size of each pixel */
  if (UNLIKELY((err = cl_image_byte_per_pixel(fmt, &bpp)) != CL_SUCCESS))
    goto error;

  /* Only a sub-set of the formats are supported */
  intel_fmt = cl_image_get_intel_format(fmt);
  if (UNLIKELY(intel_fmt == INTEL_UNSUPPORTED_FORMAT)) {
    err = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    goto error;
  }

  /* See if the user parameters match */
#define DO_IMAGE_ERROR            \
  do {                            \
    err = CL_INVALID_IMAGE_SIZE;  \
    goto error;                   \
  } while (0);
  if (UNLIKELY(w == 0)) DO_IMAGE_ERROR;
  if (UNLIKELY(h == 0)) DO_IMAGE_ERROR;

  if (image_type == CL_MEM_OBJECT_IMAGE2D) {
    size_t min_pitch = bpp * w;
    if (data && pitch == 0)
      pitch = min_pitch;
    if (UNLIKELY(w > ctx->device->image2d_max_width)) DO_IMAGE_ERROR;
    if (UNLIKELY(h > ctx->device->image2d_max_height)) DO_IMAGE_ERROR;
    if (UNLIKELY(data && min_pitch > pitch)) DO_IMAGE_ERROR;
    if (UNLIKELY(!data && pitch != 0)) DO_IMAGE_ERROR;

    /* Pick up tiling mode (we do only linear on SNB) */
    if (cl_driver_get_ver(ctx->drv) != 6)
      tiling = CL_TILE_Y;
    depth = 1;
  }

  if (image_type == CL_MEM_OBJECT_IMAGE3D) {
    size_t min_pitch = bpp * w;
    if (data && pitch == 0)
      pitch = min_pitch;
    size_t min_slice_pitch = min_pitch * h;
    if (data && slice_pitch == 0)
      slice_pitch = min_slice_pitch;
    if (UNLIKELY(w > ctx->device->image3d_max_width)) DO_IMAGE_ERROR;
    if (UNLIKELY(h > ctx->device->image3d_max_height)) DO_IMAGE_ERROR;
    if (UNLIKELY(depth > ctx->device->image3d_max_depth)) DO_IMAGE_ERROR;
    if (UNLIKELY(data && min_pitch > pitch)) DO_IMAGE_ERROR;
    if (UNLIKELY(data && min_slice_pitch > slice_pitch)) DO_IMAGE_ERROR;
    if (UNLIKELY(!data && pitch != 0)) DO_IMAGE_ERROR;
    if (UNLIKELY(!data && slice_pitch != 0)) DO_IMAGE_ERROR;

    /* Pick up tiling mode (we do only linear on SNB) */
    if (cl_driver_get_ver(ctx->drv) != 6)
      tiling = CL_TILE_Y;
  }
#undef DO_IMAGE_ERROR

  /* Tiling requires to align both pitch and height */
  if (tiling == CL_NO_TILE) {
    aligned_pitch = w * bpp;
    aligned_h     = h;
  } else if (tiling == CL_TILE_X) {
    aligned_pitch = ALIGN(w * bpp, tilex_w);
    aligned_h     = ALIGN(h, tilex_h);
  } else if (tiling == CL_TILE_Y) {
    aligned_pitch = ALIGN(w * bpp, tiley_w);
    aligned_h     = ALIGN(h, tiley_h);
  }

  sz = aligned_pitch * aligned_h * depth;
  mem = cl_mem_allocate(CL_MEM_IMAGE_TYPE, ctx, flags, sz, tiling != CL_NO_TILE, &err);
  if (mem == NULL || err != CL_SUCCESS)
    goto error;

  cl_buffer_set_tiling(mem->bo, tiling, aligned_pitch);
  slice_pitch = (image_type == CL_MEM_OBJECT_IMAGE1D
                 || image_type == CL_MEM_OBJECT_IMAGE2D) ? 0 : aligned_pitch*aligned_h;

  cl_mem_image_init(cl_mem_image(mem), w, h, image_type, depth, *fmt, intel_fmt, bpp, aligned_pitch, slice_pitch, tiling);

  /* Copy the data if required */
  if (flags & CL_MEM_COPY_HOST_PTR)
    cl_mem_copy_image(cl_mem_image(mem), pitch, slice_pitch, data);

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
cl_mem_new_image(cl_context context,
                 cl_mem_flags flags,
                 const cl_image_format *image_format,
                 const cl_image_desc *image_desc,
                 void *host_ptr,
                 cl_int *errcode_ret)
{
  switch (image_desc->image_type) {
  case CL_MEM_OBJECT_IMAGE1D:
  case CL_MEM_OBJECT_IMAGE2D:
  case CL_MEM_OBJECT_IMAGE3D:
    return _cl_mem_new_image(context, flags, image_format, image_desc->image_type,
                             image_desc->image_width, image_desc->image_height, image_desc->image_depth,
                             image_desc->image_row_pitch, image_desc->image_slice_pitch,
                             host_ptr, errcode_ret);
  case CL_MEM_OBJECT_IMAGE2D_ARRAY:
  case CL_MEM_OBJECT_IMAGE1D_ARRAY:
  case CL_MEM_OBJECT_IMAGE1D_BUFFER:
    NOT_IMPLEMENTED;
    break;
  case CL_MEM_OBJECT_BUFFER:
  default:
    assert(0);
  }
  return NULL;
}

LOCAL void
cl_mem_delete(cl_mem mem)
{
  if (UNLIKELY(mem == NULL))
    return;
  if (atomic_dec(&mem->ref_n) > 1)
    return;
  if (LIKELY(mem->bo != NULL))
    cl_buffer_unreference(mem->bo);
#ifdef HAS_EGL
  if (UNLIKELY(IS_IMAGE(mem) && cl_mem_image(mem)->egl_image != NULL)) {
     cl_mem_gl_delete(cl_mem_image(mem));
  }
#endif

  /* Remove it from the list */
  assert(mem->ctx);
  pthread_mutex_lock(&mem->ctx->buffer_lock);
    if (mem->prev)
      mem->prev->next = mem->next;
    if (mem->next)
      mem->next->prev = mem->prev;
    if (mem->prev == NULL && mem->next == NULL)
      mem->ctx->buffers = NULL;
  pthread_mutex_unlock(&mem->ctx->buffer_lock);
  cl_context_delete(mem->ctx);

  /* Someone still mapped? */
  assert(!mem->map_ref);

  if (mem->mapped_ptr)
    free(mem->mapped_ptr);

  if (mem->dstr_cb) {
    cl_mem_dstr_cb *cb = mem->dstr_cb;
    while (mem->dstr_cb) {
      cb = mem->dstr_cb;
      cb->pfn_notify(mem, cb->user_data);
      mem->dstr_cb = cb->next;
      free(cb);
    }
  }

  cl_free(mem);
}

LOCAL void
cl_mem_add_ref(cl_mem mem)
{
  assert(mem);
  atomic_inc(&mem->ref_n);
}

LOCAL void*
cl_mem_map(cl_mem mem)
{
  cl_buffer_map(mem->bo, 1);
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
  return cl_buffer_get_virtual(mem->bo);
}

LOCAL cl_int
cl_mem_unmap_gtt(cl_mem mem)
{
  cl_buffer_unmap_gtt(mem->bo);
  return CL_SUCCESS;
}

LOCAL void*
cl_mem_map_auto(cl_mem mem)
{
  if (IS_IMAGE(mem) && cl_mem_image(mem)->tiling != CL_NO_TILE)
    return cl_mem_map_gtt(mem);
  else
    return cl_mem_map(mem);
}

LOCAL cl_int
cl_mem_unmap_auto(cl_mem mem)
{
  if (IS_IMAGE(mem) && cl_mem_image(mem)->tiling != CL_NO_TILE)
    cl_buffer_unmap_gtt(mem->bo);
  else
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

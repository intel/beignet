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

#include "CL/cl.h"
#include "CL/cl_intel.h"
#include <assert.h>
#include <stdio.h>

static cl_mem
cl_mem_allocate(cl_context ctx,
                cl_mem_flags flags,
                size_t sz,
                cl_int is_tiled,
                cl_int *errcode)
{
  cl_buffer_mgr bufmgr = NULL;
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  size_t alignment = 64;

  assert(ctx);
  FATAL_IF (flags & CL_MEM_ALLOC_HOST_PTR,
            "CL_MEM_ALLOC_HOST_PTR unsupported"); /* XXX */
  FATAL_IF (flags & CL_MEM_USE_HOST_PTR,
            "CL_MEM_USE_HOST_PTR unsupported");   /* XXX */
  if (UNLIKELY(sz == 0)) {
    err = CL_INVALID_BUFFER_SIZE;
    goto error;
  }

  /* Allocate and inialize the structure itself */
  TRY_ALLOC (mem, CALLOC(struct _cl_mem));
  mem->ref_n = 1;
  mem->magic = CL_MAGIC_MEM_HEADER;
  mem->flags = flags;

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

  /* Append the buffer in the context buffer list */
  pthread_mutex_lock(&ctx->buffer_lock);
    mem->next = ctx->buffers;
    if (ctx->buffers != NULL)
      ctx->buffers->prev = mem;
    ctx->buffers = mem;
  pthread_mutex_unlock(&ctx->buffer_lock);
  mem->ctx = ctx;
  cl_context_add_ref(ctx);

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
cl_mem_new(cl_context ctx,
           cl_mem_flags flags,
           size_t sz,
           void *data,
           cl_int *errcode_ret)
{
  cl_int err = CL_SUCCESS;
  cl_mem mem = NULL;

  /* Check flags consistency */
  if (UNLIKELY(flags & CL_MEM_COPY_HOST_PTR && data == NULL)) {
    err = CL_INVALID_HOST_PTR;
    goto error;
  }

  /* Create the buffer in video memory */
  mem = cl_mem_allocate(ctx, flags, sz, CL_FALSE, &err);
  if (mem == NULL || err != CL_SUCCESS)
    goto error;

  /* Copy the data if required */
  if (flags & CL_MEM_COPY_HOST_PTR) /* TODO check other flags too */
    cl_buffer_subdata(mem->bo, 0, sz, data);

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
cl_mem_copy_data_linear(cl_mem mem,
                        size_t w,
                        size_t h,
                        size_t pitch,
                        uint32_t bpp,
                        void *data)
{
  size_t x, y, p;
  char *dst;
  cl_buffer_map(mem->bo, 1);
  dst = cl_buffer_get_virtual(mem->bo);
  for (y = 0; y < h; ++y) {
    char *src = (char*) data + pitch * y;
    for (x = 0; x < w; ++x) {
      for (p = 0; p < bpp; ++p)
        dst[p] = src[p];
      dst += bpp;
      src += bpp;
    }
  }
  cl_buffer_unmap(mem->bo);
}

static const uint32_t tile_sz = 4096; /* 4KB per tile */
static const uint32_t tilex_w = 512;  /* tileX width in bytes */
static const uint32_t tilex_h = 8;    /* tileX height in number of rows */
static const uint32_t tiley_w = 128;  /* tileY width in bytes */
static const uint32_t tiley_h = 32;   /* tileY height in number of rows */

static void
cl_mem_copy_data_tilex(cl_mem mem,
                       size_t w,
                       size_t h,
                       size_t pitch,
                       uint32_t bpp,
                       void *data)
{
  const size_t tile_w = tilex_w;
  const size_t tile_h = tilex_h;
  const size_t aligned_pitch  = ALIGN(w * bpp, tile_w);
  const size_t aligned_height = ALIGN(h, tile_h);
  const size_t tilex_n = aligned_pitch  / tile_w;
  const size_t tiley_n = aligned_height / tile_h;
  size_t x, y, tilex, tiley;
  char *img = NULL;
  char *end = (char*) data + pitch * h;

  cl_buffer_map(mem->bo, 1);
  img = cl_buffer_get_virtual(mem->bo);
  for (tiley = 0; tiley < tiley_n; ++tiley)
  for (tilex = 0; tilex < tilex_n; ++tilex) {
    char *tile = img + (tilex + tiley * tilex_n) * tile_sz;
    for (y = 0; y < tile_h; ++y) {
      char *src = (char*) data + (tiley*tile_h+y)*pitch + tilex*tile_w;
      char *dst = tile + y*tile_w;
      for (x = 0; x < tile_w; ++x, ++dst, ++src) {
        if ((uintptr_t) src < (uintptr_t) end)
          *dst = *src;
      }
    }
  }
  cl_buffer_unmap(mem->bo);
}

static void
cl_mem_copy_data_tiley(cl_mem mem,
                       size_t w,
                       size_t h,
                       size_t pitch,
                       uint32_t bpp,
                       void *data)
{
  const size_t tile_w = tiley_w;
  const size_t tile_h = tiley_h;
  const size_t aligned_pitch  = ALIGN(w * bpp, tile_w);
  const size_t aligned_height = ALIGN(h, tile_h);
  const size_t tilex_n = aligned_pitch  / tile_w;
  const size_t tiley_n = aligned_height / tile_h;
  size_t x, y, tilex, tiley, byte;
  char *img = NULL;
  char *end = (char*) data + pitch * h;

  cl_buffer_map(mem->bo, 1);
  img = cl_buffer_get_virtual(mem->bo);
  for (tiley = 0; tiley < tiley_n; ++tiley)
  for (tilex = 0; tilex < tilex_n; ++tilex) {
    char *tile = img + (tiley * tilex_n + tilex) * tile_sz;
    for (x = 0; x < tile_w; x += 16) {
      char *src = (char*) data + tiley*tile_h*pitch + tilex*tile_w+x;
      char *dst = tile + x*tile_h;
      for (y = 0; y < tile_h; ++y, dst += 16, src += pitch) {
        for (byte = 0; byte < 16; ++byte)
          if ((uintptr_t) src  + byte < (uintptr_t) end)
            dst[byte] = src[byte];
      }
    }
  }
  cl_buffer_unmap(mem->bo);
}

LOCAL cl_mem
cl_mem_new_image2D(cl_context ctx,
                   cl_mem_flags flags,
                   const cl_image_format *fmt,
                   size_t w,
                   size_t h,
                   size_t pitch,
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
  if (UNLIKELY(w > ctx->device->image2d_max_width)) DO_IMAGE_ERROR;
  if (UNLIKELY(h > ctx->device->image2d_max_height)) DO_IMAGE_ERROR;
  if (UNLIKELY(bpp*w > pitch)) DO_IMAGE_ERROR;
#undef DO_IMAGE_ERROR

  /* Pick up tiling mode (we do only linear on SNB) */
  if (cl_driver_get_ver(ctx->drv) != 6)
    tiling = CL_TILE_Y;

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

  sz = aligned_pitch * aligned_h;
  mem = cl_mem_allocate(ctx, flags, sz, tiling != CL_NO_TILE, &err);
  if (mem == NULL || err != CL_SUCCESS)
    goto error;

  /* Copy the data if required */
  if (flags & CL_MEM_COPY_HOST_PTR) {
    if (tiling == CL_NO_TILE)
      cl_mem_copy_data_linear(mem, w, h, pitch, bpp, data);
    else if (tiling == CL_TILE_X)
      cl_mem_copy_data_tilex(mem, w, h, pitch, bpp, data);
    else if (tiling == CL_TILE_Y)
      cl_mem_copy_data_tiley(mem, w, h, pitch, bpp, data);
  }

  mem->w = w;
  mem->h = h;
  mem->fmt = *fmt;
  mem->intel_fmt = intel_fmt;
  mem->bpp = bpp;
  mem->is_image = 1;
  mem->pitch = aligned_pitch;
  mem->tiling = tiling;

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
error:
  cl_mem_delete(mem);
  mem = NULL;
  goto exit;
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


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
#ifndef __CL_ENQUEUE_H__
#define __CL_ENQUEUE_H__

#include "cl_mem.h"
#include "cl_command_queue.h"
#include "cl_internals.h"
#include "CL/cl.h"

typedef enum {
  EnqueueReadBuffer = 0,
  EnqueueReadBufferRect,
  EnqueueWriteBuffer,
  EnqueueWriteBufferRect,
  EnqueueCopyBuffer,
  EnqueueCopyBufferRect,
  EnqueueReadImage,
  EnqueueWriteImage,
  EnqueueCopyImage,
  EnqueueCopyImageToBuffer,
  EnqueueCopyBufferToImage,
  EnqueueMapBuffer,
  EnqueueMapImage,
  EnqueueUnmapMemObject,
  EnqueueNDRangeKernel,
  EnqueueInvalid
} enqueue_type;

typedef struct _enqueue_data {
  enqueue_type      type;          /* Command type */
  cl_mem            mem_obj;       /* Enqueue's cl_mem */
  cl_command_queue  queue;         /* Command queue */
  size_t            offset;        /* Mem object's offset */
  size_t            size;          /* Size */
  size_t            origin[3];     /* Origin */
  size_t            region[3];     /* Region */
  size_t            row_pitch;     /* Row pitch */
  size_t            slice_pitch;   /* Slice pitch */
  cl_map_flags      map_flags;     /* Map flags */
  const void *      const_ptr;     /* Const ptr for memory read */
  void *            ptr;           /* ptr for write and return value */
} enqueue_data;

/* Do real enqueue commands */
cl_int cl_enqueue_handle(enqueue_data* data);
#endif /* __CL_ENQUEUE_H__ */

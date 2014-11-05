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
#ifndef __CL_ENQUEUE_H__
#define __CL_ENQUEUE_H__

#include "cl_internals.h"
#include "cl_driver.h"
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
  EnqueueNativeKernel,
  EnqueueMarker,
  EnqueueBarrier,
  EnqueueFillBuffer,
  EnqueueFillImage,
  EnqueueMigrateMemObj,
  EnqueueInvalid
} enqueue_type;

typedef struct _enqueue_data {
  enqueue_type      type;             /* Command type */
  cl_mem            mem_obj;          /* Enqueue's cl_mem */
  cl_command_queue  queue;            /* Command queue */
  size_t            offset;           /* Mem object's offset */
  size_t            size;             /* Size */
  size_t            origin[3];        /* Origin */
  size_t            host_origin[3];   /* Origin */
  size_t            region[3];        /* Region */
  size_t            row_pitch;        /* Row pitch */
  size_t            slice_pitch;      /* Slice pitch */
  size_t            host_row_pitch;   /* Host row pitch, used in read/write buffer rect */
  size_t            host_slice_pitch; /* Host slice pitch, used in read/write buffer rect */
  const void *      const_ptr;        /* Const ptr for memory read */
  void *            ptr;              /* Ptr for write and return value */
  const cl_mem*     mem_list;         /* mem_list of clEnqueueNativeKernel */
  uint8_t           unsync_map;       /* Indicate the clEnqueueMapBuffer/Image is unsync map */
  uint8_t           write_map;        /* Indicate if the clEnqueueMapBuffer is write enable */
  void (*user_func)(void *);          /* pointer to a host-callable user function */
} enqueue_data;

/* Do real enqueue commands */
cl_int cl_enqueue_handle(cl_event event, enqueue_data* data);
#endif /* __CL_ENQUEUE_H__ */

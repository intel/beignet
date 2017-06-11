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

#include "CL/cl.h"

typedef enum {
  EnqueueReturnSuccesss = 0, /* For some case, we have nothing to do, just return SUCCESS. */
  EnqueueReadBuffer,
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
  EnqueueSVMMemFree,
  EnqueueSVMMemMap,
  EnqueueSVMMemUnMap,
  EnqueueSVMMemCopy,
  EnqueueSVMMemFill,
  EnqueueInvalid
} cl_event_enqueue_type;

typedef struct _cl_event_enqueue_data {
  cl_event_enqueue_type type; /* Command type */
  void *exec_ctx;

  union {
    struct {
      cl_mem *mem_list; /* mem_list of clEnqueueNativeKernel */
      cl_uint mem_num;
      void *args;
      void **mem_arg_loc;
      size_t cb_args;
      void (*user_func)(void *);
    } native_kernel;
    struct {
      cl_mem mem_obj;
      size_t offset;      /* Mem object's offset */
      size_t size;        /* Size */
      cl_bool unsync_map; /* Indicate the clEnqueueMapBuffer/Image is unsync map */
      cl_bool write_map;  /* Indicate if the clEnqueueMapBuffer is write enable */
      void *ptr;          /* Ptr for write and return value */
    } map_buffer;
    struct {
      cl_mem mem_obj;
      size_t origin[3];   /* Origin */
      size_t region[3];   /* Region */
      size_t row_pitch;   /* Row pitch */
      size_t slice_pitch; /* Slice pitch */
      cl_bool unsync_map; /* Indicate the clEnqueueMapBuffer/Image is unsync map */
      cl_bool write_map;  /* Indicate if the clEnqueueMapBuffer is write enable */
      void *ptr;          /* Ptr for write and return value */
    } map_image;
    struct {
      cl_mem mem_obj;
      void *ptr;
      size_t offset;
      size_t size;
      size_t origin[3];   /* Origin */
      size_t region[3];   /* Region */
      size_t row_pitch;   /* Row pitch */
      size_t slice_pitch; /* Slice pitch */
    } unmap;
    struct {
      cl_kernel kernel;
      cl_int work_dim;
      size_t global_wk_off[3];
      size_t global_wk_sz[3];
      size_t local_wk_sz[3];
    } nd_range;
    struct {
      cl_mem src;
      cl_mem dst;
      size_t src_offset;
      size_t dst_offset;
      size_t cb;
    } copy_buffer;
    struct {
      void *pattern;
      size_t pattern_size;
      cl_mem buffer;
      size_t offset;
      size_t size;
    } fill_buffer;
    struct {
      cl_mem src_buf;
      cl_mem dst_buf;
      size_t src_origin[3];
      size_t dst_origin[3];
      size_t region[3];
      size_t src_row_pitch;
      size_t src_slice_pitch;
      size_t dst_row_pitch;
      size_t dst_slice_pitch;
    } copy_buffer_rect;
    struct {
      void *pattern;
      cl_mem image;
      size_t origin[3];
      size_t region[3];
    } fill_image;
    struct {
      cl_mem src_image;
      cl_mem dst_image;
      size_t src_origin[3];
      size_t dst_origin[3];
      size_t region[3];
    } copy_image;
    struct {
      cl_mem buffer;
      size_t offset;
      cl_mem image;
      size_t origin[3];
      size_t region[3];
    } copy_image_and_buffer;
    struct {
      cl_mem buffer;
      size_t offset;
      size_t size;
      void *ptr;
    } read_write_buffer;
    struct {
      cl_mem buffer;
      void *ptr;
      size_t origin[3];
      size_t host_origin[3];
      size_t region[3];
      size_t row_pitch;
      size_t slice_pitch;
      size_t host_row_pitch;
      size_t host_slice_pitch;
    } read_write_buffer_rect;
    struct {
      void *ptr;
      cl_mem image;
      size_t region[3];
      size_t origin[3];
      size_t row_pitch;
      size_t slice_pitch;
    } read_write_image;
    struct {
      void **ptrs;
      cl_mem *mem_ptrs;
      cl_uint mem_num;
      cl_command_queue queue;
      void (*free_func)(cl_command_queue, cl_uint, void **, void *);
      void *user_data;
    } svm_free;
    struct {
      void *ptr;
      cl_mem svm;
      size_t size;
      cl_bool write_map;
      cl_bool unsync_map;
    } svm_map;
    struct {
      void *ptr;
      cl_mem svm;
    } svm_unmap;
    struct {
      void *src_ptr;
      void *dst_ptr;
      cl_mem src;
      cl_mem dst;
      size_t size;
    } svm_copy;
    struct {
      void *ptr;
      cl_mem svm;
      void *pattern;
      size_t pattern_size;
      size_t size;
    } svm_fill;
  };
} _cl_event_enqueue_data;
typedef _cl_event_enqueue_data *cl_event_enqueue_data;

/* Do real enqueue commands */
extern cl_int cl_enqueue_handle(cl_event e, cl_int status);
extern void cl_enqueue_delete_native_kernel(cl_event e);

#endif /* __CL_ENQUEUE_H__ */

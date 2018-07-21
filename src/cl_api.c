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

#include "cl_platform_id.h"
#include "cl_device_id.h"
#include "cl_context.h"
#include "cl_command_queue.h"
#include "cl_enqueue.h"
#include "cl_event.h"
#include "cl_program.h"
#include "cl_kernel.h"
#include "cl_mem.h"
#include "cl_image.h"
#include "cl_sampler.h"
#include "cl_accelerator_intel.h"
#include "cl_alloc.h"
#include "cl_utils.h"
#include "cl_cmrt.h"

#include "CL/cl.h"
#include "CL/cl_ext.h"
#include "CL/cl_intel.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "performance.h"

#ifndef CL_VERSION_1_2
#define CL_MAP_WRITE_INVALIDATE_REGION              (1 << 2)
#define CL_DEVICE_TYPE_CUSTOM                       (1 << 4)
#define CL_MEM_HOST_WRITE_ONLY                      (1 << 7)
#define CL_MEM_HOST_READ_ONLY                       (1 << 8)
#define CL_MEM_HOST_NO_ACCESS                       (1 << 9)
typedef intptr_t cl_device_partition_property;
#endif

#define FILL_GETINFO_RET(TYPE, ELT, VAL, RET) \
	do { \
	  if (param_value && param_value_size < sizeof(TYPE)*ELT) \
	      return CL_INVALID_VALUE;  \
	  if (param_value) { \
	      memset(param_value, 0, param_value_size); \
	      memcpy(param_value, (VAL), sizeof(TYPE)*ELT); \
	  } \
          \
	  if (param_value_size_ret) \
	      *param_value_size_ret = sizeof(TYPE)*ELT; \
	  return RET; \
	} while(0)

cl_int
clGetPlatformIDs(cl_uint          num_entries,
                 cl_platform_id * platforms,
                 cl_uint *        num_platforms)
{
  if(UNLIKELY(platforms == NULL && num_platforms == NULL))
    return CL_INVALID_VALUE;
  if(UNLIKELY(num_entries == 0 && platforms != NULL))
    return CL_INVALID_VALUE;

  return cl_get_platform_ids(num_entries, platforms, num_platforms);
}

cl_mem
clCreateBuffer(cl_context    context,
               cl_mem_flags  flags,
               size_t        size,
               void *        host_ptr,
               cl_int *      errcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);

  mem = cl_mem_new_buffer(context, flags, size, host_ptr, &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
}

cl_mem
clCreateSubBuffer(cl_mem                buffer,
                  cl_mem_flags          flags,
                  cl_buffer_create_type buffer_create_type,
                  const void *          buffer_create_info,
                  cl_int *              errcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;

  CHECK_MEM(buffer);

  mem = cl_mem_new_sub_buffer(buffer, flags, buffer_create_type,
                       buffer_create_info, &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
}

cl_mem
clCreateImage(cl_context context,
              cl_mem_flags flags,
              const cl_image_format *image_format,
              const cl_image_desc *image_desc,
              void *host_ptr,
              cl_int * errcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);
  if (image_format == NULL) {
    err = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    goto error;
  }
  if (image_format->image_channel_order < CL_R ||
          (image_format->image_channel_order > CL_sBGRA && image_format->image_channel_order != CL_NV12_INTEL)) {
    err = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    goto error;
  }
  if (image_format->image_channel_data_type < CL_SNORM_INT8 ||
          image_format->image_channel_data_type > CL_FLOAT) {
    err = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    goto error;
  }

  if (image_desc == NULL) {
    err = CL_INVALID_IMAGE_DESCRIPTOR;
    goto error;
  }
  if (image_desc->image_type <= CL_MEM_OBJECT_BUFFER ||
          image_desc->image_type > CL_MEM_OBJECT_IMAGE1D_BUFFER) {
    err = CL_INVALID_IMAGE_DESCRIPTOR;
    goto error;
  }
  /* buffer refers to a valid buffer memory object if image_type is
     CL_MEM_OBJECT_IMAGE1D_BUFFER or CL_MEM_OBJECT_IMAGE2D. Otherwise it must be NULL. */
  if (image_desc->image_type != CL_MEM_OBJECT_IMAGE1D_BUFFER &&
      image_desc->image_type != CL_MEM_OBJECT_IMAGE2D &&
         image_desc->buffer) {
    err = CL_INVALID_IMAGE_DESCRIPTOR;
    goto error;
  }
  if (image_desc->num_mip_levels || image_desc->num_samples) {
    err = CL_INVALID_IMAGE_DESCRIPTOR;
    goto error;
  }

  if (image_format->image_channel_order == CL_NV12_INTEL &&
      (image_format->image_channel_data_type != CL_UNORM_INT8 ||
       image_desc->image_width % 4 || image_desc->image_height % 4)) {
    err = CL_INVALID_IMAGE_DESCRIPTOR;
    goto error;
  }

  /* Other details check for image_desc will leave to image create. */
  mem = cl_mem_new_image(context,
                         flags,
                         image_format,
                         image_desc,
                         host_ptr,
                         &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
}

void *
clSVMAlloc (cl_context context,
            cl_svm_mem_flags flags,
            size_t size,
            unsigned int alignment)
{
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);
  (void) err;
  return cl_mem_svm_allocate(context, flags, size, alignment);
error:
  return NULL;
}

void
clSVMFree (cl_context context, void* svm_pointer)
{
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);
  (void) err;
  return cl_mem_svm_delete(context, svm_pointer);
error:
  return;
}

cl_int
clEnqueueSVMFree (cl_command_queue command_queue,
                  cl_uint num_svm_pointers,
                  void *svm_pointers[],
                  void (CL_CALLBACK *pfn_free_func)( cl_command_queue queue,
                                                     cl_uint num_svm_pointers,
                                                     void *svm_pointers[],
                                                     void *user_data),
                  void *user_data,
                  cl_uint num_events_in_wait_list,
                  const cl_event *event_wait_list,
                  cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_int i = 0;
  void** pointers = NULL;
  cl_event e = NULL;
  cl_int e_status;
  enqueue_data *data;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if(num_svm_pointers == 0 || svm_pointers == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }
    for(i=0; i<num_svm_pointers; i++) {
      if(svm_pointers[i] == NULL) {
        err = CL_INVALID_VALUE;
        break;
      }
    }
    if(err != CL_SUCCESS)
        break;

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_SVM_FREE, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    pointers = malloc(num_svm_pointers * sizeof(void *));
    if(UNLIKELY(pointers == NULL)) {
      err = CL_OUT_OF_HOST_MEMORY;
      break;
    }
    memcpy(pointers, svm_pointers, num_svm_pointers * sizeof(void *));

    data = &e->exec_data;
    data->type      = EnqueueSVMFree;
    data->queue     = command_queue;
    data->pointers  = pointers;
    data->free_func = pfn_free_func;
    data->size      = num_svm_pointers;
    data->ptr       = user_data;

    if (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      err = cl_event_exec(e, CL_QUEUED, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
      cl_command_queue_enqueue_event(command_queue, e);
    }
  } while (0);

  if (err == CL_SUCCESS && event) {
    *event = e;
  } else {
    cl_event_delete(e);
  }

  return err;
}

cl_int
clEnqueueSVMMap (cl_command_queue command_queue,
                 cl_bool blocking_map,
                 cl_map_flags map_flags,
                 void *svm_ptr,
                 size_t size,
                 cl_uint num_events_in_wait_list,
                 const cl_event *event_wait_list,
                 cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_mem buffer;

  CHECK_QUEUE(command_queue);
  buffer = cl_context_get_svm_from_ptr(command_queue->ctx, svm_ptr);
  if(buffer == NULL) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  clEnqueueMapBuffer(command_queue, buffer, blocking_map, map_flags, 0, size,
                     num_events_in_wait_list, event_wait_list, event, &err);
  if(event)
    (*event)->event_type = CL_COMMAND_SVM_MAP;
error:
  return err;
}

cl_int
clEnqueueSVMUnmap (cl_command_queue command_queue,
                   void *svm_ptr,
                   cl_uint num_events_in_wait_list,
                   const cl_event *event_wait_list,
                   cl_event *event)
{
  cl_int err = CL_SUCCESS;
  cl_mem buffer;

  CHECK_QUEUE(command_queue);
  buffer = cl_context_get_svm_from_ptr(command_queue->ctx, svm_ptr);
  if(buffer == NULL) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  err = clEnqueueUnmapMemObject(command_queue, buffer, svm_ptr,
                                num_events_in_wait_list, event_wait_list, event);
  if(event)
    (*event)->event_type = CL_COMMAND_SVM_UNMAP;

error:
  return err;
}

cl_int clEnqueueSVMMemcpy (cl_command_queue command_queue,
                           cl_bool blocking_copy,
                           void *dst_ptr,
                           const void *src_ptr,
                           size_t size,
                           cl_uint num_events_in_wait_list,
                           const cl_event *event_wait_list,
                           cl_event *event)
{
  cl_int err = CL_SUCCESS;
  enqueue_data *data;
  cl_int e_status;
  cl_event e = NULL;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if(UNLIKELY(dst_ptr == NULL || src_ptr == NULL || size == 0 )) {
      err = CL_INVALID_VALUE;
      break;
    }

    if(((size_t)src_ptr < (size_t)dst_ptr && ((size_t)src_ptr + size > (size_t)dst_ptr)) ||
       ((size_t)dst_ptr < (size_t)src_ptr && ((size_t)dst_ptr + size > (size_t)src_ptr))) {
      err = CL_MEM_COPY_OVERLAP;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_SVM_MEMCPY, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    if (blocking_copy) {
      err = cl_event_wait_for_event_ready(e);
      if (err != CL_SUCCESS)
        break;

      /* Blocking call API is a sync point of flush. */
      err = cl_command_queue_wait_flush(command_queue);
      if (err != CL_SUCCESS) {
        break;
      }
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    data = &e->exec_data;
    data->type         = EnqueueSVMMemCopy;
    data->queue        = command_queue;
    data->ptr          = dst_ptr;
    data->const_ptr    = src_ptr;
    data->size         = size;

    if (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      err = cl_event_exec(e, CL_QUEUED, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
      cl_command_queue_enqueue_event(command_queue, e);
      if (blocking_copy) {
        cl_event_wait_for_events_list(1, &e);
      }
    }
  } while(0);

  if (err == CL_SUCCESS && event) {
    *event = e;
  } else {
    cl_event_delete(e);
  }

  return err;
}

cl_int clEnqueueSVMMemFill (cl_command_queue command_queue,
                            void *svm_ptr,
                            const void *pattern,
                            size_t pattern_size,
                            size_t size,
                            cl_uint num_events_in_wait_list,
                            const cl_event *event_wait_list,
                            cl_event *event)
{
  cl_int err = CL_SUCCESS;
  enqueue_data *data;
  cl_int e_status;
  cl_event e = NULL;

  do {
    if (!CL_OBJECT_IS_COMMAND_QUEUE(command_queue)) {
      err = CL_INVALID_COMMAND_QUEUE;
      break;
    }

    if(UNLIKELY(svm_ptr == NULL ||
               ((size_t)svm_ptr & (pattern_size - 1)) != 0)) {
      err = CL_INVALID_VALUE;
      break;
    }

    if(UNLIKELY(pattern == NULL ||
               (pattern_size & (pattern_size - 1)) != 0 ||
                pattern_size > 128)) {
      err = CL_INVALID_VALUE;
      break;
    }

    if(UNLIKELY(size == 0 ||
               (size % pattern_size) != 0)) {
      err = CL_INVALID_VALUE;
      break;
    }

    err = cl_event_check_waitlist(num_events_in_wait_list, event_wait_list,
                                  event, command_queue->ctx);
    if (err != CL_SUCCESS) {
      break;
    }

    e = cl_event_create(command_queue->ctx, command_queue, num_events_in_wait_list,
                        event_wait_list, CL_COMMAND_SVM_MEMFILL, &err);
    if (err != CL_SUCCESS) {
      break;
    }

    e_status = cl_event_is_ready(e);
    if (e_status < CL_COMPLETE) {
      err = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
      break;
    }

    data = &e->exec_data;
    data->type         = EnqueueSVMMemFill;
    data->queue        = command_queue;
    data->ptr          = svm_ptr;
    data->const_ptr    = pattern;
    data->pattern_size = pattern_size;
    data->size         = size;

    if (cl_command_queue_allow_bypass_submit(command_queue) && (e_status == CL_COMPLETE)) {
      // Sync mode, no need to queue event.
      err = cl_event_exec(e, CL_COMPLETE, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
    } else {
      err = cl_event_exec(e, CL_QUEUED, CL_FALSE);
      if (err != CL_SUCCESS) {
        break;
      }
      cl_command_queue_enqueue_event(command_queue, e);
    }
  } while(0);

  if (err == CL_SUCCESS && event) {
    *event = e;
  } else {
    cl_event_delete(e);
  }

  return err;
}

cl_mem
clCreateImage2D(cl_context              context,
                cl_mem_flags            flags,
                const cl_image_format * image_format,
                size_t                  image_width,
                size_t                  image_height,
                size_t                  image_row_pitch,
                void *                  host_ptr,
                cl_int *                errcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);
  cl_image_desc image_desc;
  memset(&image_desc, 0, sizeof(image_desc));

  image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  image_desc.image_width = image_width;
  image_desc.image_height = image_height;
  image_desc.image_row_pitch = image_row_pitch;

  mem = cl_mem_new_image(context,
                         flags,
                         image_format,
                         &image_desc,
                         host_ptr,
                         &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
}

cl_mem
clCreateImage3D(cl_context              context,
                cl_mem_flags            flags,
                const cl_image_format * image_format,
                size_t                  image_width,
                size_t                  image_height,
                size_t                  image_depth,
                size_t                  image_row_pitch,
                size_t                  image_slice_pitch,
                void *                  host_ptr,
                cl_int *                errcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);
  cl_image_desc image_desc;

  image_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
  image_desc.image_width = image_width;
  image_desc.image_height = image_height;
  image_desc.image_depth = image_depth;
  image_desc.image_row_pitch = image_row_pitch;
  image_desc.image_slice_pitch = image_slice_pitch;

  mem = cl_mem_new_image(context,
                         flags,
                         image_format,
                         &image_desc,
                         host_ptr,
                         &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
}

cl_int
clGetSupportedImageFormats(cl_context         ctx,
                           cl_mem_flags       flags,
                           cl_mem_object_type image_type,
                           cl_uint            num_entries,
                           cl_image_format *  image_formats,
                           cl_uint *          num_image_formats)
{
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (ctx);
  if (UNLIKELY(num_entries == 0 && image_formats != NULL)) {
    err = CL_INVALID_VALUE;
    goto error;
  }
  if (UNLIKELY(image_type != CL_MEM_OBJECT_IMAGE1D &&
               image_type != CL_MEM_OBJECT_IMAGE1D_ARRAY &&
               image_type != CL_MEM_OBJECT_IMAGE1D_BUFFER &&
               image_type != CL_MEM_OBJECT_IMAGE2D_ARRAY &&
               image_type != CL_MEM_OBJECT_IMAGE2D &&
               image_type != CL_MEM_OBJECT_IMAGE3D)) {
    err = CL_INVALID_VALUE;
    goto error;
  }
  err = cl_image_get_supported_fmt(ctx,
                                   flags,
                                   image_type,
                                   num_entries,
                                   image_formats,
                                   num_image_formats);

error:
  return err;
}

cl_sampler
clCreateSamplerWithProperties(cl_context                  context,
                              const cl_sampler_properties *sampler_properties,
                              cl_int *                    errcode_ret)
{
  cl_sampler sampler = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);
  cl_bool normalized = 0xFFFFFFFF;
  cl_addressing_mode addressing = 0xFFFFFFFF;
  cl_filter_mode filter = 0xFFFFFFFF;
  if(sampler_properties)
  {
    cl_ulong sam_type;
    cl_ulong sam_val;
    cl_uint i;
    for(i = 0;(sam_type = sampler_properties[i++])!=0;i++)
    {
      sam_val = sampler_properties[i];
      switch(sam_type)
      {
        case CL_SAMPLER_NORMALIZED_COORDS:
          if(normalized != 0xFFFFFFFF)
            err = CL_INVALID_VALUE;
          else if(sam_val == CL_TRUE || sam_val == CL_FALSE)
            normalized = sam_val;
          else
            err = CL_INVALID_VALUE;
          break;
        case CL_SAMPLER_ADDRESSING_MODE:
          if(addressing != 0xFFFFFFFF)
            err = CL_INVALID_VALUE;
          else if(sam_val == CL_ADDRESS_MIRRORED_REPEAT || sam_val == CL_ADDRESS_REPEAT ||
                  sam_val == CL_ADDRESS_CLAMP_TO_EDGE || sam_val == CL_ADDRESS_CLAMP ||
                  sam_val == CL_ADDRESS_NONE)
            addressing = sam_val;
          else
            err = CL_INVALID_VALUE;
          break;
        case CL_SAMPLER_FILTER_MODE:
          if(filter != 0xFFFFFFFF)
            err = CL_INVALID_VALUE;
          else if(sam_val == CL_FILTER_LINEAR || sam_val == CL_FILTER_NEAREST)
            filter = sam_val;
          else
            err = CL_INVALID_VALUE;
          break;
        default:
          err = CL_INVALID_VALUE;
          break;
      }
    }
  }
  if(err)
    goto error;
  if(normalized == 0xFFFFFFFF) normalized = CL_TRUE;
  if(addressing == 0xFFFFFFFF) addressing = CL_ADDRESS_CLAMP;
  if(filter == 0xFFFFFFFF) filter = CL_FILTER_NEAREST;
  sampler = cl_create_sampler(context, normalized, addressing, filter, &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return sampler;
}

cl_program
clCreateProgramWithSource(cl_context     context,
                          cl_uint        count,
                          const char **  strings,
                          const size_t * lengths,
                          cl_int *       errcode_ret)
{
  cl_program program = NULL;
  cl_int err = CL_SUCCESS;
  cl_uint i;

  CHECK_CONTEXT (context);
  INVALID_VALUE_IF (count == 0);
  INVALID_VALUE_IF (strings == NULL);
  for(i = 0; i < count; i++) {
    if(UNLIKELY(strings[i] == NULL)) {
      err = CL_INVALID_VALUE;
      goto error;
    }
  }
  program = cl_program_create_from_source(context,
                                          count,
                                          strings,
                                          lengths,
                                          &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return program;
}

cl_program
clCreateProgramWithBinary(cl_context             context,
                          cl_uint                num_devices,
                          const cl_device_id *   devices,
                          const size_t *         lengths,
                          const unsigned char ** binaries,
                          cl_int *               binary_status,
                          cl_int *               errcode_ret)
{
  cl_program program = NULL;
  cl_int err = CL_SUCCESS;

  CHECK_CONTEXT (context);
  program = cl_program_create_from_binary(context,
                                          num_devices,
                                          devices,
                                          lengths,
                                          binaries,
                                          binary_status,
                                          &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return program;
}

cl_program
clCreateProgramWithBuiltInKernels(cl_context           context,
                                  cl_uint              num_devices,
                                  const cl_device_id * device_list,
                                  const char *         kernel_names,
                                  cl_int *             errcode_ret)
{
  cl_program program = NULL;
  cl_int err = CL_SUCCESS;

  CHECK_CONTEXT (context);
  INVALID_VALUE_IF (kernel_names == NULL);
  program = cl_program_create_with_built_in_kernles(context,
                                                    num_devices,
                                                    device_list,
                                                    kernel_names,
                                                    &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return program;
}

cl_int
clRetainProgram(cl_program program)
{
  cl_int err = CL_SUCCESS;
  CHECK_PROGRAM (program);
  cl_program_add_ref(program);
error:
  return err;
}

cl_int
clReleaseProgram(cl_program program)
{
  cl_int err = CL_SUCCESS;
  CHECK_PROGRAM (program);
  cl_program_delete(program);
error:
  return err;
}

cl_int
clBuildProgram(cl_program            program,
               cl_uint               num_devices,
               const cl_device_id *  device_list,
               const char *          options,
               void (CL_CALLBACK *pfn_notify) (cl_program, void*),
               void *                user_data)
{
  cl_int err = CL_SUCCESS;
  CHECK_PROGRAM(program);
  INVALID_VALUE_IF (num_devices > 1);
  INVALID_VALUE_IF (num_devices == 0 && device_list != NULL);
  INVALID_VALUE_IF (num_devices != 0 && device_list == NULL);
  INVALID_VALUE_IF (pfn_notify  == 0 && user_data   != NULL);

  /* Everything is easy. We only support one device anyway */
  if (num_devices != 0) {
    assert(program->ctx);
    err = cl_devices_list_include_check(program->ctx->device_num,
                                        program->ctx->devices, num_devices, device_list);
    if (err)
      goto error;
  }

  assert(program->source_type == FROM_LLVM ||
         program->source_type == FROM_SOURCE ||
         program->source_type == FROM_LLVM_SPIR ||
         program->source_type == FROM_BINARY ||
         program->source_type == FROM_CMRT);
  if((err = cl_program_build(program, options)) != CL_SUCCESS) {
    goto error;
  }
  program->is_built = CL_TRUE;

  if (pfn_notify) pfn_notify(program, user_data);

error:
  return err;
}

cl_int
clCompileProgram(cl_program            program ,
                 cl_uint               num_devices ,
                 const cl_device_id *  device_list ,
                 const char *          options ,
                 cl_uint               num_input_headers ,
                 const cl_program *    input_headers ,
                 const char **         header_include_names ,
                 void (CL_CALLBACK *   pfn_notify )(cl_program, void *),
                 void *                user_data )
{
  cl_int err = CL_SUCCESS;
  CHECK_PROGRAM(program);
  INVALID_VALUE_IF (num_devices > 1);
  INVALID_VALUE_IF (num_devices == 0 && device_list != NULL);
  INVALID_VALUE_IF (num_devices != 0 && device_list == NULL);
  INVALID_VALUE_IF (pfn_notify  == 0 && user_data   != NULL);
  INVALID_VALUE_IF (num_input_headers == 0 && input_headers != NULL);
  INVALID_VALUE_IF (num_input_headers != 0 && input_headers == NULL);

  /* Everything is easy. We only support one device anyway */
  if (num_devices != 0) {
    assert(program->ctx);
    err = cl_devices_list_include_check(program->ctx->device_num,
                                        program->ctx->devices, num_devices, device_list);
    if (err)
      goto error;
  }

  /* TODO support create program from binary */
  assert(program->source_type == FROM_LLVM ||
      program->source_type == FROM_SOURCE ||
      program->source_type == FROM_LLVM_SPIR ||
      program->source_type == FROM_BINARY);
  if((err = cl_program_compile(program, num_input_headers, input_headers, header_include_names, options)) != CL_SUCCESS) {
    goto error;
  }
  program->is_built = CL_TRUE;

  if (pfn_notify) pfn_notify(program, user_data);

error:
  return err;
}

cl_program
clLinkProgram(cl_context            context,
              cl_uint               num_devices,
              const cl_device_id *  device_list,
              const char *          options,
              cl_uint               num_input_programs,
              const cl_program *    input_programs,
              void (CL_CALLBACK *   pfn_notify)(cl_program  program, void * user_data),
              void *                user_data,
              cl_int *              errcode_ret)
{
  cl_int err = CL_SUCCESS;
  cl_program program = NULL;
  CHECK_CONTEXT (context);
  INVALID_VALUE_IF (num_devices > 1);
  INVALID_VALUE_IF (num_devices == 0 && device_list != NULL);
  INVALID_VALUE_IF (num_devices != 0 && device_list == NULL);
  INVALID_VALUE_IF (pfn_notify  == 0 && user_data   != NULL);
  INVALID_VALUE_IF (num_input_programs == 0 && input_programs != NULL);
  INVALID_VALUE_IF (num_input_programs != 0 && input_programs == NULL);
  INVALID_VALUE_IF (num_input_programs == 0 && input_programs == NULL);

  program = cl_program_link(context, num_input_programs, input_programs, options, &err);

  if(program) program->is_built = CL_TRUE;

  if (pfn_notify) pfn_notify(program, user_data);

error:
  if (errcode_ret)
    *errcode_ret = err;
  return program;
}

cl_int
clUnloadCompiler(void)
{
  return CL_SUCCESS;
}

cl_int
clUnloadPlatformCompiler(cl_platform_id platform)
{
  return CL_SUCCESS;
}

cl_kernel
clCreateKernel(cl_program   program,
               const char * kernel_name,
               cl_int *     errcode_ret)
{
  cl_kernel kernel = NULL;
  cl_int err = CL_SUCCESS;

  CHECK_PROGRAM (program);
  if (program->ker_n <= 0) {
    err = CL_INVALID_PROGRAM_EXECUTABLE;
    goto error;
  }
  INVALID_VALUE_IF (kernel_name == NULL);
  kernel = cl_program_create_kernel(program, kernel_name, &err);

error:
  if (errcode_ret)
    *errcode_ret = err;
  return kernel;
}

cl_int
clCreateKernelsInProgram(cl_program      program,
                         cl_uint         num_kernels,
                         cl_kernel *     kernels,
                         cl_uint *       num_kernels_ret)
{
  cl_int err = CL_SUCCESS;

  CHECK_PROGRAM (program);
  if (program->ker_n <= 0) {
    err = CL_INVALID_PROGRAM_EXECUTABLE;
    goto error;
  }
  if (kernels && num_kernels < program->ker_n) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if(num_kernels_ret)
    *num_kernels_ret = program->ker_n;

  if(kernels)
    err = cl_program_create_kernels_in_program(program, kernels);

error:
  return err;
}

cl_int
clRetainKernel(cl_kernel kernel)
{
  cl_int err = CL_SUCCESS;
  CHECK_KERNEL(kernel);
  cl_kernel_add_ref(kernel);
error:
  return err;
}

cl_int
clReleaseKernel(cl_kernel kernel)
{
  cl_int err = CL_SUCCESS;
  CHECK_KERNEL(kernel);
  cl_kernel_delete(kernel);
error:
  return err;
}

cl_int
clSetKernelArg(cl_kernel     kernel,
               cl_uint       arg_index,
               size_t        arg_size,
               const void *  arg_value)
{
  cl_int err = CL_SUCCESS;
  CHECK_KERNEL(kernel);

#ifdef HAS_CMRT
  if (kernel->cmrt_kernel != NULL)
    err = cmrt_set_kernel_arg(kernel, arg_index, arg_size, arg_value);
  else
#endif
    err = cl_kernel_set_arg(kernel, arg_index, arg_size, arg_value);
error:
  return err;
}

cl_int
clSetKernelArgSVMPointer(cl_kernel kernel,
                          cl_uint arg_index,
                          const void *arg_value)
{
  cl_int err = CL_SUCCESS;
  CHECK_KERNEL(kernel);

  err = cl_kernel_set_arg_svm_pointer(kernel, arg_index, arg_value);
error:
  return err;
}
cl_int
clSetKernelExecInfo(cl_kernel kernel,
                     cl_kernel_exec_info  param_name,
                     size_t  param_value_size,
                     const void  *param_value)
{

  cl_int err = CL_SUCCESS;
  CHECK_KERNEL(kernel);

  if((param_name != CL_KERNEL_EXEC_INFO_SVM_PTRS &&
     param_name != CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM) ||
     param_value == NULL || param_value_size == 0) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if(param_name == CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM &&
     *(cl_bool *)param_value == CL_TRUE) {
    err = CL_INVALID_OPERATION;
    goto error;
  }

  err = cl_kernel_set_exec_info(kernel, param_value_size, param_value);
error:
  return err;
}

cl_int clGetKernelArgInfo(cl_kernel kernel, cl_uint arg_index, cl_kernel_arg_info param_name,
        size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
  cl_int err = CL_SUCCESS;
  CHECK_KERNEL(kernel);

  if(kernel->program->build_opts == NULL ||
        strstr(kernel->program->build_opts,"-cl-kernel-arg-info") == NULL ) {
    err = CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
    goto error;
  }
  if (param_name != CL_KERNEL_ARG_ADDRESS_QUALIFIER
          && param_name != CL_KERNEL_ARG_ACCESS_QUALIFIER
          && param_name != CL_KERNEL_ARG_TYPE_NAME
          && param_name != CL_KERNEL_ARG_TYPE_QUALIFIER
          && param_name != CL_KERNEL_ARG_NAME) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if (arg_index >= kernel->arg_n) {
    err = CL_INVALID_ARG_INDEX;
    goto error;
  }

  err = cl_get_kernel_arg_info(kernel, arg_index, param_name, param_value_size,
          param_value, param_value_size_ret);

error:
  return err;
}

cl_int
clGetKernelWorkGroupInfo(cl_kernel                   kernel,
                         cl_device_id                device,
                         cl_kernel_work_group_info   param_name,
                         size_t                      param_value_size,
                         void *                      param_value,
                         size_t *                    param_value_size_ret)
{
  return cl_get_kernel_workgroup_info(kernel,
                                      device,
                                      param_name,
                                      param_value_size,
                                      param_value,
                                      param_value_size_ret);
}

cl_int
clGetKernelSubGroupInfoKHR(cl_kernel                   kernel,
                          cl_device_id                device,
                          cl_kernel_work_group_info   param_name,
                          size_t                      input_value_size,
                          const void *                input_value,
                          size_t                      param_value_size,
                          void *                      param_value,
                          size_t *                    param_value_size_ret)
{
  return cl_get_kernel_subgroup_info(kernel,
                                     device,
                                     param_name,
                                     input_value_size,
                                     input_value,
                                     param_value_size,
                                     param_value,
                                     param_value_size_ret);
}

cl_int
clRetainEvent(cl_event  event)
{
  cl_int err = CL_SUCCESS;

  CHECK_EVENT(event);
  cl_event_add_ref(event);

error:
  return err;
}

cl_int
clReleaseEvent(cl_event  event)
{
  cl_int err = CL_SUCCESS;

  CHECK_EVENT(event);
  cl_event_delete(event);

error:
  return err;
}

cl_mem clCreatePipe (cl_context context,
                     cl_mem_flags flags,
                     cl_uint pipe_packet_size,
                     cl_uint pipe_max_packets,
                     const cl_pipe_properties *properties,
                     cl_int *errcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  cl_uint device_max_size = 0;

  CHECK_CONTEXT (context);

  if(UNLIKELY((flags & ~(CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS)) != 0)) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if(UNLIKELY(properties != NULL)) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if(UNLIKELY(pipe_packet_size == 0 || pipe_max_packets == 0)) {
    err = CL_INVALID_PIPE_SIZE;
    goto error;
  }
  if ((err = cl_get_device_info(context->devices[0],
                                CL_DEVICE_PIPE_MAX_PACKET_SIZE,
                                sizeof(device_max_size),
                                &device_max_size,
                                NULL)) != CL_SUCCESS) {
    goto error;
  }

  if(UNLIKELY(pipe_packet_size > device_max_size)) {
    err = CL_INVALID_PIPE_SIZE;
    goto error;
  }

  if(flags == 0)
    flags = CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS;

  mem = cl_mem_new_pipe(context, flags, pipe_packet_size, pipe_max_packets, &err);

error:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
}

cl_int clGetPipeInfo (cl_mem pipe,
                      cl_pipe_info param_name,
                      size_t param_value_size,
                      void *param_value,
                      size_t *param_value_size_ret)
{
  cl_int err = CL_SUCCESS;
  CHECK_MEM(pipe);

  err = cl_get_pipe_info(pipe,
                         param_name,
                         param_value_size,
                         param_value,
                         param_value_size_ret);

error:
  return err;
}

#define EXTFUNC(x)                      \
  if (strcmp(#x, func_name) == 0)       \
    return (void *)x;

static void*
internal_clGetExtensionFunctionAddress(const char *func_name)
{
  if (func_name == NULL)
    return NULL;
#ifdef HAS_OCLIcd
  /* cl_khr_icd */
  EXTFUNC(clIcdGetPlatformIDsKHR)
#endif
  EXTFUNC(clCreateProgramWithLLVMIntel)
  EXTFUNC(clGetGenVersionIntel)
  EXTFUNC(clMapBufferIntel)
  EXTFUNC(clUnmapBufferIntel)
  EXTFUNC(clMapBufferGTTIntel)
  EXTFUNC(clUnmapBufferGTTIntel)
  EXTFUNC(clPinBufferIntel)
  EXTFUNC(clUnpinBufferIntel)
  EXTFUNC(clReportUnfreedIntel)
  EXTFUNC(clCreateBufferFromLibvaIntel)
  EXTFUNC(clCreateImageFromLibvaIntel)
  EXTFUNC(clGetMemObjectFdIntel)
  EXTFUNC(clCreateBufferFromFdINTEL)
  EXTFUNC(clCreateImageFromFdINTEL)
  EXTFUNC(clCreateAcceleratorINTEL)
  EXTFUNC(clRetainAcceleratorINTEL)
  EXTFUNC(clReleaseAcceleratorINTEL)
  EXTFUNC(clGetAcceleratorInfoINTEL)
  EXTFUNC(clGetKernelSubGroupInfoKHR)
  return NULL;
}

void*
clGetExtensionFunctionAddress(const char *func_name)
{
  return internal_clGetExtensionFunctionAddress(func_name);
}

void*
clGetExtensionFunctionAddressForPlatform(cl_platform_id platform,
                              const char *func_name)
{
  if (UNLIKELY(platform != NULL && platform != cl_get_platform_default()))
    return NULL;
  return internal_clGetExtensionFunctionAddress(func_name);
}

#undef EXTFUNC

cl_int
clReportUnfreedIntel(void)
{
  return cl_report_unfreed();
}

void*
clMapBufferIntel(cl_mem mem, cl_int *errcode_ret)
{
  void *ptr = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_MEM (mem);
  ptr = cl_mem_map(mem, 1);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return ptr;
}

cl_int
clUnmapBufferIntel(cl_mem mem)
{
  cl_int err = CL_SUCCESS;
  CHECK_MEM (mem);
  err = cl_mem_unmap(mem);
error:
  return err;
}

void*
clMapBufferGTTIntel(cl_mem mem, cl_int *errcode_ret)
{
  void *ptr = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_MEM (mem);
  ptr = cl_mem_map_gtt(mem);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return ptr;
}

cl_int
clUnmapBufferGTTIntel(cl_mem mem)
{
  cl_int err = CL_SUCCESS;
  CHECK_MEM (mem);
  err = cl_mem_unmap_gtt(mem);
error:
  return err;
}

cl_int
clPinBufferIntel(cl_mem mem)
{
  cl_int err = CL_SUCCESS;
  CHECK_MEM (mem);
  cl_mem_pin(mem);
error:
  return err;
}

cl_int
clUnpinBufferIntel(cl_mem mem)
{
  cl_int err = CL_SUCCESS;
  CHECK_MEM (mem);
  cl_mem_unpin(mem);
error:
  return err;
}

cl_int
clGetGenVersionIntel(cl_device_id device, cl_int *ver)
{
  return cl_device_get_version(device, ver);
}

cl_program
clCreateProgramWithLLVMIntel(cl_context              context,
                             cl_uint                 num_devices,
                             const cl_device_id *    devices,
                             const char *            filename,
                             cl_int *                errcode_ret)
{
  return cl_program_create_from_llvm(context,
                                     num_devices,
                                     devices,
                                     filename,
                                     errcode_ret);
}

cl_mem
clCreateBufferFromLibvaIntel(cl_context  context,
                             unsigned int bo_name,
                             cl_int *errorcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);

  mem = cl_mem_new_libva_buffer(context, bo_name, &err);

error:
  if (errorcode_ret)
    *errorcode_ret = err;
  return mem;
}

cl_mem
clCreateImageFromLibvaIntel(cl_context context,
                            const cl_libva_image *info,
                            cl_int *errorcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);

  if (!info) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  mem = cl_mem_new_libva_image(context,
                               info->bo_name, info->offset, info->width, info->height,
                               info->fmt, info->row_pitch,
                               &err);

error:
  if (errorcode_ret)
    *errorcode_ret = err;
  return mem;
}

extern CL_API_ENTRY cl_int CL_API_CALL
clGetMemObjectFdIntel(cl_context context,
                      cl_mem memobj,
                      int* fd)
{
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);
  CHECK_MEM (memobj);

  err = cl_mem_get_fd(memobj, fd);

error:
  return err;
}

cl_mem
clCreateBufferFromFdINTEL(cl_context context,
                          const cl_import_buffer_info_intel* info,
                          cl_int *errorcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);

  if (!info) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  mem = cl_mem_new_buffer_from_fd(context, info->fd, info->size, &err);

error:
  if (errorcode_ret)
    *errorcode_ret = err;
  return mem;
}

cl_mem
clCreateImageFromFdINTEL(cl_context context,
                         const cl_import_image_info_intel* info,
                         cl_int *errorcode_ret)
{
  cl_mem mem = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);

  if (!info) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  /* Create image object from fd.
   * We just support creating CL_MEM_OBJECT_IMAGE2D image object now.
   * Other image type will be supported later if necessary.
   */
  if(info->type == CL_MEM_OBJECT_IMAGE2D){
    mem = cl_mem_new_image_from_fd(context,
                                   info->fd, info->size,
                                   info->offset,
                                   info->width, info->height,
                                   info->fmt, info->row_pitch,
                                   &err);
  }
  else{
    err = CL_INVALID_ARG_VALUE;
    goto error;
  }

error:
  if (errorcode_ret)
    *errorcode_ret = err;
  return mem;
}

cl_accelerator_intel
clCreateAcceleratorINTEL(cl_context context,
                         cl_accelerator_type_intel accel_type,
                         size_t desc_sz,
                         const void* desc,
                         cl_int* errcode_ret)
{

  cl_accelerator_intel accel = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT(context);
  accel = cl_accelerator_intel_new(context, accel_type, desc_sz, desc, &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return accel;
}

cl_int
clRetainAcceleratorINTEL(cl_accelerator_intel accel)
{
  cl_int err = CL_SUCCESS;
  CHECK_ACCELERATOR_INTEL(accel);
  cl_accelerator_intel_add_ref(accel);
error:
  return err;
}

cl_int
clReleaseAcceleratorINTEL(cl_accelerator_intel accel)
{
  cl_int err = CL_SUCCESS;
  CHECK_ACCELERATOR_INTEL(accel);
  cl_accelerator_intel_delete(accel);
error:
  return err;
}

cl_int
clGetAcceleratorInfoINTEL(cl_accelerator_intel           accel,
                            cl_accelerator_info_intel    param_name,
                            size_t                       param_value_size,
                            void*                        param_value,
                            size_t*                      param_value_size_ret)
{
  cl_int err = CL_SUCCESS;
  CHECK_ACCELERATOR_INTEL(accel);

  if (param_name == CL_ACCELERATOR_REFERENCE_COUNT_INTEL) {
    cl_uint ref = CL_OBJECT_GET_REF(accel);
    FILL_GETINFO_RET (cl_uint, 1, &ref, CL_SUCCESS);
  } else if (param_name == CL_ACCELERATOR_CONTEXT_INTEL) {
    FILL_GETINFO_RET (cl_context, 1, &accel->ctx, CL_SUCCESS);
  } else if (param_name == CL_ACCELERATOR_TYPE_INTEL) {
    FILL_GETINFO_RET (cl_uint, 1, &accel->type, CL_SUCCESS);
  } else if (param_name == CL_ACCELERATOR_DESCRIPTOR_INTEL) {
    FILL_GETINFO_RET (cl_motion_estimation_desc_intel, 1, &(accel->desc.me), CL_SUCCESS);
  } else{
    return CL_INVALID_VALUE;
  }

error:
  return err;
}

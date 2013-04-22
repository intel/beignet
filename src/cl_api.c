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

#include "cl_platform_id.h"
#include "cl_device_id.h" 
#include "cl_context.h"
#include "cl_command_queue.h"
#include "cl_program.h"
#include "cl_kernel.h"
#include "cl_mem.h"
#include "cl_image.h"
#include "cl_sampler.h"
#include "cl_alloc.h"
#include "cl_utils.h"

#include "CL/cl.h"
#include "CL/cl_ext.h"
#include "CL/cl_intel.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

cl_int
clGetPlatformIDs(cl_uint          num_entries,
                 cl_platform_id * platforms,
                 cl_uint *        num_platforms)
{
  return cl_get_platform_ids(num_entries, platforms, num_platforms);
}

cl_int
clGetPlatformInfo(cl_platform_id    platform,
                  cl_platform_info  param_name,
                  size_t            param_value_size,
                  void *            param_value,
                  size_t *          param_value_size_ret)
{
  return cl_get_platform_info(platform,
                              param_name,
                              param_value_size,
                              param_value,
                              param_value_size_ret);
}

cl_int
clGetDeviceIDs(cl_platform_id platform,
               cl_device_type device_type,
               cl_uint        num_entries,
               cl_device_id * devices,
               cl_uint *      num_devices)
{
  return cl_get_device_ids(platform,
                           device_type,
                           num_entries,
                           devices,
                           num_devices);
}

cl_int
clGetDeviceInfo(cl_device_id   device,
                cl_device_info param_name,
                size_t         param_value_size,
                void *         param_value,
                size_t *       param_value_size_ret)
{
  return cl_get_device_info(device,
                            param_name,
                            param_value_size,
                            param_value,
                            param_value_size_ret);
}

cl_context
clCreateContext(const cl_context_properties *  properties,
                cl_uint                        num_devices,
                const cl_device_id *           devices,
                void (* pfn_notify) (const char*, const void*, size_t, void*),
                void *                         user_data,
                cl_int *                       errcode_ret)
{
  return cl_create_context(properties,
                           num_devices,
                           devices,
                           pfn_notify,
                           user_data,
                           errcode_ret);
}

cl_context
clCreateContextFromType(const cl_context_properties *  properties,
                        cl_device_type                 device_type,
                        void (CL_CALLBACK *pfn_notify) (const char *, const void *, size_t, void *),
                        void *                         user_data,
                        cl_int *                       errcode_ret)
{
  cl_device_id devices[1];
  cl_uint num_devices = 1;
  cl_int err;

  err = cl_get_device_ids(NULL,
                          device_type,
                          1,
                          &devices[0],
                          &num_devices);
  if (err != CL_SUCCESS) {
    *errcode_ret = err;
    return NULL;
  }

  return cl_create_context(properties,
                           num_devices,
                           devices,
                           pfn_notify,
                           user_data,
                           errcode_ret);
}

cl_int
clRetainContext(cl_context context)
{
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);
  cl_context_add_ref(context);
error:
  return err;
}

cl_int
clReleaseContext(cl_context context)
{
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);
  cl_context_delete(context);
error:
  return err;
}

cl_int
clGetContextInfo(cl_context      context,
                 cl_context_info param_name,
                 size_t          param_value_size,
                 void *          param_value,
                 size_t *        param_value_size_ret)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_command_queue
clCreateCommandQueue(cl_context                   context,
                     cl_device_id                 device,
                     cl_command_queue_properties  properties,
                     cl_int *                     errcode_ret)
{
  cl_command_queue queue = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);
  queue = cl_context_create_queue(context, device, properties, errcode_ret);
error:
  return err == CL_SUCCESS ? queue : NULL;
}

cl_int
clRetainCommandQueue(cl_command_queue command_queue)
{
  cl_int err = CL_SUCCESS;
  CHECK_QUEUE (command_queue);
  cl_command_queue_add_ref(command_queue);
error:
  return err;
}

cl_int
clReleaseCommandQueue(cl_command_queue command_queue)
{
  cl_int err = CL_SUCCESS;
  CHECK_QUEUE (command_queue);
  cl_command_queue_delete(command_queue);
error:
  return err;
}

cl_int
clGetCommandQueueInfo(cl_command_queue       command_queue,
                      cl_command_queue_info  param_name,
                      size_t                 param_value_size,
                      void *                 param_value,
                      size_t *               param_value_size_ret)
{
  cl_int err = CL_SUCCESS;
  CHECK_QUEUE (command_queue);
  NOT_IMPLEMENTED;
error:
  return err;
}

cl_int
clSetCommandQueueProperty(cl_command_queue               command_queue,
                          cl_command_queue_properties    properties,
                          cl_bool                        enable,
                          cl_command_queue_properties *  old_properties)
{
  cl_int err = CL_SUCCESS;
  CHECK_QUEUE (command_queue);
  NOT_IMPLEMENTED;
error:
  return err;
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

  mem = cl_mem_new(context, flags, size, host_ptr, &err);
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
#if 0
  cl_int err = CL_SUCCESS;
  CHECK_MEM (buffer);
  NOT_IMPLEMENTED;
error:
#endif
  return NULL;
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

  mem = cl_mem_new_image(context,
                         flags,
                         image_format,
                         image_desc,
                         host_ptr,
                         errcode_ret);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return mem;
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

  image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  image_desc.image_width = image_width;
  image_desc.image_height = image_height;
  image_desc.image_row_pitch = image_row_pitch;

  mem = cl_mem_new_image(context,
                         flags,
                         image_format,
                         &image_desc,
                         host_ptr,
                         errcode_ret);
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
  NOT_IMPLEMENTED;
  return NULL;
}

cl_int
clRetainMemObject(cl_mem memobj)
{
  cl_int err = CL_SUCCESS;
  CHECK_MEM (memobj);
  cl_mem_add_ref(memobj);
error:
  return err;
}

cl_int
clReleaseMemObject(cl_mem memobj)
{
  cl_int err = CL_SUCCESS;
  CHECK_MEM (memobj);
  cl_mem_delete(memobj);
error:
  return err;
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
  if (UNLIKELY(image_type != CL_MEM_OBJECT_IMAGE2D &&
               image_type != CL_MEM_OBJECT_IMAGE3D)) {
    err = CL_INVALID_VALUE;
    goto error;
  }
  err = cl_image_get_supported_fmt(ctx,
                                   image_type,
                                   num_entries,
                                   image_formats,
                                   num_image_formats);

error:
  return err;
}

cl_int
clGetMemObjectInfo(cl_mem      memobj,
                   cl_mem_info param_name,
                   size_t      param_value_size,
                   void *      param_value,
                   size_t *    param_value_size_ret)
{
  return cl_get_mem_object_info(memobj,
                                param_name,
                                param_value_size,
                                param_value,
                                param_value_size_ret);
}

cl_int
clGetImageInfo(cl_mem         image,
               cl_image_info  param_name,
               size_t         param_value_size,
               void *         param_value,
               size_t *       param_value_size_ret)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clSetMemObjectDestructorCallback(cl_mem  memobj,
                                 void (CL_CALLBACK *pfn_notify) (cl_mem, void*),
                                 void * user_data)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_sampler
clCreateSampler(cl_context         context,
                cl_bool            normalized,
                cl_addressing_mode addressing,
                cl_filter_mode     filter,
                cl_int *           errcode_ret)
{
  cl_sampler sampler = NULL;
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);
  sampler = cl_sampler_new(context, normalized, addressing, filter, &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return sampler;
}

cl_int
clRetainSampler(cl_sampler sampler)
{
  cl_int err = CL_SUCCESS;
  CHECK_SAMPLER (sampler);
  cl_sampler_add_ref(sampler);
error:
  return err;
}

cl_int
clReleaseSampler(cl_sampler sampler)
{
  cl_int err = CL_SUCCESS;
  CHECK_SAMPLER (sampler);
  cl_sampler_delete(sampler);
error:
  return err;
}

cl_int
clGetSamplerInfo(cl_sampler       sampler,
                 cl_sampler_info  param_name,
                 size_t           param_value_size,
                 void *           param_value,
                 size_t *         param_value_size_ret)
{
  NOT_IMPLEMENTED;
  return 0;
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

  CHECK_CONTEXT (context);
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

  /* Everything is easy. We only support one device anyway */
  if (num_devices != 0) {
    assert(program->ctx);
    if (UNLIKELY(device_list[0] != program->ctx->device)) {
      err = CL_INVALID_DEVICE;
      goto error;
    }
  }

  /* TODO support create program from binary */
  assert(program->source_type == FROM_LLVM ||
         program->source_type == FROM_SOURCE);
  cl_program_build(program, options);
  program->is_built = CL_TRUE;

  if (pfn_notify) pfn_notify(program, user_data);

error:
  return err;
}

cl_int
clUnloadCompiler(void)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clGetProgramInfo(cl_program       program,
                 cl_program_info  param_name,
                 size_t           param_value_size,
                 void *           param_value,
                 size_t *         param_value_size_ret)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clGetProgramBuildInfo(cl_program             program,
                      cl_device_id           device,
                      cl_program_build_info  param_name,
                      size_t                 param_value_size,
                      void *                 param_value,
                      size_t *               param_value_size_ret)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_kernel
clCreateKernel(cl_program   program,
               const char * kernel_name,
               cl_int *     errcode_ret)
{
  cl_kernel kernel = NULL;
  cl_int err = CL_SUCCESS;

  CHECK_PROGRAM (program);
  if (program->is_built == CL_FALSE) {
    err = CL_INVALID_PROGRAM_EXECUTABLE;
    goto error;
  }
  kernel = cl_program_create_kernel(program, kernel_name, errcode_ret);

exit:
  return kernel;
error:
  if (errcode_ret)
    *errcode_ret = err;
  goto exit;
}

cl_int
clCreateKernelsInProgram(cl_program      program,
                         cl_uint         num_kernels,
                         cl_kernel *     kernels,
                         cl_uint *       num_kernels_ret)
{
  NOT_IMPLEMENTED;
  return 0;
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
  err = cl_kernel_set_arg(kernel, arg_index, arg_size, arg_value);
error:
  return err;
}

cl_int
clGetKernelInfo(cl_kernel        kernel,
                cl_kernel_info   param_name,
                size_t           param_value_size,
                void *           param_value,
                size_t *         param_value_size_ret)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clGetKernelWorkGroupInfo(cl_kernel                   kernel,
                         cl_device_id                device,
                         cl_kernel_work_group_info   param_name,
                         size_t                      param_value_size,
                         void *                      param_value,
                         size_t *                    param_value_size_ret)
{
  return cl_get_kernel_workgroup_info(device,
                                      param_name,
                                      param_value_size,
                                      param_value,
                                      param_value_size_ret);
}

cl_int
clWaitForEvents(cl_uint          num_events,
                const cl_event * event_list)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clGetEventInfo(cl_event      event,
               cl_event_info param_name,
               size_t        param_value_size,
               void *        param_value,
               size_t *      param_value_size_ret)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_event
clCreateUserEvent(cl_context context,
                  cl_int *   errcode_ret)
{
  NOT_IMPLEMENTED;
  return NULL;
}

cl_int
clRetainEvent(cl_event  event)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clReleaseEvent(cl_event  event)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clSetUserEventStatus(cl_event    event,
                     cl_int      execution_status)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clSetEventCallback(cl_event     event,
                   cl_int       command_exec_callback_type,
                   void (CL_CALLBACK * pfn_notify) (cl_event, cl_int, void *),
                   void *       user_data)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clGetEventProfilingInfo(cl_event             event,
                        cl_profiling_info    param_name,
                        size_t               param_value_size,
                        void *               param_value,
                        size_t *             param_value_size_ret)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clFlush(cl_command_queue command_queue)
{
  /* have nothing to do now, as currently
   * clEnqueueNDRangeKernel will flush at
   * the end of each calling. we may need
   * to optimize it latter.*/
  return 0;
}

cl_int
clFinish(cl_command_queue command_queue)
{
  cl_int err = CL_SUCCESS;

  CHECK_QUEUE (command_queue);
  err = cl_command_queue_finish(command_queue);

exit:
  return err;
error:
  goto exit;
}

cl_int
clEnqueueReadBuffer(cl_command_queue command_queue,
                    cl_mem           buffer,
                    cl_bool          blocking_read,
                    size_t           offset,
                    size_t           cb,
                    void *           ptr,
                    cl_uint          num_events_in_wait_list,
                    const cl_event * event_wait_list,
                    cl_event *       event)
{
	cl_int err = CL_SUCCESS;
	assert(ptr != NULL);
	void* temp_ptr = NULL;
	temp_ptr = clMapBufferIntel(buffer, &err);
	assert(err == CL_SUCCESS);
	memcpy(ptr, temp_ptr, cb);
	return err;
}

cl_int
clEnqueueReadBufferRect(cl_command_queue command_queue,
                        cl_mem           buffer,
                        cl_bool          blocking_read,
                        const size_t *   buffer_origin,
                        const size_t *   host_origin,
                        const size_t *   region,
                        size_t           buffer_row_pitch,
                        size_t           buffer_slice_pitch,
                        size_t           host_row_pitch,
                        size_t           host_slice_pitch,
                        void *           ptr,
                        cl_uint          num_events_in_wait_list,
                        const cl_event * event_wait_list,
                        cl_event *       event)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clEnqueueWriteBuffer(cl_command_queue    command_queue,
                     cl_mem              buffer,
                     cl_bool             blocking_write,
                     size_t              offset,
                     size_t              cb,
                     const void *        ptr,
                     cl_uint             num_events_in_wait_list,
                     const cl_event *    event_wait_list,
                     cl_event *          event)
{
  if (blocking_write != CL_TRUE)
    NOT_IMPLEMENTED;
  cl_int err;
  void *p = clMapBufferIntel(buffer, &err);
  if (err != CL_SUCCESS)
    return err;
  memcpy(p + offset, ptr, cb);
  err = clUnmapBufferIntel(buffer);
  return err;
}

cl_int
clEnqueueWriteBufferRect(cl_command_queue     command_queue,
                         cl_mem               buffer,
                         cl_bool              blocking_write,
                         const size_t *       buffer_origin,
                         const size_t *       host_origin,
                         const size_t *       region,
                         size_t               buffer_row_pitch,
                         size_t               buffer_slice_pitch,
                         size_t               host_row_pitch,
                         size_t               host_slice_pitch,
                         const void *         ptr,
                         cl_uint              num_events_in_wait_list,
                         const cl_event *     event_wait_list,
                         cl_event *           event)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clEnqueueCopyBuffer(cl_command_queue     command_queue,
                    cl_mem               src_buffer,
                    cl_mem               dst_buffer,
                    size_t               src_offset,
                    size_t               dst_offset,
                    size_t               cb,
                    cl_uint              num_events_in_wait_list,
                    const cl_event *     event_wait_list,
                    cl_event *           event)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clEnqueueCopyBufferRect(cl_command_queue     command_queue,
                        cl_mem               src_buffer,
                        cl_mem               dst_buffer,
                        const size_t *       src_origin,
                        const size_t *       dst_origin,
                        const size_t *       region,
                        size_t               src_row_pitch,
                        size_t               src_slice_pitch,
                        size_t               dst_row_pitch,
                        size_t               dst_slice_pitch,
                        cl_uint              num_events_in_wait_list,
                        const cl_event *     event_wait_list,
                        cl_event *           event)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clEnqueueReadImage(cl_command_queue      command_queue,
                   cl_mem                image,
                   cl_bool               blocking_read,
                   const size_t *        origin,
                   const size_t *        region,
                   size_t                row_pitch,
                   size_t                slice_pitch,
                   void *                ptr,
                   cl_uint               num_events_in_wait_list,
                   const cl_event *      event_wait_list,
                   cl_event *            event)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clEnqueueWriteImage(cl_command_queue     command_queue,
                    cl_mem               image,
                    cl_bool              blocking_write,
                    const size_t *       origin,
                    const size_t *       region,
                    size_t               input_row_pitch,
                    size_t               input_slice_pitch,
                    const void *         ptr,
                    cl_uint              num_events_in_wait_list,
                    const cl_event *     event_wait_list,
                    cl_event *           event)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clEnqueueCopyImage(cl_command_queue      command_queue,
                   cl_mem                src_image,
                   cl_mem                dst_image,
                   const size_t *        src_origin,
                   const size_t *        dst_origin,
                   const size_t *        region,
                   cl_uint               num_events_in_wait_list,
                   const cl_event *      event_wait_list,
                   cl_event *            event)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clEnqueueCopyImageToBuffer(cl_command_queue  command_queue,
                           cl_mem            src_image,
                           cl_mem            dst_buffer,
                           const size_t *    src_origin,
                           const size_t *    region,
                           size_t            dst_offset,
                           cl_uint           num_events_in_wait_list,
                           const cl_event *  event_wait_list,
                           cl_event *        event)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clEnqueueCopyBufferToImage(cl_command_queue  command_queue,
                           cl_mem            src_buffer,
                           cl_mem            dst_image,
                           size_t            src_offset,
                           const size_t *    dst_origin,
                           const size_t *    region,
                           cl_uint           num_events_in_wait_list,
                           const cl_event *  event_wait_list,
                           cl_event *        event)
{
  NOT_IMPLEMENTED;
  return 0;
}

void *
clEnqueueMapBuffer(cl_command_queue  command_queue,
                   cl_mem            buffer,
                   cl_bool           blocking_map,
                   cl_map_flags      map_flags,
                   size_t            offset,
                   size_t            cb,
                   cl_uint           num_events_in_wait_list,
                   const cl_event *  event_wait_list,
                   cl_event *        event,
                   cl_int *          errcode_ret)
{
  void *p;
  if (blocking_map != CL_TRUE)
     NOT_IMPLEMENTED;
  if (offset != 0)
     NOT_IMPLEMENTED;
  p = clMapBufferIntel(buffer, errcode_ret);
  return p;
}

void *
clEnqueueMapImage(cl_command_queue   command_queue,
                  cl_mem             image,
                  cl_bool            blocking_map,
                  cl_map_flags       map_flags,
                  const size_t *     origin,
                  const size_t *     region,
                  size_t *           image_row_pitch,
                  size_t *           image_slice_pitch,
                  cl_uint            num_events_in_wait_list,
                  const cl_event *   event_wait_list,
                  cl_event *         event,
                  cl_int *           errcode_ret)
{
  NOT_IMPLEMENTED;
  return NULL;
}

cl_int
clEnqueueUnmapMemObject(cl_command_queue  command_queue,
                        cl_mem            memobj,
                        void *            mapped_ptr,
                        cl_uint           num_events_in_wait_list,
                        const cl_event *  event_wait_list,
                        cl_event *        event)
{
  return clUnmapBufferIntel(memobj);
}

cl_int
clEnqueueNDRangeKernel(cl_command_queue  command_queue,
                       cl_kernel         kernel,
                       cl_uint           work_dim,
                       const size_t *    global_work_offset,
                       const size_t *    global_work_size,
                       const size_t *    local_work_size,
                       cl_uint           num_events_in_wait_list,
                       const cl_event *  event_wait_list,
                       cl_event *        event)
{
  size_t fixed_global_off[] = {0,0,0};
  size_t fixed_global_sz[] = {1,1,1};
  size_t fixed_local_sz[] = {16,1,1};
  cl_int err = CL_SUCCESS;
  cl_uint i;

  CHECK_QUEUE(command_queue);
  CHECK_KERNEL(kernel);

  /* Check number of dimensions we have */
  if (UNLIKELY(work_dim == 0 || work_dim > 3)) {
    err = CL_INVALID_WORK_DIMENSION;
    goto error;
  }

  /* We need a work size per dimension */
  if (UNLIKELY(global_work_size == NULL)) {
    err = CL_INVALID_GLOBAL_WORK_SIZE;
    goto error;
  }

  /* Local size must be non-null */
  for (i = 0; i < work_dim; ++i)
    if (UNLIKELY(local_work_size[i] == 0)) {
      err = CL_INVALID_WORK_GROUP_SIZE;
      goto error;
    }

  /* Check offset values. We add a non standard restriction. The offsets must
   * also be evenly divided by the local sizes
   */
  if (global_work_offset != NULL)
    for (i = 0; i < work_dim; ++i) {
      if (UNLIKELY(~0LL - global_work_offset[i] > global_work_size[i])) {
        err = CL_INVALID_GLOBAL_OFFSET;
        goto error;
      }
      if (UNLIKELY(global_work_offset[i] % local_work_size[i])) {
        err = CL_INVALID_GLOBAL_OFFSET;
        goto error;
      }
    }

  /* Local sizes must divide global sizes */
  if (local_work_size != NULL) 
    for (i = 0; i < work_dim; ++i) 
      if (UNLIKELY(global_work_size[i] % local_work_size[i])) {
        err = CL_INVALID_WORK_GROUP_SIZE;
        goto error;
      }

  /* Queue and kernel must share the same context */
  assert(kernel->program);
  if (command_queue->ctx != kernel->program->ctx) {
    err = CL_INVALID_CONTEXT;
    goto error;
  }

  /* XXX No event right now */
  FATAL_IF(num_events_in_wait_list > 0, "Events are not supported");
  FATAL_IF(event_wait_list != NULL, "Events are not supported");
  FATAL_IF(event != NULL, "Events are not supported");

  if (local_work_size != NULL)
    for (i = 0; i < work_dim; ++i)
      fixed_local_sz[i] = local_work_size[i];
  if (global_work_size != NULL)
    for (i = 0; i < work_dim; ++i)
      fixed_global_sz[i] = global_work_size[i];
  if (global_work_offset != NULL)
    for (i = 0; i < work_dim; ++i)
      fixed_global_off[i] = global_work_offset[i];

  /* Do device specific checks are enqueue the kernel */
  err = cl_command_queue_ND_range(command_queue,
                                  kernel,
                                  fixed_global_off,
                                  fixed_global_sz,
                                  fixed_local_sz);

error:
  return err;
}

cl_int
clEnqueueTask(cl_command_queue   command_queue,
              cl_kernel          kernel,
              cl_uint            num_events_in_wait_list,
              const cl_event *   event_wait_list,
              cl_event *         event)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clEnqueueNativeKernel(cl_command_queue   command_queue,
                      void (*user_func)(void *),
                      void *             args,
                      size_t             cb_args,
                      cl_uint            num_mem_objects,
                      const cl_mem *     mem_list,
                      const void **      args_mem_loc,
                      cl_uint            num_events_in_wait_list,
                      const cl_event *   event_wait_list,
                      cl_event *         event)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clEnqueueMarker(cl_command_queue     command_queue,
                cl_event *           event)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clEnqueueWaitForEvents(cl_command_queue  command_queue,
                       cl_uint           num_events,
                       const cl_event *  event_list)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clEnqueueBarrier(cl_command_queue  command_queue)
{
  NOT_IMPLEMENTED;
  return 0;
}

#define EXTFUNC(x)                      \
  if (strcmp(#x, func_name) == 0)       \
    return (void *)x;

void*
clGetExtensionFunctionAddress(const char *func_name)
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
  return NULL;
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
  ptr = cl_mem_map(mem);
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


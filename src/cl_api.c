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
	      memcpy(param_value, (VAL), sizeof(TYPE)*ELT); \
	  } \
          \
	  if (param_value_size_ret) \
	      *param_value_size_ret = sizeof(TYPE)*ELT; \
	  return RET; \
	} while(0)

static cl_int
cl_check_device_type(cl_device_type device_type)
{
  const cl_device_type valid =  CL_DEVICE_TYPE_GPU
                              | CL_DEVICE_TYPE_CPU
                              | CL_DEVICE_TYPE_ACCELERATOR
                              | CL_DEVICE_TYPE_DEFAULT
                              | CL_DEVICE_TYPE_CUSTOM;

  if( (device_type & valid) == 0) {
    return CL_INVALID_DEVICE_TYPE;
  }
  if(UNLIKELY(!(device_type & CL_DEVICE_TYPE_DEFAULT) && !(device_type & CL_DEVICE_TYPE_GPU)))
    return CL_DEVICE_NOT_FOUND;

  return CL_SUCCESS;
}

static cl_int
cl_device_id_is_ok(const cl_device_id device)
{
  return device != cl_get_gt_device() ? CL_FALSE : CL_TRUE;
}

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

cl_int
clGetPlatformInfo(cl_platform_id    platform,
                  cl_platform_info  param_name,
                  size_t            param_value_size,
                  void *            param_value,
                  size_t *          param_value_size_ret)
{
  /* Only one platform. This is easy */
  if (UNLIKELY(platform != NULL && platform != intel_platform))
    return CL_INVALID_PLATFORM;

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
  cl_int err = CL_SUCCESS;

  /* Check parameter consistency */
  if (UNLIKELY(devices == NULL && num_devices == NULL))
    return CL_INVALID_VALUE;
  if (UNLIKELY(platform && platform != intel_platform))
    return CL_INVALID_PLATFORM;
  if (UNLIKELY(devices && num_entries == 0))
    return CL_INVALID_VALUE;

  err = cl_check_device_type(device_type);
  if(err != CL_SUCCESS)
    return err;

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

cl_int
clCreateSubDevices(cl_device_id                         in_device,
                   const cl_device_partition_property * properties,
                   cl_uint                              num_devices,
                   cl_device_id *                       out_devices,
                   cl_uint *                            num_devices_ret)
{
  NOT_IMPLEMENTED;
  return 0;
}

cl_int
clRetainDevice(cl_device_id device)
{
  // XXX stub for C++ Bindings
  return CL_SUCCESS;
}

cl_int
clReleaseDevice(cl_device_id device)
{
  // XXX stub for C++ Bindings
  return CL_SUCCESS;
}

cl_context
clCreateContext(const cl_context_properties *  properties,
                cl_uint                        num_devices,
                const cl_device_id *           devices,
                void (* pfn_notify) (const char*, const void*, size_t, void*),
                void *                         user_data,
                cl_int *                       errcode_ret)
{
  cl_int err = CL_SUCCESS;
  cl_context context = NULL;

  /* Assert parameters correctness */
  INVALID_VALUE_IF (devices == NULL);
  INVALID_VALUE_IF (num_devices == 0);
  INVALID_VALUE_IF (pfn_notify == NULL && user_data != NULL);

  /* Now check if the user is asking for the right device */
  INVALID_DEVICE_IF (cl_device_id_is_ok(*devices) == CL_FALSE);

  context = cl_create_context(properties,
                           num_devices,
                           devices,
                           pfn_notify,
                           user_data,
                           &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return context;
}

cl_context
clCreateContextFromType(const cl_context_properties *  properties,
                        cl_device_type                 device_type,
                        void (CL_CALLBACK *pfn_notify) (const char *, const void *, size_t, void *),
                        void *                         user_data,
                        cl_int *                       errcode_ret)
{
  cl_context context = NULL;
  cl_int err = CL_SUCCESS;
  cl_device_id devices[1];
  cl_uint num_devices = 1;

  INVALID_VALUE_IF (pfn_notify == NULL && user_data != NULL);

  err = cl_check_device_type(device_type);
  if(err != CL_SUCCESS) {
    goto error;
  }

  err = cl_get_device_ids(NULL,
                          device_type,
                          1,
                          &devices[0],
                          &num_devices);
  if (err != CL_SUCCESS) {
    goto error;
  }

  context = cl_create_context(properties,
                              num_devices,
                              devices,
                              pfn_notify,
                              user_data,
                              &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return context;
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
  cl_int err = CL_SUCCESS;
  CHECK_CONTEXT (context);

  if (param_name == CL_CONTEXT_DEVICES) {
    FILL_GETINFO_RET (cl_device_id, 1, &context->device, CL_SUCCESS);
  } else if (param_name == CL_CONTEXT_NUM_DEVICES) {
    cl_uint n = 1;
    FILL_GETINFO_RET (cl_uint, 1, &n, CL_SUCCESS);
  } else if (param_name == CL_CONTEXT_REFERENCE_COUNT) {
    cl_uint ref = context->ref_n;
    FILL_GETINFO_RET (cl_uint, 1, &ref, CL_SUCCESS);
  } else if (param_name == CL_CONTEXT_PROPERTIES) {
    if(context->prop_len > 0) {
      FILL_GETINFO_RET (cl_context_properties, context->prop_len, context->prop_user, CL_SUCCESS);
    } else {
      cl_context_properties n = 0;
      FILL_GETINFO_RET (cl_context_properties, 1, &n, CL_SUCCESS);
    }
  } else {
    return CL_INVALID_VALUE;
  }

error:
  return err;
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

  INVALID_DEVICE_IF (device != context->device);
  INVALID_VALUE_IF (properties & ~(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE));

  if(properties) {
    err = CL_INVALID_QUEUE_PROPERTIES;
    goto error;
  }

  queue = cl_context_create_queue(context, device, properties, &err);
error:
  if (errcode_ret)
    *errcode_ret = err;
  return queue;
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

  if (param_name == CL_QUEUE_CONTEXT) {
    FILL_GETINFO_RET (cl_context, 1, &command_queue->ctx, CL_SUCCESS);
  } else if (param_name == CL_QUEUE_DEVICE) {
    FILL_GETINFO_RET (cl_device_id, 1, &command_queue->ctx->device, CL_SUCCESS);
  } else if (param_name == CL_QUEUE_REFERENCE_COUNT) {
    cl_uint ref = command_queue->ref_n;
    FILL_GETINFO_RET (cl_uint, 1, &ref, CL_SUCCESS);
  } else if (param_name == CL_QUEUE_PROPERTIES) {
    FILL_GETINFO_RET (cl_command_queue_properties, 1, &command_queue->props, CL_SUCCESS);
  } else {
    return CL_INVALID_VALUE;
  }

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
                         &err);
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
  return cl_get_image_info(image,
                           param_name,
                           param_value_size,
                           param_value,
                           param_value_size_ret);
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
    INVALID_DEVICE_IF (device_list[0] != program->ctx->device);
  }

  /* TODO support create program from binary */
  assert(program->source_type == FROM_LLVM ||
         program->source_type == FROM_SOURCE);
  if((err = cl_program_build(program, options)) != CL_SUCCESS) {
    goto error;
  }
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
  cl_int err = CL_SUCCESS;
  char * ret_str = "";

  CHECK_PROGRAM (program);

  if (param_name == CL_PROGRAM_REFERENCE_COUNT) {
    cl_uint ref = program->ref_n;
    FILL_GETINFO_RET (cl_uint, 1, (&ref), CL_SUCCESS);
  } else if (param_name == CL_PROGRAM_CONTEXT) {
    cl_context context = program->ctx;
    FILL_GETINFO_RET (cl_context, 1, &context, CL_SUCCESS);
  } else if (param_name == CL_PROGRAM_NUM_DEVICES) {
    cl_uint num_dev = 1; // Just 1 dev now.
    FILL_GETINFO_RET (cl_uint, 1, &num_dev, CL_SUCCESS);
  } else if (param_name == CL_PROGRAM_DEVICES) {
    cl_device_id dev_id = program->ctx->device;
    FILL_GETINFO_RET (cl_device_id, 1, &dev_id, CL_SUCCESS);
  } else if (param_name == CL_PROGRAM_SOURCE) {

    if (!program->source)
      FILL_GETINFO_RET (char, 1, &ret_str, CL_SUCCESS);
    FILL_GETINFO_RET (char, (strlen(program->source) + 1),
                   program->source, CL_SUCCESS);
  } else if (param_name == CL_PROGRAM_BINARY_SIZES) {
    FILL_GETINFO_RET (size_t, 1, (&program->bin_sz), CL_SUCCESS);
  } else if (param_name == CL_PROGRAM_BINARIES) {
    if (!param_value)
      return CL_SUCCESS;

    /* param_value points to an array of n
       pointers allocated by the caller */
    if (program->bin_sz > 0) {
      memcpy(*((void **)param_value), program->bin, program->bin_sz);
    } else {
      memcpy(*((void **)param_value), ret_str, 1);
    }

    return CL_SUCCESS;
  } else {
    return CL_INVALID_VALUE;
  }

error:
    return err;
}

cl_int
clGetProgramBuildInfo(cl_program             program,
                      cl_device_id           device,
                      cl_program_build_info  param_name,
                      size_t                 param_value_size,
                      void *                 param_value,
                      size_t *               param_value_size_ret)
{
  cl_int err = CL_SUCCESS;
  char * ret_str = "";

  CHECK_PROGRAM (program);
  INVALID_DEVICE_IF (device != program->ctx->device);

  if (param_name == CL_PROGRAM_BUILD_STATUS) {
    cl_build_status status;

    if (!program->is_built)
      status = CL_BUILD_NONE;
    else if (program->ker_n > 0)
      status = CL_BUILD_SUCCESS;
    else
      status = CL_BUILD_ERROR;
    // TODO: Support CL_BUILD_IN_PROGRESS ?

    FILL_GETINFO_RET (cl_build_status, 1, &status, CL_SUCCESS);
  } else if (param_name == CL_PROGRAM_BUILD_OPTIONS) {
    if (program->is_built && program->build_opts)
      ret_str = program->build_opts;

    FILL_GETINFO_RET (char, (strlen(ret_str)+1), ret_str, CL_SUCCESS);
  } else if (param_name == CL_PROGRAM_BUILD_LOG) {
    // TODO: need to add logs in backend when compiling.
    FILL_GETINFO_RET (char, (strlen(ret_str)+1), ret_str, CL_SUCCESS);
  } else {
    return CL_INVALID_VALUE;
  }

error:
    return err;
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
  cl_int err;

  CHECK_KERNEL(kernel);

  if (param_name == CL_KERNEL_CONTEXT) {
    FILL_GETINFO_RET (cl_context, 1, &kernel->program->ctx, CL_SUCCESS);
  } else if (param_name == CL_KERNEL_PROGRAM) {
    FILL_GETINFO_RET (cl_program, 1, &kernel->program, CL_SUCCESS);
  } else if (param_name == CL_KERNEL_NUM_ARGS) {
    cl_uint n = kernel->arg_n;
    FILL_GETINFO_RET (cl_uint, 1, &n, CL_SUCCESS);
  } else if (param_name == CL_KERNEL_REFERENCE_COUNT) {
    cl_int ref = kernel->ref_n;
    FILL_GETINFO_RET (cl_int, 1, &ref, CL_SUCCESS);
  } else if (param_name == CL_KERNEL_FUNCTION_NAME) {
    const char * n = cl_kernel_get_name(kernel);
    FILL_GETINFO_RET (cl_char, strlen(n)+1, n, CL_SUCCESS);
  } else {
    return CL_INVALID_VALUE;
  }

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

error:
  return err;
}

cl_int
clEnqueueReadBuffer(cl_command_queue command_queue,
                    cl_mem           buffer,
                    cl_bool          blocking_read,
                    size_t           offset,
                    size_t           size,
                    void *           ptr,
                    cl_uint          num_events_in_wait_list,
                    const cl_event * event_wait_list,
                    cl_event *       event)
{
  cl_int err = CL_SUCCESS;
  void* src_ptr;

  CHECK_QUEUE(command_queue);
  CHECK_MEM(buffer);
  if (command_queue->ctx != buffer->ctx) {
     err = CL_INVALID_CONTEXT;
     goto error;
  }

  if (blocking_read != CL_TRUE)
     NOT_IMPLEMENTED;

  if (!ptr || !size || offset + size > buffer->size) {
     err = CL_INVALID_VALUE;
     goto error;
  }

  if (buffer->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) {
     err = CL_INVALID_OPERATION;
     goto error;
  }

  if (!(src_ptr = cl_mem_map_auto(buffer))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  memcpy(ptr, (char*)src_ptr + offset, size);

  err = cl_mem_unmap_auto(buffer);

error:
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
                     size_t              size,
                     const void *        ptr,
                     cl_uint             num_events_in_wait_list,
                     const cl_event *    event_wait_list,
                     cl_event *          event)
{
  cl_int err = CL_SUCCESS;
  void* dst_ptr;

  CHECK_QUEUE(command_queue);
  CHECK_MEM(buffer);
  if (command_queue->ctx != buffer->ctx) {
    err = CL_INVALID_CONTEXT;
    goto error;
  }

  if (blocking_write != CL_TRUE)
    NOT_IMPLEMENTED;

  if (!ptr || !size || offset + size > buffer->size) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if (buffer->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)) {
    err = CL_INVALID_OPERATION;
    goto error;
  }

  if (!(dst_ptr = cl_mem_map_auto(buffer))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  memcpy((char*)dst_ptr + offset, ptr, size);

  err = cl_mem_unmap_auto(buffer);

error:
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
  cl_int err = CL_SUCCESS;
  void* src_ptr;

  CHECK_QUEUE(command_queue);
  CHECK_IMAGE(image);
  if (command_queue->ctx != image->ctx) {
     err = CL_INVALID_CONTEXT;
     goto error;
  }

  if (blocking_read != CL_TRUE)
     NOT_IMPLEMENTED;

  if (!origin || !region || origin[0] + region[0] > image->w || origin[1] + region[1] > image->h || origin[2] + region[2] > image->depth) {
     err = CL_INVALID_VALUE;
     goto error;
  }

  if (!row_pitch)
    row_pitch = image->bpp*region[0];
  else if (row_pitch < image->bpp*region[0]) {
     err = CL_INVALID_VALUE;
     goto error;
  }

  if (image->slice_pitch) {
    if (!slice_pitch)
      slice_pitch = row_pitch*region[1];
    else if (slice_pitch < row_pitch*region[1]) {
      err = CL_INVALID_VALUE;
      goto error;
    }
  }
  else if (slice_pitch) {
     err = CL_INVALID_VALUE;
     goto error;
  }

  if (!ptr) {
     err = CL_INVALID_VALUE;
     goto error;
  }

  if (image->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) {
     err = CL_INVALID_OPERATION;
     goto error;
  }

  if (!(src_ptr = cl_mem_map_auto(image))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  size_t offset = image->bpp*origin[0] + image->row_pitch*origin[1] + image->slice_pitch*origin[2];
  src_ptr = (char*)src_ptr + offset;

  if (!origin[0] && region[0] == image->w && row_pitch == image->row_pitch &&
      (region[2] == 1 || (!origin[1] && region[1] == image->h && slice_pitch == image->slice_pitch)))
  {
    memcpy(ptr, src_ptr, region[2] == 1 ? row_pitch*region[1] : slice_pitch*region[2]);
  }
  else {
    cl_uint y, z;
    for (z = 0; z < region[2]; z++) {
      const char* src = src_ptr;
      char* dst = ptr;
      for (y = 0; y < region[1]; y++) {
	memcpy(dst, src, image->bpp*region[0]);
	src += image->row_pitch;
	dst += row_pitch;
      }
      src_ptr = (char*)src_ptr + image->slice_pitch;
      ptr = (char*)ptr + slice_pitch;
    }
  }

  err = cl_mem_unmap_auto(image);

error:
  return err;
}

cl_int
clEnqueueWriteImage(cl_command_queue     command_queue,
                    cl_mem               image,
                    cl_bool              blocking_write,
                    const size_t *       origin,
                    const size_t *       region,
                    size_t               row_pitch,
                    size_t               slice_pitch,
                    const void *         ptr,
                    cl_uint              num_events_in_wait_list,
                    const cl_event *     event_wait_list,
                    cl_event *           event)
{
  cl_int err = CL_SUCCESS;
  void* dst_ptr;

  CHECK_QUEUE(command_queue);
  CHECK_IMAGE(image);
  if (command_queue->ctx != image->ctx) {
    err = CL_INVALID_CONTEXT;
    goto error;
  }

  if (blocking_write != CL_TRUE)
    NOT_IMPLEMENTED;

  if (!origin || !region || origin[0] + region[0] > image->w || origin[1] + region[1] > image->h || origin[2] + region[2] > image->depth) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if (!row_pitch)
    row_pitch = image->bpp*region[0];
  else if (row_pitch < image->bpp*region[0]) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if (image->slice_pitch) {
    if (!slice_pitch)
      slice_pitch = row_pitch*region[1];
    else if (slice_pitch < row_pitch*region[1]) {
      err = CL_INVALID_VALUE;
      goto error;
    }
  }
  else if (slice_pitch) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if (!ptr) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if (image->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)) {
    err = CL_INVALID_OPERATION;
    goto error;
  }

  if (!(dst_ptr = cl_mem_map_auto(image))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  size_t offset = image->bpp*origin[0] + image->row_pitch*origin[1] + image->slice_pitch*origin[2];
  dst_ptr = (char*)dst_ptr + offset;

  if (!origin[0] && region[0] == image->w && row_pitch == image->row_pitch &&
      (region[2] == 1 || (!origin[1] && region[1] == image->h && slice_pitch == image->slice_pitch)))
  {
    memcpy(dst_ptr, ptr, region[2] == 1 ? row_pitch*region[1] : slice_pitch*region[2]);
  }
  else {
    cl_uint y, z;
    for (z = 0; z < region[2]; z++) {
      const char* src = ptr;
      char* dst = dst_ptr;
      for (y = 0; y < region[1]; y++) {
	memcpy(dst, src, image->bpp*region[0]);
	src += row_pitch;
	dst += image->row_pitch;
      }
      ptr = (char*)ptr + slice_pitch;
      dst_ptr = (char*)dst_ptr + image->slice_pitch;
    }
  }

  err = cl_mem_unmap_auto(image);

error:
  return err;
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
                   size_t            size,
                   cl_uint           num_events_in_wait_list,
                   const cl_event *  event_wait_list,
                   cl_event *        event,
                   cl_int *          errcode_ret)
{
  void *ptr = NULL;
  cl_int err = CL_SUCCESS;

  CHECK_QUEUE(command_queue);
  CHECK_MEM(buffer);
  if (command_queue->ctx != buffer->ctx) {
    err = CL_INVALID_CONTEXT;
    goto error;
  }

  if (blocking_map != CL_TRUE)
    NOT_IMPLEMENTED;

  if (!size || offset + size > buffer->size) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if ((map_flags & CL_MAP_READ &&
       buffer->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) ||
      (map_flags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION) &&
       buffer->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)))
  {
    err = CL_INVALID_OPERATION;
    goto error;
  }

  if (!(ptr = cl_mem_map_auto(buffer))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  ptr = (char*)ptr + offset;

error:
  if (errcode_ret)
    *errcode_ret = err;
  return ptr;
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
  void *ptr = NULL;
  cl_int err = CL_SUCCESS;

  CHECK_QUEUE(command_queue);
  CHECK_IMAGE(image);
  if (command_queue->ctx != image->ctx) {
    err = CL_INVALID_CONTEXT;
    goto error;
  }

  if (blocking_map != CL_TRUE)
    NOT_IMPLEMENTED;

  if (!origin || !region || origin[0] + region[0] > image->w || origin[1] + region[1] > image->h || origin[2] + region[2] > image->depth) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  if (!image_row_pitch || (image->slice_pitch && !image_slice_pitch)) {
    err = CL_INVALID_VALUE;
    goto error;
  }

  *image_row_pitch = image->row_pitch;
  if (image_slice_pitch)
    *image_slice_pitch = image->slice_pitch;

  if ((map_flags & CL_MAP_READ &&
       image->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) ||
      (map_flags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION) &&
       image->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)))
  {
    err = CL_INVALID_OPERATION;
    goto error;
  }

  if (!(ptr = cl_mem_map_auto(image))) {
    err = CL_MAP_FAILURE;
    goto error;
  }

  size_t offset = image->bpp*origin[0] + image->row_pitch*origin[1] + image->slice_pitch*origin[2];
  ptr = (char*)ptr + offset;

error:
  if (errcode_ret)
    *errcode_ret = err;
  return ptr;
}

cl_int
clEnqueueUnmapMemObject(cl_command_queue  command_queue,
                        cl_mem            memobj,
                        void *            mapped_ptr,
                        cl_uint           num_events_in_wait_list,
                        const cl_event *  event_wait_list,
                        cl_event *        event)
{
  return cl_mem_unmap_auto(memobj);
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
  size_t fixed_local_sz[] = {1,1,1};
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

  /* Check offset values. We add a non standard restriction. The offsets must
   * also be evenly divided by the local sizes
   */
  if (global_work_offset != NULL)
    for (i = 0; i < work_dim; ++i) {
      if (UNLIKELY(~0LL - global_work_offset[i] > global_work_size[i])) {
        err = CL_INVALID_GLOBAL_OFFSET;
        goto error;
      }
      if (UNLIKELY(local_work_size != NULL && global_work_offset[i] % local_work_size[i])) {
        err = CL_INVALID_GLOBAL_OFFSET;
        goto error;
      }
    }

  /* Local sizes must be non-null and divide global sizes */
  if (local_work_size != NULL) 
    for (i = 0; i < work_dim; ++i) 
      if (UNLIKELY(local_work_size[i] == 0 || global_work_size[i] % local_work_size[i])) {
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
                                  work_dim,
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


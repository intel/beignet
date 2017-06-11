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
#include "cl_alloc.h"
#include "cl_context.h"
#include "cl_device_id.h"
#include "cl_program.h"
#include <string.h>

cl_int
clGetProgramInfo(cl_program program,
                 cl_program_info param_name,
                 size_t param_value_size,
                 void *param_value,
                 size_t *param_value_size_ret)
{
  const void *src_ptr = NULL;
  size_t src_size = 0;
  const char *ret_str = "";
  cl_int ref;
  cl_uint num_dev, kernels_num;
  cl_int i;

  if (!CL_OBJECT_IS_PROGRAM(program)) {
    return CL_INVALID_PROGRAM;
  }

  if (param_name == CL_PROGRAM_REFERENCE_COUNT) {
    ref = CL_OBJECT_GET_REF(program);
    src_ptr = &ref;
    src_size = sizeof(cl_int);
  } else if (param_name == CL_PROGRAM_CONTEXT) {
    src_ptr = &program->ctx;
    src_size = sizeof(cl_context);
  } else if (param_name == CL_PROGRAM_NUM_DEVICES) {
    num_dev = program->ctx->device_num;
    src_ptr = &num_dev;
    src_size = sizeof(cl_uint);
  } else if (param_name == CL_PROGRAM_DEVICES) {
    src_ptr = program->ctx->devices;
    src_size = program->ctx->device_num * sizeof(cl_device_id);
  } else if (param_name == CL_PROGRAM_NUM_KERNELS) {
    cl_int err;
    kernels_num = 0;
    err = cl_program_get_kernel_names(program, &kernels_num, 0, NULL, NULL);
    if (err != CL_SUCCESS)
      return err;

    src_ptr = &kernels_num;
    src_size = sizeof(cl_uint);
  } else if (param_name == CL_PROGRAM_SOURCE) {
    if (!program->source) {
      src_ptr = ret_str;
      src_size = 1;
    } else {
      src_ptr = program->source;
      src_size = strlen(program->source) + 1;
    }
  } else if (param_name == CL_PROGRAM_KERNEL_NAMES) {
    return cl_program_get_kernel_names(program, NULL, param_value_size,
                                       (char *)param_value, param_value_size_ret);
  } else if (param_name == CL_PROGRAM_BINARY_SIZES) {
    cl_program_for_device pd = NULL;
    size_t *bin_sz = param_value;

    if (param_value && param_value_size < program->ctx->device_num * sizeof(size_t))
      return CL_INVALID_VALUE;

    if (param_value_size_ret)
      *param_value_size_ret = program->ctx->device_num * sizeof(size_t);

    if (param_value) {
      for (i = 0; i < program->ctx->device_num; i++) {
        DEV_PRIVATE_DATA(program, program->ctx->devices[i], pd);
        if (pd->binary == NULL || pd->binary_type == CL_PROGRAM_BINARY_TYPE_NONE) {
          bin_sz[i] = 0;
        } else {
          bin_sz[i] = pd->binary_sz;
        }
      }
    }
    return CL_SUCCESS;
  } else if (param_name == CL_PROGRAM_BINARIES) {
    cl_program_for_device pd = NULL;
    char **bin_ptr = param_value;

    if (param_value && param_value_size < program->ctx->device_num * sizeof(char *))
      return CL_INVALID_VALUE;

    if (param_value_size_ret)
      *param_value_size_ret = program->ctx->device_num * sizeof(char *);

    bin_ptr = param_value;
    if (param_value) {
      for (i = 0; i < program->ctx->device_num; i++) {
        if (bin_ptr[i] == NULL)
          continue;

        DEV_PRIVATE_DATA(program, program->ctx->devices[i], pd);

        if (pd->binary == NULL || pd->binary_type == CL_PROGRAM_BINARY_TYPE_NONE) {
          bin_ptr[i][0] = 0;
        } else {
          memcpy(bin_ptr[i], pd->binary, pd->binary_sz);
        }
      }
    }
    return CL_SUCCESS;
  } else {
    return CL_INVALID_VALUE;
  }

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
}

cl_int
clRetainProgram(cl_program program)
{
  if (!CL_OBJECT_IS_PROGRAM(program)) {
    return CL_INVALID_PROGRAM;
  }

  cl_program_add_ref(program);
  return CL_SUCCESS;
}

cl_int
clReleaseProgram(cl_program program)
{
  if (!CL_OBJECT_IS_PROGRAM(program)) {
    return CL_INVALID_PROGRAM;
  }

  cl_program_delete(program);
  return CL_SUCCESS;
}

cl_int
clGetProgramBuildInfo(cl_program program,
                      cl_device_id device,
                      cl_program_build_info param_name,
                      size_t param_value_size,
                      void *param_value,
                      size_t *param_value_size_ret)
{
  const void *src_ptr = NULL;
  size_t src_size = 0;
  const char *ret_str = "";
  cl_int err = CL_SUCCESS;
  size_t result = 0;
  cl_program_for_device pd;

  if (!CL_OBJECT_IS_PROGRAM(program)) {
    return CL_INVALID_PROGRAM;
  }

  if (device == NULL)
    return CL_INVALID_DEVICE;

  err = cl_devices_list_check(1, (const cl_device_id *)&device);
  if (err != CL_SUCCESS)
    return err;

  cl_devices_list_include_check(program->ctx->device_num, program->ctx->devices,
                                1, (const cl_device_id *)&device);
  if (err != CL_SUCCESS)
    return err;

  DEV_PRIVATE_DATA(program, device, pd);

  if (param_name == CL_PROGRAM_BUILD_STATUS) {
    src_ptr = &program->build_status;
    src_size = sizeof(cl_build_status);
  } else if (param_name == CL_PROGRAM_BUILD_OPTIONS) {
    if (program->build_status != CL_BUILD_NONE && program->build_opts) {
      ret_str = program->build_opts;
    }
    src_ptr = ret_str;
    src_size = strlen(ret_str) + 1;
  } else if (param_name == CL_PROGRAM_BUILD_LOG) {
    src_ptr = pd->build_log;
    src_size = pd->build_log_sz + 1;
  } else if (param_name == CL_PROGRAM_BINARY_TYPE) {
    src_ptr = &pd->binary_type;
    src_size = sizeof(cl_uint);
  } else if (param_name == CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE) {
    if (program->build_status != CL_BUILD_NONE) {
      err = device->api.program_get_info(device, program, param_name, &result);
    } else {
      result = 0;
    }

    if (err != CL_SUCCESS)
      return result;

    src_ptr = &result;
    src_size = sizeof(result);
  } else {
    return CL_INVALID_VALUE;
  }

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
}

cl_program
clCreateProgramWithSource(cl_context context,
                          cl_uint count,
                          const char **strings,
                          const size_t *lengths,
                          cl_int *errcode_ret)
{
  cl_program program = NULL;
  cl_int err = CL_SUCCESS;
  cl_uint i;
  cl_int *lens = NULL;
  cl_int len_total = 0;
  char *p = NULL;

  do {
    if (!CL_OBJECT_IS_CONTEXT(context)) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (count == 0) {
      err = CL_INVALID_VALUE;
      break;
    }
    if (strings == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    for (i = 0; i < count; i++) {
      if (strings[i] == NULL) {
        err = CL_INVALID_VALUE;
        break;
      }
    }
    if (err != CL_SUCCESS)
      break;

    program = cl_program_new(context);
    if (program == NULL) {
      err = CL_OUT_OF_HOST_MEMORY;
      break;
    }

    lens = CL_CALLOC(count, sizeof(cl_int));
    if (lens == NULL) {
      err = CL_OUT_OF_HOST_MEMORY;
      break;
    }

    for (i = 0; i < (cl_int)count; ++i) {
      size_t len;
      if (lengths == NULL || lengths[i] == 0)
        len = strlen(strings[i]);
      else
        len = lengths[i];
      lens[i] = len;
      len_total += len;
    }

    program->source = CL_CALLOC(len_total + 1, sizeof(char));
    if (program->source == NULL) {
      err = CL_OUT_OF_HOST_MEMORY;
      break;
    }

    p = program->source;
    for (i = 0; i < (cl_int)count; ++i) {
      memcpy(p, strings[i], lens[i]);
      p += lens[i];
    }
    *p = '\0';

    program->source_sz = len_total + 1;
  } while (0);

  if (err != CL_SUCCESS) {
    if (program)
      cl_program_delete(program);
  }

  CL_FREE(lens);

  if (errcode_ret)
    *errcode_ret = err;
  return program;
}

cl_program
clCreateProgramWithBinary(cl_context context,
                          cl_uint num_devices,
                          const cl_device_id *devices,
                          const size_t *lengths,
                          const unsigned char **binaries,
                          cl_int *binary_status,
                          cl_int *errcode_ret)
{
  cl_program program = NULL;
  cl_int err = CL_SUCCESS;
  cl_int i;

  do {
    if (!CL_OBJECT_IS_CONTEXT(context)) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (devices == NULL || num_devices == 0) {
      err = CL_INVALID_VALUE;
      break;
    }

    err = cl_devices_list_check(num_devices, devices);
    if (err != CL_SUCCESS)
      break;

    err = cl_devices_list_include_check(context->device_num, context->devices, num_devices, devices);
    if (err != CL_SUCCESS)
      break;

    if (binaries == NULL || lengths == NULL) {
      if (binary_status) {
        for (i = 0; i < num_devices; i++)
          binary_status[i] = CL_INVALID_VALUE;
      }
      err = CL_INVALID_VALUE;
      break;
    }

    for (i = 0; i < num_devices; i++) {
      if (binaries[i] == NULL || lengths[i] == 0) {
        if (binary_status)
          binary_status[i] = CL_INVALID_VALUE;

        err = CL_INVALID_VALUE;
      } else {
        if (binary_status)
          binary_status[i] = CL_SUCCESS;
      }
    }
    if (err != CL_SUCCESS)
      break;

    program = cl_program_create_from_binary(context, num_devices, devices, lengths,
                                            binaries, binary_status, &err);
  } while (0);

  if (errcode_ret)
    *errcode_ret = err;
  return program;
}

cl_int
clBuildProgram(cl_program program,
               cl_uint num_devices,
               const cl_device_id *device_list,
               const char *options,
               void(CL_CALLBACK *pfn_notify)(cl_program, void *),
               void *user_data)
{
  cl_int err = CL_SUCCESS;

  do {
    if (!CL_OBJECT_IS_PROGRAM(program)) {
      err = CL_INVALID_PROGRAM;
      break;
    }

    if ((num_devices == 0 && device_list != NULL) ||
        (num_devices != 0 && device_list == NULL)) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (pfn_notify == NULL && user_data != NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (device_list) {
      err = cl_devices_list_check(num_devices, device_list);
      if (err != CL_SUCCESS)
        break;
    }

    if (device_list) {
      assert(program->ctx);
      err = cl_devices_list_check(num_devices, device_list);
      if (err != CL_SUCCESS)
        break;

      err = cl_devices_list_include_check(program->ctx->device_num,
                                          program->ctx->devices, num_devices, device_list);
      if (err != CL_SUCCESS)
        break;
    }

    err = cl_program_build(program, options, num_devices, device_list);
  } while (0);

  if (pfn_notify)
    pfn_notify(program, user_data);

  return err;
}

cl_int
clCompileProgram(cl_program program,
                 cl_uint num_devices,
                 const cl_device_id *device_list,
                 const char *options,
                 cl_uint num_input_headers,
                 const cl_program *input_headers,
                 const char **header_include_names,
                 void(CL_CALLBACK *pfn_notify)(cl_program, void *),
                 void *user_data)
{
  cl_int err = CL_SUCCESS;

  do {
    if (!CL_OBJECT_IS_PROGRAM(program)) {
      err = CL_INVALID_PROGRAM;
      break;
    }

    if (num_devices == 0 && device_list != NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (num_devices != 0 && device_list == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (pfn_notify == 0 && user_data != NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (num_input_headers == 0 && input_headers != NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (num_input_headers != 0 && input_headers == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (device_list) {
      assert(program->ctx);
      err = cl_devices_list_check(num_devices, device_list);
      if (err != CL_SUCCESS)
        break;

      err = cl_devices_list_include_check(program->ctx->device_num,
                                          program->ctx->devices, num_devices, device_list);
      if (err != CL_SUCCESS)
        break;
    }

    err = cl_program_compile(program, num_input_headers, input_headers,
                             header_include_names, options, num_devices, device_list);
    if (err != CL_SUCCESS)
      break;

  } while (0);

  if (pfn_notify)
    pfn_notify(program, user_data);
  return err;
}

cl_program
clLinkProgram(cl_context context,
              cl_uint num_devices,
              const cl_device_id *device_list,
              const char *options,
              cl_uint num_input_programs,
              const cl_program *input_programs,
              void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
              void *user_data,
              cl_int *errcode_ret)
{
  cl_int err = CL_SUCCESS;
  cl_program program = NULL;
  cl_uint i = 0;

  do {
    if (!CL_OBJECT_IS_CONTEXT(context)) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (pfn_notify == 0 && user_data != NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (num_input_programs == 0 && input_programs != NULL) {
      err = CL_INVALID_VALUE;
      break;
    }
    if (num_input_programs != 0 && input_programs == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }
    if (num_input_programs == 0 && input_programs == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (num_devices == 0 && device_list != NULL) {
      err = CL_INVALID_VALUE;
      break;
    }
    if (num_devices != 0 && device_list == NULL) {
      err = CL_INVALID_VALUE;
      break;
    }

    if (device_list) {
      err = cl_devices_list_check(num_devices, device_list);
      if (err != CL_SUCCESS)
        break;

      err = cl_devices_list_include_check(context->device_num,
                                          context->devices, num_devices, device_list);
      if (err != CL_SUCCESS)
        break;
    }

    for (i = 0; i < num_input_programs; i++) {
      if (!CL_OBJECT_IS_PROGRAM(input_programs[i])) {
        err = CL_INVALID_PROGRAM;
        break;
      }
    }
    if (err != CL_SUCCESS)
      break;

    program = cl_program_link(context, num_devices, device_list,
                              num_input_programs, input_programs, options, &err);

  } while (0);

  if (pfn_notify)
    pfn_notify(program, user_data);

  if (errcode_ret)
    *errcode_ret = err;
  return program;
}

cl_int
clCreateKernelsInProgram(cl_program program,
                         cl_uint num_kernels,
                         cl_kernel *kernels,
                         cl_uint *num_kernels_ret)
{
  cl_int err = CL_SUCCESS;

  if (!CL_OBJECT_IS_PROGRAM(program))
    return CL_INVALID_PROGRAM;

  if (kernels == NULL && num_kernels_ret == 0)
    return CL_INVALID_VALUE;

  err = cl_program_create_kernels_in_program(program, num_kernels, kernels, num_kernels_ret);

  return err;
}

cl_program
clCreateProgramWithBuiltInKernels(cl_context context,
                                  cl_uint num_devices,
                                  const cl_device_id *device_list,
                                  const char *kernel_names,
                                  cl_int *errcode_ret)
{
  cl_program program = NULL;
  cl_int err = CL_SUCCESS;

  do {
    if (!CL_OBJECT_IS_CONTEXT(context)) {
      err = CL_INVALID_CONTEXT;
      break;
    }

    if (kernel_names == NULL || kernel_names[0] == 0) {
      err = CL_INVALID_VALUE;
      break;
    }

    err = cl_devices_list_check(num_devices, device_list);
    if (err != CL_SUCCESS)
      break;

    err = cl_devices_list_include_check(context->device_num,
                                        context->devices, num_devices, device_list);
    if (err != CL_SUCCESS)
      break;

    program = cl_program_create_with_built_in_kernles(context, num_devices, device_list,
                                                      kernel_names, &err);
  } while (0);

  if (errcode_ret)
    *errcode_ret = err;
  return program;
}

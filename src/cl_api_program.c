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
#include "cl_context.h"
#include "cl_program.h"
#include "cl_device_id.h"
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
    num_dev = program->ctx->device_num; // Just 1 dev now.
    src_ptr = &num_dev;
    src_size = sizeof(cl_uint);
  } else if (param_name == CL_PROGRAM_DEVICES) {
    src_ptr = program->ctx->devices;
    src_size = program->ctx->device_num * sizeof(cl_device_id);
  } else if (param_name == CL_PROGRAM_NUM_KERNELS) {
    kernels_num = program->ker_n;
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
    // TODO: need to refine this.
    cl_program_get_kernel_names(program, param_value_size, (char *)param_value, param_value_size_ret);
    return CL_SUCCESS;
  } else if (param_name == CL_PROGRAM_BINARY_SIZES) {
    if (program->binary == NULL) {
      if (program->binary_type == CL_PROGRAM_BINARY_TYPE_EXECUTABLE) {
        program->binary_sz = compiler_program_serialize_to_binary(program->opaque, &program->binary, 0);
      } else if (program->binary_type == CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT) {
        program->binary_sz = compiler_program_serialize_to_binary(program->opaque, &program->binary, 1);
      } else if (program->binary_type == CL_PROGRAM_BINARY_TYPE_LIBRARY) {
        program->binary_sz = compiler_program_serialize_to_binary(program->opaque, &program->binary, 2);
      } else {
        return CL_INVALID_BINARY;
      }
    }

    if (program->binary == NULL || program->binary_sz == 0) {
      return CL_OUT_OF_RESOURCES;
    }
    src_ptr = &program->binary_sz;
    src_size = sizeof(size_t);
  } else if (param_name == CL_PROGRAM_BINARIES) {
    if (param_value_size_ret)
      *param_value_size_ret = sizeof(void *);
    if (!param_value)
      return CL_SUCCESS;

    /* param_value points to an array of n
       pointers allocated by the caller */
    if (program->binary == NULL) {
      if (program->binary_type == CL_PROGRAM_BINARY_TYPE_EXECUTABLE) {
        program->binary_sz = compiler_program_serialize_to_binary(program->opaque, &program->binary, 0);
      } else if (program->binary_type == CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT) {
        program->binary_sz = compiler_program_serialize_to_binary(program->opaque, &program->binary, 1);
      } else if (program->binary_type == CL_PROGRAM_BINARY_TYPE_LIBRARY) {
        program->binary_sz = compiler_program_serialize_to_binary(program->opaque, &program->binary, 2);
      } else {
        return CL_INVALID_BINARY;
      }
    }

    if (program->binary == NULL || program->binary_sz == 0) {
      return CL_OUT_OF_RESOURCES;
    }

    memcpy(*((void **)param_value), program->binary, program->binary_sz);
    return CL_SUCCESS;
  } else {
    return CL_INVALID_VALUE;
  }

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
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
  size_t global_size;

  if (!CL_OBJECT_IS_PROGRAM(program)) {
    return CL_INVALID_PROGRAM;
  }

  cl_int err = cl_devices_list_include_check(program->ctx->device_num,
                                             program->ctx->devices, 1, &device);
  if (err != CL_SUCCESS)
    return err;

  if (param_name == CL_PROGRAM_BUILD_STATUS) {
    src_ptr = &program->build_status;
    src_size = sizeof(cl_build_status);
  } else if (param_name == CL_PROGRAM_BUILD_OPTIONS) {
    if (program->is_built && program->build_opts) {
      ret_str = program->build_opts;
    }
    src_ptr = ret_str;
    src_size = strlen(ret_str) + 1;
  } else if (param_name == CL_PROGRAM_BUILD_LOG) {
    src_ptr = program->build_log;
    src_size = program->build_log_sz + 1;
  } else if (param_name == CL_PROGRAM_BINARY_TYPE) {
    src_ptr = &program->binary_type;
    src_size = sizeof(cl_uint);
  } else if (param_name == CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE) {
    global_size = 0;
    if (program->is_built)
      global_size = cl_program_get_global_variable_size(program);
    src_ptr = &global_size;
    src_size = sizeof(global_size);
  } else {
    return CL_INVALID_VALUE;
  }

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
}

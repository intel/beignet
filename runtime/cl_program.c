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

#include "cl_kernel.h"
#include "cl_program.h"
#include "cl_device_id.h"
#include "cl_context.h"
#include "cl_alloc.h"
#include <string.h>

LOCAL cl_program
cl_program_new(cl_context ctx)
{
  cl_program p = NULL;
  int i;
  cl_int err = CL_SUCCESS;

  /* Allocate the structure */
  p = CL_CALLOC(1, sizeof(struct _cl_program));
  if (p == NULL)
    return NULL;

  CL_OBJECT_INIT_BASE(p, CL_OBJECT_PROGRAM_MAGIC);
  list_init(&p->kernels);

  p->each_device = CL_CALLOC(ctx->device_num, sizeof(cl_program_for_device));
  if (p->each_device == NULL) {
    CL_FREE(p);
    return NULL;
  }
  p->each_device_num = ctx->device_num;

  for (i = 0; i < ctx->device_num; i++) {
    err = (ctx->devices[i]->api.program_create)(ctx->devices[i], p);
    if (err != CL_SUCCESS) {
      break;
    }
  }

  if (err != CL_SUCCESS) {
    for (i = 0; i < ctx->device_num; i++) {
      if (p->each_device[i])
        (ctx->devices[i]->api.program_delete)(p->each_device[i]->device, p);
    }
    CL_FREE(p);
    return NULL;
  }

  p->build_status = CL_BUILD_NONE;
  /* The queue also belongs to its context */
  cl_context_add_program(ctx, p);
  return p;
}

LOCAL void
cl_program_add_ref(cl_program p)
{
  assert(p);
  CL_OBJECT_INC_REF(p);
}

LOCAL void
cl_program_delete(cl_program p)
{
  cl_uint i = 0;

  if (p == NULL)
    return;

  /* We are not done with it yet */
  if (CL_OBJECT_DEC_REF(p) > 1)
    return;

  /* Remove it from the context list */
  if (p->ctx)
    cl_context_remove_program(p->ctx, p);

  assert(p->ker_n == 0);
  assert(list_empty(&p->kernels));

  for (i = 0; i < p->each_device_num; i++) {
    if (p->each_device[i]->binary) {
      assert(p->each_device[i]->binary_sz > 0);
      CL_FREE(p->each_device[i]->binary);
      p->each_device[i]->binary = NULL;
      p->each_device[i]->binary_sz = 0;
    }
  }

  for (i = 0; i < p->each_device_num; i++) {
    if (p->each_device[i])
      (p->each_device[i]->device->api.program_delete)(p->each_device[i]->device, p);
  }
  CL_FREE(p->each_device);

  if (p->build_opts) {
    CL_FREE(p->build_opts);
    p->build_opts = NULL;
  }

  /* Destroy the sources and binary if still allocated */
  if (p->source) {
    CL_FREE(p->source);
    p->source = NULL;
  }

  CL_OBJECT_DESTROY_BASE(p);
  CL_FREE(p);
}

/* Before we do the real work, we need to check whether our platform
   cl version can meet -cl-std= */
static int check_cl_version_option(cl_program p, const char *options)
{
  const char *s = NULL;
  int ver1 = 0;
  int ver2 = 0;
  char version_str[64] = {0};

  if (options && (s = strstr(options, "-cl-std="))) {

    if (s + strlen("-cl-std=CLX.X") > options + strlen(options)) {
      return 0;
    }

    if (s[8] != 'C' || s[9] != 'L' || s[10] > '9' || s[10] < '0' ||
        s[11] != '.' || s[12] > '9' || s[12] < '0') {
      return 0;
    }

    ver1 = (s[10] - '0') * 10 + (s[12] - '0');

    if (cl_device_get_info(p->ctx->devices[0], CL_DEVICE_OPENCL_C_VERSION,
                           sizeof(version_str), version_str, NULL) != CL_SUCCESS)
      return 0;

    assert(strstr(version_str, "OpenCL") && version_str[0] == 'O');
    ver2 = (version_str[9] - '0') * 10 + (version_str[11] - '0');

    if (ver2 < ver1)
      return 0;

    return 1;
  }

  return 1;
}

static cl_uint
cl_program_get_kernel_num(cl_program p)
{
  cl_uint num;
  CL_OBJECT_LOCK(p);
  num = p->ker_n;
  CL_OBJECT_UNLOCK(p);
  return num;
}

static cl_int
cl_program_check_rebuild(cl_program p, cl_bool just_compile,
                         cl_uint num_devices, const cl_device_id *device_list)
{
  cl_device_id device;
  cl_program_for_device pd = NULL;
  cl_uint i;
  cl_uint j;
  cl_int err = CL_SUCCESS;

  if (p->source == NULL) // If no source, e.g from binary, we never rebuild
    return CL_SUCCESS;

  for (i = 0; i < num_devices; i++) {
    device = device_list[i];
    for (j = 0; j < p->each_device_num; j++) {
      if (device == p->each_device[j]->device) {
        pd = p->each_device[j];
        break;
      }
    }
    assert(pd);

    if (pd->binary_type == CL_PROGRAM_BINARY_TYPE_EXECUTABLE ||
        (just_compile && pd->binary_type == CL_PROGRAM_BINARY_TYPE_LIBRARY) ||
        (just_compile && pd->binary_type == CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT)) {
      assert(pd->binary);
      CL_FREE(pd->binary);
      pd->binary_sz = 0;
      (device->api.program_delete)(device, p);
      p->each_device[j] = NULL;
      err = (device->api.program_create)(device, p);
      if (err != CL_SUCCESS)
        return CL_OUT_OF_HOST_MEMORY;
    }
  }

  return CL_SUCCESS;
}

LOCAL cl_int
cl_program_build(cl_program p, const char *options, cl_uint num_devices,
                 const cl_device_id *device_list)
{
  cl_bool build_ret = CL_FALSE;
  cl_device_id device;
  cl_int ret = CL_SUCCESS;
  cl_uint i;
  cl_program_for_device pd = NULL;

  if (device_list == NULL) {
    assert(num_devices == 0);
    num_devices = p->ctx->device_num;
    device_list = p->ctx->devices;
  }

  if (!check_cl_version_option(p, options))
    return CL_INVALID_BUILD_OPTIONS;

  if (cl_program_get_kernel_num(p) > 0)
    return CL_INVALID_OPERATION;

  if (CL_OBJECT_TAKE_OWNERSHIP(p, CL_FALSE) == CL_FALSE)
    return CL_INVALID_OPERATION;

  if (options) {
    if (p->build_opts == NULL || strcmp(options, p->build_opts) != 0) {
      if (p->build_opts) {
        CL_FREE(p->build_opts);
        p->build_opts = NULL;
      }
      p->build_opts = CL_CALLOC(strlen(options) + 1, sizeof(char));
      if (p->build_opts == NULL) {
        return CL_OUT_OF_HOST_MEMORY;
      }

      memcpy(p->build_opts, options, strlen(options));
    }
  }

  if (options == NULL && p->build_opts) {
    CL_FREE(p->build_opts);
    p->build_opts = NULL;
  }

  if (p->build_status < CL_BUILD_NONE) {
    CL_OBJECT_RELEASE_OWNERSHIP(p);
    return CL_INVALID_OPERATION;
  }

  ret = cl_program_check_rebuild(p, CL_FALSE, num_devices, device_list);
  if (ret != CL_SUCCESS) {
    CL_OBJECT_RELEASE_OWNERSHIP(p);
    return ret;
  }

  /* Begin to build for each device */
  p->build_status = CL_BUILD_IN_PROGRESS;
  for (i = 0; i < num_devices; i++) {
    device = device_list[i];
    DEV_PRIVATE_DATA(p, device, pd);

    ret = cl_compiler_check_available(device);
    if (ret != CL_SUCCESS)
      break;

    if ((device->compiler.check_compiler_option)(options) == CL_FALSE) {
      ret = CL_INVALID_BUILD_OPTIONS;
      break;
    }

    if (pd->binary_type == CL_PROGRAM_BINARY_TYPE_NONE) { // Build from source on shot
      assert(pd->binary_sz == 0);
      assert(pd->binary == NULL);

      if (p->source == NULL) {
        ret = CL_INVALID_OPERATION;
        break;
      }

      assert(p->source_sz > 0);

      build_ret = (device->compiler.build_program)(device->device_id, p->source, p->source_sz, p->build_opts,
                                                   pd->build_log_max_sz, pd->build_log, &pd->build_log_sz,
                                                   &pd->binary, &pd->binary_sz);
      if (build_ret == CL_FALSE) {
        if (pd->build_log_sz > 0 && strstr(pd->build_log, "error: error reading 'options'"))
          ret = CL_INVALID_COMPILER_OPTIONS;
        else
          ret = CL_COMPILE_PROGRAM_FAILURE;

        break;
      }

      CL_REGISTER_ALLOC_PTR(pd->binary, pd->binary_sz);
    } else if (pd->binary_type == CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT) { // Build from IR binary
      char *tmp_binary = pd->binary;
      size_t tmp_binary_sz = pd->binary_sz;
      assert(tmp_binary_sz != 0);
      assert(tmp_binary != NULL);
      pd->binary = NULL;
      pd->binary_sz = 0;

      build_ret = (device->compiler.link_program)(device->device_id, 1, &tmp_binary, &tmp_binary_sz, p->build_opts,
                                                  pd->build_log_max_sz, pd->build_log, &pd->build_log_sz,
                                                  &pd->binary, &pd->binary_sz);
      CL_FREE(tmp_binary);

      if (build_ret == CL_FALSE) {
        if (pd->build_log_sz > 0 && strstr(pd->build_log, "error: error reading 'options'"))
          ret = CL_INVALID_COMPILER_OPTIONS;
        else
          ret = CL_COMPILE_PROGRAM_FAILURE;

        break;
      }

      CL_REGISTER_ALLOC_PTR(pd->binary, pd->binary_sz);
    } else if (pd->binary_type == CL_PROGRAM_BINARY_TYPE_EXECUTABLE) { // Already a exec format
      // rebuild will handle the created from source logic. If created from binary, we need to do nothing
      continue;
    } else {
      ret = CL_BUILD_PROGRAM_FAILURE;
      break;
    }

    pd->binary_type = CL_PROGRAM_BINARY_TYPE_EXECUTABLE;

    ret = (device->api.program_load_binary)(device, p);
    if (ret != CL_SUCCESS)
      break;
  }

  assert(p->ker_n == 0); // No kernels generated

  //  ret = cl_program_build_kernels_gen(device, p);
  if (ret != CL_SUCCESS) {
    p->build_status = CL_BUILD_ERROR;
    CL_OBJECT_RELEASE_OWNERSHIP(p);
    return ret;
  }

  p->build_status = CL_BUILD_SUCCESS;
  CL_OBJECT_RELEASE_OWNERSHIP(p);
  return ret;
}

LOCAL cl_int
cl_program_compile(cl_program p, cl_uint num_input_headers, const cl_program *input_headers,
                   const char **header_include_names, const char *options,
                   cl_uint num_devices, const cl_device_id *device_list)
{
  cl_int err = CL_SUCCESS;
  cl_bool build_ret = CL_FALSE;
  int i = 0;
  const char **headers = NULL;
  size_t *header_lengths = NULL;
  cl_device_id device;
  cl_program_for_device pd = NULL;

  if (device_list == NULL) {
    assert(num_devices == 0);
    num_devices = p->ctx->device_num;
    device_list = p->ctx->devices;
  }

  if (!check_cl_version_option(p, options))
    return CL_INVALID_COMPILER_OPTIONS;

  if (cl_program_get_kernel_num(p) > 0)
    return CL_INVALID_OPERATION;

  if (CL_OBJECT_TAKE_OWNERSHIP(p, CL_FALSE) == CL_FALSE)
    return CL_INVALID_OPERATION;

  if (p->build_status < CL_BUILD_NONE) { // Already did something?
    CL_OBJECT_RELEASE_OWNERSHIP(p);
    return CL_INVALID_OPERATION;
  }

  err = cl_program_check_rebuild(p, CL_TRUE, num_devices, device_list);
  if (err != CL_SUCCESS) {
    CL_OBJECT_RELEASE_OWNERSHIP(p);
    return err;
  }

  if (options) {
    if (p->build_opts == NULL || strcmp(options, p->build_opts) != 0) {
      if (p->build_opts) {
        CL_FREE(p->build_opts);
        p->build_opts = NULL;
      }
      p->build_opts = CL_CALLOC(strlen(options) + 1, sizeof(char));
      if (p->build_opts == NULL) {
        CL_OBJECT_RELEASE_OWNERSHIP(p);
        return CL_OUT_OF_HOST_MEMORY;
      }

      memcpy(p->build_opts, options, strlen(options));
    }
  }

  if (options == NULL && p->build_opts) {
    CL_FREE(p->build_opts);
    p->build_opts = NULL;
  }

  if (p->source == NULL) { // No source can build
    p->build_status = CL_BUILD_ERROR;
    CL_OBJECT_RELEASE_OWNERSHIP(p);
    return CL_INVALID_OPERATION;
  }

  if (num_input_headers) {
    headers = CL_CALLOC(num_input_headers, sizeof(void *));
    if (headers == NULL) {
      CL_OBJECT_RELEASE_OWNERSHIP(p);
      return CL_OUT_OF_HOST_MEMORY;
    }
    header_lengths = CL_CALLOC(num_input_headers, sizeof(size_t));
    if (header_lengths == NULL) {
      CL_FREE(headers);
      CL_OBJECT_RELEASE_OWNERSHIP(p);
      return CL_OUT_OF_HOST_MEMORY;
    }
  }

  p->build_status = CL_BUILD_IN_PROGRESS;

  for (i = 0; i < num_devices; i++) {
    device = device_list[i];
    DEV_PRIVATE_DATA(p, device, pd);

    assert(pd->binary_sz == 0);
    assert(pd->binary == NULL);

    if (pd->binary_type == CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT ||
        pd->binary_type == CL_PROGRAM_BINARY_TYPE_LIBRARY ||
        pd->binary_type == CL_PROGRAM_BINARY_TYPE_EXECUTABLE)
      continue;

    err = cl_compiler_check_available(device);
    if (err != CL_SUCCESS)
      break;

    if ((device->compiler.check_compiler_option)(options) == CL_FALSE) {
      err = CL_INVALID_COMPILER_OPTIONS;
      break;
    }

    for (i = 0; i < num_input_headers; i++) {
      headers[i] = input_headers[i]->source;
      header_lengths[i] = input_headers[i]->source_sz;
    }

    build_ret = (device->compiler.compile_program)(
      device->device_id, p->source, p->source_sz, headers, header_lengths,
      header_include_names, num_input_headers, p->build_opts, pd->build_log_max_sz,
      pd->build_log, &pd->build_log_sz, &pd->binary, &pd->binary_sz);

    if (build_ret == CL_FALSE) {
      if (pd->build_log_sz > 0 && strstr(pd->build_log, "error: error reading 'options'"))
        err = CL_INVALID_COMPILER_OPTIONS;
      else
        err = CL_COMPILE_PROGRAM_FAILURE;

      break;
    } else {
      pd->binary_type = CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT;
    }

    CL_REGISTER_ALLOC_PTR(pd->binary, pd->binary_sz);
  }

  if (err == CL_SUCCESS) {
    p->build_status = CL_BUILD_SUCCESS;
  } else {
    p->build_status = CL_BUILD_ERROR;
  }

  if (headers)
    CL_FREE(headers);
  if (header_lengths)
    CL_FREE(header_lengths);

  CL_OBJECT_RELEASE_OWNERSHIP(p);
  return err;
}

cl_program
cl_program_link(cl_context context, cl_uint num_devices, const cl_device_id *device_list,
                cl_uint num_input_programs, const cl_program *input_programs, const char *options,
                cl_int *errcode_ret)
{
  cl_program p = NULL;
  cl_int err = CL_SUCCESS;
  cl_uint i = 0;
  cl_uint j = 0;
  cl_uint k = 0;
  cl_bool build_ret = CL_FALSE;
  char **binary = NULL;
  size_t *binary_size = NULL;
  cl_device_id device;
  cl_program_for_device pd = NULL;
  cl_program_for_device pdi = NULL;
  cl_bool build_for_lib = CL_FALSE;

  if (device_list == NULL) {
    assert(num_devices == 0);
    num_devices = context->device_num;
    device_list = context->devices;
  }

  p = cl_program_new(context);
  if (p == NULL) {
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }

  if (options && options[0]) {
    p->build_opts = CL_CALLOC(strlen(options) + 1, sizeof(char));
    if (p->build_opts == NULL) {
      cl_program_delete(p);
      *errcode_ret = CL_OUT_OF_HOST_MEMORY;
      return NULL;
    }
    memcpy(p->build_opts, options, strlen(options) + 1);
  }

  binary = CL_CALLOC(num_input_programs, sizeof(char *));
  if (binary == NULL) {
    cl_program_delete(p);
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }
  binary_size = CL_CALLOC(num_input_programs, sizeof(size_t));
  if (binary_size == NULL) {
    cl_program_delete(p);
    CL_FREE(binary);
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }

  if (p->build_opts && strstr(p->build_opts, "-create-library"))
    build_for_lib = CL_TRUE;

  p->build_status = CL_BUILD_IN_PROGRESS;

  for (k = 0; k < num_input_programs; k++) {
    if (CL_OBJECT_TAKE_OWNERSHIP(input_programs[k], CL_FALSE) == CL_FALSE)
      break;
  }
  if (k != num_input_programs) { // Some one not ready
    for (i = 0; i < k; i++) {
      CL_OBJECT_RELEASE_OWNERSHIP(input_programs[i]);
    }
    cl_program_delete(p);
    CL_FREE(binary);
    CL_FREE(binary_size);
    *errcode_ret = CL_INVALID_VALUE;
    return NULL;
  }

  for (j = 0; j < num_devices; j++) {
    device = device_list[j];
    DEV_PRIVATE_DATA(p, device, pd);

    err = cl_compiler_check_available(device);
    if (err != CL_SUCCESS)
      break;

    if ((device->compiler.check_compiler_option)(options) == CL_FALSE) {
      err = CL_INVALID_LINKER_OPTIONS;
      break;
    }

    for (i = 0; i < num_input_programs; i++) {
      DEV_PRIVATE_DATA(input_programs[i], device, pdi);

      if (input_programs[i]->build_status < CL_BUILD_NONE) {
        err = CL_INVALID_OPERATION;
        break;
      }

      if ((pdi->binary_type != CL_PROGRAM_BINARY_TYPE_LIBRARY) &&
          (pdi->binary_type != CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT)) {
        err = CL_INVALID_OPERATION;
        break;
      }
    }
    if (err != CL_SUCCESS)
      break;

    for (i = 0; i < num_input_programs; i++) {
      DEV_PRIVATE_DATA(input_programs[i], device, pdi);
      binary[i] = pdi->binary;
      binary_size[i] = pdi->binary_sz;
    }

    build_ret = (device->compiler.link_program)(device->device_id, num_input_programs, binary, binary_size,
                                                options, pd->build_log_max_sz, pd->build_log, &pd->build_log_sz,
                                                &pd->binary, &pd->binary_sz);

    if (build_ret == CL_FALSE) {
      if (pd->build_log_sz > 0 && strstr(pd->build_log, "error: error reading 'options'"))
        err = CL_INVALID_COMPILER_OPTIONS;
      else
        err = CL_COMPILE_PROGRAM_FAILURE;

      break;
    }

    CL_REGISTER_ALLOC_PTR(pd->binary, pd->binary_sz);

    if (build_for_lib) { // Create a lib, no further work
      pd->binary_type = CL_PROGRAM_BINARY_TYPE_LIBRARY;
      continue;
    }

    pd->binary_type = CL_PROGRAM_BINARY_TYPE_EXECUTABLE;
    err = (device->api.program_load_binary)(device, p);
    if (err != CL_SUCCESS)
      break;
  }

  for (k = 0; k < num_input_programs; k++) {
    CL_OBJECT_RELEASE_OWNERSHIP(input_programs[k]);
  }

  if (binary)
    CL_FREE(binary);
  if (binary_size)
    CL_FREE(binary_size);
  if (err != CL_SUCCESS) {
    cl_program_delete(p);
    p = NULL;
  } else {
    p->build_status = CL_BUILD_SUCCESS;
  }

  *errcode_ret = err;
  return p;
}

LOCAL cl_int
cl_program_get_kernel_names(cl_program p, size_t *kerne_num, size_t size, char *names, size_t *name_ret)
{
  /* This function will get all possible kernel names, at least one device has it */
  char **known_kernel_list = NULL;
  int known_kernel_num = 0;
  int i, j, k;
  int total_sz = 0;
  char *ptr;

  if (CL_OBJECT_TAKE_OWNERSHIP(p, CL_FALSE) == CL_FALSE)
    return CL_INVALID_OPERATION;

  if (p->build_status != CL_BUILD_SUCCESS) {
    CL_OBJECT_RELEASE_OWNERSHIP(p);
    return CL_INVALID_PROGRAM_EXECUTABLE;
  }

  for (i = 0; i < p->each_device_num; i++) {
    if (p->each_device[i]->kernel_names == NULL)
      continue;

    if (known_kernel_list == NULL) {
      assert(known_kernel_num == 0);
      known_kernel_num = p->each_device[i]->kernel_num;
      known_kernel_list = CL_CALLOC(known_kernel_num, sizeof(char *));
      if (known_kernel_list == NULL)
        goto ERROR;

      memcpy(known_kernel_list, p->each_device[i]->kernel_names,
             p->each_device[i]->kernel_num * sizeof(char *));
      continue;
    }

    /* Compare the same name */
    for (j = 0; j < p->each_device[i]->kernel_num; j++) {
      k = 0;
      for (; k < known_kernel_num; k++) {
        if (strcmp(known_kernel_list[k], p->each_device[i]->kernel_names[j]) == 0)
          break;
      }

      if (k == known_kernel_num) { // Append a new one
        known_kernel_list = CL_REALLOC(known_kernel_list, (known_kernel_num + 1) * sizeof(char *));
        if (known_kernel_list == NULL)
          goto ERROR;

        known_kernel_list[known_kernel_num] = p->each_device[i]->kernel_names[j];
        known_kernel_num++;
      }
    }
  }

  assert(known_kernel_num > 0);

  if (kerne_num) {
    *kerne_num = known_kernel_num;
  }

  if (names == NULL && name_ret == NULL) {
    CL_OBJECT_RELEASE_OWNERSHIP(p);
    CL_FREE(known_kernel_list);
    return CL_SUCCESS;
  }

  total_sz = 0;
  for (i = 0; i < known_kernel_num; i++) {
    assert(known_kernel_list[i] && known_kernel_list[i][0] != 0);
    total_sz += strlen(known_kernel_list[i]) + 1;
  }

  if (name_ret)
    *name_ret = total_sz;

  if (names && size < total_sz) {
    CL_OBJECT_RELEASE_OWNERSHIP(p);
    CL_FREE(known_kernel_list);
    return CL_INVALID_VALUE;
  }

  if (names) {
    ptr = names;
    for (i = 0; i < known_kernel_num; i++) {
      memcpy(ptr, known_kernel_list[i], strlen(known_kernel_list[i]));
      ptr[strlen(known_kernel_list[i])] = ';';
      ptr = ptr + strlen(known_kernel_list[i]) + 1;
    }

    names[total_sz - 1] = 0;
    assert(ptr - names == total_sz);
  }

  CL_OBJECT_RELEASE_OWNERSHIP(p);
  CL_FREE(known_kernel_list);
  return CL_SUCCESS;

ERROR:
  if (known_kernel_list)
    CL_FREE(known_kernel_list);

  CL_OBJECT_RELEASE_OWNERSHIP(p);
  return CL_OUT_OF_HOST_MEMORY;
}

LOCAL cl_program
cl_program_create_from_binary(cl_context ctx, cl_uint num_devices, const cl_device_id *devices,
                              const size_t *lengths, const unsigned char **binaries,
                              cl_int *binary_status, cl_int *errcode_ret)
{
  cl_program program = NULL;
  cl_program_for_device pd = NULL;
  cl_int err = CL_SUCCESS;
  cl_uint i;

  assert(ctx);

  program = cl_program_new(ctx);
  if (program == NULL) {
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }

  for (i = 0; i < num_devices; i++) {
    DEV_PRIVATE_DATA(program, devices[i], pd);
    pd->binary = CL_MALLOC(lengths[i] * sizeof(char));
    if (pd->binary == NULL) {
      if (binary_status) {
        binary_status[i] = CL_INVALID_VALUE; // Just set to this, no other err kind for it
      }

      err = CL_OUT_OF_HOST_MEMORY;
      break;
    }
    memcpy(pd->binary, binaries[i], lengths[i]);
    pd->binary_sz = lengths[i];
    err = (devices[i]->api.program_load_binary)(devices[i], program);
    if (binary_status) {
      binary_status[i] = err;
    }

    if (err != CL_SUCCESS) {
      break;
    }

    assert(pd->binary_type != CL_PROGRAM_BINARY_TYPE_NONE); // Must be something
  }

  *errcode_ret = err;
  if (err != CL_SUCCESS) {
    cl_program_delete(program);
    return NULL;
  }
  return program;
}

LOCAL cl_program
cl_program_create_with_built_in_kernles(cl_context ctx,
                                        cl_uint num_devices,
                                        const cl_device_id *devices,
                                        const char *kernel_names,
                                        cl_int *errcode_ret)
{

  cl_uint i;
  char *required_names = NULL;
  char *name = NULL;
  char *ptr;
  int find;
  cl_int err = CL_SUCCESS;
  size_t *lengths;
  const unsigned char **binaries;
  cl_program prog = NULL;

  assert(ctx);

  required_names = CL_MALLOC(strlen(kernel_names) + 1);
  if (required_names == NULL) {
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }
  memcpy(required_names, kernel_names, strlen(kernel_names) + 1);

  name = strtok(required_names, ";");
  assert(name);

  // check whether all kernel names are included in all devices's binary
  while (name) {
    for (i = 0; i < num_devices; i++) {
      if (devices[i]->built_in_kernels == NULL) {
        CL_FREE(required_names);
        *errcode_ret = CL_INVALID_VALUE;
        return NULL;
      }

      ptr = NULL;
      find = 0;

      ptr = strstr(devices[i]->built_in_kernels, name);
      while (ptr != NULL) {
        /* Need to be whole match */
        if (ptr != devices[i]->built_in_kernels && *(ptr - 1) != ';') { // Not the frist one
          ptr = strstr(ptr, name);
          continue;
        }

        if (ptr[strlen(name)] != ';' && ptr[strlen(name)] != 0) {
          ptr = strstr(ptr, name);
          continue;
        }

        find = 1;
        break;
      }

      if (find == 0) {
        CL_FREE(required_names);
        *errcode_ret = CL_INVALID_VALUE;
        return NULL;
      }
    }

    name = strtok(NULL, ";");
  }

  CL_FREE(required_names);

  /* OK, all kernels' name supported, create program */
  lengths = CL_CALLOC(num_devices, sizeof(size_t));
  if (lengths == NULL) {
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }
  binaries = CL_CALLOC(num_devices, sizeof(char *));
  if (binaries == NULL) {
    CL_FREE(lengths);
    *errcode_ret = CL_OUT_OF_HOST_MEMORY;
    return NULL;
  }

  for (i = 0; i < num_devices; i++) {
    lengths[i] = devices[i]->built_in_kernels_binary_sz;
    binaries[i] = (unsigned char *)devices[i]->built_in_kernels_binary;
  }

  prog = cl_program_create_from_binary(ctx, num_devices, devices,
                                       lengths, binaries, NULL, &err);

  CL_FREE(lengths);
  CL_FREE(binaries);

  if (prog) {
    assert(err == CL_SUCCESS);
    err = cl_program_build(prog, NULL, num_devices, devices);
    if (err != CL_SUCCESS) {
      cl_program_delete(prog);
      prog = NULL;
    }
  }

  *errcode_ret = err;
  return prog;
}

LOCAL cl_program
cl_program_create_from_llvm(cl_context ctx,
                            cl_uint num_devices,
                            const cl_device_id *devices,
                            const char *file_name,
                            cl_int *errcode_ret)
{
  return NULL;
}

LOCAL cl_int
cl_program_create_kernels_in_program(cl_program program, cl_uint num_kernels,
                                     cl_kernel *kernels, cl_uint *num_kernels_ret)
{
  cl_int err = CL_SUCCESS;
  char **known_kernel_list = NULL;
  int known_kernel_num = 0;
  int all_kernel_num = 0;
  int i, j, k;

  if (CL_OBJECT_TAKE_OWNERSHIP(program, CL_FALSE) == CL_FALSE)
    return CL_INVALID_OPERATION;

  for (i = 0; i < program->each_device_num; i++) {
    if (program->each_device[i]->kernel_names == NULL) {
      all_kernel_num = 0;
      break;
    }

    if (known_kernel_list == NULL) {
      assert(known_kernel_num == 0);
      known_kernel_num = program->each_device[i]->kernel_num;
      known_kernel_list = CL_CALLOC(known_kernel_num, sizeof(char *));
      if (known_kernel_list == NULL) {
        CL_OBJECT_RELEASE_OWNERSHIP(program);
        return CL_OUT_OF_HOST_MEMORY;
      }

      all_kernel_num = known_kernel_num;
      memcpy(known_kernel_list, program->each_device[i]->kernel_names,
             program->each_device[i]->kernel_num * sizeof(char *));
      continue;
    }

    /* Find kernels name availible for all devices */
    all_kernel_num = known_kernel_num;
    for (k = 0; k < known_kernel_num; k++) {
      if (known_kernel_list[k] == NULL)
        continue;

      for (j = 0; j < program->each_device[i]->kernel_num; j++) {
        if (strcmp(known_kernel_list[k], program->each_device[i]->kernel_names[j]) == 0)
          break;
      }

      if (j == program->each_device[i]->kernel_num) { // Not found
        known_kernel_list[k] = NULL;
        all_kernel_num--;
      }
    }
  }

  if (all_kernel_num == 0) {
    if (known_kernel_list)
      CL_FREE(known_kernel_list);

    CL_OBJECT_RELEASE_OWNERSHIP(program);
    return CL_INVALID_PROGRAM_EXECUTABLE;
  }

  assert(known_kernel_list);
  if (kernels && all_kernel_num > num_kernels) {
    CL_OBJECT_RELEASE_OWNERSHIP(program);
    CL_FREE(known_kernel_list);
    return CL_INVALID_VALUE;
  }

  if (num_kernels_ret)
    *num_kernels_ret = all_kernel_num;

  if (kernels == NULL) { // Done
    CL_OBJECT_RELEASE_OWNERSHIP(program);
    CL_FREE(known_kernel_list);
    return CL_SUCCESS;
  }
  CL_OBJECT_RELEASE_OWNERSHIP(program);

  /* Create each kernel */
  j = 0;
  for (i = 0; i < known_kernel_num; i++) {
    if (known_kernel_list[i] == NULL)
      continue;

    kernels[j] = cl_kernel_create(program, known_kernel_list[i], &err);
    if (err != CL_SUCCESS)
      break;

    j++;
  }

  if (err != CL_SUCCESS) {
    for (i = 0; i < j; i++) {
      assert(kernels[i] != NULL);
      cl_kernel_delete(kernels[i]);
      kernels[i] = NULL;
    }
  }

  CL_FREE(known_kernel_list);
  return err;
}

/* Some program are internal used, we need to cache them all the time.
   NOT a MT safe and user need to pay attention to its reference and device list.
   This kind of program can not be built and compiled. */
LOCAL void
cl_program_take_out_of_context(cl_program p)
{
  assert(p->ctx);

  /* Remove it from the context list */
  cl_context_remove_program(p->ctx, p);
}

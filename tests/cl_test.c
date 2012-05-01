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

#include "cl_file_map.h"
#include "cl_test.h"
#include "common.h"
#include "CL/cl.h"
#include "CL/cl_intel.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define FATAL(...)                                          \
do {                                                        \
  fprintf(stderr, "error: ");                               \
  fprintf(stderr, __VA_ARGS__);                             \
  assert(0);                                                \
  exit(-1);                                                 \
} while (0)

#define FATAL_IF(COND, ...)                                 \
do {                                                        \
  if (COND) FATAL(__VA_ARGS__);                             \
} while (0)

cl_platform_id platform;
cl_device_id device;
cl_context ctx;
cl_program program;
cl_kernel kernel;
cl_command_queue queue;

static const char*
cl_test_channel_order_string(cl_channel_order order)
{
  switch(order) {
#define DECL_ORDER(WHICH) case CL_##WHICH: return "CL_"#WHICH
    DECL_ORDER(R);
    DECL_ORDER(A);
    DECL_ORDER(RG);
    DECL_ORDER(RA);
    DECL_ORDER(RGB);
    DECL_ORDER(RGBA);
    DECL_ORDER(BGRA);
    DECL_ORDER(ARGB);
    DECL_ORDER(INTENSITY);
    DECL_ORDER(LUMINANCE);
    DECL_ORDER(Rx);
    DECL_ORDER(RGx);
    DECL_ORDER(RGBx);
#undef DECL_ORDER
    default: return "Unsupported image channel order";
  };
}

static const char*
cl_test_channel_type_string(cl_channel_type type)
{
  switch(type) {
#define DECL_TYPE(WHICH) case CL_##WHICH: return "CL_"#WHICH
    DECL_TYPE(SNORM_INT8);
    DECL_TYPE(SNORM_INT16);
    DECL_TYPE(UNORM_INT8);
    DECL_TYPE(UNORM_INT16);
    DECL_TYPE(UNORM_SHORT_565);
    DECL_TYPE(UNORM_SHORT_555);
    DECL_TYPE(UNORM_INT_101010);
    DECL_TYPE(SIGNED_INT8);
    DECL_TYPE(SIGNED_INT16);
    DECL_TYPE(SIGNED_INT32);
    DECL_TYPE(UNSIGNED_INT8);
    DECL_TYPE(UNSIGNED_INT16);
    DECL_TYPE(UNSIGNED_INT32);
    DECL_TYPE(HALF_FLOAT);
    DECL_TYPE(FLOAT);
#undef DECL_TYPE
    default: return "Unsupported image channel type";
  };
}

int
cl_test_init(const char *file_name, const char *kernel_name, int format)
{
  cl_file_map_t *fm = NULL;
  cl_int status = CL_SUCCESS;
  char *ker_path = NULL;
  char name[128];
  cl_uint platform_n;
  size_t i;

  /* Get the platform number */
  CALL (clGetPlatformIDs, 0, NULL, &platform_n);
  printf("platform number %u\n", platform_n);
  assert(platform_n >= 1);

  /* Get a valid platform */
  CALL (clGetPlatformIDs, 1, &platform, &platform_n);
  CALL (clGetPlatformInfo, platform, CL_PLATFORM_PROFILE, sizeof(name), name, NULL);
  printf("platform_profile \"%s\"\n", name);
  CALL (clGetPlatformInfo, platform, CL_PLATFORM_NAME, sizeof(name), name, NULL);
  printf("platform_name \"%s\"\n", name);
  CALL (clGetPlatformInfo, platform, CL_PLATFORM_VENDOR, sizeof(name), name, NULL);
  printf("platform_vendor \"%s\"\n", name);
  CALL (clGetPlatformInfo, platform, CL_PLATFORM_VERSION, sizeof(name), name, NULL);
  printf("platform_version \"%s\"\n", name);

  /* Get the device (only GPU device is supported right now) */
  CALL (clGetDeviceIDs, platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  CALL (clGetDeviceInfo, device, CL_DEVICE_PROFILE, sizeof(name), name, NULL);
  printf("device_profile \"%s\"\n", name);
  CALL (clGetDeviceInfo, device, CL_DEVICE_NAME, sizeof(name), name, NULL);
  printf("device_name \"%s\"\n", name);
  CALL (clGetDeviceInfo, device, CL_DEVICE_VENDOR, sizeof(name), name, NULL);
  printf("device_vendor \"%s\"\n", name);
  CALL (clGetDeviceInfo, device, CL_DEVICE_VERSION, sizeof(name), name, NULL);
  printf("device_version \"%s\"\n", name);

  /* Now create a context */
  ctx = clCreateContext(0, 1, &device, NULL, NULL, &status);
  if (status != CL_SUCCESS) {
    fprintf(stderr, "error calling clCreateContext\n");
    goto error;
  }

  /* All image types currently supported by the context */
  cl_image_format fmt[256];
  cl_uint fmt_n;
  clGetSupportedImageFormats(ctx, 0, CL_MEM_OBJECT_IMAGE2D, 256, fmt, &fmt_n);
  printf("%u image formats are supported\n", fmt_n);
  for (i = 0; i < fmt_n; ++i)
    printf("[%s %s]\n",
        cl_test_channel_order_string(fmt[i].image_channel_order),
        cl_test_channel_type_string(fmt[i].image_channel_data_type));

  /* Load the program and build it */
  ker_path = do_kiss_path(file_name, device);
  if (format == LLVM)
    program = clCreateProgramWithLLVM(ctx, 1, &device, ker_path, &status);
  else if (format == SOURCE) {
    cl_file_map_t *fm = cl_file_map_new();
    FATAL_IF (cl_file_map_open(fm, ker_path) != CL_FILE_MAP_SUCCESS,
              "Failed to open file");
    const char *src = cl_file_map_begin(fm);
    const size_t sz = cl_file_map_size(fm);
    program = clCreateProgramWithSource(ctx, 1, &src, &sz, &status);
  } else
    FATAL("Not able to create program from binary");

  if (status != CL_SUCCESS) {
    fprintf(stderr, "error calling clCreateProgramWithBinary\n");
    goto error;
  }

  /* OCL requires to build the program even if it is created from a binary */
  CALL (clBuildProgram, program, 1, &device, NULL, NULL, NULL);

  /* Create a kernel from the program */
  kernel = clCreateKernel(program, kernel_name, &status);
  if (status != CL_SUCCESS) {
    fprintf(stderr, "error calling clCreateKernel\n");
    goto error;
  }

  /* We are going to push NDRange kernels here */
  queue = clCreateCommandQueue(ctx, device, 0, &status);
  if (status != CL_SUCCESS) {
    fprintf(stderr, "error calling clCreateCommandQueue\n");
    goto error;
  }

exit:
  free(ker_path);
  cl_file_map_delete(fm);
  return status;
error:
  goto exit;
}

void
cl_test_destroy(void)
{
  clReleaseCommandQueue(queue);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseContext(ctx);
}

static const char *err_msg[] = {
  [-CL_SUCCESS] = "CL_SUCCESS",
  [-CL_DEVICE_NOT_FOUND] = "CL_DEVICE_NOT_FOUND",
  [-CL_DEVICE_NOT_AVAILABLE] = "CL_DEVICE_NOT_AVAILABLE",
  [-CL_COMPILER_NOT_AVAILABLE] = "CL_COMPILER_NOT_AVAILABLE",
  [-CL_MEM_ALLOCATION_FAILURE] = "CL_MEM_ALLOCATION_FAILURE",
  [-CL_OUT_OF_RESOURCES] = "CL_OUT_OF_RESOURCES",
  [-CL_OUT_OF_HOST_MEMORY] = "CL_OUT_OF_HOST_MEMORY",
  [-CL_PROFILING_INFO_NOT_AVAILABLE] = "CL_PROFILING_INFO_NOT_AVAILABLE",
  [-CL_MEM_COPY_OVERLAP] = "CL_MEM_COPY_OVERLAP",
  [-CL_IMAGE_FORMAT_MISMATCH] = "CL_IMAGE_FORMAT_MISMATCH",
  [-CL_IMAGE_FORMAT_NOT_SUPPORTED] = "CL_IMAGE_FORMAT_NOT_SUPPORTED",
  [-CL_BUILD_PROGRAM_FAILURE] = "CL_BUILD_PROGRAM_FAILURE",
  [-CL_MAP_FAILURE] = "CL_MAP_FAILURE",
  [-CL_MISALIGNED_SUB_BUFFER_OFFSET] = "CL_MISALIGNED_SUB_BUFFER_OFFSET",
  [-CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST] = "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST",
  [-CL_INVALID_VALUE] = "CL_INVALID_VALUE",
  [-CL_INVALID_DEVICE_TYPE] = "CL_INVALID_DEVICE_TYPE",
  [-CL_INVALID_PLATFORM] = "CL_INVALID_PLATFORM",
  [-CL_INVALID_DEVICE] = "CL_INVALID_DEVICE",
  [-CL_INVALID_CONTEXT] = "CL_INVALID_CONTEXT",
  [-CL_INVALID_QUEUE_PROPERTIES] = "CL_INVALID_QUEUE_PROPERTIES",
  [-CL_INVALID_COMMAND_QUEUE] = "CL_INVALID_COMMAND_QUEUE",
  [-CL_INVALID_HOST_PTR] = "CL_INVALID_HOST_PTR",
  [-CL_INVALID_MEM] = "CL_INVALID_MEM",
  [-CL_INVALID_IMAGE_FORMAT_DESCRIPTOR] = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
  [-CL_INVALID_IMAGE_SIZE] = "CL_INVALID_IMAGE_SIZE",
  [-CL_INVALID_SAMPLER] = "CL_INVALID_SAMPLER",
  [-CL_INVALID_BINARY] = "CL_INVALID_BINARY",
  [-CL_INVALID_BUILD_OPTIONS] = "CL_INVALID_BUILD_OPTIONS",
  [-CL_INVALID_PROGRAM] = "CL_INVALID_PROGRAM",
  [-CL_INVALID_PROGRAM_EXECUTABLE] = "CL_INVALID_PROGRAM_EXECUTABLE",
  [-CL_INVALID_KERNEL_NAME] = "CL_INVALID_KERNEL_NAME",
  [-CL_INVALID_KERNEL_DEFINITION] = "CL_INVALID_KERNEL_DEFINITION",
  [-CL_INVALID_KERNEL] = "CL_INVALID_KERNEL",
  [-CL_INVALID_ARG_INDEX] = "CL_INVALID_ARG_INDEX",
  [-CL_INVALID_ARG_VALUE] = "CL_INVALID_ARG_VALUE",
  [-CL_INVALID_ARG_SIZE] = "CL_INVALID_ARG_SIZE",
  [-CL_INVALID_KERNEL_ARGS] = "CL_INVALID_KERNEL_ARGS",
  [-CL_INVALID_WORK_DIMENSION] = "CL_INVALID_WORK_DIMENSION",
  [-CL_INVALID_WORK_GROUP_SIZE] = "CL_INVALID_WORK_GROUP_SIZE",
  [-CL_INVALID_WORK_ITEM_SIZE] = "CL_INVALID_WORK_ITEM_SIZE",
  [-CL_INVALID_GLOBAL_OFFSET] = "CL_INVALID_GLOBAL_OFFSET",
  [-CL_INVALID_EVENT_WAIT_LIST] = "CL_INVALID_EVENT_WAIT_LIST",
  [-CL_INVALID_EVENT] = "CL_INVALID_EVENT",
  [-CL_INVALID_OPERATION] = "CL_INVALID_OPERATION",
  [-CL_INVALID_GL_OBJECT] = "CL_INVALID_GL_OBJECT",
  [-CL_INVALID_BUFFER_SIZE] = "CL_INVALID_BUFFER_SIZE",
  [-CL_INVALID_MIP_LEVEL] = "CL_INVALID_MIP_LEVEL",
  [-CL_INVALID_GLOBAL_WORK_SIZE] = "CL_INVALID_GLOBAL_WORK_SIZE",
  [-CL_INVALID_PROPERTY] = "CL_INVALID_PROPERTY"
};
static const size_t err_msg_n = sizeof(err_msg) / sizeof(err_msg[0]);

void
cl_report_error(cl_int err)
{
  if (err > 0)
    return;
  if (-err > err_msg_n)
    return;
  if (err == CL_SUCCESS)
    return;
  fprintf(stderr, "error %s\n", err_msg[-err]);
}

void
cl_report_perf_counters(cl_mem perf)
{
  cl_int status = CL_SUCCESS;
  uint32_t *start = NULL, *end = NULL;
  uint32_t i;
  if (perf == NULL)
    return;
  start = clIntelMapBuffer(perf, &status);
  assert(status == CL_SUCCESS && start != NULL);
  end = start + 128;

  printf("BEFORE\n");
  for (i = 0; i < 6*8; ++i) {
    if (i % 8 == 0) printf("\n");
    printf("[%3u 0x%8x] ", i, start[i]);
  }
  printf("\n\n");

  printf("AFTER\n");
  for (i = 0; i < 6*8; ++i) {
    if (i % 8 == 0) printf("\n");
    printf("[%3u 0x%8x] ", i, end[i]);
  }
  printf("\n\n");

  printf("DIFF\n");
  for (i = 0; i < 6*8; ++i) {
    if (i % 8 == 0) printf("\n");
    printf("[%3u %8i] ", i, end[i] - start[i]);
  }
  printf("\n\n");

  clIntelUnmapBuffer(perf);
}


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

/**
 * \file utest_helper.hpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __UTEST_HELPER_HPP__
#define __UTEST_HELPER_HPP__

#include "CL/cl.h"
#include "CL/cl_intel.h"
#include "utest.hpp"
#include "utest_assert.hpp"
#include "utest_error.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define OCL_THROW_ERROR(FN, STATUS) \
  do { \
    char msg[2048]; \
    sprintf(msg, "error calling %s with error%s \n", #FN, err_msg[-STATUS]); \
    OCL_ASSERTM(false, msg); \
  } while (0)

#define OCL_CALL(FN, ...) \
  do { \
    int status = FN(__VA_ARGS__); \
    if (status != CL_SUCCESS) OCL_THROW_ERROR(FN, status); \
  } while (0)

#define OCL_CREATE_KERNEL(NAME) \
  do { \
    OCL_CALL (cl_kernel_init, NAME".cl", NAME, SOURCE); \
  } while (0)

#define OCL_CREATE_BUFFER(BUFFER, FLAGS, SIZE, DATA) \
  do { \
    cl_int status; \
    BUFFER = clCreateBuffer(ctx, FLAGS, SIZE, DATA, &status); \
    if (status != CL_SUCCESS) OCL_THROW_ERROR(FN, status); \
  } while (0)

#define OCL_MAP_BUFFER(ID) \
  do { \
    cl_int status; \
    buf_data[ID] = (int *) clIntelMapBuffer(buf[ID], &status); \
    if (status != CL_SUCCESS) OCL_THROW_ERROR(FN, status); \
  } while (0)

#define OCL_UNMAP_BUFFER(ID) \
  do { \
    OCL_CALL (clIntelUnmapBuffer, buf[ID]); \
    buf_data[ID] = NULL; \
  } while (0)

#define OCL_NDRANGE(DIM_N) \
  do { \
    OCL_CALL (clEnqueueNDRangeKernel, queue, kernel, DIM_N, NULL, globals, locals, 0, NULL, NULL); \
  } while (0)

#define OCL_SET_ARG(ID, SIZE, ARG) \
  do { \
    OCL_CALL (clSetKernelArg, kernel, ID, SIZE, ARG); \
  } while (0)

enum { MAX_BUFFER_N = 16 };
extern cl_platform_id platform;
extern cl_device_id device;
extern cl_context ctx;
extern cl_program program;
extern cl_kernel kernel;
extern cl_command_queue queue;
extern cl_mem buf[MAX_BUFFER_N];
extern void* buf_data[MAX_BUFFER_N];
extern size_t globals[3];
extern size_t locals[3];

enum {
  SOURCE = 0,
  LLVM = 1,
  BIN = 2
};

/* Init OpenCL */
extern int cl_ocl_init(void);

/* Init program and kernel for the test */
extern int cl_kernel_init(const char *file_name, const char *kernel_name, int format);

/* init the bunch of global varaibles here */
extern int cl_test_init(const char *file_name, const char *kernel_name, int format);

/* Unmap and release all the created buffers */
extern void cl_buffer_destroy(void);

/* Release OCL queue, context and device */
extern void cl_ocl_destroy(void);

/* Release kernel and program */
extern void cl_kernel_destroy(void);

/* Release everything allocated in cl_test_init */
extern void cl_test_destroy(void);

/* Nicely output the performance counters */
extern void cl_report_perf_counters(cl_mem perf);

#endif /* __UTEST_HELPER_HPP__ */


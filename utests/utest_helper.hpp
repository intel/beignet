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

/**
 * \file utest_helper.hpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __UTEST_HELPER_HPP__
#define __UTEST_HELPER_HPP__

#include "CL/cl.h"
#include "CL/cl_ext.h"
#include "CL/cl_intel.h"
#include "utest.hpp"
#include "utest_assert.hpp"
#include "utest_error.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

#if defined(__ANDROID__)
#define __thread
#endif

#ifdef HAS_GL_EGL_X11
#define EGL_WINDOW_WIDTH 256
#define EGL_WINDOW_HEIGHT 256
#define GL_GLEXT_PROTOTYPES
#include  <GL/gl.h>
#include  <GL/glext.h>
#include  <EGL/egl.h>
#include  <EGL/eglext.h>
#include  <CL/cl_gl.h>

extern EGLDisplay  eglDisplay;
extern EGLContext  eglContext;
extern EGLSurface  eglSurface;
extern Display *xDisplay;
extern Window xWindow;
#endif

union uint32_cast {
  uint32_t _uint;
  float _float;
};

#define OCL_THROW_ERROR(FN, STATUS) \
  do { \
    char msg[2048]; \
    sprintf(msg, "error calling %s with error %s \n", #FN, err_msg[-STATUS]); \
    OCL_ASSERTM(false, msg); \
  } while (0)

#define OCL_CALL(FN, ...) \
  do { \
    int status = FN(__VA_ARGS__); \
    if (status != CL_SUCCESS) OCL_THROW_ERROR(FN, status); \
  } while (0)

#define OCL_CREATE_KERNEL(NAME) \
  do { \
    OCL_CALL (cl_kernel_init, NAME".cl", NAME, SOURCE, NULL); \
  } while (0)

#define OCL_DESTROY_KERNEL_KEEP_PROGRAM(KEEP_PROGRAM) \
  do { \
    cl_kernel_destroy(!(KEEP_PROGRAM)); \
  } while(0)

#define OCL_CREATE_KERNEL_FROM_FILE(FILE_NAME, KERNEL_NAME) \
  do { \
    OCL_CALL(cl_kernel_init, FILE_NAME".cl", KERNEL_NAME, SOURCE, NULL); \
  } while (0)

#define OCL_FLUSH() \
  do { \
    OCL_CALL(clFlush, queue); \
  } while(0)

#define OCL_FINISH() \
  do { \
    OCL_CALL(clFinish, queue); \
  } while(0)

#define OCL_CALL2(FN, RET, ...) \
  do { \
    cl_int status; \
    RET = FN(__VA_ARGS__, &status);\
    if (status != CL_SUCCESS) OCL_THROW_ERROR(FN, status); \
  } while (0)

#define OCL_CREATE_BUFFER(BUFFER, FLAGS, SIZE, DATA) \
        OCL_CALL2(clCreateBuffer, BUFFER, ctx, FLAGS, SIZE, DATA)

#define OCL_CREATE_USER_EVENT(EVENT) \
    OCL_CALL2(clCreateUserEvent, EVENT, ctx)

#define OCL_SET_USER_EVENT_STATUS(EVENT, STATUS) \
    OCL_CALL(clSetUserEventStatus, EVENT, STATUS)

#define OCL_CREATE_IMAGE(IMAGE, FLAGS, FORMAT, DESC, DATA) \
    OCL_CALL2(clCreateImage, IMAGE, ctx, FLAGS, FORMAT, DESC, DATA)

#define OCL_READ_IMAGE(IMAGE, ORIGIN, REGION, DATA) \
    OCL_CALL(clEnqueueReadImage, queue, IMAGE, CL_TRUE, ORIGIN, REGION, 0, 0, DATA, 0, NULL, NULL)

#define OCL_WRITE_IMAGE(IMAGE, ORIGIN, REGION, DATA) \
    OCL_CALL(clEnqueueWriteImage, queue, IMAGE, CL_TRUE, ORIGIN, REGION, 0, 0, DATA, 0, NULL, NULL)

#define OCL_CREATE_GL_IMAGE(IMAGE, FLAGS, TARGET, LEVEL, TEXTURE) \
    OCL_CALL2(clCreateFromGLTexture, IMAGE, ctx, FLAGS, TARGET, LEVEL, TEXTURE)

#define OCL_ENQUEUE_ACQUIRE_GL_OBJECTS(ID) \
    OCL_CALL(clEnqueueAcquireGLObjects, queue, 1, &buf[ID], 0, 0, 0)

#define OCL_ENQUEUE_RELEASE_GL_OBJECTS(ID) \
    OCL_CALL(clEnqueueReleaseGLObjects, queue, 1, &buf[ID], 0, 0, 0)

#define OCL_SWAP_EGL_BUFFERS() \
  eglSwapBuffers(eglDisplay, eglSurface);

#define OCL_CREATE_SAMPLER(SAMPLER, ADDRESS_MODE, FILTER_MODE)          \
    OCL_CALL2(clCreateSampler, SAMPLER, ctx, 0, ADDRESS_MODE, FILTER_MODE)

#define OCL_CALL_MAP(FN, ID, RET, ...) \
  do { \
    cl_int status; \
    size_t size = 0; \
    status = clGetMemObjectInfo(buf[ID], CL_MEM_SIZE, sizeof(size), &size, NULL);\
    if (status != CL_SUCCESS) OCL_THROW_ERROR(FN, status); \
    RET = FN(__VA_ARGS__, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, size, 0, NULL, NULL, &status);\
    if (status != CL_SUCCESS) OCL_THROW_ERROR(FN, status); \
  } while (0)

#define OCL_MAP_BUFFER(ID) \
    OCL_CALL_MAP(clEnqueueMapBuffer, ID, buf_data[ID], queue, buf[ID])

#define OCL_UNMAP_BUFFER(ID) \
  do { \
    if (buf[ID] != NULL) { \
      OCL_CALL (clEnqueueUnmapMemObject, queue, buf[ID], buf_data[ID], 0, NULL, NULL); \
      buf_data[ID] = NULL; \
    } \
  } while (0)

#define OCL_CALL_MAP_GTT(FN, ID, RET, ...) \
  do { \
    cl_int status; \
    size_t image_row_pitch = 0; \
    status = clGetImageInfo(buf[ID], CL_IMAGE_ROW_PITCH, sizeof(image_row_pitch), &image_row_pitch, NULL);\
    if (status != CL_SUCCESS) OCL_THROW_ERROR(FN, status); \
    size_t image_slice_pitch = 0; \
    status = clGetImageInfo(buf[ID], CL_IMAGE_ROW_PITCH, sizeof(image_slice_pitch), &image_slice_pitch, NULL);\
    if (status != CL_SUCCESS) OCL_THROW_ERROR(FN, status); \
    size_t image_width = 0; \
    status = clGetImageInfo(buf[ID], CL_IMAGE_WIDTH, sizeof(image_width), &image_width, NULL);\
    if (status != CL_SUCCESS) OCL_THROW_ERROR(FN, status); \
    size_t image_height = 0; \
    status = clGetImageInfo(buf[ID], CL_IMAGE_HEIGHT, sizeof(image_height), &image_height, NULL);\
    if (status != CL_SUCCESS) OCL_THROW_ERROR(FN, status); \
    size_t image_depth= 0; \
    status = clGetImageInfo(buf[ID], CL_IMAGE_DEPTH, sizeof(image_depth), &image_depth, NULL);\
    if (status != CL_SUCCESS) OCL_THROW_ERROR(FN, status); \
    if(image_depth == 0) image_depth = 1; \
    if(image_height == 0) image_height = 1; \
    size_t origin[3] = {0, 0, 0}; \
    size_t region[3] = {image_width, image_height, image_depth}; \
    RET = FN(__VA_ARGS__, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, origin, region, &image_row_pitch, &image_slice_pitch, 0, NULL, NULL, &status);\
    if (status != CL_SUCCESS) OCL_THROW_ERROR(FN, status); \
  } while (0)

#define OCL_MAP_BUFFER_GTT(ID) \
    OCL_CALL_MAP_GTT(clEnqueueMapImage, ID, buf_data[ID], queue, buf[ID])

#define OCL_UNMAP_BUFFER_GTT(ID) \
  do { \
    if (buf[ID] != NULL) { \
      OCL_CALL (clEnqueueUnmapMemObject, queue, buf[ID], buf_data[ID], 0, NULL, NULL); \
      buf_data[ID] = NULL; \
    } \
  } while (0)

#define OCL_NDRANGE(DIM_N) \
    OCL_CALL (clEnqueueNDRangeKernel, queue, kernel, DIM_N, NULL, globals, locals, 0, NULL, NULL)

#define OCL_SET_ARG(ID, SIZE, ARG) \
    OCL_CALL (clSetKernelArg, kernel, ID, SIZE, ARG)

#define OCL_CHECK_IMAGE(DATA, W, H, FILENAME) \
  if (cl_check_image(DATA, W, H, FILENAME) == 0) \
    OCL_ASSERTM(false, "image mismatch")

enum { MAX_BUFFER_N = 16 };
extern cl_platform_id platform;
extern cl_device_id device;
extern cl_context ctx;
extern __thread cl_program program;
extern __thread cl_kernel kernel;
extern cl_command_queue queue;
extern __thread cl_mem buf[MAX_BUFFER_N];
extern __thread void* buf_data[MAX_BUFFER_N];
extern __thread size_t globals[3];
extern __thread size_t locals[3];
extern float ULPSIZE_FAST_MATH;

enum {
  SOURCE = 0,
  LLVM = 1,
  BIN = 2
};

/* The SF is float type spliter*/
typedef struct
{
  unsigned int mantissa:23;
  unsigned int exponent:8;
  unsigned int sign:1;
} FLOAT;

typedef union
{
  float f;
  unsigned int i;
  FLOAT spliter;
} SF;

/* Init OpenCL */
extern int cl_ocl_init(void);

/* Init program and kernel for the test */
extern int cl_kernel_init(const char *file_name,
                const char *kernel_name, int format, const char * build_opt);
extern int cl_kernel_compile(const char *file_name, const char *kernel_name, 
                const char * compile_opt);
extern int cl_kernel_link(const char *file_name, const char *kernel_name, 
                const char * link_opt);

/* Get the file path */
extern char* cl_do_kiss_path(const char *file, cl_device_id device);

/* init the bunch of global varaibles here */
extern int cl_test_init(const char *file_name, const char *kernel_name, int format);

/* Unmap and release all the created buffers */
extern void cl_buffer_destroy(void);

/* Release OCL queue, context and device */
extern void cl_ocl_destroy(void);

/* Release kernel and program */
extern void cl_kernel_destroy(bool needDestroyProgram = true);

/* Release everything allocated in cl_test_init */
extern void cl_test_destroy(void);

/* Nicely output the performance counters */
extern void cl_report_perf_counters(cl_mem perf);

/* Read a bmp from file */
extern int *cl_read_bmp(const char *filename, int *width, int *height);

/* Write a bmp to a file */
extern void cl_write_bmp(const int *data, int width, int height, const char *filename);

/* Check data from img against bmp file located at "bmp" */
extern int cl_check_image(const int *img, int w, int h, const char *bmp);

/* Calculator ULP of each FLOAT value */
extern float cl_FLT_ULP(float float_number);

/* Calculator ULP of each INT value */
extern int cl_INT_ULP(int int_number);

/* subtract the time */
double time_subtract(struct timeval *y, struct timeval *x, struct timeval *result);

/* check ulpsize */
float select_ulpsize(float ULPSIZE_FAST_MATH, float ULPSIZE_NO_FAST_MATH);

/* Check is FP64 enabled. */
extern int cl_check_double(void);

/* Check is beignet device. */
extern int cl_check_beignet(void);

/* Check is intel subgroups enabled. */
extern int cl_check_subgroups(void);

/* Check is intel_media_block_io enabled. */
extern int cl_check_media_block_io(void);

typedef cl_int(clGetKernelSubGroupInfoKHR_cb)(cl_kernel, cl_device_id,
                                              cl_kernel_sub_group_info, size_t,
                                              const void *, size_t, void *,
                                              size_t *);
extern clGetKernelSubGroupInfoKHR_cb* utestclGetKernelSubGroupInfoKHR;

/* Check if cl_intel_motion_estimation enabled. */
extern int cl_check_motion_estimation(void);

/* Check is cl version 2.0 or Beignet extension. */
extern int cl_check_ocl20(bool or_beignet = true);

/* Check is FP16 enabled. */
extern int cl_check_half(void);

/* Helper function for half type numbers */
extern uint32_t __half_to_float(uint16_t h, bool* isInf = NULL, bool* infSign = NULL);
extern uint16_t __float_to_half(uint32_t x);
extern float as_float(uint32_t i);
extern uint32_t as_uint(float f);
/* Check is intel subgroups short enabled. */
extern int cl_check_subgroups_short(void);

/* Check is intel_required_subgroup_size enabled. */
extern int cl_check_reqd_subgroup(void);
#endif /* __UTEST_HELPER_HPP__ */

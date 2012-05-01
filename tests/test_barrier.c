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

#include "cl_test.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef union kernel_arg {
  float f;
  uint32_t u32;
  uint16_t u16;
  uint8_t  u8;
  int32_t  i32;
  int16_t  i16;
  int8_t   i8;
} kernel_arg_t;

#define N 1024 
int
main (int argc, char *argv[])
{
  cl_mem dst, src;
  IF_DEBUG(uint32_t *dst_buffer = NULL);
  uint32_t *src_buffer = NULL;
  const size_t n = N;
  const size_t global_work_size = N;
  const size_t local_work_size = 256;
  kernel_arg_t arg;
  int status = 0, i;

#if TEST_SIMD8
  if ((status = cl_test_init("test_barrier_kernels_0.bin8", "test_barrier")) != 0)
    goto error;
#else
  if ((status = cl_test_init("test_barrier_kernels_0.bin", "test_barrier")) != 0)
    goto error;
#endif

  /* Fill the buffer with random values */
  if ((src_buffer = malloc(sizeof(uint32_t) * n)) == NULL) {
    fprintf(stderr, "Allocation failed\n");
    status = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }
  for (i = 0; i < n; ++i)
    src_buffer[i] = i;

  /* Allocate the two buffers */
  dst = clCreateBuffer(ctx, 0, n * sizeof(uint32_t), NULL, &status);
  if (status != CL_SUCCESS)
    goto error;
  src = clCreateBuffer(ctx, CL_MEM_COPY_HOST_PTR, n * sizeof(uint32_t), src_buffer, &status);
  if (status != CL_SUCCESS)
    goto error;
  free(src_buffer);

  /* Set source and destination */
  CALL (clSetKernelArg, kernel, 0, sizeof(cl_mem), &dst);
  CALL (clSetKernelArg, kernel, 4, sizeof(cl_mem), &src);
  arg.f = 1.f;
  CALL (clSetKernelArg, kernel, 1, sizeof(float), &arg);
  arg.i8 = 2;
  CALL (clSetKernelArg, kernel, 2, sizeof(int8_t), &arg);
  arg.i32 = 3;
  CALL (clSetKernelArg, kernel, 3, sizeof(int32_t), &arg);
  arg.i16 = 4;
  CALL (clSetKernelArg, kernel, 5, sizeof(int16_t), &arg);
  arg.u32 = 5;
  CALL (clSetKernelArg, kernel, 6, sizeof(uint32_t), &arg);
  arg.i32 = 6;
  CALL (clSetKernelArg, kernel, 7, sizeof(int32_t), &arg);

  /* Run the kernel */
  CALL (clEnqueueNDRangeKernel, queue,
                                kernel,
                                1,
                                NULL,
                                &global_work_size,
                                &local_work_size,
                                0,
                                NULL,
                                NULL);

  /* Be sure that everything run fine */
#ifndef NDEBUG
  dst_buffer = (uint32_t *) clIntelMapBuffer(dst, &status);
  if (status != CL_SUCCESS)
    goto error;
#endif
  src_buffer = (uint32_t *) clIntelMapBuffer(src, &status);
  if (status != CL_SUCCESS)
    goto error;
  for (i = 0; i < N; ++i)
    assert(dst_buffer[i] == src_buffer[i] + 21);

  IF_DEBUG(CALL (clIntelUnmapBuffer, dst));
  CALL (clIntelUnmapBuffer, src);
  CALL (clReleaseMemObject, dst);
  CALL (clReleaseMemObject, src);
  cl_test_destroy();
  printf("%i memory leaks\n", clIntelReportUnfreed());
  assert(clIntelReportUnfreed() == 0);

error:
  cl_report_error(status);
  return status;
}


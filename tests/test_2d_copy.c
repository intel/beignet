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

#define W 32
#define H 32

int
main (int argc, char *argv[])
{
  cl_mem dst0, dst1, src;
  uint32_t *src_buffer = NULL;
  IF_DEBUG (uint32_t *dst0_buffer = NULL);
  IF_DEBUG (uint32_t *dst1_buffer = NULL);
  const size_t n = W*H;
  const size_t global_work_size[2] = {W, H};
  const size_t local_work_size[2] = {8,8};
  kernel_arg_t arg;
  int status = 0, i, j;

#if TEST_SIMD8
  if ((status = cl_test_init("test_2d_copy_kernels_0.bin8", "test_2d_copy")) != 0)
    goto error;
#else
  if ((status = cl_test_init("test_2d_copy_kernels_0.bin", "test_2d_copy")) != 0)
    goto error;
#endif

  /* Fill the buffer with random values */
  if ((src_buffer = malloc(sizeof(uint32_t) * n)) == NULL) {
    fprintf(stderr, "Allocation failed\n");
    status = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }
  for (i = 0; i < n; ++i)
    src_buffer[i] = rand();

  /* Allocate the two buffers */
  dst0 = clCreateBuffer(ctx, 0, n * sizeof(uint32_t), NULL, &status);
  if (status != CL_SUCCESS)
    goto error;
  dst1 = clCreateBuffer(ctx, 0, n * sizeof(uint32_t), NULL, &status);
  if (status != CL_SUCCESS)
    goto error;
  src = clCreateBuffer(ctx, CL_MEM_COPY_HOST_PTR, n * sizeof(uint32_t), src_buffer, &status);
  if (status != CL_SUCCESS)
    goto error;
  free(src_buffer);

  /* Set source and destination */
  arg.i32 = W;
  CALL (clSetKernelArg, kernel, 0, sizeof(cl_mem), &dst0);
  CALL (clSetKernelArg, kernel, 1, sizeof(cl_mem), &dst1);
  CALL (clSetKernelArg, kernel, 2, sizeof(cl_mem), &src);
  CALL (clSetKernelArg, kernel, 3, sizeof(float),  &arg);

  /* Run the kernel */
  CALL (clEnqueueNDRangeKernel, queue,
                                kernel,
                                2,
                                NULL,
                                global_work_size,
                                local_work_size,
                                0,
                                NULL,
                                NULL);

  /* Be sure that everything run fine */
#ifndef NDEBUG
  dst0_buffer = (uint32_t *) clIntelMapBuffer(dst0, &status);
  if (status != CL_SUCCESS)
    goto error;
  dst1_buffer = (uint32_t *) clIntelMapBuffer(dst1, &status);
  if (status != CL_SUCCESS)
    goto error;
#endif /* NDEBUG */
  src_buffer = (uint32_t *) clIntelMapBuffer(src, &status);
  if (status != CL_SUCCESS)
    goto error;

  for (j = 0; j < H; ++j)
    for (i = 0; i < W; ++i) {
      assert(dst0_buffer[i+j*W] == src_buffer[i+j*W]);
      assert(dst1_buffer[i+j*W] == i+j);
    }
  IF_DEBUG(CALL (clIntelUnmapBuffer, dst0));
  IF_DEBUG(CALL (clIntelUnmapBuffer, dst1));
  CALL (clIntelUnmapBuffer, src);
  CALL (clReleaseMemObject, dst0);
  CALL (clReleaseMemObject, dst1);
  CALL (clReleaseMemObject, src);
  cl_test_destroy();
  printf("%i memory leaks\n", clIntelReportUnfreed());
  assert(clIntelReportUnfreed() == 0);

error:
  cl_report_error(status);
  return status;
}


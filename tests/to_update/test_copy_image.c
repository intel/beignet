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

int
main (int argc, char *argv[])
{
  cl_mem dst, src;
  IF_DEBUG(uint32_t *dst_buffer = NULL);
  uint32_t *src_buffer = NULL;
  const size_t w = 64, h = 64;
  const size_t n = w * h;
  const size_t global_work_size[2] = {w,h};
  const size_t local_work_size[2] = {16,1};
  int status = 0, i;

#if TEST_SIMD8
  if ((status = cl_test_init("test_copy_image_0.bin8", "test_copy_image")) != 0)
    goto error;
#else
  if ((status = cl_test_init("test_copy_image_0.bin", "test_copy_image")) != 0)
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
  const cl_image_format fmt = {
    .image_channel_order = CL_RGBA,
    .image_channel_data_type = CL_UNSIGNED_INT8
  };
  src = clCreateImage2D(ctx,
                        CL_MEM_COPY_HOST_PTR,
                        &fmt,
                        w,
                        h,
                        w * sizeof(uint32_t),
                        src_buffer,
                        &status);
  if (status != CL_SUCCESS)
    goto error;

  /* Set source and destination */
  CALL (clSetKernelArg, kernel, 0, sizeof(cl_mem), &src);
  CALL (clSetKernelArg, kernel, 1, sizeof(cl_mem), &dst);

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
  dst_buffer = (uint32_t *) clIntelMapBuffer(dst, &status);
  if (status != CL_SUCCESS)
    goto error;
  for (i = 0; i < n; ++i) assert(src_buffer[i] == dst_buffer[i]);
  CALL (clIntelUnmapBuffer, dst);
#endif /* NDEBUG */

  free(src_buffer);
  CALL (clReleaseMemObject, dst);
  CALL (clReleaseMemObject, src);
  cl_test_destroy();
  printf("%i memory leaks\n", clIntelReportUnfreed());
  assert(clIntelReportUnfreed() == 0);

error:
  cl_report_error(status);
  return status;
}


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
  cl_mem dst, src, data;
  IF_DEBUG(uint32_t *dst_buffer = NULL);
  uint32_t *src_buffer = NULL;
  int *data_buffer = NULL;
  const int row = 8192;
  const int row_n = 2;
  const int n =  row * row_n;
  const size_t global_work_size = row;
  const size_t local_work_size = 256;
  int status = 0, i;

  if ((status = cl_test_init("test_copy_buffer_row.cl", "test_copy_buffer_row", SOURCE)) != 0)
    goto error;

  /* Fill the buffer with some values */
  src_buffer = (uint32_t *) malloc(sizeof(uint32_t) * n);
  for (i = 0; i < n; ++i) src_buffer[i] = i;

  /* Just put copy info in a buffer */
  data_buffer = (int *) malloc(sizeof(int) * 2);
  data_buffer[0] = row;
  data_buffer[1] = n;

  /* Allocate the two buffers */
  dst = clCreateBuffer(ctx, 0, n * sizeof(uint32_t), NULL, &status);
  if (status != CL_SUCCESS) goto error;
  src = clCreateBuffer(ctx, CL_MEM_COPY_HOST_PTR, n * sizeof(uint32_t), src_buffer, &status);
  if (status != CL_SUCCESS) goto error;
  data = clCreateBuffer(ctx, CL_MEM_COPY_HOST_PTR, 2 * sizeof(int), data_buffer, &status);
  if (status != CL_SUCCESS) goto error;

  /* Set source and destination */
  CALL (clSetKernelArg, kernel, 0, sizeof(cl_mem), &src);
  CALL (clSetKernelArg, kernel, 1, sizeof(cl_mem), &dst);
  CALL (clSetKernelArg, kernel, 2, sizeof(cl_mem), &data);

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

#ifndef NDEBUG
  /* Be sure that everything run fine */
  dst_buffer = (uint32_t *) clIntelMapBuffer(dst, &status);
  if (status != CL_SUCCESS)
    goto error;
  for (i = 0; i < n; ++i)
    assert(src_buffer[i] == dst_buffer[i]);
  CALL (clIntelUnmapBuffer, dst);
#endif /* NDEBUG */

  free(src_buffer);
  free(data_buffer);
  CALL (clReleaseMemObject, dst);
  CALL (clReleaseMemObject, src);
  CALL (clReleaseMemObject, data);
  cl_test_destroy();
  printf("%i memory leaks\n", clIntelReportUnfreed());
  assert(clIntelReportUnfreed() == 0);

error:
  cl_report_error(status);
  return status;
}


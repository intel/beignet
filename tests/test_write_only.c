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
  cl_mem dst;
  int *dst_buffer = NULL;
  //const size_t n = 1024 * 1024;
  const size_t n = 2048;
  const size_t global_work_size = n;
  const size_t local_work_size = 16;
  int status = 0, i;

  if ((status = cl_test_init("test_write_only.cl", "test_write_only", SOURCE)) != 0)
    goto error;

  /* Allocate the two buffers */
  dst = clCreateBuffer(ctx, 0, n * sizeof(uint32_t), NULL, &status);
  if (status != CL_SUCCESS) goto error;

  /* Set source and destination */
  CALL (clSetKernelArg, kernel, 0, sizeof(cl_mem), &dst);

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
  dst_buffer = (int *) clIntelMapBuffer(dst, &status);
  if (status != CL_SUCCESS)
    goto error;
  for (i = 0; i < n; ++i) assert(dst_buffer[i] == i);
  CALL (clIntelUnmapBuffer, dst);
  CALL (clReleaseMemObject, dst);
  cl_test_destroy();
  printf("%i memory leaks\n", clIntelReportUnfreed());
  assert(clIntelReportUnfreed() == 0);

error:
  cl_report_error(status);
  return status;
}


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
  uint32_t *src_buffer = NULL;
  const size_t n = 1024;
  //const size_t global_work_size = 1024;
  //const size_t local_work_size = 16;
  int status = 0, i;

  if ((status = cl_test_init("CopyBuffer_0.bin", "CopyBuffer")) != 0)
    goto error;

  /* Fill the buffer with random values */
  if ((src_buffer = malloc(sizeof(uint32_t) * n)) == NULL) {
    fprintf(stderr, "Allocation failed\n");
    status = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }
  for (i = 0; i < n; ++i)
    src_buffer[i] = rand();

  /* Allocate the two buffers */
  dst = clCreateBuffer(ctx, 0, n * sizeof(uint32_t), NULL, &status);
  if (status != CL_SUCCESS)
    goto error;
  src = clCreateBuffer(ctx, CL_MEM_COPY_HOST_PTR, n * sizeof(uint32_t), src_buffer, &status);
  if (status != CL_SUCCESS)
    goto error;
  free(src_buffer);

  /* Set source and destination */
  CALL (clSetKernelArg, kernel, 0, sizeof(cl_mem), &src);
  CALL (clSetKernelArg, kernel, 1, sizeof(cl_mem), &dst);

  /* Release the buffers we allocate here */
  cl_test_destroy();
  CALL (clReleaseMemObject, dst);
  CALL (clReleaseMemObject, src);
  printf("%i memory leaks\n", clIntelReportUnfreed());

error:
  cl_report_error(status);
  return status;
}


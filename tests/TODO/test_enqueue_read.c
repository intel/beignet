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
  uint32_t *dst_buffer = NULL, *src_buffer = NULL;
  const size_t n = 16;
  const size_t global_work_size = n;
  const size_t local_work_size = 16;
  int status = 0, i;

  if ((status = cl_test_init("CopyBuffer_0.bin", "CopyBuffer")) != 0)
    goto error;

  /* Fill the buffer with random values */
  if ((src_buffer = malloc(sizeof(uint32_t) * n)) == NULL) {
    fprintf(stderr, "Allocation failed\n");
    status = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }
  if ((dst_buffer = malloc(sizeof(uint32_t) * n)) == NULL) {
    fprintf(stderr, "Allocation failed\n");
    status = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }
  for (i = 0; i < n; ++i)
    src_buffer[i] = i;

  /* Allocate the two buffers */
  dst = clCreateBuffer(ctx, CL_MEM_COPY_HOST_PTR | CL_MEM_WRITE_ONLY, n * sizeof(uint32_t), dst_buffer, &status);
  if (status != CL_SUCCESS)
    goto error;
  src = clCreateBuffer(ctx, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY, n * sizeof(uint32_t), src_buffer, &status);
  if (status != CL_SUCCESS)
    goto error;

  /* Set source and destination */
  CALL (clSetKernelArg, kernel, 0, sizeof(cl_mem), &src);
  CALL (clSetKernelArg, kernel, 1, sizeof(cl_mem), &dst);

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
  CALL (clEnqueueReadBuffer, queue,
			     dst,
			     CL_TRUE,
			     0,
			     n* sizeof(uint32_t),
			     (void *)dst_buffer,
			     0,		
		  	     NULL,
			     NULL	 );
  if (status != CL_SUCCESS)
    goto error;
  for (i = 0; i < n; ++i) {
    printf("src_buffer[%d]:[%d], dst_buffer[%d]:[%d]\n",
	i, src_buffer[i], i, dst_buffer[i]);
//    assert(src_buffer[i] == dst_buffer[i]);
  }	

  free(src_buffer);
  free(dst_buffer);	
  CALL (clReleaseMemObject, dst);
  CALL (clReleaseMemObject, src);
  cl_test_destroy();
  printf("%i memory leaks\n", clIntelReportUnfreed());
  assert(clIntelReportUnfreed() == 0);

error:
  cl_report_error(status);
  return status;
}


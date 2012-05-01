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

#ifndef NDEBUG
static const uint32_t wrk_value[32] = {
  71, 69, 67, 65, 63, 61, 59, 57, 55, 53, 51, 49, 47, 45, 43, 41, 39, 37, 35,
  33, 31, 29, 27, 25, 23, 21, 19, 17, 15, 13, 11, 9
};
#endif /* NDEBUG */

#define N 512
int
main (int argc, char *argv[])
{
  cl_mem dst, constants;
  IF_DEBUG(uint32_t *dst_buffer = NULL);
  uint32_t *cst_buffer = NULL;
  const size_t n = N;
  const size_t global_work_size = N;
  const size_t local_work_size = 32;
  int status = 0, i;

#if TEST_SIMD8
  if ((status = cl_test_init("test_constant_memory_0.bin8", "test_constant_memory")) != 0)
    goto error;
#else
  if ((status = cl_test_init("test_constant_memory_0.bin", "test_constant_memory")) != 0)
    goto error;
#endif

  /* Allocate the dst buffer */
  dst = clCreateBuffer(ctx, 0, n * sizeof(uint32_t), NULL, &status);
  if (status != CL_SUCCESS)
    goto error;

  /* Allocate the constant buffer and fill it with ones */
  if ((cst_buffer = (uint32_t*) malloc(sizeof(uint32_t) * 32)) == NULL) {
    status = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }
  for (i = 0; i < 32; ++i)
    cst_buffer[i] = 1;

  constants = clCreateBuffer(ctx, CL_MEM_COPY_HOST_PTR,
                             32 * sizeof(uint32_t),
                             cst_buffer,
                             &status);
  if (status != CL_SUCCESS)
    goto error;
  free(cst_buffer);

  /* Set source and destination */
  CALL (clSetKernelArg, kernel, 0, sizeof(cl_mem), &dst);
  CALL (clSetKernelArg, kernel, 1, 32*sizeof(int), NULL);
  CALL (clSetKernelArg, kernel, 2, sizeof(cl_mem), &constants);

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
  for (i = 0; i < N; ++i)
    assert(dst_buffer[i] == wrk_value[i % 32]);
  CALL (clIntelUnmapBuffer, dst);
#endif /* NDEBUG */

  CALL (clReleaseMemObject, dst);
  CALL (clReleaseMemObject, constants);
  cl_test_destroy();
  printf("%i memory leaks\n", clIntelReportUnfreed());
  assert(clIntelReportUnfreed() == 0);

error:
  cl_report_error(status);
  return status;
}


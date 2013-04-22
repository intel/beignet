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

#include "utest_helper.hpp"

int
main(int argc, char *argv[])
{
  cl_mem dst[24];
  int *dst_buffer = NULL;
  const size_t n = 32 * 1024 * 1024;
  const size_t global_work_size = n;
  const size_t local_work_size = 16;
  int status = 0;

  if ((status = cl_test_init("test_write_only.cl", "test_write_only", SOURCE)) != 0)
    goto error;

  for (uint32_t j = 0; j < 24; ++j)
  {
    // Allocate the two buffers
    dst[j] = clCreateBuffer(ctx, 0, n * sizeof(uint32_t), NULL, &status);
    if (status != CL_SUCCESS) goto error;

    // Set source and destination
    OCL_CALL (clSetKernelArg, kernel, 0, sizeof(cl_mem), &dst[j]);

    // Run the kernel
    OCL_CALL (clEnqueueNDRangeKernel, queue,
                                  kernel,
                                  1,
                                  NULL,
                                  &global_work_size,
                                  &local_work_size,
                                  0,
                                  NULL,
                                  NULL);

    // Be sure that everything run fine
    dst_buffer = (int *) clMapBufferIntel(dst[j], &status);
    if (status != CL_SUCCESS)
      goto error;
    for (uint32_t i = 0; i < n; ++i)
      if (dst_buffer[i] != int(i)) {
        fprintf(stderr, "run-time flat address space failed\n");
        exit(-1);
      }
    OCL_CALL (clUnmapBufferIntel, dst[j]);
  }

  for (uint32_t j = 0; j < 24; ++j) OCL_CALL (clReleaseMemObject, dst[j]);
  cl_test_destroy();
  printf("%i memory leaks\n", clReportUnfreedIntel());
  assert(clReportUnfreedIntel() == 0);

error:
  return status;
}


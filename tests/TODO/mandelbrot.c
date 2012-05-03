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

static int *dst = NULL;
static const size_t w = 64;
static const size_t h = 64; 
static const size_t iter = 4;

int
main(int argc, char **argv) 
{
  const size_t global[2] = {w, h};
  const size_t local[2] = {16, 1};
  const size_t sz = w * h * sizeof(char[4]);
  int status = 0;

#if TEST_SIMD8
  if ((status = cl_test_init("mandelbrot_0.bin8", "render")) != 0)
    goto error;
#else
  if ((status = cl_test_init("mandelbrot_0.bin", "render")) != 0)
    goto error;
#endif

  /* One dry run */
  cl_mem cl_dst = clCreateBuffer(ctx, CL_MEM_PINNABLE, sz, NULL, &status);

  /* Set source and destination */
  CALL (clSetKernelArg, kernel, 0, sizeof(cl_mem), &cl_dst);

  /* Run the kernel */
  CALL (clEnqueueNDRangeKernel, queue, kernel, 2, NULL, global, local, 0, NULL, NULL);
  dst = (int *) clIntelMapBuffer(cl_dst, &status);

  writeBmp(dst, w, h, "mandelbrot.bmp");
  CALL (clIntelUnmapBuffer, cl_dst);
  CALL (clReleaseMemObject, cl_dst);
  cl_test_destroy();
  printf("%i memory leaks\n", clIntelReportUnfreed());

error:
  cl_report_error(status);
  return status;
}


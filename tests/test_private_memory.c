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
  64, 60, 58, 56, 54, 52, 50, 48, 46, 44, 42, 40, 38, 36, 34, 32, 30, 28, 26,
  24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0
};
#endif /* NDEBUG */

#define N 16
int
main (int argc, char *argv[])
{
  cl_mem dst;
  uint32_t *dst_buffer = NULL;
  const size_t n = N;
  const size_t global_work_size = N;
  const size_t local_work_size = 16;
  int status = 0;

#if TEST_SIMD8
  if ((status = cl_test_init("test_private_memory_0.bin8", "test_private_memory")) != 0)
    goto error;
#else
  if ((status = cl_test_init("test_private_memory_0.bin", "test_private_memory")) != 0)
    goto error;
#endif

  dst = clCreateBuffer(ctx, 0, n * sizeof(uint32_t), NULL, &status);
  if (status != CL_SUCCESS)
    goto error;

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
  dst_buffer = (uint32_t *) clIntelMapBuffer(dst, &status);
  if (status != CL_SUCCESS)
    goto error;
  int i;
  for (i = 0; i < N; ++i)
    //assert(dst_buffer[i] == wrk_value[i % 32]);
    //printf("[%u %u]", (dst_buffer[i] >> 8)& 0xf, dst_buffer[i] & 0x3);
    printf("[%u]", dst_buffer[i]);
  assert(dst_buffer[0] == 496);
  assert(dst_buffer[1] == 528);
  assert(dst_buffer[2] == 560);
  assert(dst_buffer[3] == 592);
  assert(dst_buffer[4] == 624);
  assert(dst_buffer[5] == 656);
  assert(dst_buffer[6] == 688);
  assert(dst_buffer[7] == 720);
  assert(dst_buffer[8] == 752);
  assert(dst_buffer[9] == 784);
  assert(dst_buffer[10] == 816);
  assert(dst_buffer[11] == 848);
  assert(dst_buffer[12] == 880);
  assert(dst_buffer[13] == 912);
  assert(dst_buffer[14] == 944);
  assert(dst_buffer[15] == 976);

  CALL (clIntelUnmapBuffer, dst);
  CALL (clReleaseMemObject, dst);
  cl_test_destroy();
  printf("%i memory leaks\n", clIntelReportUnfreed());
  assert(clIntelReportUnfreed() == 0);

error:
  cl_report_error(status);
  return status;
}


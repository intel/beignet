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

#include "common.h"
#include "cl_test.h"
#include "CL/cl_intel.h"

void verify();

int width = 256;
int height = 256;
int blockSizeX = 8;
int blockSizeY = 8;
int mulFactor = 2;
float *seeds, *deviceResult;

int main(int argc, char**argv) 
{
  char *ker_path = NULL;
  struct args args = {0};
  int err, i;

  parseArgs(argc, argv, &args);

  cl_device_id     device  = getDeviceID(args.d);
#if TEST_SIMD8
  ker_path = do_kiss_path("mersenne_kernels_0.bin8", device);
#else
  ker_path = do_kiss_path("mersenne_kernels_0.bin", device);
#endif
  cl_context       context = clCreateContext(0, 1, &device, NULL, NULL, &err); CHK_ERR(err);
  cl_command_queue queue   = clCreateCommandQueue(context, device, 0, &err); CHK_ERR(err);
  cl_kernel        kernel  = getKernelFromBinary(device, context, ker_path, "gaussianRand");

  seeds = newBuffer( width * height * sizeof(cl_uint4), 0); 
//  deviceResult = newBuffer( width * height * mulFactor * sizeof(cl_float4), 0); 
  for (i = 0; i < width * height * 4; ++i) {
    seeds[i] = (unsigned int)rand();
  }

  cl_mem seedsBuf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, width * height * sizeof(cl_float4), seeds, &err); CHK_ERR(err);
  cl_mem resultBuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, width * height * sizeof(cl_float4) * mulFactor, NULL, &err); CHK_ERR(err);

  size_t globalThreads[2] = {width, height};
  size_t localThreads[2] = {blockSizeX, blockSizeY};

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &seedsBuf); CHK_ERR(err);
  err = clSetKernelArg(kernel, 1, sizeof(cl_uint), &width); CHK_ERR(err);
  err = clSetKernelArg(kernel, 2, sizeof(cl_uint), &mulFactor); CHK_ERR(err);
  err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &resultBuf); CHK_ERR(err);

  err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalThreads, localThreads, 0, NULL, NULL);
#if 0
  err = clEnqueueReadBuffer(queue, resultBuf, CL_TRUE, 0, width * height * mulFactor * sizeof(cl_float4),
      deviceResult, 1, &eND, NULL);
#endif
  deviceResult = clIntelMapBuffer(resultBuf, &err);
  CHK_ERR(err);
  verify();
  clIntelUnmapBuffer(resultBuf);

  return err;
}

void verify() 
{
  int i;
  // comparef(d, c, MAX, 1.0e-6);
  /* check mean value of generated random numbers */
  float meanVal = 0.0f;

  for(i = 0; i < height * width * (int)mulFactor * 4; ++i) {
    meanVal += deviceResult[i];
  }

  meanVal = fabs(meanVal) / (height * width * (int)mulFactor * 4);
  printf("Mean Value of random numbers: %12.8f\n", meanVal);
  printf("%s\n", (meanVal < 0.1f) ? "Passed" : "Failed");

  printf("Done\n");
}


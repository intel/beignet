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

void verify(int compareCall, int comparePut);

float *randArray, *devCallPrice, *devPutPrice, *hostCallPrice, *hostPutPrice;
int width = 32;
int height = 16;

int main(int argc, char **argv)
{
  struct args args = { 0 };
  char *ker_path = NULL;
  int err;

  parseArgs(argc, argv, &args);

  cl_device_id device = getDeviceID(args.d);
#if TEST_SIMD8
  ker_path = do_kiss_path("blackscholes_kernel_0.bin8", device);
#else
  ker_path = do_kiss_path("blackscholes_kernel_0.bin", device);
#endif
  cl_context context = clCreateContext(0, 1, &device, NULL, NULL, &err);
  CHK_ERR(err);
  cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
  CHK_ERR(err);
  cl_kernel kernel = getKernelFromBinary(device, context, ker_path, "blackScholes");

  int whf4 = width * height * sizeof(cl_float4);

  randArray     = newBuffer(whf4, 'f');
  devCallPrice  = newBuffer(whf4, 0);
  //devPutPrice   = newBuffer(whf4, 0);
  hostCallPrice = newBuffer(whf4, 0);
  hostPutPrice  = newBuffer(whf4, 0);

  cl_mem randBuf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, whf4, randArray, &err);
  CHK_ERR(err);
  cl_mem callPriceBuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, whf4, NULL, &err);
  CHK_ERR(err);
  cl_mem putPriceBuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, whf4, NULL, &err);
  CHK_ERR(err);

  size_t globalThreads[2] = { width, height };
  //size_t localThreads[2] = { 4, 4 };
  size_t localThreads[2] = { 16, 1 };

  /* whether sort is to be in increasing order. CL_TRUE implies increasing */
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *) &randBuf);
  CHK_ERR(err);
  err = clSetKernelArg(kernel, 1, sizeof(width), (const void *) &width);
  CHK_ERR(err);
  err = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *) &callPriceBuf);
  CHK_ERR(err);
  err = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *) &putPriceBuf);
  CHK_ERR(err);

  err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalThreads, localThreads, 0, NULL, NULL);
  CHK_ERR(err);

  devPutPrice = clIntelMapBuffer(putPriceBuf, &err);
  CHK_ERR(err);
  devCallPrice = clIntelMapBuffer(callPriceBuf, &err);
  CHK_ERR(err);
  verify(1, 1);
#if 0
  int i;
  for (i = 0; i < width*height*4; i++) {
    printf("%12.8f, %12.8f | %12.8f %12.8f\n", 
        devCallPrice[i], 
        hostCallPrice[i], 
        devPutPrice[i], 
        hostPutPrice[i]);
    fflush(stdout);
  }
#endif
  clIntelUnmapBuffer(putPriceBuf);
  return err;
}

#define S_LOWER_LIMIT 10.0f
#define S_UPPER_LIMIT 100.0f
#define K_LOWER_LIMIT 10.0f
#define K_UPPER_LIMIT 100.0f
#define T_LOWER_LIMIT 1.0f
#define T_UPPER_LIMIT 10.0f
#define R_LOWER_LIMIT 0.01f
#define R_UPPER_LIMIT 0.05f
#define SIGMA_LOWER_LIMIT 0.01f
#define SIGMA_UPPER_LIMIT 0.10f

float phi(float X)
{
  float y, absX, t;

  // the coeffs
  const float c1 = 0.319381530f;
  const float c2 = -0.356563782f;
  const float c3 = 1.781477937f;
  const float c4 = -1.821255978f;
  const float c5 = 1.330274429f;

  const float oneBySqrt2pi = 0.398942280f;

  absX = fabsf(X);
  t = 1.0f / (1.0f + 0.2316419f * absX);

  y = 1.0f - oneBySqrt2pi * exp(-X * X / 2.0f) *
    t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * c5))));

  return (X < 0) ? (1.0f - y) : y;
}

void blackScholesCPU(int width, int height, float *randArray,
    float *hostCallPrice, float *hostPutPrice)
{
  int y;
  for (y = 0; y < width * height * 4; ++y) {
    float d1, d2;
    float sigmaSqrtT;
    float KexpMinusRT;
    float s = S_LOWER_LIMIT * randArray[y] + S_UPPER_LIMIT * (1.0f - randArray [y]);
    float k = K_LOWER_LIMIT * randArray[y] + K_UPPER_LIMIT * (1.0f - randArray [y]);
    float t = T_LOWER_LIMIT * randArray[y] + T_UPPER_LIMIT * (1.0f - randArray [y]);
    float r = R_LOWER_LIMIT * randArray[y] + R_UPPER_LIMIT * (1.0f - randArray [y]);
    float sigma =
      SIGMA_LOWER_LIMIT * randArray[y] +
      SIGMA_UPPER_LIMIT * (1.0f - randArray[y]);

    sigmaSqrtT = sigma * sqrt(t);

    d1 = (log(s / k) +
        (r + sigma * sigma / 2.0f) * t) / sigmaSqrtT;
    d2 = d1 - sigmaSqrtT;

    KexpMinusRT = k * exp(-r * t);
    hostCallPrice[y] = s * phi(d1) - KexpMinusRT * phi(d2);
    hostPutPrice[y] = KexpMinusRT * phi(-d2) - s * phi(-d1);
  }
}

void verify(int compareCall, int comparePut)
{
  blackScholesCPU(width, height, randArray, hostCallPrice, hostPutPrice);
  int resC = 1, resP = 1;
  if (compareCall)
    resC = comparef(hostCallPrice, devCallPrice, width * height * 4, 1.0e-5f);
  if (comparePut)
    resP = comparef(hostPutPrice, devPutPrice, width * height * 4, 1.0e-5f);
  printf("resC=%d, resP=%d: ", resC, resP);
  if (resC && resP)
    printf("Passed!\n");
  else
    printf("Failed!\n");
}


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
void verify();

int length = 1024;
int *input, *refInput;

int main(int argc, char**argv) 
{
  struct args args = {0};
  int err, i;

  parseArgs(argc, argv, &args);

  cl_device_id device = getDeviceID(args.d);
  cl_context context = clCreateContext(0, 1, &device, NULL, NULL, &err); CHK_ERR(err);
  cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err); CHK_ERR(err);
  cl_kernel kernel = getKernel(device, context, "bitonic_kernels.cl", "bitonicSort");

  int length_bytes = length * sizeof(cl_uint);

  input    = newBuffer(length_bytes, 0);
  refInput = newBuffer(length_bytes, 0);
  for (i = 0; i < length; i++) { 
    input[i] = refInput[i] = rand() & 0x0fffff;
  }
  cl_mem inputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
      length_bytes, input, &err);
  CHK_ERR(err);

  /*
   * Execute
   *
   * This algorithm is run as NS stages. Each stage has NP passes.
   * so the total number of times the kernel call is enqueued is NS * NP.
   *
   * For every stage S, we have S + 1 passes.
   * eg: For stage S = 0, we have 1 pass.
   *     For stage S = 1, we have 2 passes.
   *
   * if length is 2^N, then the number of stages (numStages) is N.
   * Do keep in mind the fact that the algorithm only works for
   * arrays whose size is a power of 2.
   * 
   * here, numStages is N.
   *
   * For an explanation of how the algorithm works, please go through
   * the documentation of this sample.
   */

  /*
   * 2^numStages should be equal to length.
   * i.e the number of times you halve length to get 1 should be numStages 
   */
  int numStages = 0;
  int sortDescending = 1;
  for(i = length; i > 1; i >>= 1)
    ++numStages;

  /*** Set appropriate arguments to the kernel ***/
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputBuffer); CHK_ERR(err);
  err = clSetKernelArg(kernel, 3, sizeof(cl_uint), &length); CHK_ERR(err);
  err = clSetKernelArg(kernel, 4, sizeof(cl_uint), &sortDescending);  CHK_ERR(err);

  int stage, passOfStage;
  size_t globalThreads[1] = {length / 2};
  size_t localThreads[1] = {1};

  for(stage = 0; stage < numStages; stage++) {
    /* stage of the algorithm */
    err = clSetKernelArg(kernel, 1, sizeof(cl_uint), &stage);
    CHK_ERR(err);

    /* Every stage has stage+1 passes. */
    for(passOfStage = 0; passOfStage < stage + 1; passOfStage++) {
      /* pass of the current stage */
      err = clSetKernelArg( kernel, 2, sizeof(cl_uint), &passOfStage);
      CHK_ERR(err);

      /* 
       * Enqueue a kernel run call.
       * For simplicity, the groupsize used is 1.
       * 
       * Each thread writes a sorted pair.
       * So, the number of  threads (global) is half the length.
       */
      err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL,
          globalThreads, localThreads,
          0, NULL, NULL);
      CHK_ERR(err);

      /* wait for the kernel call to finish execution */
      err = clFinish(queue);
      CHK_ERR(err);
    }
  }
  err = clEnqueueReadBuffer(queue, inputBuffer, CL_TRUE, 0,
      length_bytes, input, 
      0, NULL, NULL);
  CHK_ERR(err);

  verify();

}

int compare(const void *x, const void *y) 
{
  unsigned int a = *(unsigned int *)x;
  unsigned int b = *(unsigned int *)y;
  return (a-b);
}


void verify() 
{
  int i;

  qsort(refInput, length, sizeof(int), compare);
  for (i = 0; i < length; i++) {
    if (i < 20) {
      printf("%8d %8d\n", input[i], refInput[i]);
    }
    if (input[i] != refInput[i]) {
      printf("Failed at %d: %d vs. %d\n", i,
          input[i], refInput[i]);
      printf("Failed");
      exit(-1);
    }
  }
  printf("Passed\n");
}


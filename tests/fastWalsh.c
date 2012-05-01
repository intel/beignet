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

void verify();

int length = 2048;
float *input, *output, *refout;

int main(int argc, char**argv) 
{
  struct args args = {0};
  int err;
  char *ker_path = NULL;

  parseArgs(argc, argv, &args);

  cl_device_id     device  = getDeviceID(args.d);
#if TEST_SIMD8
  ker_path = do_kiss_path("fastWalsh_kernels_0.bin8", device);
#else
  ker_path = do_kiss_path("fastWalsh_kernels_0.bin", device);
#endif
  cl_context       context = clCreateContext(0, 1, &device, NULL, NULL, &err); CHK_ERR(err);
  cl_command_queue queue   = clCreateCommandQueue(context, device, 0, &err); CHK_ERR(err);
  cl_kernel        kernel  = getKernelFromBinary(device, context, ker_path, "fastWalshTransform");

  input  = newBuffer(length * sizeof(float), 'p');
  refout = newBuffer(length * sizeof(float),  0 );
  memcpy(refout, input, length * sizeof(float));

  cl_mem inputBuffer = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, length * sizeof(float), input, &err); CHK_ERR(err);

  /* Execute */
  size_t globalThreads[1] = { length/2};
  size_t localThreads[1]  = { 256};

  /* the input array - also acts as output*/
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&inputBuffer); CHK_ERR(err);

  int step;

  for(step = 1; step < length; step<<= 1) {
    /* stage of the algorithm */
    err = clSetKernelArg( kernel, 1, sizeof(int), &step); CHK_ERR(err);

    /* Enqueue a kernel run call */
    err = clEnqueueNDRangeKernel(queue,
                                 kernel, 1, NULL,
                                 globalThreads, 
                                 localThreads,
                                 0,
                                 NULL,
                                 NULL); CHK_ERR(err);

    /* wait for the kernel call to finish execution */
    /* err  = clWaitForEvents(1, &eND); CHK_ERR(err); */
  }

#if 0
  /* Enqueue readBuffer*/
  err = clEnqueueReadBuffer(queue, inputBuffer, CL_TRUE, 0, length *  sizeof(cl_float), output, 0, NULL, NULL); CHK_ERR(err);
#else
  output = clIntelMapBuffer(inputBuffer, &err);

#endif
  verify();
  return 0;
}

void fastWalshCPU(float *vinput, int length)
{
  int step;

  /* for each pass of the algorithm */
  for(step=1; step < length; step <<=1) {
    /* length of each block */
    cl_uint jump = step << 1;

    /* for each blocks */
    cl_uint group;
    for(group = 0; group < step; ++group) {
      /* for each pair of elements with in the block */
      cl_uint pair;

      for(pair = group; pair < length; pair += jump) {
        /* find its partner */
        cl_uint match = pair + step;

        cl_float T1 = vinput[pair];
        cl_float T2 = vinput[match];

        /* store the sum and difference of the numbers in the same locations */
        vinput[pair] = T1 + T2;
        vinput[match] = T1 - T2;
      }
    }
  }
}

void verify() 
{
  int i;

  fastWalshCPU(refout, length);
  comparef(output, refout, length, 1.0e-6);
  for (i = 0; i < 20; i++) {
    printf("%20.5f %20.8f\n", output[i], refout[i]);
  }
  printf("Done\n");
}


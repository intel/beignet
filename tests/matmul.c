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

float *input0, *input1, *output, *refout;
int vectorsize = 4;
int blockSize = 8;

/* must be multiples of vectorsize and blocksize */
#define	M	32
#define	N	32
#define	K	32

int width0 = M;
int height0 = N;

int width1 = K;
int height1 = M;

int main(int argc, char**argv) 
{
  struct args args = {0};
  char *ker_path = NULL;
  int err;

  parseArgs(argc, argv, &args);

  cl_device_id     device  = getDeviceID(args.d);
  ker_path = do_kiss_path("matmul_kernels_0.bin", device);
  cl_context       context = clCreateContext(0, 1, &device, NULL, NULL, &err); CHK_ERR(err);
  cl_command_queue queue   = clCreateCommandQueue(context, device, 0, &err); CHK_ERR(err);
  cl_kernel        kernel  = getKernelFromBinary(device, context, ker_path, "mmmKernel");

  input0 = (cl_float *) newBuffer(width0 * height0 * sizeof(cl_float), 'p'); 
  input1 = (cl_float *) newBuffer(width1 * height1 * sizeof(cl_float), 'p'); 
  output = (cl_float *) newBuffer(width1 * height0 * sizeof(cl_float),  0 );
  refout = (cl_float *) newBuffer(width1 * height0 * sizeof(cl_float),  0 );

  cl_mem inputBuffer0 = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, 
      width0 * height0 * sizeof(cl_float), input0, &err); CHK_ERR(err);
  cl_mem inputBuffer1 = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, 
      width1 * height1 * sizeof(cl_float), input1, &err); CHK_ERR(err);
  cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, 
      width1 * height0 * sizeof(cl_float), output, &err); CHK_ERR(err);

  /* Execute */
  size_t globalThreads[2] = {width1 / 4, height0/ 4};
  size_t localThreads [2] = {blockSize, blockSize};

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputBuffer0); CHK_ERR(err);
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &inputBuffer1); CHK_ERR(err);
  err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &outputBuffer); CHK_ERR(err);
  err = clSetKernelArg(kernel, 3, sizeof(cl_int), &width0); CHK_ERR(err);
  err = clSetKernelArg(kernel, 4, sizeof(cl_int), &width1); CHK_ERR(err);

  err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalThreads, localThreads, 0, NULL, NULL);
  output = clIntelMapBuffer(outputBuffer, &err);

  verify();
  clIntelUnmapBuffer(outputBuffer);
  return err;
}

void 
matmulCPU(float *output, float *input0, float * input1, int y, int x, int z)
{
  int i, j, k;

  for(i=0; i < y; i++)
  for(j=0; j < z; j++)
  for(k=0; k < x; k++)
    output[i*z + j] += (input0[i*x + k]*input1[k*z + j]);
}

void verify() 
{
  matmulCPU(refout, input0, input1, height0, width0, width1);
  comparef(refout, output, width1 * height0,  1.0e-5f );
  printf("Done\n");
}


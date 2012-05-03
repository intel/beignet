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

int length = 1024;
float *input_i, *input_r, *output_i, *output_r, *refOutput_r, *refOutput_i;

int main(int argc, char**argv) 
{
  char *ker_path = NULL;
  struct args args = {0};
  int err;

  parseArgs(argc, argv, &args);

  cl_device_id device = getDeviceID(args.d);
#if TEST_SIMD8
  ker_path = do_kiss_path("fft_kernels_0.bin", device);
#else
  ker_path = do_kiss_path("fft_kernels_0.bin8", device);
#endif
  cl_context context = clCreateContext(0, 1, &device, NULL, NULL, &err); CHK_ERR(err);
  cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err); CHK_ERR(err);
  cl_kernel kernel = getKernelFromBinary(device, context, ker_path, "kfft");

  cl_uint inputSizeBytes = length * sizeof(cl_float);

  /* allocate and init memory used by host */
  input_r  = newBuffer(inputSizeBytes, 'p');
  input_i  = newBuffer(inputSizeBytes,  0 );
  output_i = newBuffer(inputSizeBytes,  0 );
  output_r = newBuffer(inputSizeBytes,  0 );

  memcpy(output_i, input_i, inputSizeBytes);
  memcpy(output_r, input_r, inputSizeBytes);

  cl_mem buffer_r = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, inputSizeBytes, input_r, &err); CHK_ERR(err);
  cl_mem buffer_i = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, inputSizeBytes, input_i, &err); CHK_ERR(err);

  size_t globalThreads[1] = {64};
  size_t localThreads[1] = {64};

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_r); CHK_ERR(err);
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_i); CHK_ERR(err);

  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, globalThreads, localThreads, 0, NULL, NULL);
  CHK_ERR(err);

  output_i = clIntelMapBuffer(buffer_i, &err); CHK_ERR(err);
  output_r = clIntelMapBuffer(buffer_r, &err); CHK_ERR(err);
  verify();

  return 0;
}

/*
   This computes an in-place complex-to-complex FFT 
   x and y are the real and imaginary arrays of 2^m points.
   dir =  1 gives forward transform
   dir = -1 gives reverse transform 
   */
void fftCPU(int dir,long m,cl_float *x,cl_float *y)
{
  long n,i,i1,j,k,i2,l,l1,l2;
  double c1,c2,tx,ty,t1,t2,u1,u2,z;

  /* Calculate the number of points */
  n = 1;
  for (i=0;i<m;i++) 
    n *= 2;

  /* Do the bit reversal */
  i2 = n >> 1;
  j = 0;
  for (i=0;i<n-1;i++) 
  {
    if (i < j) 
    {
      tx = x[i];
      ty = y[i];
      x[i] = x[j];
      y[i] = y[j];
      x[j] = (cl_float)tx;
      y[j] = (cl_float)ty;
    }
    k = i2;
    while (k <= j)
    {
      j -= k;
      k >>= 1;
    }
    j += k;
  }

  /* Compute the FFT */
  c1 = -1.0; 
  c2 = 0.0;
  l2 = 1;
  for (l=0;l<m;l++) 
  {
    l1 = l2;
    l2 <<= 1;
    u1 = 1.0; 
    u2 = 0.0;
    for (j=0;j<l1;j++)
    {
      for (i=j;i<n;i+=l2) 
      {
        i1 = i + l1;
        t1 = u1 * x[i1] - u2 * y[i1];
        t2 = u1 * y[i1] + u2 * x[i1];
        x[i1] = (cl_float)(x[i] - t1); 
        y[i1] = (cl_float)(y[i] - t2);
        x[i] += (cl_float)t1;
        y[i] += (cl_float)t2;
      }
      z =  u1 * c1 - u2 * c2;
      u2 = u1 * c2 + u2 * c1;
      u1 = z;
    }
    c2 = sqrt((1.0 - c1) / 2.0);
    if (dir == 1) 
      c2 = -c2;
    c1 = sqrt((1.0 + c1) / 2.0);
  }

  /* Scaling for forward transform */
  /*if (dir == 1) {
    for (i=0;i<n;i++) {
    x[i] /= n;
    y[i] /= n;
    }
    }*/
}

/**
 * Reference CPU implementation of FFT Convolution 
 * for performance comparison
 */
void fftCPUReference()
{
  refOutput_r = newBuffer(length * sizeof(float),  0 );
  refOutput_i = newBuffer(length * sizeof(float),  0 );

  /* Copy data from input to reference buffers */
  memcpy(refOutput_r, input_r, length * sizeof(cl_float));
  memcpy(refOutput_i, input_i, length * sizeof(cl_float));

  /* Compute reference FFT */
  fftCPU(1, 10, refOutput_r, refOutput_i);
}

void verify()
{
  fftCPUReference();
  int i;
  comparef(refOutput_r, output_r, length, 1.0e-5f);
  comparef(refOutput_i, output_i, length, 1.0e-5f);
  printf("Passed\n");

  for (i = 0; i < length; i++) {
    printf("%i %12.5f, %12.5f | %12.5f, %12.5f | %12.5f, %12.5f\n",
        i, input_r[i], input_i[i],
        refOutput_r[i], output_r[i],
        refOutput_i[i], output_i[i]);
  }
  printf("...\n");

  for (i = 0; i < 8; i++) {
    printf("%12.5f, %12.5f | %12.5f, %12.5f | %12.5f, %12.5f\n",
        input_r[length-1-i], input_i[length-1-i],
        refOutput_r[length-1-i], output_r[length-1-i],
        refOutput_i[length-1-i], output_i[length-1-i]);
  }
}


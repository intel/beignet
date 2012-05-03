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

void verify(float*);

int width = 16;
int height = 16;
int blockWidth = 8;
int blockSize = 8 * 8;
int inverse = 0;

float *input, *output, *ref;

int main(int argc, char**argv) 
{
  struct args args = {0};
  int err, i;
  char *ker_path = NULL;

  const cl_float a = cos(M_PI/16)/2;
  const cl_float b = cos(M_PI/8 )/2;
  const cl_float c = cos(3*M_PI/16)/2;
  const cl_float d = cos(5*M_PI/16)/2;
  const cl_float e = cos(3*M_PI/8)/2;
  const cl_float f = cos(7*M_PI/16)/2;
  const cl_float g = 1.0f/sqrt(8.0f);

  cl_float dct8x8[64] = {
    g,  a,  b,  c,  g,  d,  e,  f,
    g,  c,  e, -f, -g, -a, -b, -d,
    g,  d, -e, -a, -g,  f,  b,  c,
    g,  f, -b, -d,  g,  c, -e, -a,
    g, -f, -b,  d,  g, -c, -e,  a,
    g, -d, -e,  a, -g, -f,  b, -c,
    g, -c,  e,  f, -g,  a, -b,  d,
    g, -a,  b, -c,  g, -d,  e,  f
  };

  parseArgs(argc, argv, &args);

  cl_device_id device = getDeviceID(args.d);
#if TEST_SIMD8
  ker_path = do_kiss_path("dct_kernels_0.bin8", device);
#else
  ker_path = do_kiss_path("dct_kernels_0.bin", device);
#endif
  cl_context context = clCreateContext(0, 1, &device, NULL, NULL, &err); CHK_ERR(err);
  cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err); CHK_ERR(err);
  cl_kernel kernel = getKernelFromBinary(device, context, ker_path, "DCT");

  int bytes  = width * height * sizeof(cl_float);
  input  = newBuffer(bytes, 0);
  output = newBuffer(bytes, 0);

  for (i = 0; i < width*height; i++)
    input[i] = randf() * 255.0f;

  cl_mem inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, bytes, input, &err);
  CHK_ERR(err);
  cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, bytes, output, &err);
  CHK_ERR(err);
  cl_mem dctBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * blockSize, dct8x8, &err);
  CHK_ERR(err);

  /* Execute */
  /*** Set appropriate arguments to the kernel ***/
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &outputBuffer); CHK_ERR(err);
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &inputBuffer); CHK_ERR(err);
  err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &dctBuffer); CHK_ERR(err);
  err = clSetKernelArg(kernel, 3, blockWidth * blockWidth * sizeof(cl_float), NULL); CHK_ERR(err);
  err = clSetKernelArg(kernel, 4, sizeof(cl_uint), &width); CHK_ERR(err);
  err = clSetKernelArg(kernel, 5, sizeof(cl_uint), &blockWidth); CHK_ERR(err);
  err = clSetKernelArg(kernel, 6, sizeof(cl_uint), &inverse); CHK_ERR(err);

  size_t globalThreads[2] = {width, height};
  size_t localThreads[2] = {blockWidth, blockWidth};

  err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalThreads, localThreads, 0, NULL, NULL);
  CHK_ERR(err);

  output = clIntelMapBuffer(outputBuffer, &err);
  verify(dct8x8);
  return 0;
}

/*
 * Reference implementation of the Discrete Cosine Transfrom on the CPU
 */
  cl_uint
getIdx(cl_uint blockIdx, cl_uint blockIdy, cl_uint localIdx, cl_uint localIdy, cl_uint blockWidth, cl_uint globalWidth)
{
  cl_uint globalIdx = blockIdx * blockWidth + localIdx;
  cl_uint globalIdy = blockIdy * blockWidth + localIdy;

  return (globalIdy * globalWidth  + globalIdx);
}

  void 
DCTCPUReference(cl_float *ref,
    const cl_float * input , 
    const cl_float * dct8x8 ,
    const cl_uint    width,
    const cl_uint    height,
    const cl_uint    numBlocksX,
    const cl_uint    numBlocksY,
    const cl_uint    inverse)
{
  cl_float * temp = (cl_float *)malloc(width*height*sizeof(cl_float));

  /* for each block in the image */
  cl_uint blockIdy, blockIdx, i,j, k;
  for(blockIdy=0; blockIdy < numBlocksY; ++blockIdy)
    for(blockIdx=0; blockIdx < numBlocksX; ++blockIdx)
    {
      //  First calculate A^T * X
      for(j=0; j < blockWidth ; ++j)
        for(i = 0; i < blockWidth ; ++i)
        {
          cl_uint index = getIdx(blockIdx, blockIdy, i, j, blockWidth, width);
          cl_float tmp = 0.0f;
          for(k=0; k < blockWidth; ++k)
          {
            // multiply with dct8x8(k,i) if forward DCT and dct8x8(i,k) if inverse DCT
            cl_uint index1 =  (inverse) ? i*blockWidth +k : k*blockWidth + i;
            //input(k,j)
            cl_uint index2 = getIdx(blockIdx, blockIdy, j, k, blockWidth, width);

            tmp += dct8x8[index1]*input[index2];
          }
          temp[index] = tmp;
          //ref[index] = tmp;
        }
      // And now multiply the result of previous step with A i.e. calculate (A^T * X) * A
      for(j=0; j < blockWidth ; ++j)
        for(i = 0; i < blockWidth ; ++i)
        {
          cl_uint index = getIdx(blockIdx, blockIdy, i, j, blockWidth, width);
          cl_float tmp = 0.0f;
          for(k=0; k < blockWidth; ++k)
          {
            //input(i,k)
            cl_uint index1 = getIdx(blockIdx, blockIdy, k, i, blockWidth, width);

            // multiply with dct8x8(k,j) if forward DCT and dct8x8(j,k) if inverse DCT
            cl_uint index2 =  (inverse) ? j*blockWidth +k : k*blockWidth + j;

            tmp += temp[index1]*dct8x8[index2];
          }
          ref[index] = tmp;
        }
    }
  free(temp);
}

void verify(float* dct8x8) 
{
  printf("Passed\n");
  ref = (cl_float*) newBuffer(width*height*sizeof(cl_float), 0);
  DCTCPUReference(ref, input, dct8x8, width, height, width/blockWidth, height/blockWidth, inverse);
#if 1
  int i, j;
  for (j=0; j<height; j++, printf("\n"))
  for (i=0; i<width; i++) {
    printf("[%4.4f %4.4f]", output[i+j*width], ref[i+j*width]);
  }
#endif
  int resC = comparef(output, ref, width * height, 1.0e-6f);
  printf("%d\n", resC);
}


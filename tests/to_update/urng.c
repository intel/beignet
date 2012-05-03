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
static cl_uchar4 *inputImageData, *outputImageData, *refOut;
static int	*pixelData;
static int	width;
static int	height;
static int	pixelSize = sizeof(cl_uchar4);
static int	factor = 25.0;
static int	blockSizeX = 64;
static int	blockSizeY = 1;

int main(int argc, char**argv) 
{
  struct args args = {0};
  int err;
  char *ker_path = NULL;
  char *input_path = NULL;
  char *output_path = NULL;
  parseArgs(argc, argv, &args);

  cl_device_id     device  = getDeviceID(args.d);
  ker_path = do_kiss_path("urng_kernels_0.bin", device);
  input_path = do_kiss_path("urng_input.bmp", NULL);
  output_path = do_kiss_path("urng_output.bmp", NULL);
  cl_context       context = clCreateContext(0, 1, &device, NULL, NULL, &err); CHK_ERR(err);
  cl_command_queue queue   = clCreateCommandQueue(context, device, 0, &err); CHK_ERR(err);
  cl_kernel        kernel  = getKernelFromBinary(device, context, ker_path, "noise_uniform");

  pixelData = readBmp(input_path, &width, &height);
  inputImageData  = (cl_uchar4*) newBuffer(width * height * pixelSize, 0);
  outputImageData = (cl_uchar4*) newBuffer(width * height * pixelSize, 0);
  refOut          = (cl_uchar4*) newBuffer(width * height * pixelSize, 0);
  memcpy(inputImageData, pixelData, width * height * pixelSize);

  cl_mem inputImageBuffer = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, width * height * pixelSize, inputImageData, &err); CHK_ERR(err);
  cl_mem outputImageBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, width * height * pixelSize,
      outputImageData, &err); CHK_ERR(err);

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputImageBuffer); CHK_ERR(err);
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &outputImageBuffer); CHK_ERR(err);
  err = clSetKernelArg(kernel, 2, sizeof(factor), &factor); CHK_ERR(err);

  size_t globalThreads[] = {width, height};
  size_t localThreads[] = {blockSizeX, blockSizeY};

  err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalThreads, localThreads, 0, NULL, NULL);
  CHK_ERR(err);
  outputImageData = clIntelMapBuffer(outputImageBuffer, &err);
  CHK_ERR(err);

  writeBmp((int *) outputImageData, width, height, output_path);
  verify();
  clIntelUnmapBuffer(outputImageBuffer);
  return err;
}

void verify() 
{
  int i;
  // comparef(d, c, MAX, 1.0e-6);
  float mean = 0.0f;
  for(i = 0; i < (int)(width * height); i++) {
    mean += outputImageData[i].s[0] - inputImageData[i].s[0];
  }
  mean /= (width * height * factor);

  if(fabs(mean) < 1.0) {
    printf("%12.8f Passed!\n", mean);
  } else {
    printf("%12.8f Failed!\n", mean);
  }
  printf("Done\n");
}


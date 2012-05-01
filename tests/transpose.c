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
float *input, *output, *refout;
int width = 128;
int height = 128;
int blockSize = 16;

int main(int argc, char**argv) 
{
	struct args args = {0};
	int err, i;

	parseArgs(argc, argv, &args);

	cl_device_id     device  = getDeviceID(args.d);
	cl_context       context = clCreateContext(0, 1, &device, NULL, NULL, &err); CHK_ERR(err);
	cl_command_queue queue   = clCreateCommandQueue(context, device, 0, &err); CHK_ERR(err);
	cl_kernel        kernel  = getKernel(device, context, "transpose_kernels.cl", "matrixTranspose");

	input  = newBuffer(width * height * sizeof(cl_float), 'p');
	output = newBuffer(width * height * sizeof(cl_float), '0');
	refout = newBuffer(width * height * sizeof(cl_float), '0');


	cl_mem inputBuffer  = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      				sizeof(cl_float) * width * height, input, &err); CHK_ERR(err);
	cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                      				sizeof(cl_float) * width * height, output, &err); CHK_ERR(err);

	/* Execute */
	size_t globalThreads[2] = {width, height};
	size_t localThreads [2] = {blockSize, blockSize};


	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &outputBuffer); CHK_ERR(err);
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &inputBuffer);  CHK_ERR(err);
	err = clSetKernelArg(kernel, 2, sizeof(cl_float)*blockSize*blockSize, NULL); CHK_ERR(err);
	err = clSetKernelArg(kernel, 3, sizeof(cl_int), &width);     CHK_ERR(err);
	err = clSetKernelArg(kernel, 4, sizeof(cl_int), &height);    CHK_ERR(err);
	err = clSetKernelArg(kernel, 5, sizeof(cl_int), &blockSize); CHK_ERR(err);
  
	cl_event eND;
	err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalThreads, localThreads,
                 0, NULL, &eND);
	CHK_ERR(err);
    
	err = clEnqueueReadBuffer(queue, outputBuffer, CL_TRUE, 0, width * height * sizeof(cl_float), output,
                0, NULL, &eND); 
	CHK_ERR(err);
    
	verify();
}
void transposeCPU(float *output,  float *input, int width, int height)
{
	int i, j;

	for(j=0; j < height; j++) 
		for(i=0; i < width; i++)
			output[i*height + j] = input[j*width + i];
}


void verify() 
{
	int i;
	transposeCPU(refout, input, width, height);
	comparef(refout, output, width * height, 1e-5f);
	for (i=0; i<20; i++) {
		printf("%12.8f %12.8f\n", refout[i], input[i]);
	}
	printf("Done\n");
}


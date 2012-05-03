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

#define	MAX	1000000
float *a, *b, *c;

int main(int argc, char**argv) 
{
	struct args args = {0};
	int err, i;

	parseArgs(argc, argv, &args);

	cl_device_id     device  = getDeviceID(args.d);
	cl_context       context = clCreateContext(0, 1, &device, NULL, NULL, &err); CHK_ERR(err);
	cl_command_queue queue   = clCreateCommandQueue(context, device, 0, &err); CHK_ERR(err);
	cl_kernel        kernel  = getKernel(device, context, "vadd_kernel.cl", "vadd_gpu");

	a = newBuffer(MAX * sizeof(float), 'f');
	b = newBuffer(MAX * sizeof(float), 'f');
	c = newBuffer(MAX * sizeof(float), '0');

	cl_mem aBuffer = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, MAX * sizeof(float), a, &err); CHK_ERR(err);
	cl_mem bBuffer = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, MAX * sizeof(float), b, &err); CHK_ERR(err);
	cl_mem cBuffer = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, MAX * sizeof(float), c, &err); CHK_ERR(err);

	/* Execute */
	int gws = MAX;
	int lws = 16;

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &aBuffer);  CHK_ERR(err);
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &bBuffer);  CHK_ERR(err);
	err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &cBuffer);  CHK_ERR(err);
	err = clSetKernelArg(kernel, 3, sizeof(size_t), &gws); CHK_ERR(err);

	cl_event eND;
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &gws, &lws, 0, NULL, &eND); CHK_ERR(err);
	err = clEnqueueReadBuffer(queue, cBuffer, CL_TRUE, 0, MAX*sizeof(float), c, 1, &eND, NULL); CHK_ERR(err);

	verify();
}

void verify() 
{
	int i;
	float *d;
	
	d = newBuffer(MAX * sizeof(float), '0');
	for (i = 0; i < MAX; i++) {
		d[i] = a[i] + b[i];
	}
	comparef(d, c, MAX, 1.0e-6);
	printf("Done\n");
}


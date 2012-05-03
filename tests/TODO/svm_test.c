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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <CL/cl.h>
#include <assert.h>

#ifndef _WIN32
#include "CL/cl_intel.h"
#include "common.h"
#endif

#define USE_CL_SOURCE

#ifdef USE_CL_SOURCE
const char *progsrc = 
"__kernel void test1(                                                  \n" \
"   __global char* svm,                                                \n" \
"   uint svmBase,                                                      \n" \
"   uint context)                                                      \n" \
"{                                                                     \n" \
"   int i = get_global_id(0);                                          \n" \
"   //__global int* ptr = (__global int*)(svm+context-svmBase);          \n" \
"   //__global int* ptr = (__global int*)&svm[context-svmBase];          \n" \
"   svm -= svmBase;                                                    \n" \
"   __global int *ptr = (__global int *)&svm[context];                 \n" \
"   ptr[i]=i;                                                     \n" \
"}                                                                     \n" \
"\n";
#else
static const char* progsrc =
"	SHADER test1                               \n"
"	VERSION_2_1                                \n"
"	DCL_THREADGROUP VARIABLE;                  \n"
"	DCL_UAVRAW u0, TRUE =                      \n"
"	{                                          \n"
"	  KERNEL_ARGUMENT,0                        \n"
"	};                                         \n"
"	DCL_CONSTANTREGISTER c0 =                  \n"
"	{                                          \n"
"		KERNEL_ARGUMENT, 1, 0,                 \n"
"		KERNEL_ARGUMENT, 2, 0,                 \n"
"		UNUSED, 0, 0,                          \n"
"		UNUSED, 0, 0                           \n"
"	};                                         \n"
"	DCL_INPUT i0.xyz, THREAD_ID;               \n"
"	DCL_POINTER ptr1;                          \n"
"	DCL_TEMP r0;                               \n"
"	DCL_TEMP r1;                               \n"
"	DCL_POINTER ptr0;                          \n"
"	ADDRESS_OF ptr0, u0;                       \n"
"	DCL_TEMP r2;                               \n"
"	DCL_TEMP r3;                               \n"
"	DCL_TEMP r4;                               \n"
"	DCL_POINTER ptr2;                          \n"
"	DCL_TEMP r5;                               \n"
"	MOV r2.x, i0.x;                            \n"   
"	PADD ptr1, ptr0,-c0.x;                     \n"
"   PADD ptr1, ptr1,c0.y;\n"
"	MOV r0.x, r2.x;                            \n"
"	MOV r5.x, r0.x;                            \n"
"	MOV r1.x, r5.x;                            \n"
"	SHL r3.x, r1.x, 2;                         \n"
//"	MOV r4.x, 0;                               \n"
"	PADD ptr2, ptr1, r3.x;                     \n"
//"   UADD r4.x, r4.x, 2;                        \n"
//"	STORE_RAW_PTR ptr2.x, r4.x;                \n"
"	STORE_RAW_PTR ptr2.x, r2.x;                \n"
"	RET;                                       \n"
"	END                                        \n"
;
#endif

const char *src;
const char *kernel_name;

#define PAGE_SIZE  (4 << 10)
#define PAGE_ALIGNMENT_MASK (~(PAGE_SIZE-1))
#define SVM_SIZE (128 << 20)

cl_device_id device_id;           // device ID
cl_context context;               // context
cl_command_queue queue;           // command queue
char* svmBase = NULL;
cl_mem svmMemObject;

char *load_program_source(const char *filename)
{
	FILE *fh;
	struct stat statbuf;
	char *source;

	if (!(fh=fopen(filename,"r"))) 
		return NULL;

	stat(filename, &statbuf);
	source = (char *)malloc(statbuf.st_size+1);
	fread(source, statbuf.st_size,1,fh);
	source[statbuf.st_size]=0;
	return source;
}

int init() {
	cl_uint num_devices;
	char device_name[1024];
	cl_int err;
	cl_platform_id platform;
	
	err = clGetPlatformIDs(1, &platform, NULL);
	if (err != CL_SUCCESS) {
		fprintf(stderr, "Error: clGetPlatformIDs failed. Error code = %d.\n", err);
		return err;
	}

	// Get an ID for the device
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1,
		&device_id, &num_devices);
	if (err != CL_SUCCESS)
	{
		printf("Error: clGetDeviceIDs");
		return err;
	}
	printf("num_gpu_devices: %d\n",num_devices);
	err = clGetDeviceInfo(device_id,
		CL_DEVICE_NAME,
		1024,
		device_name,
		NULL);
	if (err != CL_SUCCESS) {
		printf("Error: clGetDeviceInfo %d\n",err);
		return err;
	}

	printf("device name is %s\n",device_name);
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	if (!context)
	{ 
		printf("Error: clCreateContext. error code=%d\n",err);
		return err;
	}

	// Create a command queue                                              [5]
	//
	queue = clCreateCommandQueue(context, device_id, 0, &err);
	if (!queue)
	{ 
		printf("Error:clCreateCommandQueue %d",err);
		return err;
	}
#ifdef _WIN32
	svmBase = malloc(SVM_SIZE+PAGE_SIZE);
	(cl_uint)svmBase &= PAGE_ALIGNMENT_MASK;
	svmMemObject = clCreateBuffer(context,  CL_MEM_READ_WRITE | 
		CL_MEM_USE_HOST_PTR,  
		(size_t)SVM_SIZE, 
		svmBase,
		&err);
	if (err != CL_SUCCESS) {
		printf("Error: clCreateBuffer of SVM: %d\n",err);
		return err;
	}
#else // Linux
	svmMemObject = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_PINNABLE,
				      SVM_SIZE, NULL, &err);
	if (err != CL_SUCCESS) {
		printf("Error: clCreateBuffer of SVM: %d\n",err);
		return err;
	}
  if ((err = clIntelPinBuffer(svmMemObject)) != CL_SUCCESS) {
		printf("Error: clIntelPinBuffer: %d\n",err);
		return err;
  }
	svmBase = clIntelMapBuffer(svmMemObject,&err);
	if (err != CL_SUCCESS) {
		printf("Error: clIntelMapBuffer: %d\n",err);
		return err;
	}
#endif

	printf("initializing svm with zeros\n");
	memset(svmBase,0,SVM_SIZE);
	return err;
}

int test2() {
	size_t global;                    // global domain size for our calculation
	size_t local;                     // local domain size for our calculation
	int err;                          // error code returned from api calls
	cl_kernel kernel;
#ifndef _WIN32
  /*cl_event event;*/
#endif
	int i;
	char *start;
	size_t offset;
	int failed;

#ifdef _WIN32
	// Create the compute program from the source buffer                   [6]
	//
	cl_program program = clCreateProgramWithSource(context, 1,
		(const char **) & src, NULL, &err);
	if (!program)
	{ 
		printf("Error:clCreateProgramWithSource");
		return 0;
	}
	// Build the program executable                                        [7]
	//
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		size_t len;
		char buffer[2048];
		//printf("Error: Failed to build program executable\n");             [8]
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG,
			sizeof(buffer), buffer, &len);
		printf("%s\n", buffer);
		exit(1);
	}
#else
	char *ker_path = NULL;
#endif

	// Create the compute kernel in the program we wish to run            [9]
	//
#ifdef _WIN32
	kernel = clCreateKernel(program, kernel_name, &err);
	if (!kernel || err != CL_SUCCESS)
	{ 
		printf("Error: clCreateKernel");
		return 0;
	}
#else
  ker_path = do_kiss_path("svm_test_kernel_0.bin", device_id);
	kernel  = getKernelFromBinary(device_id, context, ker_path, "test1");
#endif
	// Set the arguments to our compute kernel                           [12]
	//
	//offset = SVM_SIZE >> 1;
	offset = 0;
	//offset = 16;
	//offset = 1;
	//global = (64 << 20) / sizeof(int);
	//global = SVM_SIZE / sizeof(int);
	//global = (SVM_SIZE >> 1) / sizeof(int);
	global = 1<<10;
	start = svmBase + offset;
	err = 0;
	err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &svmMemObject);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_uint), &svmBase);
	err |= clSetKernelArg(kernel, 2, sizeof(cl_uint), &start);
	if (err != CL_SUCCESS)
	{ 
		printf("Error:clSetKernelArg %d",err);
		return 0;
	}

#if 0
	// Get the maximum work-group size for executing the kernel on the device
	//                                                                   [13]
	err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE,
		sizeof(size_t), &local, NULL);
	if (err != CL_SUCCESS)
	{ 
		printf("Error:clGetKernelWorkGroupInfo: %d",err);
		return 0;
	}
	printf("workgroup size is %d\n",(int)local);
#endif
	local = 16;
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, 
				     &global, &local /*NULL*/,
				     0, NULL, NULL /*&event*/);
	if (err != CL_SUCCESS)
	{ 
		printf("Error:clEnqueueNDRangeKernel: %d",err);
		return 0;
	}

	clFinish(queue);

	failed = 0;
	for (i=0;i<(int)global;i++)  {
		if (((int*)start)[i]!=i) {
			printf("svmBase[%d]=%d.\n",(int) i+(int) offset,(int) start[i]);
			failed = 1;
		}
	}

	printf(failed?"Test failed\n":"Test passed.\n");

	return 0;
}


int main(int argc, char** argv)
{
	//src = load_program_source(argv[1]);
	src = progsrc;
	//kernel_name = argv[2];
	kernel_name = "test1";
	if (init() != CL_SUCCESS)
		return -1; 
	test2();
	return 0;
}

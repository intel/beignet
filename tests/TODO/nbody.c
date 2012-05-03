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

float *initPos, *initVel, *pos, *vel;
int numBodies = 1024;
float delT = 0.005f;
float espSqr = 50.0f;
int GROUP_SIZE = 256;
int ITERATIONS = 1;

int main(int argc, char**argv) 
{
  char *ker_path = NULL;
  struct args args = {0};
  int err, i;

  parseArgs(argc, argv, &args);

  cl_device_id     device  = getDeviceID(args.d);
#if TEST_SIMD8
  ker_path = do_kiss_path("nbody_kernels_0.bin8", device);
#else
  ker_path = do_kiss_path("nbody_kernels_0.bin", device);
#endif
  cl_context       context = clCreateContext(0, 1, &device, NULL, NULL, &err); CHK_ERR(err);
  cl_command_queue queue   = clCreateCommandQueue(context, device, 0, &err); CHK_ERR(err);
  cl_kernel        kernel  = getKernelFromBinary(device, context, ker_path, "nbody_sim");

  initPos = newBuffer(numBodies * sizeof(cl_float4), 0); 
  initVel = newBuffer(numBodies * sizeof(cl_float4), 0);
  pos     = newBuffer(numBodies * sizeof(cl_float4), 0);
  vel     = newBuffer(numBodies * sizeof(cl_float4), 0);

  /* initialization of inputs */
  for (i = 0; i < numBodies; ++i) {
    int index = 4 * i;
    int j;

    // First 3 values are position in x,y and z direction
    for(j = 0; j < 3; ++j) {
      initPos[index + j] = randf2(3.0f, 50.0f); 
    }

    // Mass value
    initPos[index + 3] = randf2(1.0, 1000.0); 

    // First 3 values are velocity in x,y and z direction
    for (j = 0; j < 3; ++j) {
      initVel[index + j] = 0.0f;
    }

    // unused
    initVel[3] = 0.0f;
  }

  memcpy(pos, initPos, 4 * numBodies * sizeof(cl_float));
  memcpy(vel, initVel, 4 * numBodies * sizeof(cl_float));

  cl_mem curPosBuf = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, numBodies * sizeof(cl_float4),  pos, &err); CHK_ERR(err);
  cl_mem newPosBuf = clCreateBuffer(context, CL_MEM_READ_WRITE,   numBodies * sizeof(cl_float4),    0, &err); CHK_ERR(err);
  cl_mem curVelBuf = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, numBodies * sizeof(cl_float4),  vel, &err); CHK_ERR(err);
  cl_mem newVelBuf = clCreateBuffer(context, CL_MEM_READ_WRITE,   numBodies * sizeof(cl_float4),    0, &err); CHK_ERR(err);

  /* Execute */
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &curPosBuf);
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &curVelBuf);
  err = clSetKernelArg(kernel, 2, sizeof(cl_int), &numBodies);
  err = clSetKernelArg(kernel, 3, sizeof(cl_float), &delT);
  err = clSetKernelArg(kernel, 4, sizeof(cl_float), &espSqr);
  err = clSetKernelArg(kernel, 5, GROUP_SIZE * 4 * sizeof(float), NULL);
  err = clSetKernelArg(kernel, 6, sizeof(cl_mem), &newPosBuf);
  err = clSetKernelArg(kernel, 7, sizeof(cl_mem), &newVelBuf);

  size_t globalThreads[] = {numBodies};
  size_t localThreads[] = {GROUP_SIZE};

  assert(ITERATIONS == 1);
  for (i = 0; i < ITERATIONS; i++) {

    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, globalThreads, localThreads, 0, NULL, NULL); CHK_ERR(err);

    /* Copy data from new to old */
    // err = clEnqueueCopyBuffer(queue, newPosBuf, curPosBuf, 0, 0, sizeof(cl_float4) * numBodies, 0, 0, 0); CHK_ERR(err);
    // err = clEnqueueCopyBuffer(queue, newVelBuf, curVelBuf, 0, 0, sizeof(cl_float4) * numBodies, 0, 0, 0); CHK_ERR(err);
    // err = clFinish(queue); CHK_ERR(err);

    /* Enqueue readBuffer*/
    /* Wait for the read buffer to finish execution */
    // err = clEnqueueReadBuffer(queue, curPosBuf, CL_TRUE, 0, numBodies* sizeof(cl_float4), pos, 0, NULL, NULL); CHK_ERR(err);
  }

  pos = clIntelMapBuffer(newPosBuf, &err);
  verify();
  return 0;
}

void 
nBodyCPU(float* refPos, float* refVel)
{
  int i, j, k;

  //Iterate for all samples
  for(i = 0; i < numBodies; ++i) {
    int myIndex = 4 * i;
    float acc[3] = {0.0f, 0.0f, 0.0f};

    for(j = 0; j < numBodies; ++j) {
      float r[3];
      int index = 4 * j;

      float distSqr = 0.0f;
      for(k = 0; k < 3; ++k) {
        r[k] = refPos[index + k] - refPos[myIndex + k];
        distSqr += r[k] * r[k];
      }

      float invDist = 1.0f / sqrt(distSqr + espSqr);
      float invDistCube =  invDist * invDist * invDist;
      float s = refPos[index + 3] * invDistCube;

      for(k = 0; k < 3; ++k) {
        acc[k] += s * r[k];
      }
    }

    for(k = 0; k < 3; ++k) {
      refPos[myIndex + k] += refVel[myIndex + k] * delT + 0.5f * acc[k] * delT * delT;
      refVel[myIndex + k] += acc[k] * delT;
    }
  }
}

void verify() 
{
  int i;

  float* refPos = newBuffer(numBodies * sizeof(cl_float4), 0); 
  float* refVel = newBuffer(numBodies * sizeof(cl_float4), 0); 
  memcpy(refPos, initPos, 4 * numBodies * sizeof(cl_float));
  memcpy(refVel, initVel, 4 * numBodies * sizeof(cl_float));

  for (i = 0; i < ITERATIONS; i++) {
    nBodyCPU(refPos, refVel);
  }

  comparef(refPos, pos, numBodies * 4, 1.0e-4f);
  for (i = 0; i < 20; i++) {
    printf("%12.8f | %12.8f\n", refPos[i], pos[i]);
  }
  printf("Done\n");
}


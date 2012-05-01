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

float *randArray, *output;
float *refOutput, *stepsArray;
int numSamples = 64;
int numSteps = 255;


int main(int argc, char**argv) 
{
  struct args args = {0};
  int err;
  char *ker_path = NULL;

  parseArgs(argc, argv, &args);

  cl_device_id device = getDeviceID(args.d);
#if TEST_SIMD8
  ker_path = do_kiss_path("binomialOption_kernels_0.bin", device);
#else
  ker_path = do_kiss_path("binomialOption_kernels_0.bin8", device);
#endif
  cl_context context = clCreateContext(0, 1, &device, NULL, NULL, &err); CHK_ERR(err);
  cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err); CHK_ERR(err);
  cl_kernel kernel = getKernelFromBinary(device, context, ker_path, "binomial_options");

  randArray = newBuffer(numSamples * sizeof(cl_float4), 'f');
  output    = newBuffer(numSamples * sizeof(cl_float4),  0 );
  cl_mem randBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
      numSamples * sizeof(cl_float4), randArray, &err);
  CHK_ERR(err);
  cl_mem outBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
      numSamples * sizeof(cl_float4), output, &err);
  CHK_ERR(err);

  /* Execute */
  size_t gws[1] = {numSamples * (numSteps+1)};
  size_t lws[1] = {numSteps + 1};

  err = clSetKernelArg(kernel, 0, sizeof(int), &numSteps); CHK_ERR(err);
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &randBuffer); CHK_ERR(err);
  err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &outBuffer); CHK_ERR(err);
  err = clSetKernelArg(kernel, 3, (numSteps + 1) * sizeof(cl_float4), NULL); CHK_ERR(err);
  err = clSetKernelArg(kernel, 4, numSteps * sizeof(cl_float4), NULL); CHK_ERR(err);
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, gws, lws, 0, NULL, NULL); CHK_ERR(err);
#if 0
  err = clEnqueueReadBuffer(queue, outBuffer, CL_TRUE, 0,
      numSamples * sizeof(cl_float4), output,
      0, NULL, NULL);
#else
  output = clIntelMapBuffer(outBuffer, &err);
  CHK_ERR(err);
#endif

  verify();
  return 0;
}

#define RISKFREE 0.02f
#define VOLATILITY 0.30f

/*
 * Reduces the input array (in place)
 * length specifies the length of the array
 */
  void 
binomialOptionCPU()
{
  refOutput = newBuffer(numSamples * sizeof(cl_float4), 0);
  stepsArray = newBuffer((numSteps + 1) * sizeof(cl_float4), 0);
  int bid;

  /* Iterate for all samples */
  for(bid = 0; bid < numSamples; ++bid)
  {
    float s[4];
    float x[4];
    float vsdt[4];
    float puByr[4];
    float pdByr[4];
    float optionYears[4];

    float inRand[4];
    int i, j, k;


    for(i = 0; i < 4; ++i)
    {
      inRand[i] = randArray[bid + i];
      s[i] = (1.0f - inRand[i]) * 5.0f + inRand[i] * 30.f;
      x[i] = (1.0f - inRand[i]) * 1.0f + inRand[i] * 100.f;
      optionYears[i] = (1.0f - inRand[i]) * 0.25f + inRand[i] * 10.f; 
      float dt = optionYears[i] * (1.0f / (float)numSteps);
      vsdt[i] = VOLATILITY * sqrtf(dt);
      float rdt = RISKFREE * dt;
      float r = expf(rdt);
      float rInv = 1.0f / r;
      float u = expf(vsdt[i]);
      float d = 1.0f / u;
      float pu = (r - d)/(u - d);
      float pd = 1.0f - pu;
      puByr[i] = pu * rInv;
      pdByr[i] = pd * rInv;
    }
    // Compute values at expiration date:
    // Call option value at period end is v(t) = s(t) - x
    // If s(t) is greater than x, or zero otherwise...
    // The computation is similar for put options...
    for(j = 0; j <= numSteps; j++)
    {
      for(i = 0; i < 4; ++i)
      {
        float profit = s[i] * expf(vsdt[i] * (2.0f * j - numSteps)) - x[i];
        stepsArray[j * 4 + i] = profit > 0.0f ? profit : 0.0f;
      }
    }

    //walk backwards up on the binomial tree of depth numSteps
    //Reduce the price step by step
    for(j = numSteps; j > 0; --j)
    {
      for(k = 0; k <= j - 1; ++k)
      {
        for(i = 0; i < 4; ++i)
        {
          stepsArray[k * 4 + i] = pdByr[i] * stepsArray[(k + 1) * 4 + i] + puByr[i] * stepsArray[k * 4 + i];
        }
      }   
    }

    //Copy the root to result
    refOutput[bid] = stepsArray[0];
  }
}

void verify() 
{
  binomialOptionCPU();
  int i;
  for (i=0; i<20; i++) {
    printf("%13.8f %13.8f\n", output[i], refOutput[i]);
  }
  int resC = comparef(output, refOutput, numSamples * 4, 0.001f);
  resC ? printf("Passed\n") : printf("Failed\n");
}


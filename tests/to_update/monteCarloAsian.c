/* 
 * Copyright Â© 2012 Intel Corporation
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
int width = 64;
int height = 64;
int blockSizeX = 256;
int blockSizeY = 1;


cl_float *sigma;                    /**< Array of sigma values */
cl_float *price;                    /**< Array of price values */
cl_float *vega;                     /**< Array of vega values */
cl_float *refPrice;                 /**< Array of reference price values */
cl_float *refVega;                  /**< Array of reference vega values */
cl_uint *randNum;                   /**< Array of random numbers */
cl_float *priceVals;                /**< Array of price values for given samples */
cl_float *priceDeriv;               /**< Array of price derivative values for given samples */
cl_int	steps = 10;
    
 typedef struct _MonteCalroAttrib
 {
     cl_float4 strikePrice;
     cl_float4 c1;
     cl_float4 c2;
     cl_float4 c3;
     cl_float4 initPrice;
     cl_float4 sigma;
     cl_float4 timeStep;
 }MonteCarloAttrib;

        float maturity = 1.f;
	int noOfSum = 12;
	int noOfTraj = 1024;
	float initPrice = 50.f;
	float strikePrice = 55.f;
        float interest = 0.06f;

int main(int argc, char**argv) 
{
	struct args args = {0};
	int err, i;
	const cl_float finalValue = 0.8f;
	const cl_float stepValue = finalValue / (cl_float)steps;

	parseArgs(argc, argv, &args);

	cl_device_id     device  = getDeviceID(args.d);
	cl_context       context = clCreateContext(0, 1, &device, NULL, NULL, &err); CHK_ERR(err);
	cl_command_queue queue   = clCreateCommandQueue(context, device, 0, &err); CHK_ERR(err);
	cl_kernel        kernel  = getKernel(device, context, "monteCarloAsian_kernels.cl", "calPriceVega");

	sigma = (cl_float*) newBuffer(steps * sizeof(cl_float), 0);
	sigma[0] = 0.01f;
	for (i = 1; i < steps; i++) {
		sigma[i] = sigma[i - 1] + stepValue;
	}
    
	price = (cl_float*) newBuffer(steps * sizeof(cl_float), 0);
	vega  = (cl_float*) newBuffer(steps * sizeof(cl_float), 0);
	refPrice = (cl_float*) newBuffer(steps * sizeof(cl_float), 0);
	refVega = (cl_float*) newBuffer(steps * sizeof(cl_float), 0);

	/* Set samples and exercize points */

	width = noOfTraj / 4;
	height = noOfTraj / 2;

	randNum = (cl_uint*) newBuffer(width * height * sizeof(cl_uint4), 0);
	priceVals = (cl_float*) newBuffer(width * height * 2 * sizeof(cl_float4), 0);
	priceDeriv = (cl_float*) newBuffer(width * height * 2 * sizeof(cl_float4), 0);

	cl_mem randBuf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(cl_uint4) * width  * height, randNum, &err);
	CHK_ERR(err);

	cl_mem priceBuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_float4) * width * height * 2, NULL, &err);
	CHK_ERR(err);

	cl_mem priceDerivBuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_float4) * width * height * 2, NULL, &err);
	CHK_ERR(err);

	cl_event eND;
	size_t globalThreads[2] = {width, height};
	size_t localThreads[2] = {blockSizeX, blockSizeY};

	MonteCarloAttrib attributes;

	err = clSetKernelArg(kernel, 2, sizeof(cl_uint), &width); CHK_ERR(err);
	err = clSetKernelArg(kernel, 3, sizeof(cl_mem), &randBuf);  CHK_ERR(err);
	err = clSetKernelArg(kernel, 4, sizeof(cl_mem), &priceBuf);  CHK_ERR(err);
	err = clSetKernelArg(kernel, 5, sizeof(cl_mem), &priceDerivBuf);  CHK_ERR(err);
	err = clSetKernelArg(kernel, 1, sizeof(cl_int), &noOfSum); CHK_ERR(err);

	float timeStep = maturity / (noOfSum - 1);
	// Initialize random number generator
	srand(1);
    
    	int k, j;
	for(k = 0; k < steps; k++) {
		for(j = 0; j < (width * height * 4); j++) {
			randNum[j] = (cl_uint)rand();
        	}

		float c1 = (interest - 0.5f * sigma[k] * sigma[k]) * timeStep;
		float c2 = sigma[k] * sqrt(timeStep);
		float c3 = (interest + 0.5f * sigma[k] * sigma[k]);
        
		const cl_float4 c1F4 = {c1, c1, c1, c1};
		attributes.c1 = c1F4;

		const cl_float4 c2F4 = {c2, c2, c2, c2};
		attributes.c2 = c2F4;

		const cl_float4 c3F4 = {c3, c3, c3, c3};
		attributes.c3 = c3F4;

		cl_float4 initPriceF4 = {initPrice, initPrice, initPrice, initPrice};
		attributes.initPrice = initPriceF4;

		const cl_float4 strikePriceF4 = {strikePrice, strikePrice, strikePrice, strikePrice};
		attributes.strikePrice = strikePriceF4;

		const cl_float4 sigmaF4 = {sigma[k], sigma[k], sigma[k], sigma[k]};
		attributes.sigma = sigmaF4;

		const cl_float4 timeStepF4 = {timeStep, timeStep, timeStep, timeStep};
		attributes.timeStep = timeStepF4;


		/* Set appropriate arguments to the kernel */
		/* the input array - also acts as output for this pass (input for next) */
		err = clSetKernelArg(kernel, 0, sizeof(attributes), &attributes);
		CHK_ERR(err);

		/* 
		 * Enqueue a kernel run call.
		*/
		err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalThreads, localThreads,
                                        0, NULL, &eND);
		CHK_ERR(err);
        
		/* Enqueue the results to application pointer*/
		err = clEnqueueReadBuffer(queue, priceBuf, CL_TRUE, 0, width * height * 2 * sizeof(cl_float4), priceVals,
                                     1, &eND, NULL);
		CHK_ERR(err);
        
	        /* Enqueue the results to application pointer*/
		err = clEnqueueReadBuffer(queue, priceDerivBuf, CL_TRUE, 0, width * height * 2 * sizeof(cl_float4), priceDeriv,
                                     0, NULL, NULL);
		CHK_ERR(err);

		/* Replace Following "for" loop with reduction kernel */
		for (i = 0; i < noOfTraj * noOfTraj; i++) {
			price[k] += priceVals[i];
			vega[k] += priceDeriv[i];
		}

		price[k] /= (noOfTraj * noOfTraj);
		vega[k] /= (noOfTraj * noOfTraj);

		price[k] = exp(-interest * maturity) * price[k];
		vega[k] = exp(-interest * maturity) * vega[k];
    	}
	verify();
}

void
lshift128(unsigned int* input,
          unsigned int shift, 
          unsigned int * output)
{
    unsigned int invshift = 32u - shift;

    output[0] = input[0] << shift;
    output[1] = (input[1] << shift) | (input[0] >> invshift);
    output[2] = (input[2] << shift) | (input[1] >> invshift);
    output[3] = (input[3] << shift) | (input[2] >> invshift);
}

void
rshift128(unsigned int* input, 
          unsigned int shift, 
          unsigned int* output)
{
    unsigned int invshift = 32u - shift;
    output[3]= input[3] >> shift;
    output[2] = (input[2] >> shift) | (input[0] >> invshift);
    output[1] = (input[1] >> shift) | (input[1] >> invshift);
    output[0] = (input[0] >> shift) | (input[2] >> invshift);
}

void
generateRand(unsigned int* seed, 
	float *gaussianRand1,
        float *gaussianRand2,
        unsigned int* nextRand)
{

    unsigned int mulFactor = 4;
    unsigned int temp[8][4];
    
    unsigned int state1[4] = {seed[0], seed[1], seed[2], seed[3]};
    unsigned int state2[4] = {0u, 0u, 0u, 0u}; 
    unsigned int state3[4] = {0u, 0u, 0u, 0u}; 
    unsigned int state4[4] = {0u, 0u, 0u, 0u}; 
    unsigned int state5[4] = {0u, 0u, 0u, 0u}; 
    
    unsigned int stateMask = 1812433253u;
    unsigned int thirty = 30u;
    unsigned int mask4[4] = {stateMask, stateMask, stateMask, stateMask};
    unsigned int thirty4[4] = {thirty, thirty, thirty, thirty};
    unsigned int one4[4] = {1u, 1u, 1u, 1u};
    unsigned int two4[4] = {2u, 2u, 2u, 2u};
    unsigned int three4[4] = {3u, 3u, 3u, 3u};
    unsigned int four4[4] = {4u, 4u, 4u, 4u};

    unsigned int r1[4] = {0u, 0u, 0u, 0u}; 
    unsigned int r2[4] = {0u, 0u, 0u, 0u}; 

    unsigned int a[4] = {0u, 0u, 0u, 0u}; 
    unsigned int b[4] = {0u, 0u, 0u, 0u}; 

    unsigned int e[4] = {0u, 0u, 0u, 0u}; 
    unsigned int f[4] = {0u, 0u, 0u, 0u}; 
    
    unsigned int thirteen  = 13u;
    unsigned int fifteen = 15u;
    unsigned int shift = 8u * 3u;

    unsigned int mask11 = 0xfdff37ffu;
    unsigned int mask12 = 0xef7f3f7du;
    unsigned int mask13 = 0xff777b7du;
    unsigned int mask14 = 0x7ff7fb2fu;
    
    const float one = 1.0f;
    const float intMax = 4294967296.0f;
    const float PI = 3.14159265358979f;
    const float two = 2.0f;

    float r[4] = {0.0f, 0.0f, 0.0f, 0.0f}; 
    float phi[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    float temp1[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float temp2[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    int c;
    
    //Initializing states.
    for(c = 0; c < 4; ++c) {
        state2[c] = mask4[c] * (state1[c] ^ (state1[c] >> thirty4[c])) + one4[c];
        state3[c] = mask4[c] * (state2[c] ^ (state2[c] >> thirty4[c])) + two4[c];
        state4[c] = mask4[c] * (state3[c] ^ (state3[c] >> thirty4[c])) + three4[c];
        state5[c] = mask4[c] * (state4[c] ^ (state4[c] >> thirty4[c])) + four4[c];
    }
    
    unsigned int i = 0;
    for(i = 0; i < mulFactor; ++i) {
        switch(i)
        {
            case 0:
                for(c = 0; c < 4; ++c)
                {
                    r1[c] = state4[c];
                    r2[c] = state5[c];
                    a[c] = state1[c];
                    b[c] = state3[c];
                }
                break;
            case 1:
                for(c = 0; c < 4; ++c)
                {
                    r1[c] = r2[c];
                    r2[c] = temp[0][c];
                    a[c] = state2[c];
                    b[c] = state4[c];
                }
                break;
            case 2:
                for(c = 0; c < 4; ++c)
                {
                    r1[c] = r2[c];
                    r2[c] = temp[1][c];
                    a[c] = state3[c];
                    b[c] = state5[c];
                }
                break;
            case 3:
                for(c = 0; c < 4; ++c)
                {
                    r1[c] = r2[c];
                    r2[c] = temp[2][c];
                    a[c] = state4[c];
                    b[c] = state1[c];
                }
                break;
            default:
                break;            
                
        }
        
        lshift128(a, shift, e);
        rshift128(r1, shift, f);
        
        temp[i][0] = a[0] ^ e[0] ^ ((b[0] >> thirteen) & mask11) ^ f[0] ^ (r2[0] << fifteen);
        temp[i][1] = a[1] ^ e[1] ^ ((b[1] >> thirteen) & mask12) ^ f[1] ^ (r2[1] << fifteen);
        temp[i][2] = a[2] ^ e[2] ^ ((b[2] >> thirteen) & mask13) ^ f[2] ^ (r2[2] << fifteen);
        temp[i][3] = a[3] ^ e[3] ^ ((b[3] >> thirteen) & mask14) ^ f[3] ^ (r2[3] << fifteen);
 
    }        

    for(c = 0; c < 4; ++c) {    
        temp1[c] = temp[0][c] * one / intMax;
        temp2[c] = temp[1][c] * one / intMax;
    }

    for(c = 0; c < 4; ++c) { 
        // Applying Box Mullar Transformations.
        r[c] = sqrt((-two) * log(temp1[c]));
        phi[c]  = two * PI * temp2[c];
        gaussianRand1[c] = r[c] * cos(phi[c]);
        gaussianRand2[c] = r[c] * sin(phi[c]);
        
        nextRand[c] = temp[2][c];
    }
}

void 
calOutputs(float strikePrice, 
           float* meanDeriv1, 
           float*  meanDeriv2, 
           float* meanPrice1,
           float* meanPrice2, 
           float* pathDeriv1, 
           float* pathDeriv2, 
           float* priceVec1, 
           float* priceVec2)
{
    float temp1[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	float temp2[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	float temp3[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	float temp4[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	
    float tempDiff1[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	float tempDiff2[4] = {0.0f, 0.0f, 0.0f, 0.0f};

	int c;

    for(c = 0; c < 4; ++c)
    {
    	tempDiff1[c] = meanPrice1[c] - strikePrice;
	    tempDiff2[c] = meanPrice2[c] - strikePrice;
    }
	if(tempDiff1[0] > 0.0f)
	{
		temp1[0] = 1.0f;
		temp3[0] = tempDiff1[0];
	}
	if(tempDiff1[1] > 0.0f)
	{
		temp1[1] = 1.0f;
		temp3[1] = tempDiff1[1];
	}
	if(tempDiff1[2] > 0.0f)
	{
		temp1[2] = 1.0f;
		temp3[2] = tempDiff1[2];
	}
	if(tempDiff1[3] > 0.0f)
	{
		temp1[3] = 1.0f;
		temp3[3] = tempDiff1[3];
	}

	if(tempDiff2[0] > 0.0f)
	{
		temp2[0] = 1.0f;
		temp4[0] = tempDiff2[0];
	}
	if(tempDiff2[1] > 0.0f)
	{
		temp2[1] = 1.0f;
		temp4[1] = tempDiff2[1];
	}
	if(tempDiff2[2] > 0.0f)
	{
		temp2[2] = 1.0f;
		temp4[2] = tempDiff2[2];
	}
	if(tempDiff2[3] > 0.0f)
	{
		temp2[3] = 1.0f;
		temp4[3] = tempDiff2[3];
	}
	
    for(c = 0; c < 4; ++c) {
	    pathDeriv1[c] = meanDeriv1[c] * temp1[c]; 
	    pathDeriv2[c] = meanDeriv2[c] * temp2[c]; 
	    priceVec1[c] = temp3[c]; 
	    priceVec2[c] = temp4[c];	
    }
}

void cpuRef()
{

	float timeStep = maturity / (noOfSum - 1);
    
	// Initialize random number generator
	srand(1);
	int i, j, k, c; 
    
	for(k = 0; k < steps; k++) {
		float c1 = (interest - 0.5f * sigma[k] * sigma[k]) * timeStep;
		float c2 = sigma[k] * sqrt(timeStep);
		float c3 = (interest + 0.5f * sigma[k] * sigma[k]);  

		for(j = 0; j < (width * height); j++) {
			unsigned int nextRand[4] = {0u, 0u, 0u, 0u};
			for (c = 0; c < 4; ++c)
				nextRand[c] = (cl_uint)rand();

			float trajPrice1[4] = {initPrice, initPrice, initPrice, initPrice};
			float sumPrice1[4] = {initPrice, initPrice, initPrice, initPrice};
			float sumDeriv1[4] = {0.0f, 0.0f, 0.0f, 0.0f};
			float meanPrice1[4] = {0.0f, 0.0f, 0.0f, 0.0f};
			float meanDeriv1[4] = {0.0f, 0.0f, 0.0f, 0.0f};
			float price1[4] = {0.0f, 0.0f, 0.0f, 0.0f};
			float pathDeriv1[4] = {0.0f, 0.0f, 0.0f, 0.0f};

			float trajPrice2[4] = {initPrice, initPrice, initPrice, initPrice};
			float sumPrice2[4] = {initPrice, initPrice, initPrice, initPrice};
			float sumDeriv2[4] = {0.0f, 0.0f, 0.0f, 0.0f};
			float meanPrice2[4] = {0.0f, 0.0f, 0.0f, 0.0f};
			float meanDeriv2[4] = {0.0f, 0.0f, 0.0f, 0.0f};
			float price2[4] = {0.0f, 0.0f, 0.0f, 0.0f};
			float pathDeriv2[4] = {0.0f, 0.0f, 0.0f, 0.0f};

			//Run the Monte Carlo simulation a total of Num_Sum - 1 times
			for(i = 1; i < noOfSum; i++) {
				unsigned int tempRand[4] =  {0u, 0u, 0u, 0u};
				for(c = 0; c < 4; ++c)
					tempRand[c] = nextRand[c];

				float gaussian1[4] = {0.0f, 0.0f, 0.0f, 0.0f};
				float gaussian2[4] = {0.0f, 0.0f, 0.0f, 0.0f};
				generateRand(tempRand, gaussian1, gaussian2, nextRand);
                
				//Calculate the trajectory price and sum price for all trajectories
				for(c = 0; c < 4; ++c) {
					trajPrice1[c] = trajPrice1[c] * exp(c1 + c2 * gaussian1[c]);
					trajPrice2[c] = trajPrice2[c] * exp(c1 + c2 * gaussian2[c]);
			    
					sumPrice1[c] = sumPrice1[c] + trajPrice1[c];
					sumPrice2[c] = sumPrice2[c] + trajPrice2[c];
			    
					float temp = c3 * timeStep * i;
			    
					// Calculate the derivative price for all trajectories
					sumDeriv1[c] = sumDeriv1[c] + trajPrice1[c] 
						* ((log(trajPrice1[c] / initPrice) - temp) / sigma[k]);
	    
					sumDeriv2[c] = sumDeriv2[c] + trajPrice2[c] 
						* ((log(trajPrice2[c] / initPrice) - temp) / sigma[k]);			            
                		}
			}
    
   			//Calculate the average price and “average derivative” of each simulated path
			for(c = 0; c < 4; ++c) {
				meanPrice1[c] = sumPrice1[c] / noOfSum;
				meanPrice2[c] = sumPrice2[c] / noOfSum;
				meanDeriv1[c] = sumDeriv1[c] / noOfSum;
				meanDeriv2[c] = sumDeriv2[c] / noOfSum;
			}

			calOutputs(strikePrice, meanDeriv1, meanDeriv2, meanPrice1, meanPrice2, 
                       		 pathDeriv1, pathDeriv2, price1, price2);

			for(c = 0; c < 4; ++c) {
				priceVals[j * 8 + c] = price1[c];
				priceVals[j * 8 + 1 * 4 + c] = price2[c];
				priceDeriv[j * 8 + c] = pathDeriv1[c]; 
				priceDeriv[j * 8 + 1 * 4 + c] = pathDeriv2[c];
            		}
        	}
       
		/* Replace Following "for" loop with reduction kernel */
		for(i = 0; i < noOfTraj * noOfTraj; i++) {
			refPrice[k] += priceVals[i];
			refVega[k] += priceDeriv[i];
		}

		refPrice[k] /= (noOfTraj * noOfTraj);
		refVega[k] /= (noOfTraj * noOfTraj);

		refPrice[k] = exp(-interest * maturity) * refPrice[k];
		refVega[k] = exp(-interest * maturity) * refVega[k];
    	}

        /* compare the results and see if they match */
        for(i = 0; i < steps; ++i)
        {
            if(fabs(price[i] - refPrice[i]) > 0.2f)
            {
		printf("Failed\n");
		exit(-1);
            }
            if(fabs(vega[i] - refVega[i]) > 0.2f)
            {
		printf("Failed\n");
		exit(-1);
            }
        }
	printf("Passed\n");
}

void verify() 
{
	int i;
	// comparef(d, c, MAX, 1.0e-6);
	cpuRef();
	printf("Done\n");
}


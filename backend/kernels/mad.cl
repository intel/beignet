#include "stdlib.h"
__attribute__((pure, overloadable)) int mad(int,int,int);
__attribute__((pure, overloadable)) float mad(float,float,float);
__attribute__((pure, overloadable)) float4 mad(float4,float4,float4);

__kernel void add(__global int *dst, unsigned int x, float z)
{
  for (int i = 0; i < x; ++i) {
    int y = mad(dst[i], 2, 3);
    y = mad(dst[i], 2, 3);
    float z = mad((float) dst[i], 2.f, 3.f);
    float4 z0 = mad((float4) dst[i], (float4)(0.f,1.f,2.f,3.f), (float4)3.f);
    float4 x0 = z0 * (float4) 2.f;
    dst[i] = y + (int) z + x0.x + x0.y + x0.z;
  }
}



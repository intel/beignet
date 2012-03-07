#include "stdlib.h"
__kernel void insert(__global int4 *dst, __global int4 *src, int c)
{
  int4 x = src[0];
  src[0].z = 1.f;
  dst[0] = src[0];
}


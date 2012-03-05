#include "stdlib.h"
__kernel void insert(__global int4 *dst, __global int4 *src, int c)
{
  dst[0].x = dst[0][c];
  dst[0].yzw = dst[1].xyz + src[0].xyz;
}


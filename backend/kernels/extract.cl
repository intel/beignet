#include "stdlib.h"
__kernel void extract(__global int4 *dst, __global int4 *src, int c)
{
  const int4 from = src[0];
  dst[0] = (int4)(from.x, 1, 2, 3);
}



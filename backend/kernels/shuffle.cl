#include <stdlib.h>
__kernel void shuffle(__global int4 *dst, __global int4 *src, int c)
{
  const int4 from = src[0];
  dst[0] = from.xywz;
}


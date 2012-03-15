#include "stdlib.h"

void write(__global int *dst)
{
  dst[0] = 1;
}

__kernel void write2(__global int *dst, int x)
{
  write(dst);
  dst[x] = 1;
}


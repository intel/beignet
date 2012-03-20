#include "stdlib.h"
__kernel void add(__global unsigned int *dst, unsigned int x, unsigned int y)
{
  dst[0] = x + y;
}



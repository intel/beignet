#include <stdlib.h>
__kernel void add(__global int *dst, unsigned int x)
{
  for (int i = 0; i < x; ++i) dst[i]++;
}


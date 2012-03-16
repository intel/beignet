#include "stdlib.h"

struct big { int x[10]; };

__kernel void add(__global int *dst, unsigned int x, struct big b)
{
  for (int i = 0; i < x; ++i) {
    if (dst[i+1] > 0)
      dst[i]++;
    else
      dst[i] += 2;
  }
}


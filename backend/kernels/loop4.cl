#include "stdlib.h"

struct big { int x[10]; };

__kernel void add(__global int *dst, unsigned int x, struct big b)
{
  if (get_local_id(1) > 4)
    for (int i = 0; i < x; ++i) dst[get_local_id(0) + i]++;
  else
    for (int i = 0; i < 2*x; ++i) dst[get_local_id(0) + i + x]++;
}


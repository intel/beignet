#include "stdlib.h"

__kernel void test_global_id(__global int *dst, __global int *p)
{
  short hop = get_local_id(0);
  dst[get_global_id(0)] = hop;
  p[get_global_id(0)] = get_local_id(0);
}



#include <stdlib.h>
__kernel void test_select(__global int *dst, __global int *src)
{

  if (src[get_global_id(0)] > 1)
    dst[get_global_id(0)] = 1;
  else
    dst[get_global_id(0)] = 2;
}


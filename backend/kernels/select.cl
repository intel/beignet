#include "stdlib.h"

__kernel void test_select(__global int4 *dst,
                          __global int4 *src0,
                          __global int4 *src1)
{
  const int4 from = select(src0[0], src0[1], src0[1]);
  dst[0] = from;
}


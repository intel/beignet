#include <stdlib.h>

__kernel void simple_float4(__global float4 *dst, __global float4 *src, bool b)
{
  dst[get_global_id(0)] = select(b, src[get_global_id(0)], src[get_global_id(1)]);
  dst[get_global_id(0)] += (float4) (src[2].x, 1.f, 2.f, 3.f);
}


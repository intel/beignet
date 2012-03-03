#include "stdlib.h"

__kernel void simple_float4(__global float4 *dst, __global float4 *src)
{
  dst[get_global_id(0)] = src[get_global_id(0)] * src[get_global_id(0)];
}


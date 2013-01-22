__kernel void
compiler_integer_division(__global int *src, __global int *dst, int x)
{
  dst[get_global_id(0)] = src[get_global_id(0)] / x;
}


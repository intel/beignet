__kernel void
test_trigo(__global float *dst, __global const float *src)
{
  const int offset = get_global_id(0);
  dst[offset] = sin(src[offset]);
}


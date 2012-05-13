__kernel void
compiler_uint2_copy(__global uint2 *src, __global uint2 *dst)
{
  int id = (int)get_global_id(0);
  dst[id] = src[id];
}


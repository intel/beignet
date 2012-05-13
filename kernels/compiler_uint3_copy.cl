__kernel void
compiler_uint3_copy(__global uint3 *src, __global uint3 *dst)
{
  int id = (int)get_global_id(0);
  dst[id] = src[id];
}


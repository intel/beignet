__kernel void
compiler_uint8_copy(__global uint8 *src, __global uint8 *dst)
{
  int id = (int)get_global_id(0);
  dst[id] = src[id];
}


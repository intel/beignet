__kernel void
compiler_uint16_copy(__global uint16 *src, __global uint16 *dst)
{
  int id = (int)get_global_id(0);
  dst[id] = src[id];
}



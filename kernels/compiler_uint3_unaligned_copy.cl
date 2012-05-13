__kernel void
compiler_uint3_unaligned_copy(__global uint *src, __global uint *dst)
{
  const int id = (int)get_global_id(0);
  const uint3 from = vload3(id, src);
  vstore3(from, id, dst);
}


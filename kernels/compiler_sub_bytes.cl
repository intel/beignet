__kernel void
compiler_sub_bytes(__global char *src0, __global char *src1, __global char *dst)
{
  int id = (int)get_global_id(0);
  dst[id] = src0[id] - src1[id];
}


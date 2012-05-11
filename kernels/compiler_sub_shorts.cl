__kernel void
compiler_sub_shorts(__global short *src0, __global short *src1, __global short *dst)
{
  int id = (int)get_global_id(0);
  dst[id] = src0[id] - src1[id];
}


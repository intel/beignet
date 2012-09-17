__kernel void
compiler_vote_all(__global uint *src, __global uint *dst)
{
  int id = (int)get_global_id(0);
  if (__gen_ocl_all(id > 8))
    dst[id] = src[id];
  else
    dst[id] = 0;
}


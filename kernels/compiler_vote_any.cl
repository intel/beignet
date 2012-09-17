__kernel void
compiler_vote_any(__global uint *src, __global uint *dst)
{
  int id = (int)get_global_id(0);
  if (__gen_ocl_any(id > 6))
    dst[id] = src[id];
  else
    dst[id] = 0;
}


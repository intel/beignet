__kernel void
compiler_obwrite(__global uint *src, __global uint *dst)
{
  int id = (int)get_global_id(0);
  const int to =  src[id];
  __gen_ocl_obwrite(dst+id,to);
}


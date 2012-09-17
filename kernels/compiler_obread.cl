__kernel void
compiler_obread(__global uint *src, __global uint *dst)
{
  int id = (int)get_global_id(0);
  const int to =  __gen_ocl_obread(src+id);
  dst[id] = to;
}


__kernel void
compiler_gather_register_file(__global uint *src, __global uint *dst)
{
  __gen_ocl_force_simd16();
  int id = (int)get_global_id(0);
  const int x0 = src[id];
  const unsigned short index = get_global_id(0);
  dst[id] = __gen_ocl_rgather(index, x0);
}


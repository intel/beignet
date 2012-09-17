__kernel void
compiler_region(__global uint *src, __global uint *dst)
{
  __gen_ocl_force_simd16();
  int id = (int)get_global_id(0);
  const int x0 = src[id];
  const int x1 = src[id+16];
  dst[id] = __gen_ocl_region(0, 16, 8, 2, x0, x1);
}


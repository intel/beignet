__kernel void
compiler_region0(__global uint *src, __global uint *dst)
{
  __gen_ocl_force_simd16();
  int id = (int)get_global_id(0);
  const int x0 = src[id];
  const int x1 = src[id+16];
  const int x2 = src[id+32];
  dst[id] = __gen_ocl_region(1, 16, 8, 2, x0, x1, x2);
}


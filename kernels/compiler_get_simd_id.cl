__kernel void compiler_get_simd_id(global int *dst)
{
  int i = get_global_id(0);
  if (i == 0)
    dst[0] = __gen_ocl_get_simd_size();

  dst[i+1] = __gen_ocl_get_simd_id();
}

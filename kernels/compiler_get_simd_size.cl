__kernel void compiler_get_simd_size(global int *dst)
{
  int i = get_global_id(0);
  dst[i] = __gen_ocl_get_simd_size();
}

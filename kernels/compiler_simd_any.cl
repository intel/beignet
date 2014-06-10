__kernel void compiler_simd_any(global int *src, global int *dst)
{
  int i = get_global_id(0);

  if (i % 2 == 1) {
    if (__gen_ocl_simd_any(src[i] == 5) || __gen_ocl_simd_any(src[i] == 9))
      dst[i] = 1;
    else if (__gen_ocl_simd_any(src[i] == 6))
      dst[i] = 0;
    else
      dst[i] = 2;
  }
  else
    dst[i] = 3;
}

__kernel void compiler_simd_all(global int *src, global int *dst)
{
  int i = get_global_id(0);
  if (i % 2 == 1) {
    if (__gen_ocl_simd_all((src[i] < 12) && (src[i] > 0)))
      dst[i] = 1;
    else
      dst[i] = 2;
  }
  else
    dst[i] = 3;
}

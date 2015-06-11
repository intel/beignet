#pragma OPENCL EXTENSION cl_khr_fp16 : enable
kernel void compiler_half_isnan(global half2 *src, global short2 *dst) {
  int i = get_global_id(0);
  dst[i] = isnan(src[i]);
}

kernel void compiler_half_isinf(global half *src, global int *dst) {
  int i = get_global_id(0);
  dst[i] = isinf(src[i]);
}

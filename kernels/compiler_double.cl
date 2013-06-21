#pragma OPENCL EXTENSION cl_khr_fp64 : enable
kernel void compiler_double(global double *src, global double *dst) {
  int i = get_global_id(0);
  double d = 1.234567890123456789;
  if (i < 14)
    dst[i] = d * (src[i] + d);
  else
    dst[i] = 14;
}

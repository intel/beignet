#pragma OPENCL EXTENSION cl_khr_fp64 : enable
kernel void compiler_double_2(global float *src, global double *dst) {
  int i = get_global_id(0);
  float d = 1.234567890123456789f;
  if (i < 14)
    dst[i] = d * (d + src[i]);
  else
    dst[i] = 14;
}

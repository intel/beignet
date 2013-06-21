#pragma OPENCL EXTENSION cl_khr_fp64 : enable
kernel void compiler_double_3(global float *src, global double *dst) {
  int i = get_global_id(0);
  float d = 1.234567890123456789f;
  dst[i] = i < 14 ? d : 14;
}

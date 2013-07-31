#pragma OPENCL EXTENSION cl_khr_fp64 : enable
kernel void compiler_double_4(global double *src1, global double *src2, global double *dst) {
  int i = get_global_id(0);
  dst[i] = src1[i] + src2[i];
}

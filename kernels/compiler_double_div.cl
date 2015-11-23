#pragma OPENCL EXTENSION cl_khr_fp64 : enable
kernel void compiler_double_div(global double *src1, global double *src2, global double *dst) {
  int i = get_global_id(0);
  if (i % 3 != 0)
    dst[i] = src1[i] / src2[i];
  else
    dst[i] = 0.0;
}

kernel void compiler_double_div_uniform(double src1, double src2, double tmp, global double *dst) {
  tmp = src1 / src2;
  dst[0] = tmp;
}

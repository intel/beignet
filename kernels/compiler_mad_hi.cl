kernel void compiler_mad_hi(global int *src1, global int *src2, global int *src3, global int *dst) {
  int i = get_global_id(0);
  dst[i] = mad_hi(src1[i], src2[i], src3[i]);
}

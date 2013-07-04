kernel void compiler_mad24(global int *src1, global int *src2, global int *src3, global int *dst) {
  int i = get_global_id(0);
  dst[i] = mad24(src1[i], src2[i], src3[i]);
}

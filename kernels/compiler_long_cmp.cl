kernel void compiler_long_cmp_l(global long *src1, global long *src2, global long *dst) {
  int i = get_global_id(0);
  dst[i] = (src1[i] < src2[i]) ? 3 : 4;
}

kernel void compiler_long_cmp_le(global long *src1, global long *src2, global long *dst) {
  int i = get_global_id(0);
  dst[i] = (src1[i] <= src2[i]) ? 3 : 4;
}

kernel void compiler_long_cmp_g(global long *src1, global long *src2, global long *dst) {
  int i = get_global_id(0);
  dst[i] = (src1[i] > src2[i]) ? 3 : 4;
}

kernel void compiler_long_cmp_ge(global long *src1, global long *src2, global long *dst) {
  int i = get_global_id(0);
  dst[i] = (src1[i] >= src2[i]) ? 3 : 4;
}

kernel void compiler_long_cmp_eq(global long *src1, global long *src2, global long *dst) {
  int i = get_global_id(0);
  dst[i] = (src1[i] == src2[i]) ? 3 : 4;
}

kernel void compiler_long_cmp_neq(global long *src1, global long *src2, global long *dst) {
  int i = get_global_id(0);
  dst[i] = (src1[i] != src2[i]) ? 3 : 4;
}

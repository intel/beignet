kernel void compiler_mul_hi(global int *src1, global int *src2, global int *dst) {
  int i = get_global_id(0);
  dst[i] = mul_hi(src1[i], src2[i]);
}

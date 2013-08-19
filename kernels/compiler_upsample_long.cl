kernel void compiler_upsample_long(global int *src1, global uint *src2, global long *dst) {
  int i = get_global_id(0);
  dst[i] = upsample(src1[i], src2[i]);
}

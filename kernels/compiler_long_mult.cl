kernel void compiler_long_mult(global long *src1, global long *src2, global long *dst) {
  int i = get_global_id(0);
  if(i < 3)
    dst[i] = src1[i] + src2[i];
  else
    dst[i] = src1[i] * src2[i];
}

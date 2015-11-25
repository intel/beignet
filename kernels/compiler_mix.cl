kernel void compiler_mix(global float *src1, global float *src2, global float *src3, global float *dst) {
  int i = get_global_id(0);
  dst[i] = mix(src1[i], src2[i], src3[i]);
}

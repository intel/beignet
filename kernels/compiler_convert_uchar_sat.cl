kernel void compiler_convert_uchar_sat(global float *src, global uint *dst) {
  int i = get_global_id(0);
  dst[i] = convert_uchar_sat(src[i]);
}

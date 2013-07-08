kernel void compiler_degrees(global float *src, global float *dst) {
  int i = get_global_id(0);
  dst[i] = degrees(src[i]);
}

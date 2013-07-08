kernel void compiler_radians(global float *src, global float *dst) {
  int i = get_global_id(0);
  dst[i] = radians(src[i]);
}

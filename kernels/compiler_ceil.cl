kernel void compiler_ceil(global float *src, global float *dst) {
  int i = get_global_id(0);
  dst[i] = ceil(src[i]);
}

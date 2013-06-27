kernel void compiler_rotate(global int *src, global int *dst, global int *y) {
  int i = get_global_id(0);
  dst[i] = rotate(src[i], y[i]);
}


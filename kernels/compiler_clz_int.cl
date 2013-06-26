kernel void compiler_clz_int(global int *src, global int *dst) {
  int i = get_global_id(0);
  dst[i] = clz(src[i]);
}


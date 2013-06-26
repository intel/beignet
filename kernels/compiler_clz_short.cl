kernel void compiler_clz_short(global short *src, global short *dst) {
  int i = get_global_id(0);
  dst[i] = clz(src[i]);
}


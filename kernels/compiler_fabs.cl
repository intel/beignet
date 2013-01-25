kernel void compiler_fabs(global float *src, global float *dst) {
  int i = get_global_id(0);
  dst[i] = fabs(src[i]);
}


kernel void compiler_remove_negative_add(global float *src, global float *dst) {
  int i = get_global_id(0);
  dst[i] = exp2(-src[i]);
};

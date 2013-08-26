kernel void builtin_tgamma(global float *src, global float *dst) {
  int i = get_global_id(0);
  dst[i] = tgamma(src[i]);
};

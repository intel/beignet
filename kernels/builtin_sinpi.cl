kernel void builtin_sinpi(global float *src, global float *dst) {
  int i = get_global_id(0);
  dst[i] = sinpi(src[i]);
};

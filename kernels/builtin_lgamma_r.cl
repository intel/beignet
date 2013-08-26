kernel void builtin_lgamma_r(global float *src, global float *dst, global int *signp) {
  int i = get_global_id(0);
  dst[i] = lgamma_r(src[i], signp+i);
};

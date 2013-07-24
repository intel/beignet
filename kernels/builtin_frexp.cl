kernel void builtin_frexp(global float *src, global float *dst, global int *e) {
  int i = get_global_id(0);
  dst[i] = frexp(src[i], &e[i]);
}

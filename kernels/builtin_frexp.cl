kernel void builtin_frexp(global float *src, global float *dst, global int *e) {
  int i = get_global_id(0);
  int v;
  dst[i] = frexp(src[i], &v);
  e[i] = v;
}

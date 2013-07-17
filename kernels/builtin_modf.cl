kernel void builtin_modf(global float *src, global float *dst, global float *it) {
  int i = get_global_id(0);
  float x;
  dst[i] = modf(src[i], &x);
  it[i] = x;
}

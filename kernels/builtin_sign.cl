kernel void builtin_sign(global float *src, global float *dst) {
  int i = get_global_id(0);
  dst[i] = sign(src[i]);
}

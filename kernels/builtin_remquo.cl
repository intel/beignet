kernel void builtin_remquo(global float *x, global float *y, global float *dst, global int *quo) {
  int i = get_global_id(0);
  int q;
  dst[i] = remquo(x[i], y[i], & q);
  quo[i] = q;
}

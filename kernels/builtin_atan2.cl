kernel void builtin_atan2(global float *y, global float *x, global float *dst) {
  int i = get_global_id(0);
  dst[i] = atan2(y[i], x[i]);
};

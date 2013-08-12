__kernel void builtin_acos_asin(__global float *dst, __global float *src, __global int *max_func) {
  int i = get_global_id(0);
  float x = src[i];

  dst[i * (*max_func) + 0] = acos(x);
  dst[i * (*max_func) + 1] = acosh(x);
  dst[i * (*max_func) + 2] = asin(x);
  dst[i * (*max_func) + 3] = asinh(x);
  dst[i * (*max_func) + 4] = x;
};

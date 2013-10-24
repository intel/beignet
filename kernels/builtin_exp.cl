__kernel void builtin_exp(__global float *dst, __global float *src, __global int *max_func) {
  int i = get_global_id(0);
  float x = src[i];

  dst[i * (*max_func) + 0] = exp(x);
  dst[i * (*max_func) + 1] = exp2(x);
  dst[i * (*max_func) + 2] = exp10(x);
  dst[i * (*max_func) + 3] = expm1(x);
  dst[i * (*max_func) + 4] = x;
};

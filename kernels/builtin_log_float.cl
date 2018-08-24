__kernel void builtin_log_float(__global float *dst,  __global float *src1, __global int *vector) {
  int i = get_global_id(0);
  float x1 = (float) (src1[i * (*vector) + 0]);

  float ret;
  ret = log(x1);
  dst[i] = ret;
};
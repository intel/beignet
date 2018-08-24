__kernel void builtin_fdim_float(__global float *dst,  __global float *src1, __global float *src2, __global int *vector) {
  int i = get_global_id(0);
  float x1 = (float) (src1[i * (*vector) + 0]);
  float x2 = (float) (src2[i * (*vector) + 0]);

  float ret;
  ret = fdim(x1,x2);
  dst[i] = ret;
};
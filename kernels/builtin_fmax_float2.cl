__kernel void builtin_fmax_float2(__global float *dst,  __global float *src1, __global float *src2, __global int *vector) {
  int i = get_global_id(0);
  float2 x1 = (float2) (src1[i * (*vector) + 0],src1[i * (*vector) + 1]);
  float2 x2 = (float2) (src2[i * (*vector) + 0],src2[i * (*vector) + 1]);

  float2 ret;
  ret = fmax(x1,x2);
  dst[i * (*vector) + 0] = ret[0];
  dst[i * (*vector) + 1] = ret[1];
};
__kernel void builtin_ilogb_float4(__global int *dst,  __global float *src1, __global int *vector) {
  int i = get_global_id(0);
  float4 x1 = (float4) (src1[i * (*vector) + 0],src1[i * (*vector) + 1],src1[i * (*vector) + 2],src1[i * (*vector) + 3]);

  int4 ret;
  ret = ilogb(x1);
  dst[i * (*vector) + 0] = ret[0];
  dst[i * (*vector) + 1] = ret[1];
  dst[i * (*vector) + 2] = ret[2];
  dst[i * (*vector) + 3] = ret[3];
};
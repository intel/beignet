__kernel void builtin_rootn_float8(__global float *dst,  __global float *src1, __global int *src2, __global int *vector) {
  int i = get_global_id(0);
  float8 x1 = (float8) (src1[i * (*vector) + 0],src1[i * (*vector) + 1],src1[i * (*vector) + 2],src1[i * (*vector) + 3],src1[i * (*vector) + 4],src1[i * (*vector) + 5],src1[i * (*vector) + 6],src1[i * (*vector) + 7]);
  int8 x2 = (int8) (src2[i * (*vector) + 0],src2[i * (*vector) + 1],src2[i * (*vector) + 2],src2[i * (*vector) + 3],src2[i * (*vector) + 4],src2[i * (*vector) + 5],src2[i * (*vector) + 6],src2[i * (*vector) + 7]);

  float8 ret;
  ret = rootn(x1,x2);
  dst[i * (*vector) + 0] = ret[0];
  dst[i * (*vector) + 1] = ret[1];
  dst[i * (*vector) + 2] = ret[2];
  dst[i * (*vector) + 3] = ret[3];
  dst[i * (*vector) + 4] = ret[4];
  dst[i * (*vector) + 5] = ret[5];
  dst[i * (*vector) + 6] = ret[6];
  dst[i * (*vector) + 7] = ret[7];
};
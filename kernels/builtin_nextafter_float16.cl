__kernel void builtin_nextafter_float16(__global float *dst,  __global float *src1, __global float *src2, __global int *vector) {
  int i = get_global_id(0);
  float16 x1 = (float16) (src1[i * (*vector) + 0],src1[i * (*vector) + 1],src1[i * (*vector) + 2],src1[i * (*vector) + 3],src1[i * (*vector) + 4],src1[i * (*vector) + 5],src1[i * (*vector) + 6],src1[i * (*vector) + 7],src1[i * (*vector) + 8],src1[i * (*vector) + 9],src1[i * (*vector) + 10],src1[i * (*vector) + 11],src1[i * (*vector) + 12],src1[i * (*vector) + 13],src1[i * (*vector) + 14],src1[i * (*vector) + 15]);
  float16 x2 = (float16) (src2[i * (*vector) + 0],src2[i * (*vector) + 1],src2[i * (*vector) + 2],src2[i * (*vector) + 3],src2[i * (*vector) + 4],src2[i * (*vector) + 5],src2[i * (*vector) + 6],src2[i * (*vector) + 7],src2[i * (*vector) + 8],src2[i * (*vector) + 9],src2[i * (*vector) + 10],src2[i * (*vector) + 11],src2[i * (*vector) + 12],src2[i * (*vector) + 13],src2[i * (*vector) + 14],src2[i * (*vector) + 15]);

  float16 ret;
  ret = nextafter(x1,x2);
  dst[i * (*vector) + 0] = ret[0];
  dst[i * (*vector) + 1] = ret[1];
  dst[i * (*vector) + 2] = ret[2];
  dst[i * (*vector) + 3] = ret[3];
  dst[i * (*vector) + 4] = ret[4];
  dst[i * (*vector) + 5] = ret[5];
  dst[i * (*vector) + 6] = ret[6];
  dst[i * (*vector) + 7] = ret[7];
  dst[i * (*vector) + 8] = ret[8];
  dst[i * (*vector) + 9] = ret[9];
  dst[i * (*vector) + 10] = ret[10];
  dst[i * (*vector) + 11] = ret[11];
  dst[i * (*vector) + 12] = ret[12];
  dst[i * (*vector) + 13] = ret[13];
  dst[i * (*vector) + 14] = ret[14];
  dst[i * (*vector) + 15] = ret[15];
};
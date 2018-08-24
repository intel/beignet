__kernel void builtin_ilogb_float(__global int *dst,  __global float *src1, __global int *vector) {
  int i = get_global_id(0);
  float x1 = (float) (src1[i * (*vector) + 0]);

  int ret;
  ret = ilogb(x1);
  dst[i] = ret;
};
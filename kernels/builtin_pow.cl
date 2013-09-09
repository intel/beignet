kernel void builtin_pow(global float *dst, global float *src1, global float *src2, global int *max_func) {

  int i = get_global_id(0);
  dst[i * (*max_func) + 0] = pow(src1[i], src2[i]);
  dst[i * (*max_func) + 1] = src1[i];

}

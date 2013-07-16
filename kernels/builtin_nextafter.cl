kernel void builtin_nextafter(global float *src1, global float *src2, global float *dst) {
  int i = get_global_id(0);
  dst[i] = nextafter(src1[i], src2[i]);
}

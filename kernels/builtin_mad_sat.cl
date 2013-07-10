kernel void builtin_mad_sat(global short *src1, global short *src2, global short *src3, global short *dst) {
  short i = get_global_id(0);
  dst[i] = mad_sat(src1[i], src2[i], src3[i]);
}

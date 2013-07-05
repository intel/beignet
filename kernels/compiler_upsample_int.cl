kernel void compiler_upsample_int(global short *src1, global ushort *src2, global int *dst) {
  int i = get_global_id(0);
  dst[i] = upsample(src1[i], src2[i]);
}

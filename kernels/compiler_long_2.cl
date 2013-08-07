kernel void compiler_long_2(global long *src1, global long *src2, global long *dst) {
  int i = get_global_id(0);
  switch(i) {
    case 0:
      dst[i] = 0xFEDCBA9876543210UL;
      break;
    case 1:
      dst[i] = src1[i] & src2[i];
      break;
    case 2:
      dst[i] = src1[i] | src2[i];
      break;
    case 3:
      dst[i] = src1[i] ^ src2[i];
      break;
    case 4:
      dst[i] = src1[i] ? 0x1122334455667788L : 0x8877665544332211UL;
      break;
  }
}

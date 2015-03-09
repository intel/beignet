kernel void compiler_bswap(global uint * src0, global uint * dst0, global ushort * src1, global ushort * dst1,
    int src2, global int * dst2,  short src3, global short * dst3) {
  if (get_global_id(0) % 2 == 0) {
    dst0[get_global_id(0)] = __builtin_bswap32(src0[get_global_id(0)]);
  } else {
    dst0[get_global_id(0)] = src0[get_global_id(0)];
  }

  dst1[get_global_id(0)] = __builtin_bswap16(src1[get_global_id(0)]);
  if (get_global_id(0) % 2 == 1) {
    dst1[get_global_id(0)] = __builtin_bswap16(dst1[get_global_id(0)] + 1);
  }

  dst2[get_global_id(0)] = __builtin_bswap32(src2);
  dst3[get_global_id(0)] = __builtin_bswap16(src3);
}


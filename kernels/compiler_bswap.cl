#define SWAP64(A)	  \
((((A) & 0xff00000000000000) >> 56) | \
    (((A) & 0x00ff000000000000) >> 40) | \
    (((A) & 0x0000ff0000000000) >> 24) | \
    (((A) & 0x000000ff00000000) >> 8) |  \
    (((A) & 0x00000000ff000000) << 8) |  \
    (((A) & 0x0000000000ff0000) << 24) | \
    (((A) & 0x000000000000ff00) << 40) | \
    (((A) & 0x00000000000000ff) << 56) )

kernel void compiler_bswap(global uint * src0, global uint * dst0, global ushort * src1, global ushort * dst1,
    int src2, global int * dst2,  short src3, global short * dst3, global ulong* src4, global ulong* dst4, long src5, global long* dst5) {
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
  dst4[get_global_id(0)] = SWAP64(src4[get_global_id(0)]);
  dst5[get_global_id(0)] = SWAP64(src5);
}


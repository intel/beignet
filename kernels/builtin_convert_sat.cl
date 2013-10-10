#define DEF(DSTTYPE, SRCTYPE) \
  kernel void builtin_convert_ ## SRCTYPE ## _to_ ## DSTTYPE ## _sat(global SRCTYPE *src, global DSTTYPE *dst) { \
  int i = get_global_id(0); \
  dst[i] = convert_ ## DSTTYPE ## _sat(src[i]); \
}

DEF(char, uchar);
DEF(char, short);
DEF(char, ushort);
DEF(char, int);
DEF(char, uint);
DEF(char, float);
DEF(uchar, char);
DEF(uchar, short);
DEF(uchar, ushort);
DEF(uchar, int);
DEF(uchar, uint);
DEF(uchar, float);
DEF(short, ushort);
DEF(short, int);
DEF(short, uint);
DEF(short, float);
DEF(ushort, short);
DEF(ushort, int);
DEF(ushort, uint);
DEF(ushort, float);
DEF(int, uint);
DEF(int, float);
DEF(uint, int);
DEF(uint, float);
#undef DEF


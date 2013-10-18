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
DEF(char, long);
DEF(char, ulong);
DEF(char, float);
DEF(uchar, char);
DEF(uchar, short);
DEF(uchar, ushort);
DEF(uchar, int);
DEF(uchar, uint);
DEF(uchar, long);
DEF(uchar, ulong);
DEF(uchar, float);
DEF(short, ushort);
DEF(short, int);
DEF(short, uint);
DEF(short, long);
DEF(short, ulong);
DEF(short, float);
DEF(ushort, short);
DEF(ushort, int);
DEF(ushort, uint);
DEF(ushort, long);
DEF(ushort, ulong);
DEF(ushort, float);
DEF(int, uint);
DEF(int, long);
DEF(int, ulong);
DEF(int, float);
DEF(uint, int);
DEF(uint, long);
DEF(uint, ulong);
DEF(uint, float);
DEF(long, ulong);
DEF(long, float);
DEF(ulong, long);
DEF(ulong, float);
#undef DEF


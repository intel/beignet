/*
 * Copyright © 2012 - 2014 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "ocl_integer.h"

PURE CONST uint __gen_ocl_fbh(uint);
PURE CONST uint __gen_ocl_fbl(uint);
PURE CONST uint __gen_ocl_cbit(uint);

OVERLOADABLE char clz(char x) {
  if (x < 0)
    return 0;
  if (x == 0)
    return 8;
  return __gen_ocl_fbh(x) - 24;
}

OVERLOADABLE uchar clz(uchar x) {
  if (x == 0)
    return 8;
  return __gen_ocl_fbh(x) - 24;
}

OVERLOADABLE short clz(short x) {
  if (x < 0)
    return 0;
  if (x == 0)
    return 16;
  return __gen_ocl_fbh(x) - 16;
}

OVERLOADABLE ushort clz(ushort x) {
  if (x == 0)
    return 16;
  return __gen_ocl_fbh(x) - 16;
}

OVERLOADABLE int clz(int x) {
  if (x < 0)
    return 0;
  if (x == 0)
    return 32;
  return __gen_ocl_fbh(x);
}

OVERLOADABLE uint clz(uint x) {
  if (x == 0)
    return 32;
  return __gen_ocl_fbh(x);
}

OVERLOADABLE long clz(long x) {
  union { int i[2]; long x; } u;
  u.x = x;
  if (u.i[1] & 0x80000000u)
    return 0;
  if (u.i[1] == 0 && u.i[0] == 0)
    return 64;
  uint v = clz(u.i[1]);
  if(v == 32)
    v += clz(u.i[0]);
  return v;
}

OVERLOADABLE ulong clz(ulong x) {
  if (x == 0)
    return 64;
  union { uint i[2]; ulong x; } u;
  u.x = x;
  uint v = clz(u.i[1]);
  if(v == 32)
    v += clz(u.i[0]);
  return v;
}

OVERLOADABLE char popcount(char x) {
  return x == 0 ? 0 : x < 0?__gen_ocl_cbit(x) - 24 : __gen_ocl_cbit(x);
}
OVERLOADABLE short popcount(short x) {
  return x == 0 ? 0 : x < 0?__gen_ocl_cbit(x) - 16 : __gen_ocl_cbit(x);
}
#define SDEF(TYPE)        \
OVERLOADABLE TYPE popcount(TYPE x){ return x == 0? 0:__gen_ocl_cbit(x);}
SDEF(uchar);
SDEF(ushort);
SDEF(int);
SDEF(uint);
#undef SDEF

OVERLOADABLE long popcount(long x) {
  union { int i[2]; long x; } u;
  u.x = x;
  uint v = popcount(u.i[1]);
  v += popcount(u.i[0]);
  return v;
}

OVERLOADABLE ulong popcount(ulong x) {
  union { uint i[2]; ulong x; } u;
  u.x = x;
  uint v = popcount(u.i[1]);
  v += popcount(u.i[0]);
  return v;
}

// sat
#define SDEF(TYPE)        \
OVERLOADABLE TYPE ocl_sadd_sat(TYPE x, TYPE y);   \
OVERLOADABLE TYPE ocl_ssub_sat(TYPE x, TYPE y);   \
OVERLOADABLE TYPE add_sat(TYPE x, TYPE y) { return ocl_sadd_sat(x, y); } \
OVERLOADABLE TYPE sub_sat(TYPE x, TYPE y) { return ocl_ssub_sat(x, y); }
SDEF(char);
SDEF(short);
#undef SDEF
OVERLOADABLE int ocl_sadd_sat(int x, int y);
OVERLOADABLE int add_sat(int x, int y) { return ocl_sadd_sat(x, y); }
OVERLOADABLE int ocl_ssub_sat(int x, int y);
OVERLOADABLE int sub_sat(int x, int y) {
  return (y == 0x80000000u) ? (ocl_sadd_sat(ocl_sadd_sat(0x7fffffff, x), 1)) : ocl_ssub_sat(x, y);
}
OVERLOADABLE long ocl_sadd_sat(long x, long y);
OVERLOADABLE long add_sat(long x, long y) {
  union {long l; uint i[2];} ux, uy;
  ux.l = x;
  uy.l = y;
  if((ux.i[1] ^ uy.i[1]) & 0x80000000u)
    return x + y;
  return ocl_sadd_sat(x, y);
}
OVERLOADABLE long ocl_ssub_sat(long x, long y);
OVERLOADABLE long sub_sat(long x, long y) {
  union {long l; uint i[2];} ux, uy;
  ux.l = x;
  uy.l = y;
  if((ux.i[1] ^ uy.i[1]) & 0x80000000u)
    return ocl_ssub_sat(x, y);
  return x - y;
}
#define UDEF(TYPE)    \
OVERLOADABLE TYPE ocl_uadd_sat(TYPE x, TYPE y);                          \
OVERLOADABLE TYPE ocl_usub_sat(TYPE x, TYPE y);                          \
OVERLOADABLE TYPE add_sat(TYPE x, TYPE y) { return ocl_uadd_sat(x, y); } \
OVERLOADABLE TYPE sub_sat(TYPE x, TYPE y) { return ocl_usub_sat(x, y); }
UDEF(uchar);
UDEF(ushort);
UDEF(uint);
UDEF(ulong);
#undef UDEF


OVERLOADABLE int __gen_ocl_mul_hi(int x, int y);
OVERLOADABLE uint __gen_ocl_mul_hi(uint x, uint y);
OVERLOADABLE long __gen_ocl_mul_hi(long x, long y);
OVERLOADABLE ulong __gen_ocl_mul_hi(ulong x, ulong y);
OVERLOADABLE char mul_hi(char x, char y) { return (x * y) >> 8; }
OVERLOADABLE uchar mul_hi(uchar x, uchar y) { return (x * y) >> 8; }
OVERLOADABLE short mul_hi(short x, short y) { return (x * y) >> 16; }
OVERLOADABLE ushort mul_hi(ushort x, ushort y) { return (x * y) >> 16; }
OVERLOADABLE int mul_hi(int x, int y) { return __gen_ocl_mul_hi(x, y); }
OVERLOADABLE uint mul_hi(uint x, uint y) { return __gen_ocl_mul_hi(x, y); }
OVERLOADABLE long mul_hi(long x, long y) {
  return __gen_ocl_mul_hi(x, y);
}
OVERLOADABLE ulong mul_hi(ulong x, ulong y) {
  return __gen_ocl_mul_hi(x, y);
}

#define DEF(type) OVERLOADABLE type mad_hi(type a, type b, type c) { return mul_hi(a, b) + c; }
DEF(char)
DEF(uchar)
DEF(short)
DEF(ushort)
DEF(int)
DEF(uint)
DEF(long)
DEF(ulong)
#undef DEF

OVERLOADABLE int mul24(int a, int b) { return ((a << 8) >> 8) * ((b << 8) >> 8); }
OVERLOADABLE uint mul24(uint a, uint b) { return (a & 0xFFFFFF) * (b & 0xFFFFFF); }

OVERLOADABLE int mad24(int a, int b, int c) { return mul24(a, b) + c; }
OVERLOADABLE uint mad24(uint a, uint b, uint c) { return mul24(a, b) + c; }

OVERLOADABLE char mad_sat(char a, char b, char c) {
  int x = (int)a * (int)b + (int)c;
  if (x > 127)
    x = 127;
  if (x < -128)
    x = -128;
  return x;
}

OVERLOADABLE uchar mad_sat(uchar a, uchar b, uchar c) {
  uint x = (uint)a * (uint)b + (uint)c;
  if (x > 255)
    x = 255;
  return x;
}

OVERLOADABLE short mad_sat(short a, short b, short c) {
  int x = (int)a * (int)b + (int)c;
  if (x > 32767)
    x = 32767;
  if (x < -32768)
    x = -32768;
  return x;
}

OVERLOADABLE ushort mad_sat(ushort a, ushort b, ushort c) {
  uint x = (uint)a * (uint)b + (uint)c;
  if (x > 65535)
    x = 65535;
  return x;
}

OVERLOADABLE int mad_sat(int a, int b, int c) {
  long x = (long)a * (long)b + (long)c;
  if (x > 0x7FFFFFFF)
    x = 0x7FFFFFFF;
  else if (x < -0x7FFFFFFF-1)
    x = -0x7FFFFFFF-1;
  return (int)x;
}

OVERLOADABLE uint mad_sat(uint a, uint b, uint c) {
  ulong x = (ulong)a * (ulong)b + (ulong)c;
  if (x > 0xFFFFFFFFu)
    x = 0xFFFFFFFFu;
  return (uint)x;
}

OVERLOADABLE long __gen_ocl_mad_sat(long a, long b, long c);
OVERLOADABLE ulong __gen_ocl_mad_sat(ulong a, ulong b, ulong c);

OVERLOADABLE long mad_sat(long a, long b, long c) {
  return __gen_ocl_mad_sat(a, b, c);
}

OVERLOADABLE ulong mad_sat(ulong a, ulong b, ulong c) {
  return __gen_ocl_mad_sat(a, b, c);
}

OVERLOADABLE uchar __rotate_left(uchar x, uchar y) { return (x << y) | (x >> (8 - y)); }
OVERLOADABLE char __rotate_left(char x, char y) { return __rotate_left((uchar)x, (uchar)y); }
OVERLOADABLE ushort __rotate_left(ushort x, ushort y) { return (x << y) | (x >> (16 - y)); }
OVERLOADABLE short __rotate_left(short x, short y) { return __rotate_left((ushort)x, (ushort)y); }
OVERLOADABLE uint __rotate_left(uint x, uint y) { return (x << y) | (x >> (32 - y)); }
OVERLOADABLE int __rotate_left(int x, int y) { return __rotate_left((uint)x, (uint)y); }
OVERLOADABLE ulong __rotate_left(ulong x, ulong y) { return (x << y) | (x >> (64 - y)); }
OVERLOADABLE long __rotate_left(long x, long y) { return __rotate_left((ulong)x, (ulong)y); }
#define DEF(type, m) OVERLOADABLE type rotate(type x, type y) { return __rotate_left(x, (type)(y & m)); }
DEF(char, 7)
DEF(uchar, 7)
DEF(short, 15)
DEF(ushort, 15)
DEF(int, 31)
DEF(uint, 31)
DEF(long, 63)
DEF(ulong, 63)
#undef DEF

OVERLOADABLE short __gen_ocl_upsample(short hi, short lo);
OVERLOADABLE int __gen_ocl_upsample(int hi, int lo);
OVERLOADABLE long __gen_ocl_upsample(long hi, long lo);
OVERLOADABLE short upsample(char hi, uchar lo) { return __gen_ocl_upsample((short)hi, (short)lo); }
OVERLOADABLE ushort upsample(uchar hi, uchar lo) { return __gen_ocl_upsample((short)hi, (short)lo); }
OVERLOADABLE int upsample(short hi, ushort lo) { return __gen_ocl_upsample((int)hi, (int)lo); }
OVERLOADABLE uint upsample(ushort hi, ushort lo) { return __gen_ocl_upsample((int)hi, (int)lo); }
OVERLOADABLE long upsample(int hi, uint lo) {
  return __gen_ocl_upsample((long)hi, (long)lo);
}
OVERLOADABLE ulong upsample(uint hi, uint lo) {
  return __gen_ocl_upsample((long)hi, (long)lo);
}

OVERLOADABLE uint __gen_ocl_hadd(uint x, uint y);
OVERLOADABLE uint __gen_ocl_rhadd(uint x, uint y);
#define DEC DEF(char); DEF(uchar); DEF(short); DEF(ushort)
#define DEF(type) OVERLOADABLE type hadd(type x, type y) { return (x + y) >> 1; }
DEC
#undef DEF
#define DEF(type) OVERLOADABLE type rhadd(type x, type y) { return (x + y + 1) >> 1; }
DEC
#undef DEF
#undef DEC
OVERLOADABLE int hadd(int x, int y) {
  return (x < 0 && y > 0) || (x > 0 && y < 0) ?
         ((x + y) >> 1) :
         __gen_ocl_hadd((uint)x, (uint)y);
}
OVERLOADABLE uint hadd(uint x, uint y) { return __gen_ocl_hadd(x, y); }
OVERLOADABLE int rhadd(int x, int y) {
  return (x < 0 && y > 0) || (x > 0 && y < 0) ?
         ((x + y + 1) >> 1) :
         __gen_ocl_rhadd((uint)x, (uint)y);
 }
OVERLOADABLE uint rhadd(uint x, uint y) { return __gen_ocl_rhadd(x, y); }
OVERLOADABLE ulong __gen_ocl_hadd(ulong x, ulong y);
OVERLOADABLE ulong __gen_ocl_rhadd(ulong x, ulong y);
OVERLOADABLE long hadd(long x, long y) {
  return (x < 0 && y > 0) || (x > 0 && y < 0) ?
         ((x + y) >> 1) :
         __gen_ocl_hadd((ulong)x, (ulong)y);
}
OVERLOADABLE ulong hadd(ulong x, ulong y) {
  return __gen_ocl_hadd(x, y);
}
OVERLOADABLE long rhadd(long x, long y) {
  return (x < 0 && y > 0) || (x > 0 && y < 0) ?
         ((x + y + 1) >> 1) :
         __gen_ocl_rhadd((ulong)x, (ulong)y);
}
OVERLOADABLE ulong rhadd(ulong x, ulong y) {
  return __gen_ocl_rhadd(x, y);
}

int __gen_ocl_abs(int x);
#define DEC(TYPE) OVERLOADABLE u##TYPE abs(TYPE x) { return (u##TYPE) __gen_ocl_abs(x); }
DEC(int)
DEC(short)
DEC(char)
#undef DEC
OVERLOADABLE ulong abs(long x) { return x < 0 ? -x : x; }
/* For unsigned types, do nothing. */
#define DEC(TYPE) OVERLOADABLE TYPE abs(TYPE x) { return x; }
DEC(uint)
DEC(ushort)
DEC(uchar)
DEC(ulong)
#undef DEC

/* Char and short type abs diff */
/* promote char and short to int and will be no module overflow */
#define DEC(TYPE, UTYPE) OVERLOADABLE UTYPE abs_diff(TYPE x, TYPE y) \
                         { return (UTYPE) (abs((int)x - (int)y)); }
DEC(char, uchar)
DEC(uchar, uchar)
DEC(short, ushort)
DEC(ushort, ushort)
#undef DEC

OVERLOADABLE uint abs_diff (uint x, uint y) {
    /* same signed will never overflow. */
    return y > x ? (y -x) : (x - y);
}

OVERLOADABLE uint abs_diff (int x, int y) {
    /* same signed will never module overflow. */
    if ((x >= 0 && y >= 0) || (x <= 0 && y <= 0))
        return abs(x - y);

    return (abs(x) + abs(y));
}

OVERLOADABLE ulong abs_diff (long x, long y) {
  if ((x >= 0 && y >= 0) || (x <= 0 && y <= 0))
    return abs(x - y);
  return abs(x) + abs(y);
}
OVERLOADABLE ulong abs_diff (ulong x, ulong y) {
  return y > x ? (y - x) : (x - y);
}


#define DECL_MIN_MAX_CLAMP(TYPE) \
OVERLOADABLE TYPE max(TYPE a, TYPE b) { \
  return a > b ? a : b; \
} \
OVERLOADABLE TYPE min(TYPE a, TYPE b) { \
  return a < b ? a : b; \
} \
OVERLOADABLE TYPE clamp(TYPE v, TYPE l, TYPE u) { \
  return max(min(v, u), l); \
}
DECL_MIN_MAX_CLAMP(int)
DECL_MIN_MAX_CLAMP(short)
DECL_MIN_MAX_CLAMP(char)
DECL_MIN_MAX_CLAMP(uint)
DECL_MIN_MAX_CLAMP(unsigned short)
DECL_MIN_MAX_CLAMP(unsigned char)
DECL_MIN_MAX_CLAMP(long)
DECL_MIN_MAX_CLAMP(ulong)
#undef DECL_MIN_MAX_CLAMP

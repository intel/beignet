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
#include "ocl_relational.h"

OVERLOADABLE int isequal(float x, float y) {
  return x == y;
}

OVERLOADABLE int isnotequal(float x, float y) {
  return x != y;
}

OVERLOADABLE int isgreater(float x, float y) {
  return x > y;
}

OVERLOADABLE int isgreaterequal(float x, float y) {
  return x >= y;
}

OVERLOADABLE int isless(float x, float y) {
  return x < y;
}

OVERLOADABLE int islessequal(float x, float y) {
  return x <= y;
}

OVERLOADABLE int islessgreater(float x, float y) {
  return (x < y) || (x > y);
}

OVERLOADABLE int isfinite(float x) {
  union { uint u; float f; } u;
  u.f = x;
  return (u.u & 0x7FFFFFFF) < 0x7F800000;
}

OVERLOADABLE int isinf(float x) {
  union { uint u; float f; } u;
  u.f = x;
  return (u.u & 0x7FFFFFFF) == 0x7F800000;
}

OVERLOADABLE int isnan(float x) {
  return x != x;
}

OVERLOADABLE int isnormal(float x) {
  union { uint u; float f; } u;
  u.f = x;
  u.u &= 0x7FFFFFFF;
  return (u.u < 0x7F800000) && (u.u >= 0x800000);
}


OVERLOADABLE int isordered(float x, float y) {
  return isequal(x, x) && isequal(y, y);
}
OVERLOADABLE int isunordered(float x, float y) {
  return isnan(x) || isnan(y);
}
OVERLOADABLE int signbit(float x) {
  union { uint u; float f; } u;
  u.f = x;
  return u.u >> 31;
}


// any
#define DEC1(type) OVERLOADABLE int any(type a) { return a<0; }
#define DEC2(type) OVERLOADABLE int any(type a) { return a.s0<0 || a.s1<0; }
#define DEC3(type) OVERLOADABLE int any(type a) { return a.s0<0 || a.s1<0 || a.s2<0; }
#define DEC4(type) OVERLOADABLE int any(type a) { return a.s0<0 || a.s1<0 || a.s2<0 || a.s3<0; }
#define DEC8(type) OVERLOADABLE int any(type a) { return a.s0<0 || a.s1<0 || a.s2<0 || a.s3<0 || a.s4<0 || a.s5<0 || a.s6<0 || a.s7<0; }
#define DEC16(type) OVERLOADABLE int any(type a) { return a.s0<0 || a.s1<0 || a.s2<0 || a.s3<0 || a.s4<0 || a.s5<0 || a.s6<0 || a.s7<0 || a.s8<0 || a.s9<0 || a.sA<0 || a.sB<0 || a.sC<0 || a.sD<0 || a.sE<0 || a.sF<0; }
DEC1(char);
DEC1(short);
DEC1(int);
DEC1(long);
#define DEC(n) DEC##n(char##n); DEC##n(short##n); DEC##n(int##n); DEC##n(long##n);
DEC(2);
DEC(3);
DEC(4);
DEC(8);
DEC(16);
#undef DEC
#undef DEC1
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16

// all
#define DEC1(type) OVERLOADABLE int all(type a) { return a<0; }
#define DEC2(type) OVERLOADABLE int all(type a) { return a.s0<0 && a.s1<0; }
#define DEC3(type) OVERLOADABLE int all(type a) { return a.s0<0 && a.s1<0 && a.s2<0; }
#define DEC4(type) OVERLOADABLE int all(type a) { return a.s0<0 && a.s1<0 && a.s2<0 && a.s3<0; }
#define DEC8(type) OVERLOADABLE int all(type a) { return a.s0<0 && a.s1<0 && a.s2<0 && a.s3<0 && a.s4<0 && a.s5<0 && a.s6<0 && a.s7<0; }
#define DEC16(type) OVERLOADABLE int all(type a) { return a.s0<0 && a.s1<0 && a.s2<0 && a.s3<0 && a.s4<0 && a.s5<0 && a.s6<0 && a.s7<0 && a.s8<0 && a.s9<0 && a.sA<0 && a.sB<0 && a.sC<0 && a.sD<0 && a.sE<0 && a.sF<0; }
DEC1(char);
DEC1(short);
DEC1(int);
DEC1(long);
#define DEC(n) DEC##n(char##n); DEC##n(short##n); DEC##n(int##n); DEC##n(long##n);
DEC(2);
DEC(3);
DEC(4);
DEC(8);
DEC(16);
#undef DEC
#undef DEC1
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16

#define DEF(type) OVERLOADABLE type bitselect(type a, type b, type c) { return (a & ~c) | (b & c); }
DEF(char); DEF(uchar); DEF(short); DEF(ushort); DEF(int); DEF(uint)
DEF(long); DEF(ulong)
#undef DEF
OVERLOADABLE float bitselect(float a, float b, float c) {
  return as_float(bitselect(as_int(a), as_int(b), as_int(c)));
}


// select
#define DEF(TYPE1, TYPE2) \
OVERLOADABLE TYPE1 select(TYPE1 src0, TYPE1 src1, TYPE2 cond) { \
  return cond ? src1 : src0; \
}
DEF(char, char)
DEF(char, uchar)
DEF(uchar, char)
DEF(uchar, uchar)
DEF(short, short)
DEF(short, ushort)
DEF(ushort, short)
DEF(ushort, ushort)
DEF(int, int)
DEF(int, uint)
DEF(uint, int)
DEF(uint, uint)
DEF(long, long)
DEF(long, ulong)
DEF(ulong, long)
DEF(ulong, ulong)
DEF(float, int)
DEF(float, uint)
#undef DEF

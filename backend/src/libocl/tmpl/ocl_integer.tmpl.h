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
#ifndef __OCL_INTEGER_H__
#define __OCL_INTEGER_H__

#include "ocl_types.h"

#define CHAR_BIT    8
#define CHAR_MAX    SCHAR_MAX
#define CHAR_MIN    SCHAR_MIN
#define INT_MAX     2147483647
#define INT_MIN     (-2147483647 - 1)
#define LONG_MAX    0x7fffffffffffffffL
#define LONG_MIN    (-0x7fffffffffffffffL - 1)
#define SCHAR_MAX   127
#define SCHAR_MIN   (-127 - 1)
#define SHRT_MAX    32767
#define SHRT_MIN    (-32767 - 1)
#define UCHAR_MAX   255
#define USHRT_MAX   65535
#define UINT_MAX    0xffffffff
#define ULONG_MAX   0xffffffffffffffffUL

OVERLOADABLE char clz(char x);
OVERLOADABLE uchar clz(uchar x);
OVERLOADABLE short clz(short x);
OVERLOADABLE ushort clz(ushort x);
OVERLOADABLE int clz(int x);
OVERLOADABLE uint clz(uint x);
OVERLOADABLE long clz(long x);
OVERLOADABLE ulong clz(ulong x);

OVERLOADABLE char popcount(char x);
OVERLOADABLE uchar popcount(uchar x);
OVERLOADABLE short popcount(short x);
OVERLOADABLE ushort popcount(ushort x);
OVERLOADABLE int popcount(int x);
OVERLOADABLE uint popcount(uint x);
OVERLOADABLE long popcount(long x);
OVERLOADABLE ulong popcount(ulong x);

OVERLOADABLE char mul_hi(char x, char y);
OVERLOADABLE uchar mul_hi(uchar x, uchar y);
OVERLOADABLE short mul_hi(short x, short y);
OVERLOADABLE ushort mul_hi(ushort x, ushort y);
OVERLOADABLE int mul_hi(int x, int y);
OVERLOADABLE uint mul_hi(uint x, uint y);
OVERLOADABLE long mul_hi(long x, long y);
OVERLOADABLE ulong mul_hi(ulong x, ulong y);

#define SDEF(TYPE)        \
OVERLOADABLE TYPE add_sat(TYPE x, TYPE y);   \
OVERLOADABLE TYPE sub_sat(TYPE x, TYPE y);
SDEF(char);
SDEF(short);
SDEF(int);
SDEF(long);
#undef SDEF
#define UDEF(TYPE)  \
OVERLOADABLE TYPE add_sat(TYPE x, TYPE y);   \
OVERLOADABLE TYPE sub_sat(TYPE x, TYPE y);
UDEF(uchar);
UDEF(ushort);
UDEF(uint);
UDEF(ulong);
#undef UDEF

#define DEF(type) OVERLOADABLE type mad_hi(type a, type b, type c);
DEF(char)
DEF(uchar)
DEF(short)
DEF(ushort)
DEF(int)
DEF(uint)
DEF(long)
DEF(ulong)
#undef DEF

OVERLOADABLE int mul24(int a, int b);
OVERLOADABLE uint mul24(uint a, uint b);

OVERLOADABLE int mad24(int a, int b, int c);
OVERLOADABLE uint mad24(uint a, uint b, uint c);

OVERLOADABLE char mad_sat(char a, char b, char c) ;
OVERLOADABLE uchar mad_sat(uchar a, uchar b, uchar c);
OVERLOADABLE short mad_sat(short a, short b, short c);
OVERLOADABLE ushort mad_sat(ushort a, ushort b, ushort c);
OVERLOADABLE int mad_sat(int a, int b, int c);
OVERLOADABLE uint mad_sat(uint a, uint b, uint c);
OVERLOADABLE long mad_sat(long a, long b, long c);
OVERLOADABLE ulong mad_sat(ulong a, ulong b, ulong c);

#define DEF(type, m) OVERLOADABLE type rotate(type x, type y);
DEF(char, 7)
DEF(uchar, 7)
DEF(short, 15)
DEF(ushort, 15)
DEF(int, 31)
DEF(uint, 31)
DEF(long, 63)
DEF(ulong, 63)
#undef DEF

OVERLOADABLE short upsample(char hi, uchar lo);
OVERLOADABLE ushort upsample(uchar hi, uchar lo);
OVERLOADABLE int upsample(short hi, ushort lo);
OVERLOADABLE uint upsample(ushort hi, ushort lo);
OVERLOADABLE long upsample(int hi, uint lo);
OVERLOADABLE ulong upsample(uint hi, uint lo);

#define DEC DEF(char); DEF(uchar); DEF(short); DEF(ushort)
#define DEF(type) OVERLOADABLE type hadd(type x, type y);
DEC
#undef DEF
#define DEF(type) OVERLOADABLE type rhadd(type x, type y);
DEC
#undef DEF
#undef DEC
OVERLOADABLE int hadd(int x, int y);
OVERLOADABLE uint hadd(uint x, uint y);
OVERLOADABLE int rhadd(int x, int y);
OVERLOADABLE uint rhadd(uint x, uint y);
OVERLOADABLE long hadd(long x, long y);
OVERLOADABLE ulong hadd(ulong x, ulong y);
OVERLOADABLE long rhadd(long x, long y);
OVERLOADABLE ulong rhadd(ulong x, ulong y);

#define DEC(TYPE) OVERLOADABLE u##TYPE abs(TYPE x);
DEC(int)
DEC(short)
DEC(char)
#undef DEC
OVERLOADABLE ulong abs(long x);
/* For unsigned types, do nothing. */
#define DEC(TYPE) OVERLOADABLE TYPE abs(TYPE x);
DEC(uint)
DEC(ushort)
DEC(uchar)
DEC(ulong)
#undef DEC

/* Char and short type abs diff */
/* promote char and short to int and will be no module overflow */
#define DEC(TYPE, UTYPE) OVERLOADABLE UTYPE abs_diff(TYPE x, TYPE y);
DEC(char, uchar)
DEC(uchar, uchar)
DEC(short, ushort)
DEC(ushort, ushort)
#undef DEC

OVERLOADABLE uint abs_diff (uint x, uint y);
OVERLOADABLE uint abs_diff (int x, int y);
OVERLOADABLE ulong abs_diff (long x, long y);
OVERLOADABLE ulong abs_diff (ulong x, ulong y);


#define DECL_MIN_MAX_CLAMP(TYPE) \
OVERLOADABLE TYPE max(TYPE a, TYPE b);  \
OVERLOADABLE TYPE min(TYPE a, TYPE b);  \
OVERLOADABLE TYPE clamp(TYPE v, TYPE l, TYPE u);
DECL_MIN_MAX_CLAMP(int)
DECL_MIN_MAX_CLAMP(short)
DECL_MIN_MAX_CLAMP(char)
DECL_MIN_MAX_CLAMP(uint)
DECL_MIN_MAX_CLAMP(unsigned short)
DECL_MIN_MAX_CLAMP(unsigned char)
DECL_MIN_MAX_CLAMP(long)
DECL_MIN_MAX_CLAMP(ulong)
#undef DECL_MIN_MAX_CLAMP

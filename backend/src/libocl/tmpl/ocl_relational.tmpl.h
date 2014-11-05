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
#ifndef __OCL_RELATIONAL_H__
#define __OCL_RELATIONAL_H__

#include "ocl_types.h"
#include "ocl_as.h"

OVERLOADABLE int isequal(float x, float y);
OVERLOADABLE int isnotequal(float x, float y);
OVERLOADABLE int isgreater(float x, float y);
OVERLOADABLE int isgreaterequal(float x, float y);
OVERLOADABLE int isless(float x, float y);
OVERLOADABLE int islessequal(float x, float y);
OVERLOADABLE int islessgreater(float x, float y);

OVERLOADABLE int isfinite(float x);
OVERLOADABLE int isinf(float x);
OVERLOADABLE int isnan(float x);
OVERLOADABLE int isnormal(float x);


OVERLOADABLE int isordered(float x, float y);
OVERLOADABLE int isunordered(float x, float y);
OVERLOADABLE int signbit(float x);

// any
#define DEC1(type) OVERLOADABLE int any(type a);
#define DEC2(type) OVERLOADABLE int any(type a);
#define DEC3(type) OVERLOADABLE int any(type a);
#define DEC4(type) OVERLOADABLE int any(type a);
#define DEC8(type) OVERLOADABLE int any(type a);
#define DEC16(type) OVERLOADABLE int any(type a);
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
#define DEC1(type) OVERLOADABLE int all(type a);
#define DEC2(type) OVERLOADABLE int all(type a);
#define DEC3(type) OVERLOADABLE int all(type a);
#define DEC4(type) OVERLOADABLE int all(type a);
#define DEC8(type) OVERLOADABLE int all(type a);
#define DEC16(type) OVERLOADABLE int all(type a);
DEC1(char)
DEC1(short)
DEC1(int)
DEC1(long)
#define DEC(n) DEC##n(char##n) DEC##n(short##n) DEC##n(int##n) DEC##n(long##n)
DEC(2)
DEC(3)
DEC(4)
DEC(8)
DEC(16)
#undef DEC
#undef DEC1
#undef DEC2
#undef DEC3
#undef DEC4
#undef DEC8
#undef DEC16

#define DEF(type) OVERLOADABLE type bitselect(type a, type b, type c);
DEF(char) DEF(uchar) DEF(short) DEF(ushort) DEF(int) DEF(uint)
DEF(long) DEF(ulong)
#undef DEF
OVERLOADABLE float bitselect(float a, float b, float c);


#define DEF(TYPE1, TYPE2) \
OVERLOADABLE TYPE1 select(TYPE1 src0, TYPE1 src1, TYPE2 cond);
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

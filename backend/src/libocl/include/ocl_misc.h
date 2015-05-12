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
#ifndef __OCL_MISC_H__
#define __OCL_MISC_H__

#include "ocl_types.h"

#define DEC2(TYPE, XTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##2 shuffle(XTYPE x, MASKTYPE##2 mask);

#define DEC4(TYPE, XTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##4 shuffle(XTYPE x, MASKTYPE##4 mask);

#define DEC8(TYPE, XTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##8 shuffle(XTYPE x, MASKTYPE##8 mask);

#define DEC16(TYPE, XTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##16 shuffle(XTYPE x, MASKTYPE##16 mask);

#define DEFMASK(TYPE, MASKTYPE) \
  DEC2(TYPE, TYPE##2, MASKTYPE); DEC2(TYPE, TYPE##4, MASKTYPE); DEC2(TYPE, TYPE##8, MASKTYPE); DEC2(TYPE, TYPE##16, MASKTYPE) \
  DEC4(TYPE, TYPE##2, MASKTYPE); DEC4(TYPE, TYPE##4, MASKTYPE); DEC4(TYPE, TYPE##8, MASKTYPE); DEC4(TYPE, TYPE##16, MASKTYPE) \
  DEC8(TYPE, TYPE##2, MASKTYPE); DEC8(TYPE, TYPE##4, MASKTYPE); DEC8(TYPE, TYPE##8, MASKTYPE); DEC8(TYPE, TYPE##16, MASKTYPE) \
  DEC16(TYPE, TYPE##2, MASKTYPE); DEC16(TYPE, TYPE##4, MASKTYPE); DEC16(TYPE, TYPE##8, MASKTYPE); DEC16(TYPE, TYPE##16, MASKTYPE)

#define DEF(TYPE) \
  DEFMASK(TYPE, uchar) \
  DEFMASK(TYPE, ushort) \
  DEFMASK(TYPE, uint) \
  DEFMASK(TYPE, ulong)

DEF(char)
DEF(uchar)
DEF(short)
DEF(ushort)
DEF(int)
DEF(uint)
DEF(float)
DEF(long)
DEF(ulong)
#undef DEF
#undef DEFMASK
#undef DEC2
#undef DEC4
#undef DEC8
#undef DEC16

#define DEC2(TYPE, ARGTYPE, TEMPTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##2 shuffle2(ARGTYPE x, ARGTYPE y, MASKTYPE##2 mask);

#define DEC2X(TYPE, MASKTYPE) \
  OVERLOADABLE TYPE##2 shuffle2(TYPE##16 x, TYPE##16 y, MASKTYPE##2 mask);

#define DEC4(TYPE, ARGTYPE, TEMPTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##4 shuffle2(ARGTYPE x, ARGTYPE y, MASKTYPE##4 mask);

#define DEC4X(TYPE, MASKTYPE) \
  OVERLOADABLE TYPE##4 shuffle2(TYPE##16 x, TYPE##16 y, MASKTYPE##4 mask);

#define DEC8(TYPE, ARGTYPE, TEMPTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##8 shuffle2(ARGTYPE x, ARGTYPE y, MASKTYPE##8 mask);

#define DEC8X(TYPE, MASKTYPE) \
  OVERLOADABLE TYPE##8 shuffle2(TYPE##16 x, TYPE##16 y, MASKTYPE##8 mask);

#define DEC16(TYPE, ARGTYPE, TEMPTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##16 shuffle2(ARGTYPE x, ARGTYPE y, MASKTYPE##16 mask);

#define DEC16X(TYPE, MASKTYPE) \
  OVERLOADABLE TYPE##16 shuffle2(TYPE##16 x, TYPE##16 y, MASKTYPE##16 mask);

#define DEFMASK(TYPE, MASKTYPE) \
  DEC2(TYPE, TYPE##2, TYPE##4, MASKTYPE) \
  DEC2(TYPE, TYPE##4, TYPE##8, MASKTYPE) \
  DEC2(TYPE, TYPE##8, TYPE##16, MASKTYPE) \
  DEC2X(TYPE, MASKTYPE) \
  DEC4(TYPE, TYPE##2, TYPE##4, MASKTYPE) \
  DEC4(TYPE, TYPE##4, TYPE##8, MASKTYPE) \
  DEC4(TYPE, TYPE##8, TYPE##16, MASKTYPE) \
  DEC4X(TYPE, MASKTYPE) \
  DEC8(TYPE, TYPE##2, TYPE##4, MASKTYPE) \
  DEC8(TYPE, TYPE##4, TYPE##8, MASKTYPE) \
  DEC8(TYPE, TYPE##8, TYPE##16, MASKTYPE) \
  DEC8X(TYPE, MASKTYPE) \
  DEC16(TYPE, TYPE##2, TYPE##4, MASKTYPE) \
  DEC16(TYPE, TYPE##4, TYPE##8, MASKTYPE) \
  DEC16(TYPE, TYPE##8, TYPE##16, MASKTYPE) \
  DEC16X(TYPE, MASKTYPE)

#define DEF(TYPE) \
  DEFMASK(TYPE, uchar) \
  DEFMASK(TYPE, ushort) \
  DEFMASK(TYPE, uint) \
  DEFMASK(TYPE, ulong)

DEF(char)
DEF(uchar)
DEF(short)
DEF(ushort)
DEF(int)
DEF(uint)
DEF(float)
DEF(long)
DEF(ulong)
#undef DEF
#undef DEFMASK
#undef DEC2
#undef DEC2X
#undef DEC4
#undef DEC4X
#undef DEC8
#undef DEC8X
#undef DEC16
#undef DEC16X

struct time_stamp {
  // time tick
  ulong tick;
  // If context-switch or frequency change occurs since last read of tm,
  // event will be non-zero, otherwise, it will be zero.
  uint event;
};

struct time_stamp __gen_ocl_get_timestamp(void);
#endif

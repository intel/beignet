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
#include "ocl_misc.h"

#define DEC2(TYPE, XTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##2 shuffle(XTYPE x, MASKTYPE##2 mask) { \
    TYPE##2 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & (vec_step(x) - 1)]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & (vec_step(x) - 1)]; \
    return y; \
  }

#define DEC4(TYPE, XTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##4 shuffle(XTYPE x, MASKTYPE##4 mask) { \
    TYPE##4 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & (vec_step(x) - 1)]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & (vec_step(x) - 1)]; \
    y.s2 = ((TYPE *) &x)[mask.s2 & (vec_step(x) - 1)]; \
    y.s3 = ((TYPE *) &x)[mask.s3 & (vec_step(x) - 1)]; \
    return y; \
  }

#define DEC8(TYPE, XTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##8 shuffle(XTYPE x, MASKTYPE##8 mask) { \
    TYPE##8 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & (vec_step(x) - 1)]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & (vec_step(x) - 1)]; \
    y.s2 = ((TYPE *) &x)[mask.s2 & (vec_step(x) - 1)]; \
    y.s3 = ((TYPE *) &x)[mask.s3 & (vec_step(x) - 1)]; \
    y.s4 = ((TYPE *) &x)[mask.s4 & (vec_step(x) - 1)]; \
    y.s5 = ((TYPE *) &x)[mask.s5 & (vec_step(x) - 1)]; \
    y.s6 = ((TYPE *) &x)[mask.s6 & (vec_step(x) - 1)]; \
    y.s7 = ((TYPE *) &x)[mask.s7 & (vec_step(x) - 1)]; \
    return y; \
  }

#define DEC16(TYPE, XTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##16 shuffle(XTYPE x, MASKTYPE##16 mask) { \
    TYPE##16 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & (vec_step(x) - 1)]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & (vec_step(x) - 1)]; \
    y.s2 = ((TYPE *) &x)[mask.s2 & (vec_step(x) - 1)]; \
    y.s3 = ((TYPE *) &x)[mask.s3 & (vec_step(x) - 1)]; \
    y.s4 = ((TYPE *) &x)[mask.s4 & (vec_step(x) - 1)]; \
    y.s5 = ((TYPE *) &x)[mask.s5 & (vec_step(x) - 1)]; \
    y.s6 = ((TYPE *) &x)[mask.s6 & (vec_step(x) - 1)]; \
    y.s7 = ((TYPE *) &x)[mask.s7 & (vec_step(x) - 1)]; \
    y.s8 = ((TYPE *) &x)[mask.s8 & (vec_step(x) - 1)]; \
    y.s9 = ((TYPE *) &x)[mask.s9 & (vec_step(x) - 1)]; \
    y.sa = ((TYPE *) &x)[mask.sa & (vec_step(x) - 1)]; \
    y.sb = ((TYPE *) &x)[mask.sb & (vec_step(x) - 1)]; \
    y.sc = ((TYPE *) &x)[mask.sc & (vec_step(x) - 1)]; \
    y.sd = ((TYPE *) &x)[mask.sd & (vec_step(x) - 1)]; \
    y.se = ((TYPE *) &x)[mask.se & (vec_step(x) - 1)]; \
    y.sf = ((TYPE *) &x)[mask.sf & (vec_step(x) - 1)]; \
    return y; \
  }

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
  OVERLOADABLE TYPE##2 shuffle2(ARGTYPE x, ARGTYPE y, MASKTYPE##2 mask) { \
    return shuffle((TEMPTYPE)(x, y), mask); \
  }

#define DEC2X(TYPE, MASKTYPE) \
  OVERLOADABLE TYPE##2 shuffle2(TYPE##16 x, TYPE##16 y, MASKTYPE##2 mask) { \
    TYPE##2 z; \
    z.s0 = mask.s0 < 16 ? ((TYPE *)&x)[mask.s0] : ((TYPE *)&y)[mask.s0 & 15]; \
    z.s1 = mask.s1 < 16 ? ((TYPE *)&x)[mask.s1] : ((TYPE *)&y)[mask.s1 & 15]; \
    return z; \
  }

#define DEC4(TYPE, ARGTYPE, TEMPTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##4 shuffle2(ARGTYPE x, ARGTYPE y, MASKTYPE##4 mask) { \
    return shuffle((TEMPTYPE)(x, y), mask); \
  }

#define DEC4X(TYPE, MASKTYPE) \
  OVERLOADABLE TYPE##4 shuffle2(TYPE##16 x, TYPE##16 y, MASKTYPE##4 mask) { \
    TYPE##4 z; \
    z.s0 = mask.s0 < 16 ? ((TYPE *)&x)[mask.s0] : ((TYPE *)&y)[mask.s0 & 15]; \
    z.s1 = mask.s1 < 16 ? ((TYPE *)&x)[mask.s1] : ((TYPE *)&y)[mask.s1 & 15]; \
    z.s2 = mask.s2 < 16 ? ((TYPE *)&x)[mask.s2] : ((TYPE *)&y)[mask.s2 & 15]; \
    z.s3 = mask.s3 < 16 ? ((TYPE *)&x)[mask.s3] : ((TYPE *)&y)[mask.s3 & 15]; \
    return z; \
  }

#define DEC8(TYPE, ARGTYPE, TEMPTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##8 shuffle2(ARGTYPE x, ARGTYPE y, MASKTYPE##8 mask) { \
    return shuffle((TEMPTYPE)(x, y), mask); \
  }

#define DEC8X(TYPE, MASKTYPE) \
  OVERLOADABLE TYPE##8 shuffle2(TYPE##16 x, TYPE##16 y, MASKTYPE##8 mask) { \
    TYPE##8 z; \
    z.s0 = mask.s0 < 16 ? ((TYPE *)&x)[mask.s0] : ((TYPE *)&y)[mask.s0 & 15]; \
    z.s1 = mask.s1 < 16 ? ((TYPE *)&x)[mask.s1] : ((TYPE *)&y)[mask.s1 & 15]; \
    z.s2 = mask.s2 < 16 ? ((TYPE *)&x)[mask.s2] : ((TYPE *)&y)[mask.s2 & 15]; \
    z.s3 = mask.s3 < 16 ? ((TYPE *)&x)[mask.s3] : ((TYPE *)&y)[mask.s3 & 15]; \
    z.s4 = mask.s4 < 16 ? ((TYPE *)&x)[mask.s4] : ((TYPE *)&y)[mask.s4 & 15]; \
    z.s5 = mask.s5 < 16 ? ((TYPE *)&x)[mask.s5] : ((TYPE *)&y)[mask.s5 & 15]; \
    z.s6 = mask.s6 < 16 ? ((TYPE *)&x)[mask.s6] : ((TYPE *)&y)[mask.s6 & 15]; \
    z.s7 = mask.s7 < 16 ? ((TYPE *)&x)[mask.s7] : ((TYPE *)&y)[mask.s7 & 15]; \
    return z; \
  }

#define DEC16(TYPE, ARGTYPE, TEMPTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##16 shuffle2(ARGTYPE x, ARGTYPE y, MASKTYPE##16 mask) { \
    return shuffle((TEMPTYPE)(x, y), mask); \
  }

#define DEC16X(TYPE, MASKTYPE) \
  OVERLOADABLE TYPE##16 shuffle2(TYPE##16 x, TYPE##16 y, MASKTYPE##16 mask) { \
    TYPE##16 z; \
    z.s0 = mask.s0 < 16 ? ((TYPE *)&x)[mask.s0] : ((TYPE *)&y)[mask.s0 & 15]; \
    z.s1 = mask.s1 < 16 ? ((TYPE *)&x)[mask.s1] : ((TYPE *)&y)[mask.s1 & 15]; \
    z.s2 = mask.s2 < 16 ? ((TYPE *)&x)[mask.s2] : ((TYPE *)&y)[mask.s2 & 15]; \
    z.s3 = mask.s3 < 16 ? ((TYPE *)&x)[mask.s3] : ((TYPE *)&y)[mask.s3 & 15]; \
    z.s4 = mask.s4 < 16 ? ((TYPE *)&x)[mask.s4] : ((TYPE *)&y)[mask.s4 & 15]; \
    z.s5 = mask.s5 < 16 ? ((TYPE *)&x)[mask.s5] : ((TYPE *)&y)[mask.s5 & 15]; \
    z.s6 = mask.s6 < 16 ? ((TYPE *)&x)[mask.s6] : ((TYPE *)&y)[mask.s6 & 15]; \
    z.s7 = mask.s7 < 16 ? ((TYPE *)&x)[mask.s7] : ((TYPE *)&y)[mask.s7 & 15]; \
    z.s8 = mask.s8 < 16 ? ((TYPE *)&x)[mask.s8] : ((TYPE *)&y)[mask.s8 & 15]; \
    z.s9 = mask.s9 < 16 ? ((TYPE *)&x)[mask.s9] : ((TYPE *)&y)[mask.s9 & 15]; \
    z.sa = mask.sa < 16 ? ((TYPE *)&x)[mask.sa] : ((TYPE *)&y)[mask.sa & 15]; \
    z.sb = mask.sb < 16 ? ((TYPE *)&x)[mask.sb] : ((TYPE *)&y)[mask.sb & 15]; \
    z.sc = mask.sc < 16 ? ((TYPE *)&x)[mask.sc] : ((TYPE *)&y)[mask.sc & 15]; \
    z.sd = mask.sd < 16 ? ((TYPE *)&x)[mask.sd] : ((TYPE *)&y)[mask.sd & 15]; \
    z.se = mask.se < 16 ? ((TYPE *)&x)[mask.se] : ((TYPE *)&y)[mask.se & 15]; \
    z.sf = mask.sf < 16 ? ((TYPE *)&x)[mask.sf] : ((TYPE *)&y)[mask.sf & 15]; \
    return z; \
  }

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

uint __gen_ocl_read_tm(void);
uint __gen_ocl_region(ushort offset, uint data);

struct time_stamp __gen_ocl_get_timestamp(void) {
  struct time_stamp val;

  uint tm = __gen_ocl_read_tm();
  val.tick = ((ulong)__gen_ocl_region(1, tm) << 32) | __gen_ocl_region(0, tm);
  val.event = __gen_ocl_region(2, tm);

  return val;
};

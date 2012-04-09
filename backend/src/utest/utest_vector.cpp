/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "backend/sim/sim_vector.h"
#include "utest/utest.hpp"

static INLINE bool ok(float x, float y) {
  return fabs(x-y) / (1.f + std::max(fabs(x), fabs(y))) < 1.e-6;
}

NOINLINE void hop(const float *p0, const float *p1, float *p2)
{
  genf16 _0,_1,_2;
  LOAD(_0, p0);
  LOAD(_1, p1);
  LOAD(_2, p2);
  MUL(_0, _0, _1);
  ADD(_0, _1, _2);
  MUL(_1, _1, _2);
  SUB(_0, _0, _2);
  SUB(_0, _1, _0);
  STORE(_0, p2);
}

static void utestFP(void)
{
  genf1 _0, _4, _5;
  genf16 _1, _2, _3;
  const float fdata16[32] = {1.f,1.f,2.f, 3.f, 4.f, 5.f, 6.f, 7.f,
                             8.f,9.f,10.f,11.f,12.f,13.f,14.f,15.f,
                             8.f,9.f,10.f,11.f,12.f,13.f,14.f,15.f,
                             1.f,1.f,2.f, 3.f, 4.f, 5.f, 6.f, 7.f};

  LOAD(_0, fdata16+4);
  LOAD(_4, fdata16+5);
  LOAD(_1, fdata16);
  LOAD(_2, fdata16);
#define CHECK_BIN_FLOAT(FN,OP,DST,SRC0,SRC1,ELEM0,ELEM1)\
  do {\
    FN(DST, SRC0, SRC1);\
    float tmp[32];\
    STORE(DST, tmp);\
    for (uint32_t i = 0; i < elemNum(DST); ++i) {\
      const float verification = ELEM0 OP ELEM1;\
      GBE_ASSERT(ok(verification, tmp[i]));\
    }\
  } while (0);
  CHECK_BIN_FLOAT(MUL,*,_3,_2,_1,fdata16[i],fdata16[i]);
  CHECK_BIN_FLOAT(DIV,/,_3,_2,_1,fdata16[i],fdata16[i]);
  CHECK_BIN_FLOAT(ADD,+,_3,_2,_1,fdata16[i],fdata16[i]);
  CHECK_BIN_FLOAT(SUB,-,_3,_2,_1,fdata16[i],fdata16[i]);
  CHECK_BIN_FLOAT(MUL,*,_3,_2,_0,fdata16[i],fdata16[4]);
  CHECK_BIN_FLOAT(DIV,/,_3,_2,_0,fdata16[i],fdata16[4]);
  CHECK_BIN_FLOAT(ADD,+,_3,_2,_0,fdata16[i],fdata16[4]);
  CHECK_BIN_FLOAT(SUB,-,_3,_2,_0,fdata16[i],fdata16[4]);
  CHECK_BIN_FLOAT(MUL,*,_3,_2,_0,fdata16[i],fdata16[4]);
  CHECK_BIN_FLOAT(DIV,/,_3,_2,_0,fdata16[i],fdata16[4]);
  CHECK_BIN_FLOAT(ADD,+,_3,_2,_0,fdata16[i],fdata16[4]);
  CHECK_BIN_FLOAT(SUB,-,_3,_2,_0,fdata16[i],fdata16[4]);
  CHECK_BIN_FLOAT(MUL,*,_5,_4,_0,fdata16[5],fdata16[4]);
  CHECK_BIN_FLOAT(DIV,/,_5,_4,_0,fdata16[5],fdata16[4]);
  CHECK_BIN_FLOAT(ADD,+,_5,_4,_0,fdata16[5],fdata16[4]);
  CHECK_BIN_FLOAT(SUB,-,_5,_4,_0,fdata16[5],fdata16[4]);
#undef CHECK_BIN_FLOAT
  float t0[16], t1[16], t2[16];
  hop(t0,t1,t2);
}

static void utestVector(void)
{
  UTEST_EXPECT_SUCCESS(utestFP());
}

UTEST_REGISTER(utestVector)


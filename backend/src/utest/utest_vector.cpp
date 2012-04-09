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
static INLINE bool ok(int x, int y) { return x == y; }

#define CHECK_BINARY_OP(TYPE,FN,OP,DST,SRC0,SRC1,ELEM0,ELEM1)\
  do {\
    FN(DST, SRC0, SRC1);\
    TYPE tmp[32];\
    STORE(DST, (char*) tmp);\
    for (uint32_t i = 0; i < elemNum(DST); ++i) {\
      const TYPE verification = ELEM0 OP ELEM1;\
      GBE_ASSERT(ok(verification, tmp[i]));\
    }\
  } while (0);

static void utestFP(void)
{
  genf1 _0, _4, _5;
  genf16 _1, _2, _3;
  const float data[32] = {1.f,1.f,2.f, 3.f, 4.f, 5.f, 6.f, 7.f,
                          8.f,9.f,10.f,11.f,12.f,13.f,14.f,15.f,
                          8.f,9.f,10.f,11.f,12.f,13.f,14.f,15.f,
                          1.f,1.f,2.f, 3.f, 4.f, 5.f, 6.f, 7.f};

  LOAD(_0, (const char *) (data+4));
  LOAD(_4, (const char *) (data+5));
  LOAD(_1, (const char *) (data));
  LOAD(_2, (const char *) (data));
  CHECK_BINARY_OP(float,MUL,*,_3,_2,_1,data[i],data[i]);
  CHECK_BINARY_OP(float,DIV,/,_3,_2,_1,data[i],data[i]);
  CHECK_BINARY_OP(float,ADD,+,_3,_2,_1,data[i],data[i]);
  CHECK_BINARY_OP(float,SUB,-,_3,_2,_1,data[i],data[i]);
  CHECK_BINARY_OP(float,MUL,*,_3,_2,_0,data[i],data[4]);
  CHECK_BINARY_OP(float,DIV,/,_3,_2,_0,data[i],data[4]);
  CHECK_BINARY_OP(float,ADD,+,_3,_2,_0,data[i],data[4]);
  CHECK_BINARY_OP(float,SUB,-,_3,_2,_0,data[i],data[4]);
  CHECK_BINARY_OP(float,MUL,*,_3,_2,_0,data[i],data[4]);
  CHECK_BINARY_OP(float,DIV,/,_3,_2,_0,data[i],data[4]);
  CHECK_BINARY_OP(float,ADD,+,_3,_2,_0,data[i],data[4]);
  CHECK_BINARY_OP(float,SUB,-,_3,_2,_0,data[i],data[4]);
  CHECK_BINARY_OP(float,MUL,*,_5,_4,_0,data[5],data[4]);
  CHECK_BINARY_OP(float,DIV,/,_5,_4,_0,data[5],data[4]);
  CHECK_BINARY_OP(float,ADD,+,_5,_4,_0,data[5],data[4]);
  CHECK_BINARY_OP(float,SUB,-,_5,_4,_0,data[5],data[4]);
}

static void utestInt(void)
{
  geni1 _0, _4, _5;
  geni16 _1, _2, _3;
  const int data[32] = {1,1,2, 3, 4, 5, 6, 7,
                        8,9,10,11,12,13,14,15,
                        8,9,10,11,12,13,14,15,
                        1,1,2, 3, 4, 5, 6, 7};
  LOAD(_0, (const char *) (data+4));
  LOAD(_4, (const char *) (data+5));
  LOAD(_1, (const char *) (data));
  LOAD(_2, (const char *) (data));
  CHECK_BINARY_OP(int,ADD,+,_3,_2,_1,data[i],data[i]);
  CHECK_BINARY_OP(int,SUB,-,_3,_2,_1,data[i],data[i]);
  CHECK_BINARY_OP(int,ADD,+,_3,_2,_0,data[i],data[4]);
  CHECK_BINARY_OP(int,SUB,-,_3,_2,_0,data[i],data[4]);
  CHECK_BINARY_OP(int,ADD,+,_5,_4,_0,data[5],data[4]);
  CHECK_BINARY_OP(int,SUB,-,_5,_4,_0,data[5],data[4]);

}

static void utestVector(void)
{
  UTEST_EXPECT_SUCCESS(utestFP());
  UTEST_EXPECT_SUCCESS(utestInt());
}

UTEST_REGISTER(utestVector)


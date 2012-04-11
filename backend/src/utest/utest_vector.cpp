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
#include <algorithm>

static INLINE bool ok(float x, float y) {
  return fabs(x-y) / (1.f + std::max(fabs(x), fabs(y))) < 1.e-6;
}
static INLINE bool ok(int32_t x, int32_t y) { return x == y; }
static INLINE bool ok(uint32_t x, uint32_t y) { return x == y; }

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
  simd1dw _0, _4, _5;
  simd16dw _1, _2, _3;
  const float data[32] = {1.f,1.f,2.f,3.f,4.f,5.f,6.f,7.f,
                          8.f,9.f,10.f,11.f,12.f,13.f,14.f,15.f,
                          8.f,9.f,10.f,11.f,12.f,13.f,14.f,15.f,
                          1.f,1.f,2.f,3.f,4.f,5.f,6.f,7.f};
  for (uint32_t i = 0; i < 32; ++i) {
    const int index0 = rand() % 32;
    const int index1 = rand() % 16;
    const int index2 = rand() % 16;
    const int index4 = rand() % 32;
    LOAD(_0, (const char *) (data+index0));
    LOAD(_1, (const char *) (data+index1));
    LOAD(_2, (const char *) (data+index2));
    LOAD(_4, (const char *) (data+index4));
    CHECK_BINARY_OP(float,MUL_F,*,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(float,DIV_F,/,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(float,ADD_F,+,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(float,SUB_F,-,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(float,MUL_F,*,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(float,DIV_F,/,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(float,ADD_F,+,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(float,SUB_F,-,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(float,MUL_F,*,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(float,DIV_F,/,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(float,ADD_F,+,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(float,SUB_F,-,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(float,MUL_F,*,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(float,DIV_F,/,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(float,ADD_F,+,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(float,SUB_F,-,_5,_4,_0,data[index4],data[index0]);
  }
}

static void utestINT32(void)
{
  simd1dw _0, _4, _5;
  simd16dw _1, _2, _3;
  const int32_t data[32] = {-1,1,-2,-3,4,-5,6,7,-8,9,10,11,12,13,14,15,8,
                            9,10,11,12,-13,14,-15,-1,1,-2,3,4,5,6,7};
  for (uint32_t i = 0; i < 32; ++i) {
    const int index0 = rand() % 32;
    const int index1 = rand() % 16;
    const int index2 = rand() % 16;
    const int index4 = rand() % 32;
    LOAD(_0, (const char *) (data+index0));
    LOAD(_1, (const char *) (data+index1));
    LOAD(_2, (const char *) (data+index2));
    LOAD(_4, (const char *) (data+index4));
    CHECK_BINARY_OP(int32_t,ADD_S32,+,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(int32_t,SUB_S32,-,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(int32_t,MUL_S32,*,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(int32_t,DIV_S32,/,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(int32_t,REM_S32,%,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(int32_t,AND_S32,&,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(int32_t,XOR_S32,^,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(int32_t,OR_S32, |,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(int32_t,ADD_S32,+,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(int32_t,SUB_S32,-,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(int32_t,MUL_S32,*,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(int32_t,DIV_S32,/,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(int32_t,REM_S32,%,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(int32_t,AND_S32,&,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(int32_t,XOR_S32,^,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(int32_t,OR_S32, |,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(int32_t,ADD_S32,+,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(int32_t,SUB_S32,-,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(int32_t,MUL_S32,*,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(int32_t,DIV_S32,/,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(int32_t,REM_S32,%,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(int32_t,AND_S32,&,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(int32_t,XOR_S32,^,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(int32_t,OR_S32, |,_5,_4,_0,data[index4],data[index0]);
  }
}

static void utestUINT32(void)
{
  simd1dw _0, _4, _5;
  simd16dw _1, _2, _3;
  const uint32_t data[32] = {1,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,8,
                             9,10,11,12,13,14,15,1,1,2,3,4,5,6,7};
  for (uint32_t i = 0; i < 32; ++i) {
    const int index0 = rand() % 32;
    const int index1 = rand() % 16;
    const int index2 = rand() % 16;
    const int index4 = rand() % 32;
    LOAD(_0, (const char *) (data+index0));
    LOAD(_1, (const char *) (data+index1));
    LOAD(_2, (const char *) (data+index2));
    LOAD(_4, (const char *) (data+index4));
    CHECK_BINARY_OP(uint32_t,ADD_U32,+,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(uint32_t,SUB_U32,-,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(uint32_t,MUL_U32,*,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(uint32_t,DIV_U32,/,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(uint32_t,REM_U32,%,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(uint32_t,AND_U32,&,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(uint32_t,XOR_U32,^,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(uint32_t,OR_U32, |,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_BINARY_OP(uint32_t,ADD_U32,+,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(uint32_t,SUB_U32,-,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(uint32_t,MUL_U32,*,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(uint32_t,DIV_U32,/,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(uint32_t,REM_U32,%,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(uint32_t,AND_U32,&,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(uint32_t,XOR_U32,^,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(uint32_t,OR_U32, |,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_BINARY_OP(uint32_t,ADD_U32,+,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(uint32_t,SUB_U32,-,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(uint32_t,MUL_U32,*,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(uint32_t,DIV_U32,/,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(uint32_t,REM_U32,%,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(uint32_t,AND_U32,&,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(uint32_t,XOR_U32,^,_5,_4,_0,data[index4],data[index0]);
    CHECK_BINARY_OP(uint32_t,OR_U32, |,_5,_4,_0,data[index4],data[index0]);
  }
}
#undef CHECK_BINARY_OP

#define CHECK_CMP_OP(FN,OP,DST,SRC0,SRC1,ELEM0,ELEM1)\
  do {\
    FN(DST, SRC0, SRC1);\
    uint32_t m = 0;\
    for (uint32_t i = 0; i < elemNum(DST); ++i)\
      m |= (((ELEM0 OP ELEM1) ? 1 : 0) << i);\
    GBE_ASSERT(m == mask(DST));\
  } while (0);

static void utestUINT32Cmp(void)
{
  simd1dw _0, _4;
  simd16dw _1, _2;
  simd8dw _6, _7;
  simd1m _5;
  simd16m _3;
  simd8m _8;
  const uint32_t data[64] = {11,12,13,14,15,8,1,1,2,3,4,5,6,7,8,9,10,
                             9,10,11,12,13,14,15,1,1,2,3,4,5,6,7,
                             10,11,12,13,14,15,8,1,1,2,3,4,5,6,7,8,9,
                             9,10,11,12,13,14,15,1,1,2,3,4,5,6,7};
  for (uint32_t i = 0; i < 32; ++i) {
    const int index0 = rand() % 32;
    const int index1 = rand() % 16;
    const int index2 = rand() % 16;
    const int index4 = rand() % 32;
    const int index6 = rand() % 16;
    const int index7 = rand() % 32;
    LOAD(_0, (const char *) (data+index0));
    LOAD(_1, (const char *) (data+index1));
    LOAD(_2, (const char *) (data+index2));
    LOAD(_4, (const char *) (data+index4));
    LOAD(_6, (const char *) (data+index6));
    LOAD(_7, (const char *) (data+index7));
    CHECK_CMP_OP(GE_U32,>=,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(LE_U32,<=,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(GT_U32,>,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(LT_U32,<,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(EQ_U32,==,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(NE_U32,!=,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(GE_U32,>=,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(LE_U32,<=,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(GT_U32,>,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(LT_U32,<,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(EQ_U32,==,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(NE_U32,!=,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(GE_U32,>=,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(LE_U32,<=,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(GT_U32,>,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(LT_U32,<,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(EQ_U32,==,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(NE_U32,!=,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(GE_U32,>=,_5,_4,_0,data[index4],data[index0]);
    CHECK_CMP_OP(LE_U32,<=,_5,_4,_0,data[index4],data[index0]);
    CHECK_CMP_OP(GT_U32,>,_5,_4,_0,data[index4],data[index0]);
    CHECK_CMP_OP(LT_U32,<,_5,_4,_0,data[index4],data[index0]);
    CHECK_CMP_OP(EQ_U32,==,_5,_4,_0,data[index4],data[index0]);
    CHECK_CMP_OP(NE_U32,!=,_5,_4,_0,data[index4],data[index0]);
  }
}

static void utestINT32Cmp(void)
{
  simd1dw _0, _4;
  simd16dw _1, _2;
  simd8dw _6, _7;
  simd1m _5;
  simd16m _3;
  simd8m _8;
  const int32_t data[64] = {-11,-12,13,14,-15,8,-1,-1,2,3,4,5,-6,7,8,9,10,
                            9,10,-11,12,-13,14,15,1,1,2,-3,4,-5,6,7,
                            10,11,-12,13,14,15,-8,1,1,2,-3,-4,5,-6,7,8,9,
                            9,10,11,12,-13,14,15,-1,-1,-2,-3,-4,5,6,7};

  for (uint32_t i = 0; i < 32; ++i) {
    const int index0 = rand() % 32;
    const int index1 = rand() % 16;
    const int index2 = rand() % 16;
    const int index4 = rand() % 32;
    const int index6 = rand() % 16;
    const int index7 = rand() % 32;
    LOAD(_0, (const char *) (data+index0));
    LOAD(_1, (const char *) (data+index1));
    LOAD(_2, (const char *) (data+index2));
    LOAD(_4, (const char *) (data+index4));
    LOAD(_6, (const char *) (data+index6));
    LOAD(_7, (const char *) (data+index7));
    CHECK_CMP_OP(GE_S32,>=,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(LE_S32,<=,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(GT_S32,>,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(LT_S32,<,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(EQ_S32,==,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(NE_S32,!=,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(GE_S32,>=,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(LE_S32,<=,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(GT_S32,>,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(LT_S32,<,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(EQ_S32,==,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(NE_S32,!=,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(GE_S32,>=,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(LE_S32,<=,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(GT_S32,>,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(LT_S32,<,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(EQ_S32,==,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(NE_S32,!=,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(GE_S32,>=,_5,_4,_0,data[index4],data[index0]);
    CHECK_CMP_OP(LE_S32,<=,_5,_4,_0,data[index4],data[index0]);
    CHECK_CMP_OP(GT_S32,>,_5,_4,_0,data[index4],data[index0]);
    CHECK_CMP_OP(LT_S32,<,_5,_4,_0,data[index4],data[index0]);
    CHECK_CMP_OP(EQ_S32,==,_5,_4,_0,data[index4],data[index0]);
    CHECK_CMP_OP(NE_S32,!=,_5,_4,_0,data[index4],data[index0]);
  }
}

static void utestFPCmp(void)
{
  simd1dw _0, _4;
  simd16dw _1, _2;
  simd8dw _6, _7;
  simd1m _5;
  simd16m _3;
  simd8m _8;
  const float data[64] = {1.f,-1.f,2.f,3.f,4.f,5.f,-6.f,7.f,
                          8.f,9.f,10.f,11.f,12.f,-13.f,14.f,15.f,
                          -8.f,9.f,-10.f,11.f,-12.f,13.f,-14.f,15.f,
                          1.f,1.f,2.f,3.f,4.f,5.f,6.f,-7.f,
                          8.f,9.f,10.f,11.f,12.f,-13.f,14.f,15.f,
                          -8.f,9.f,-10.f,11.f,-12.f,13.f,-14.f,15.f,
                          8.f,9.f,10.f,11.f,12.f,-13.f,14.f,15.f};

  for (uint32_t i = 0; i < 32; ++i) {
    const int index0 = rand() % 32;
    const int index1 = rand() % 16;
    const int index2 = rand() % 16;
    const int index4 = rand() % 32;
    const int index6 = rand() % 16;
    const int index7 = rand() % 32;
    LOAD(_0, (const char *) (data+index0));
    LOAD(_1, (const char *) (data+index1));
    LOAD(_2, (const char *) (data+index2));
    LOAD(_4, (const char *) (data+index4));
    LOAD(_6, (const char *) (data+index6));
    LOAD(_7, (const char *) (data+index7));
    CHECK_CMP_OP(GE_F,>=,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(LE_F,<=,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(GT_F,>,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(LT_F,<,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(EQ_F,==,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(NE_F,!=,_3,_2,_1,data[i+index2],data[i+index1]);
    CHECK_CMP_OP(GE_F,>=,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(LE_F,<=,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(GT_F,>,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(LT_F,<,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(EQ_F,==,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(NE_F,!=,_8,_7,_6,data[i+index7],data[i+index6]);
    CHECK_CMP_OP(GE_F,>=,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(LE_F,<=,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(GT_F,>,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(LT_F,<,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(EQ_F,==,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(NE_F,!=,_3,_2,_0,data[i+index2],data[index0]);
    CHECK_CMP_OP(GE_F,>=,_5,_4,_0,data[index4],data[index0]);
    CHECK_CMP_OP(LE_F,<=,_5,_4,_0,data[index4],data[index0]);
    CHECK_CMP_OP(GT_F,>,_5,_4,_0,data[index4],data[index0]);
    CHECK_CMP_OP(LT_F,<,_5,_4,_0,data[index4],data[index0]);
    CHECK_CMP_OP(EQ_F,==,_5,_4,_0,data[index4],data[index0]);
    CHECK_CMP_OP(NE_F,!=,_5,_4,_0,data[index4],data[index0]);
  }
}
#undef CHECK_CMP_OP

static void utestScatterGather(void)
{
  uint32_t data[64], gatherOffsets[64], scatterOffsets[64], dst[64];
  simd1dw _0, _0s, _0g, _4, _4s, _4g;
  simd16dw _1, _1s, _1g, _2, _2s, _2g;
  simd8dw _6, _6s, _6g, _7, _7s, _7g;

  // Create the value and offset arrays
  for (uint32_t i = 0; i < 64; ++i) {
    data[i] = i;
    scatterOffsets[i] = gatherOffsets[i] = i * sizeof(uint32_t);
  }
  for (uint32_t i = 0; i < 63; ++i) {
    const int gatherIndex = rand() % (63-i)+i+1;
    const int scatterIndex = rand() % (63-i)+i+1;
    std::swap(gatherOffsets[i], gatherOffsets[gatherIndex]);
    std::swap(scatterOffsets[i], scatterOffsets[scatterIndex]);
  }

#define CHECK_SCATTER_GATHER_OP(INDEX)\
    LOAD(_##INDEX##g, (const char *) (gatherOffsets+index##INDEX));\
    LOAD(_##INDEX##s, (const char *) (scatterOffsets+index##INDEX));\
    GATHER(_##INDEX, _##INDEX##g, (const char *) data);\
    SCATTER(_##INDEX, _##INDEX##s, (char *) dst);\
    for (uint32_t i = 0; i < elemNum(_##INDEX); ++i)\
      GBE_ASSERT(data[gatherOffsets[index##INDEX+i] / sizeof(uint32_t)] ==\
                 dst[scatterOffsets[index##INDEX+i] / sizeof(uint32_t)]);
  for (uint32_t i = 0; i < 32; ++i) {
    const int index0 = rand() % 32;
    const int index1 = rand() % 16;
    const int index2 = rand() % 16;
    const int index4 = rand() % 32;
    const int index6 = rand() % 16;
    const int index7 = rand() % 32;
    CHECK_SCATTER_GATHER_OP(0);
    CHECK_SCATTER_GATHER_OP(1);
    CHECK_SCATTER_GATHER_OP(2);
    CHECK_SCATTER_GATHER_OP(4);
    CHECK_SCATTER_GATHER_OP(6);
    CHECK_SCATTER_GATHER_OP(7);
  }
#undef CHECK_SCATTER_GATHER_OP

}

static void utestVector(void)
{
  UTEST_EXPECT_SUCCESS(utestFP());
  UTEST_EXPECT_SUCCESS(utestINT32());
  UTEST_EXPECT_SUCCESS(utestUINT32());
  UTEST_EXPECT_SUCCESS(utestFPCmp());
  UTEST_EXPECT_SUCCESS(utestINT32Cmp());
  UTEST_EXPECT_SUCCESS(utestUINT32Cmp());
  UTEST_EXPECT_SUCCESS(utestScatterGather());
}


UTEST_REGISTER(utestVector)


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

/**
 * \file value.hpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_IMMEDIATE_HPP__
#define __GBE_IR_IMMEDIATE_HPP__

#include <string.h>
#include "ir/type.hpp"
#include "sys/platform.hpp"

namespace gbe {
namespace ir {

  /*! The value as stored in the instruction */
  class Immediate
  {
  public:
    INLINE Immediate(void) {}

    Type getType(void) const {
      return type;
    }

    uint32_t getTypeSize(void) const {
      switch(type) {
        default:
          GBE_ASSERT(0 && "Invalid immeidate type.\n");
        case TYPE_BOOL:
        case TYPE_S8:
        case TYPE_U8:   return 1;
        case TYPE_S16:
        case TYPE_U16:  return 2;
        case TYPE_FLOAT:
        case TYPE_S32:
        case TYPE_U32:  return 4;
        case TYPE_DOUBLE:
        case TYPE_S64:
        case TYPE_U64:  return 8;
      }
    }

#define DECL_CONSTRUCTOR(TYPE, FIELD, IR_TYPE)                  \
    Immediate(TYPE FIELD) {                                     \
      this->type = IR_TYPE;                                     \
      this->elemNum = 1;                                        \
      this->data.p = &defaultData;                              \
      defaultData = 0ull;                                       \
      *this->data.FIELD = FIELD;                                \
    }

    DECL_CONSTRUCTOR(bool, b, TYPE_BOOL)
    DECL_CONSTRUCTOR(int8_t, s8, TYPE_S8)
    DECL_CONSTRUCTOR(uint8_t, u8, TYPE_U8)
    DECL_CONSTRUCTOR(int16_t, s16, TYPE_S16)
    DECL_CONSTRUCTOR(uint16_t, u16, TYPE_S16)
    DECL_CONSTRUCTOR(int32_t, s32, TYPE_S32)
    DECL_CONSTRUCTOR(uint32_t, u32, TYPE_S32)
    DECL_CONSTRUCTOR(int64_t, s64, TYPE_S64)
    DECL_CONSTRUCTOR(uint64_t, u64, TYPE_S64)
    DECL_CONSTRUCTOR(float, f32, TYPE_FLOAT)
    DECL_CONSTRUCTOR(double, f64, TYPE_DOUBLE)
#undef DECL_CONSTRUCTOR

#define DECL_CONSTRUCTOR(TYPE, FIELD, IR_TYPE, ELEMNUM)         \
    Immediate(TYPE *FIELD, uint32_t ELEMNUM) {                  \
      this->type = IR_TYPE;                                     \
      this->elemNum = ELEMNUM;                                  \
      if (elemNum * ELEMNUM > 8)                                \
        this->data.p = malloc(ELEMNUM * getTypeSize());         \
      else                                                      \
        this->data.p = &defaultData;                            \
      defaultData = 0ull;                                       \
      memcpy(this->data.FIELD, FIELD, ELEMNUM * getTypeSize()); \
    }

    DECL_CONSTRUCTOR(bool, b, TYPE_BOOL, elemNum)
    DECL_CONSTRUCTOR(int8_t, s8, TYPE_S8, elemNum)
    DECL_CONSTRUCTOR(uint8_t, u8, TYPE_U8, elemNum)
    DECL_CONSTRUCTOR(int16_t, s16, TYPE_S16, elemNum)
    DECL_CONSTRUCTOR(uint16_t, u16, TYPE_S16, elemNum)
    DECL_CONSTRUCTOR(int32_t, s32, TYPE_S32, elemNum)
    DECL_CONSTRUCTOR(uint32_t, u32, TYPE_S32, elemNum)
    DECL_CONSTRUCTOR(int64_t, s64, TYPE_S64, elemNum)
    DECL_CONSTRUCTOR(uint64_t, u64, TYPE_S64, elemNum)
    DECL_CONSTRUCTOR(float, f32, TYPE_FLOAT, elemNum)
    DECL_CONSTRUCTOR(double, f64, TYPE_DOUBLE, elemNum)
#undef DECL_CONSTRUCTOR

    int64_t getIntegerValue(void) const {
      switch (type) {
        default:
          GBE_ASSERT(0 && "Invalid immediate type.\n");
        case TYPE_BOOL: return *data.b;
        case TYPE_S8:   return *data.s8;
        case TYPE_U8:   return *data.u8;
        case TYPE_S16:  return *data.s16;
        case TYPE_U16:  return *data.u16;
        case TYPE_S32:  return *data.s32;
        case TYPE_U32:  return *data.u32;
        case TYPE_S64:  return *data.s64;
        case TYPE_U64:  return *data.u64;
      }
    }

    float getFloatValue(void) const {
      GBE_ASSERT(type == TYPE_FLOAT);
      return *data.f32;
    }

    float asFloatValue(void) const {
      GBE_ASSERT(type == TYPE_FLOAT || type == TYPE_U32 || type == TYPE_S32);
      return *data.f32;
    }

    int64_t asIntegerValue(void) const {
      GBE_ASSERT(elemNum == 1);
      return *data.s64;
    }

    double getDoubleValue(void) const {
      GBE_ASSERT(type == TYPE_DOUBLE);
      return *data.f64;
    }

    Immediate(const Immediate & other) {
      if (this != &other) {
        this->type = other.type;
        this->elemNum = other.elemNum;
        if (other.data.p != &other.defaultData) {
          this->data.p = malloc(other.elemNum * other.getTypeSize());
          memcpy(this->data.p, other.data.p, other.elemNum * other.getTypeSize());
        }
        else {
          this->defaultData = other.defaultData;
          this->data.p = &this->defaultData;
        }
      }
    }

    Immediate & operator= (const Immediate & other) {
      *this = Immediate(other);
      return *this;
    }

    ~Immediate() {
      if (data.p != &defaultData) {
        free(data.p);
        data.p = NULL;
      }
    }

  private:
    union {
      bool *b;
      int8_t *s8;
      uint8_t *u8;
      int16_t *s16;
      uint16_t *u16;
      int32_t *s32;
      uint32_t *u32;
      int64_t *s64;
      uint64_t *u64;
      float *f32;
      double *f64;
      void *p;
    } data;     //!< Value to store
    Type type;  //!< Type of the value
    uint32_t elemNum; //!< vector imm data type
    uint64_t defaultData;

    GBE_CLASS(Immediate);
  };

  /*! Compare two immediates */
  INLINE bool operator< (const Immediate &imm0, const Immediate &imm1) {
    if (imm0.getType() != imm1.getType())
      return uint32_t(imm0.getType()) < uint32_t(imm1.getType());
    else if (imm0.getType() == TYPE_FLOAT || imm0.getType() == TYPE_DOUBLE)
      return imm0.asIntegerValue() < imm1.asIntegerValue();
    else
      return imm0.getIntegerValue() < imm1.getIntegerValue();
  }

  /*! A value is stored in a per-function vector. This is the index to it */
  TYPE_SAFE(ImmediateIndex, uint16_t)

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_IMMEDIATE_HPP__ */


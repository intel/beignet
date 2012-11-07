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

#include "ir/type.hpp"
#include "sys/platform.hpp"

namespace gbe {
namespace ir {

  /*! The value as stored in the instruction */
  class Immediate
  {
  public:
    INLINE Immediate(void) {}
#define DECL_CONSTRUCTOR(TYPE, FIELD, IR_TYPE)  \
    Immediate(TYPE FIELD) {                     \
      this->type = IR_TYPE;                     \
      this->data.u64 = 0llu;                    \
      this->data.FIELD = FIELD;                 \
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
    union {
      bool b;
      int8_t s8;
      uint8_t u8;
      int16_t s16;
      uint16_t u16;
      int32_t s32;
      uint32_t u32;
      int64_t s64;
      uint64_t u64;
      float f32;
      double f64;
    } data;     //!< Value to store
    Type type;  //!< Type of the value
    GBE_CLASS(Immediate);
  };

  /*! Compare two immediates */
  INLINE bool operator< (const Immediate &imm0, const Immediate &imm1) {
    if (imm0.type != imm1.type)
      return uint32_t(imm0.type) < uint32_t(imm1.type);
    return imm0.data.u64 < imm1.data.u64;
  }

  /*! A value is stored in a per-function vector. This is the index to it */
  TYPE_SAFE(ImmediateIndex, uint16_t)

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_IMMEDIATE_HPP__ */


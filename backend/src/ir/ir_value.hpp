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

#ifndef __GBE_IR_VALUE_HPP__
#define __GBE_IR_VALUE_HPP__

#include "ir_type.hpp"
#include "sys/platform.hpp"

namespace gbe
{
  /*! The value as stored in the instruction */
  class Value
  {
  public:
#define DECL_CONSTRUCTOR(TYPE, FIELD) \
    Value(TYPE FIELD) { this->data.u64 = 0llu; this->data.FIELD = FIELD; }
    DECL_CONSTRUCTOR(int8, s8);
    DECL_CONSTRUCTOR(uint8, u8);
    union {
      int8 s8;
      uint8 u8;
      int16 i16;
      uint16 u16;
      int32 i32;
      uint32 u32;
      int64 i64;
      uint64 u64;
      float f32;
      double f64;
    } data;     //!< Value to store
    Type type;  //!< Type of the value
#undef DECL_CONSTRUCTOR
  };

} /* namespace gbe */

#endif /* __GBE_IR_VALUE_HPP__ */


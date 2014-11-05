/* 
 * Copyright © 2012 Intel Corporation
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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

/**
 * \file type.hpp
 *
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_TYPE_HPP__
#define __GBE_IR_TYPE_HPP__

#include "sys/platform.hpp"
#include "ir/register.hpp"

#include <ostream>

namespace gbe {
namespace ir {

  /*! All types possibly supported by the instruction */
  enum Type : uint8_t {
    TYPE_BOOL = 0, //!< boolean value
    TYPE_S8,       //!< signed 8 bits integer
    TYPE_U8,       //!< unsigned 8 bits integer
    TYPE_S16,      //!< signed 16 bits integer
    TYPE_U16,      //!< unsigned 16 bits integer
    TYPE_S32,      //!< signed 32 bits integer
    TYPE_U32,      //!< unsigned 32 bits integer
    TYPE_S64,      //!< signed 64 bits integer
    TYPE_U64,      //!< unsigned 64 bits integer
    TYPE_HALF,     //!< 16 bits floating point value
    TYPE_FLOAT,    //!< 32 bits floating point value
    TYPE_DOUBLE,   //!< 64 bits floating point value
    TYPE_LARGE_INT //!< integer larger than 64 bits.
  };

  /*! Output a string for the type in the given stream */
  std::ostream &operator<< (std::ostream &out, const Type &type);

  /*! Get the register family for each type */
  INLINE RegisterFamily getFamily(Type type) {
    switch (type) {
      case TYPE_BOOL:
        return FAMILY_BOOL;
      case TYPE_S8:
      case TYPE_U8:
        return FAMILY_BYTE;
      case TYPE_S16:
      case TYPE_U16:
      case TYPE_HALF:
        return FAMILY_WORD;
      case TYPE_S32:
      case TYPE_U32:
      case TYPE_FLOAT:
        return FAMILY_DWORD;
      case TYPE_S64:
      case TYPE_U64:
      case TYPE_DOUBLE:
        return FAMILY_QWORD;
      default:
        return FAMILY_DWORD;
    };
  }

  /*! Return a type for each register family */
  INLINE Type getType(RegisterFamily family) {
    switch (family) {
      case FAMILY_BOOL: return TYPE_BOOL;
      case FAMILY_BYTE: return TYPE_U8;
      case FAMILY_WORD: return TYPE_U16;
      case FAMILY_DWORD: return TYPE_U32;
      case FAMILY_QWORD: return TYPE_U64;
    };
    return TYPE_U32;
  }

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_TYPE_HPP__ */


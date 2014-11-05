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
 * \file instruction.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "ir/type.hpp"

namespace gbe {
namespace ir {
  std::ostream &operator<< (std::ostream &out, const Type &type) {
    switch (type) {
      case TYPE_BOOL: return out << "bool";
      case TYPE_S8: return out << "int8";
      case TYPE_U8: return out << "uint8";
      case TYPE_S16: return out << "int16";
      case TYPE_U16: return out << "uin16";
      case TYPE_S32: return out << "int32";
      case TYPE_U32: return out << "uin32";
      case TYPE_S64: return out << "int64";
      case TYPE_U64: return out << "uin64";
      case TYPE_HALF: return out << "half";
      case TYPE_FLOAT: return out << "float";
      case TYPE_DOUBLE: return out << "double";
      default :
        GBE_ASSERT(0 && "Unsupported type\n");
    };
    return out;
  }

} /* namespace ir */
} /* namespace gbe */


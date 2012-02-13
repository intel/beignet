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

#ifndef __GBE_IR_TYPE_HPP__
#define __GBE_IR_TYPE_HPP__

#include "sys/platform.hpp"

namespace gbe
{
  /*! All types possibly supported by the instruction */
  enum Type : uint8_t {
    TYPE_S8 = 0,  //!< signed 8 bits integer
    TYPE_U8,      //!< unsigned 8 bits integer
    TYPE_S16,     //!< signed 16 bits integer
    TYPE_U16,     //!< unsigned 16 bits integer
    TYPE_S32,     //!< signed 32 bits integer
    TYPE_U32,     //!< unsigned 32 bits integer
    TYPE_S64,     //!< signed 64 bits integer
    TYPE_U64,     //!< unsigned 64 bits integer
    TYPE_FLOAT,   //!< 32 bits floating point value
    TYPE_DOUBLE   //!< 64 bits floating point value
  };
} /* namespace gbe */

#endif /* __GBE_IR_TYPE_HPP__ */


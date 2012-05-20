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
 * \file constant_push.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_CONSTANT_PUSH_HPP__
#define __GBE_IR_CONSTANT_PUSH_HPP__

#include "ir/register.hpp"
#include "sys/map.hpp"

namespace gbe {
namespace ir {

  /*! Maps the register to the function argument */
  struct ArgLocation {
    INLINE ArgLocation(void) {}
    INLINE ArgLocation(uint32_t argID, uint32_t offset) :
      argID(argID), offset(offset) {}
    uint32_t argID;  //!< Function argument
    uint32_t offset; //!< Offset in the function argument
  };

  /*! Structure arguments can be directly pushed to the register file. We store
   *  here the mapping the function argument and the registers that contains the
   *  pushed data
   */
  class ConstantPush
  {
  public:
    /*! Set the contanst pushing description */
    ConstantPush(void);
    /*! Set the contanst pushing description */
    ~ConstantPush(void);
    /*! Maps each register with the function argument */
    map<Register, ArgLocation> constantMap;
    GBE_CLASS(ConstantPush); // Uses GBE allocators
  };

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_CONSTANT_PUSH_HPP__ */

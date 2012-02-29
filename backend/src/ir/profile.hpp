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
 * \file profile.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#ifndef __GBE_IR_PROFILE_HPP__
#define __GBE_IR_PROFILE_HPP__

#include "ir/register.hpp"

namespace gbe {
namespace ir {

  /*! Profile is defined *per-function* and mostly predefined registers */
  enum Profile : uint32_t {
    PROFILE_C = 0,  // Not used now
    PROFILE_OCL = 1
  };

  // Will be pre-initialized
  class Function;

  /*! Registers used for ocl */
  namespace ocl
  {
    static const Register lid0 = Register(0); // get_local_id(0)
    static const Register lid1 = Register(1); // get_local_id(1)
    static const Register lid2 = Register(2); // get_local_id(2)
    static const Register gid0 = Register(3); // get_global_id(0)
    static const Register gid1 = Register(4); // get_global_id(1)
    static const Register gid2 = Register(5); // get_global_id(2)
    static const uint32_t regNum = 6;         // number of special registers
  } /* namespace ocl */

  /*! Initialize the profile of the given function */
  void initProfile(Function &fn);

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_PROFILE_HPP__ */


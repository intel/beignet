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

  // Will be pre-initialized based on its profile
  class Function;

  /*! Registers used for ocl */
  namespace ocl
  {
    static const Register lid0 = Register(0);      // get_local_id(0)
    static const Register lid1 = Register(1);      // get_local_id(1)
    static const Register lid2 = Register(2);      // get_local_id(2)
    static const Register groupid0 = Register(3);  // get_group_id(0)
    static const Register groupid1 = Register(4);  // get_group_id(1)
    static const Register groupid2 = Register(5);  // get_group_id(2)
    static const Register numgroup0 = Register(6); // get_num_groups(0)
    static const Register numgroup1 = Register(7); // get_num_groups(1)
    static const Register numgroup2 = Register(8); // get_num_groups(2)
    static const Register lsize0 = Register(9);    // get_local_size(0)
    static const Register lsize1 = Register(10);   // get_local_size(1)
    static const Register lsize2 = Register(11);   // get_local_size(2)
    static const Register gsize0 = Register(12);   // get_global_size(0)
    static const Register gsize1 = Register(13);   // get_global_size(1)
    static const Register gsize2 = Register(14);   // get_global_size(2)
    static const Register goffset0 = Register(15); // get_global_offset(0)
    static const Register goffset1 = Register(16); // get_global_offset(1)
    static const Register goffset2 = Register(17); // get_global_offset(2)
    static const Register stackptr = Register(18); // stack pointer
    static const Register stackbuffer = Register(19); // stack buffer base address.
    static const Register blockip = Register(20);  // blockip
    static const Register barrierid = Register(21);// barrierid
    static const Register threadn = Register(22);  // number of threads
    static const Register workdim = Register(23);  // work dimention.
    static const Register zero = Register(24);     //  scalar register holds zero.
    static const Register one = Register(25);     //  scalar register holds one. 
    static const Register retVal = Register(26);   // helper register to do data flow analysis.
    static const Register slmoffset = Register(27);  // Group's SLM offset in total 64K SLM
    static const Register printfbptr = Register(28); // printf buffer address .
    static const Register printfiptr = Register(29); // printf index buffer address.
    static const Register invalid = Register(30);  // used for valid comparation.
    static const uint32_t regNum = 31;             // number of special registers
    extern const char *specialRegMean[];           // special register name.
  } /* namespace ocl */

  /*! Initialize the profile of the given function */
  void initProfile(Function &fn);

} /* namespace ir */
} /* namespace gbe */

#endif /* __GBE_IR_PROFILE_HPP__ */


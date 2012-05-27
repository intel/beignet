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
 * \file gen_reg_allocation.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __GBE_GEN_REG_ALLOCATION_HPP__
#define __GBE_GEN_REG_ALLOCATION_HPP__

#include "ir/register.hpp"
#include "backend/gen_context.hpp"
#include "backend/program.h"

namespace gbe
{
  class GenRegAllocator
  {
  public:
    /*! Initialize the register allocator */
    GenRegAllocator(GenContext &ctx);
    /*! Return the Gen register from the GenIR one */
    GenReg genReg(ir::Register reg, ir::Type type = ir::TYPE_FLOAT);
    /*! Compute the quarterth register part when using SIMD8 with Qn (n in 2,3,4) */
    GenReg genRegQn(ir::Register reg, uint32_t quarter, ir::Type type = ir::TYPE_FLOAT);
    /*! Create a Gen register from a register set in the payload */
    void allocatePayloadReg(gbe_curbe_type, ir::Register, uint32_t subValue = 0, uint32_t subOffset = 0);
    /*! Very stupid register allocator to start with */
    void allocateRegister(void);
    /*! Create a GenReg from a ir::Register */
    uint32_t createGenReg(ir::Register reg, uint32_t grfOffset);
    /*! The context owns the register allocator */
    GenContext &ctx;
    /*! Map virtual registers to physical registers */
    map<ir::Register, GenReg> RA;
  };

} /* namespace gbe */

#endif /* __GBE_GEN_REG_ALLOCATION_HPP__ */


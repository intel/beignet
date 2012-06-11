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
  class Selection;    // Pre-register allocation code generation
  class SelectionReg; // Pre-register allocation Gen register

#define OLD_ALLOCATOR 0

  /*! Provides the location of a register in a vector */
  typedef std::pair<SelectionVector*, uint32_t> VectorLocation;

  class GenRegAllocator
  {
  public:
    /*! Initialize the register allocator */
    GenRegAllocator(GenContext &ctx);
    /*! Perform the register allocation */
    void allocate(Selection &selection);
    /*! Return the Gen register from the selection register */
    GenReg genReg(const SelectionReg &reg);
    /*! Return the Gen register from the GenIR one */
    GenReg genReg(ir::Register, ir::Type type = ir::TYPE_FLOAT);
    /*! Compute the quarterth register part when using SIMD8 with Qn (n in 2,3,4) */
    GenReg genRegQn(ir::Register, uint32_t quarter, ir::Type type = ir::TYPE_FLOAT);
  private:
    /*! Create a Gen register from a register set in the payload */
    void allocatePayloadReg(gbe_curbe_type, ir::Register, uint32_t subValue = 0, uint32_t subOffset = 0);
    /*! Allocate the vectors detected in the instruction selection pass */
    void allocateVector(Selection &selection);
    /*! Create a GenReg from a ir::Register */
    void createGenReg(ir::Register);
    /*! Indicate if the registers are already allocated in vectors */
    bool isAllocated(const SelectionVector *vector) const;
    /*! Reallocate registers if needed to make the registers in the vector
     *  contigous in memory
     */
    void coalesce(Selection &selection, SelectionVector *vector);
    /*! The context owns the register allocator */
    GenContext &ctx;
    /*! Map virtual registers to physical registers */
    map<ir::Register, GenReg> RA;
    /*! Provides the position of each register in a vector */
    map<ir::Register, VectorLocation> vectorMap;
  };

} /* namespace gbe */

#endif /* __GBE_GEN_REG_ALLOCATION_HPP__ */


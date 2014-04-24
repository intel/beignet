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
#include "backend/gen_register.hpp"

namespace gbe
{
  class Selection;      // Pre-register allocation code generation
  class GenRegister;    // Pre-register allocation Gen register
  struct GenRegInterval; // Liveness interval for each register
  class GenContext;     // Gen specific context

  typedef struct SpillRegTag {
    bool isTmpReg;
    int32_t addr;
  } SpillRegTag;

  typedef map<ir::Register, SpillRegTag> SpilledRegs;

  /*! Register allocate (i.e. virtual to physical register mapping) */
  class GenRegAllocator
  {
  public:
    /*! Initialize the register allocator */
    GenRegAllocator(GenContext &ctx);
    /*! Release all taken resources */
    ~GenRegAllocator(void);
    /*! Perform the register allocation */
    bool allocate(Selection &selection);
    /*! Virtual to physical translation */
    GenRegister genReg(const GenRegister &reg);
    /*! Output the register allocation */
    void outputAllocation(void);
    /*! Get register actual size in byte. */
    uint32_t getRegSize(ir::Register reg);
  private:
    /*! Actual implementation of the register allocator (use Pimpl) */
    class Opaque;
    /*! Created and destroyed in cpp */
    Opaque *opaque;
    /*! Use custom allocator */
    GBE_CLASS(GenRegAllocator);
  };

} /* namespace gbe */

#endif /* __GBE_GEN_REG_ALLOCATION_HPP__ */


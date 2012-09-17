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
#include "backend/gen_register.hpp"
#include "backend/program.h"

namespace gbe
{
  class Selection;      // Pre-register allocation code generation
  class GenRegister;   // Pre-register allocation Gen register
  class GenRegInterval; // Liveness interval for each register

  /*! Provides the location of a register in a vector */
  typedef std::pair<SelectionVector*, uint32_t> VectorLocation;

  /*! Perform the register allocation (i.e. virtual to physical register
   * mapping). TODO only define an abstract class in the header instead of this
   * blob
   */
  class GenRegAllocator
  {
  public:
    /*! Initialize the register allocator */
    GenRegAllocator(GenContext &ctx);
    /*! Release all taken resources */
    ~GenRegAllocator(void);
    /*! Perform the register allocation */
    void allocate(Selection &selection);
    /*! Return the Gen register from the selection register */
    GenRegister genReg(const GenRegister &reg);
    /*! Return the Gen register from the GenIR one */
    GenRegister genReg(ir::Register, ir::Type type = ir::TYPE_FLOAT);
  private:
    /*! Expire one GRF interval. Return true if one was successfully expired */
    bool expireGRF(const GenRegInterval &limit);
    /*! Expire a flag register. Return true if one was successfully expired */
    bool expireFlag(const GenRegInterval &limit);
    /*! Allocate the virtual boolean (== flags) registers */
    void allocateFlags(Selection &selection);
    /*! Allocate the GRF registers */
    void allocateGRFs(Selection &selection);
    /*! Create a Gen register from a register set in the payload */
    void allocatePayloadReg(gbe_curbe_type, ir::Register, uint32_t subValue = 0, uint32_t subOffset = 0);
    /*! Create the intervals for each register */
    /*! Allocate the vectors detected in the instruction selection pass */
    void allocateVector(Selection &selection);
    /*! Create the given interval */
    void createGenReg(const GenRegInterval &interval);
    /*! Indicate if the registers are already allocated in vectors */
    bool isAllocated(const SelectionVector *vector) const;
    /*! Reallocate registers if needed to make the registers in the vector
     *  contigous in memory
     */
    void coalesce(Selection &selection, SelectionVector *vector);
    /*! The context owns the register allocator */
    GenContext &ctx;
    /*! Map virtual registers to offset in the (physical) register file */
    map<ir::Register, uint32_t> RA;
    /*! Provides the position of each register in a vector */
    map<ir::Register, VectorLocation> vectorMap;
    /*! All vectors used in the selection */
    vector<SelectionVector*> vectors;
    /*! All vectors that are already expired */
    set<SelectionVector*> expired;
    /*! The set of booleans that will go to GRF (cannot be kept into flags) */
    set<ir::Register> grfBooleans;
    /*! All the register intervals */
    vector<GenRegInterval> intervals;
    /*! Intervals sorting based on starting point positions */
    vector<GenRegInterval*> starting;
    /*! Intervals sorting based on ending point positions */
    vector<GenRegInterval*> ending;
    /*! Current vector to expire */
    uint32_t expiringID;
    /*! Use custom allocator */
    GBE_CLASS(GenRegAllocator);
  };

} /* namespace gbe */

#endif /* __GBE_GEN_REG_ALLOCATION_HPP__ */


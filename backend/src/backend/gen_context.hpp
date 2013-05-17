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
 * \file gen_context.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __GBE_GEN_CONTEXT_HPP__
#define __GBE_GEN_CONTEXT_HPP__

#include "backend/context.hpp"
#include "backend/program.h"
#include "ir/function.hpp"
#include "ir/liveness.hpp"
#include "sys/map.hpp"
#include <string>

namespace gbe
{
  class Kernel;               // We build this structure
  class GenEncoder;           // Helps emitting Gen ISA
  class GenRegAllocator;      // Handle the register allocation
  class Selection;            // Performs instruction selection
  class SelectionInstruction; // Pre-RA Gen instruction
  class SelectionReg;         // Pre-RA Gen register

  /*! Context is the helper structure to build the Gen ISA or simulation code
   *  from GenIR
   */
  class GenContext : public Context
  {
  public:
    /*! Create a new context. name is the name of the function we want to
     *  compile
     */
    GenContext(const ir::Unit &unit, const std::string &name, bool limitRegisterPressure = false);
    /*! Release everything needed */
    ~GenContext(void);
    /*! Implements base class */
    virtual bool emitCode(void);
    /*! Function we emit code for */
    INLINE const ir::Function &getFunction(void) const { return fn; }
    /*! Simd width chosen for the current function */
    INLINE uint32_t getSimdWidth(void) const { return simdWidth; }
    /*! Emit the per-lane stack pointer computation */
    void emitStackPointer(void);
    /*! Emit the instructions */
    void emitInstructionStream(void);
    /*! Set the correct target values for the branches */
    void patchBranches(void);
    /*! Forward ir::Function isSpecialReg method */
    INLINE bool isSpecialReg(ir::Register reg) const {
      return fn.isSpecialReg(reg);
    }
    /*! Get the liveOut information for the given block */
    INLINE const ir::Liveness::LiveOut &getLiveOut(const ir::BasicBlock *bb) const {
      return this->liveness->getLiveOut(bb);
    }
    /*! Final Gen ISA emission helper functions */
    void emitLabelInstruction(const SelectionInstruction &insn);
    void emitUnaryInstruction(const SelectionInstruction &insn);
    void emitBinaryInstruction(const SelectionInstruction &insn);
    void emitTernaryInstruction(const SelectionInstruction &insn);
    void emitCompareInstruction(const SelectionInstruction &insn);
    void emitJumpInstruction(const SelectionInstruction &insn);
    void emitIndirectMoveInstruction(const SelectionInstruction &insn);
    void emitEotInstruction(const SelectionInstruction &insn);
    void emitNoOpInstruction(const SelectionInstruction &insn);
    void emitWaitInstruction(const SelectionInstruction &insn);
    void emitBarrierInstruction(const SelectionInstruction &insn);
    void emitMathInstruction(const SelectionInstruction &insn);
    void emitUntypedReadInstruction(const SelectionInstruction &insn);
    void emitUntypedWriteInstruction(const SelectionInstruction &insn);
    void emitByteGatherInstruction(const SelectionInstruction &insn);
    void emitByteScatterInstruction(const SelectionInstruction &insn);
    void emitSampleInstruction(const SelectionInstruction &insn);
    void emitTypedWriteInstruction(const SelectionInstruction &insn);
    void emitGetImageInfoInstruction(const SelectionInstruction &insn);

    /*! Implements base class */
    virtual Kernel *allocateKernel(void);
    /*! Store the position of each label instruction in the Gen ISA stream */
    map<ir::LabelIndex, uint32_t> labelPos;
    /*! Store the Gen instructions to patch */
    vector<std::pair<ir::LabelIndex, uint32_t>> branchPos2;
    /*! Encode Gen ISA */
    GenEncoder *p;
    /*! Instruction selection on Gen ISA (pre-register allocation) */
    Selection *sel;
    /*! Perform the register allocation */
    GenRegAllocator *ra;
    /*! Indicate if we need to tackle a register pressure issue when
     * regenerating the code
     */
    bool limitRegisterPressure;
  };

} /* namespace gbe */

#endif /* __GBE_GEN_CONTEXT_HPP__ */


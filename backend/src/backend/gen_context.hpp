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
 * \file context.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __GBE_GEN_CONTEXT_HPP__
#define __GBE_GEN_CONTEXT_HPP__

#include "backend/context.hpp"
#include "backend/gen_eu.hpp"
#include "backend/program.h"
#include "ir/function.hpp"
#include "sys/map.hpp"
#include <string>

namespace gbe
{
  class Kernel;               // We build this structure
  class GenEmitter;           // Helps emitting Gen ISA
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
    GenContext(const ir::Unit &unit, const std::string &name);
    /*! Release everything needed */
    ~GenContext(void);
    /*! Implements base class */
    virtual void emitCode(void);
    /*! Function we emit code for */
    INLINE const ir::Function &getFunction(void) const { return fn; }
    /*! Simd width chosen for the current function */
    INLINE uint32_t getSimdWidth(void) const { return simdWidth; }
    /*! Emit the per-lane stack pointer computation */
    void emitStackPointer(void);
    /*! Emit the instructions */
    void emitInstructionStream(void);
    /*! Emit the instructions */
    void emitInstructionStream2(void);
    /*! Set the correct target values for the branches */
    void patchBranches(void);
    /*! Set the correct target values for the branches */
    void patchBranches2(void);
    /*! Bool registers will use scalar words. So we will consider them as
     *  scalars in Gen backend
     */
    bool isScalarOrBool(ir::Register reg) const;
    /*! Forward ir::Function method */
    INLINE bool isSpecialReg(ir::Register reg) const {
      return fn.isSpecialReg(reg);
    }

    /*! Emit instruction per family */
    void emitUnaryInstruction(const ir::UnaryInstruction &insn);
    void emitBinaryInstruction(const ir::BinaryInstruction &insn);
    void emitTernaryInstruction(const ir::TernaryInstruction &insn);
    void emitSelectInstruction(const ir::SelectInstruction &insn);
    void emitCompareInstruction(const ir::CompareInstruction &insn);
    void emitConvertInstruction(const ir::ConvertInstruction &insn);
    void emitBranchInstruction(const ir::BranchInstruction &insn);
    void emitLoadImmInstruction(const ir::LoadImmInstruction &insn);
    void emitLoadInstruction(const ir::LoadInstruction &insn);
    void emitStoreInstruction(const ir::StoreInstruction &insn);
    void emitSampleInstruction(const ir::SampleInstruction &insn);
    void emitTypedWriteInstruction(const ir::TypedWriteInstruction &insn);
    void emitFenceInstruction(const ir::FenceInstruction &insn);
    void emitLabelInstruction(const ir::LabelInstruction &insn);
    /*! It is not natively suppored on Gen. We implement it here */
    void emitIntMul32x32(const ir::Instruction &insn, GenReg dst, GenReg src0, GenReg src1);
    /*! Use untyped writes and reads for everything aligned on 4 bytes */
    void emitUntypedRead(const ir::LoadInstruction &insn, GenReg address);
    void emitUntypedWrite(const ir::StoreInstruction &insn);
    /*! Use byte scatters and gathers for everything not aligned on 4 bytes */
    void emitByteGather(const ir::LoadInstruction &insn, GenReg address, GenReg value);
    void emitByteScatter(const ir::StoreInstruction &insn, GenReg address, GenReg value);
    /*! Backward and forward branches are handled slightly differently */
    void emitForwardBranch(const ir::BranchInstruction&, ir::LabelIndex dst, ir::LabelIndex src);
    void emitBackwardBranch(const ir::BranchInstruction&, ir::LabelIndex dst, ir::LabelIndex src);

    ///////////////////// XXX ///////////////////////

    void emitLabelInstruction(const SelectionInstruction &insn);
    void emitUnaryInstruction(const SelectionInstruction &insn);
    void emitBinaryInstruction(const SelectionInstruction &insn);
    void emitSelectInstruction(const SelectionInstruction &insn);
    void emitCompareInstruction(const SelectionInstruction &insn);
    void emitJumpInstruction(const SelectionInstruction &insn);
    void emitEotInstruction(const SelectionInstruction &insn);
    void emitNoOpInstruction(const SelectionInstruction &insn);
    void emitWaitInstruction(const SelectionInstruction &insn);
    void emitMathInstruction(const SelectionInstruction &insn);
    void emitUntypedReadInstruction(const SelectionInstruction &insn);
    void emitUntypedWriteInstruction(const SelectionInstruction &insn);
    void emitByteGatherInstruction(const SelectionInstruction &insn);
    void emitByteScatterInstruction(const SelectionInstruction &insn);

    /*! Implements base class */
    virtual Kernel *allocateKernel(void);
    /*! Store the position of each label instruction in the Gen ISA stream */
    map<ir::LabelIndex, uint32_t> labelPos;
    /*! Store the position of each branch instruction in the Gen ISA stream */
    map<const ir::Instruction*, uint32_t> branchPos;
    /*! Store the Gen instructions to patch */
    vector<std::pair<ir::LabelIndex, uint32_t>> branchPos2;
    /*! Encode Gen ISA */
    GenEmitter *p;
    /*! Instruction selection on Gen ISA (pre-register allocation) */
    Selection *sel;
    /*! Perform the register allocation */
    GenRegAllocator *ra;
  };

} /* namespace gbe */

#endif /* __GBE_GEN_CONTEXT_HPP__ */


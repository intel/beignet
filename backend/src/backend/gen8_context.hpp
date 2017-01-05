/*
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * \file gen8_context.hpp
 */
#ifndef __GBE_GEN8_CONTEXT_HPP__
#define __GBE_GEN8_CONTEXT_HPP__

#include "backend/gen_context.hpp"
#include "backend/gen8_encoder.hpp"

namespace gbe
{
  /* This class is used to implement the HSW
     specific logic for context. */
  class Gen8Context : public GenContext
  {
  public:
    virtual ~Gen8Context(void) { }
    Gen8Context(const ir::Unit &unit, const std::string &name, uint32_t deviceID, bool relaxMath = false)
            : GenContext(unit, name, deviceID, relaxMath) {
    };
    /*! device's max srcatch buffer size */
    #define GEN8_SCRATCH_SIZE  (2 * KB * KB)
    /*! Align the scratch size to the device's scratch unit size */
    virtual uint32_t alignScratchSize(uint32_t size);
    /*! Get the device's max srcatch size */
    virtual uint32_t getScratchSize(void) {
      //Because the allocate is use uint16_t, so clamp it, need refine
      return std::min(GEN8_SCRATCH_SIZE, 0x7fff);
    }
    /*! Get the pointer argument size for curbe alloc */
    virtual uint32_t getPointerSize(void) { return 8; }
    /*! Set the correct target values for the branches */
    virtual bool patchBranches(void);

    virtual void emitUnaryInstruction(const SelectionInstruction &insn);
    virtual void emitUnaryWithTempInstruction(const SelectionInstruction &insn);
    virtual void emitSimdShuffleInstruction(const SelectionInstruction &insn);
    virtual void emitBinaryInstruction(const SelectionInstruction &insn);
    virtual void emitBinaryWithTempInstruction(const SelectionInstruction &insn);
    virtual void emitI64MULHIInstruction(const SelectionInstruction &insn);
    virtual void emitI64RHADDInstruction(const SelectionInstruction &insn);
    virtual void emitI64HADDInstruction(const SelectionInstruction &insn);
    virtual void emitI64ShiftInstruction(const SelectionInstruction &insn);
    virtual void emitI64CompareInstruction(const SelectionInstruction &insn);
    virtual void emitI64SATADDInstruction(const SelectionInstruction &insn);
    virtual void emitI64SATSUBInstruction(const SelectionInstruction &insn);
    virtual void emitI64ToFloatInstruction(const SelectionInstruction &insn);
    virtual void emitFloatToI64Instruction(const SelectionInstruction &insn);
    virtual void emitI64MADSATInstruction(const SelectionInstruction &insn);

    virtual void emitUntypedWriteA64Instruction(const SelectionInstruction &insn);
    virtual void emitUntypedReadA64Instruction(const SelectionInstruction &insn);
    virtual void emitByteGatherA64Instruction(const SelectionInstruction &insn);
    virtual void emitByteScatterA64Instruction(const SelectionInstruction &insn);
    virtual void emitWrite64Instruction(const SelectionInstruction &insn);
    virtual void emitRead64Instruction(const SelectionInstruction &insn);
    virtual void emitWrite64A64Instruction(const SelectionInstruction &insn);
    virtual void emitRead64A64Instruction(const SelectionInstruction &insn);
    virtual void emitAtomicA64Instruction(const SelectionInstruction &insn);
    virtual void emitI64MULInstruction(const SelectionInstruction &insn);
    virtual void emitI64DIVREMInstruction(const SelectionInstruction &insn);

    virtual void emitPackLongInstruction(const SelectionInstruction &insn);
    virtual void emitUnpackLongInstruction(const SelectionInstruction &insn);

    virtual void emitF64DIVInstruction(const SelectionInstruction &insn);

    virtual void emitWorkGroupOpInstruction(const SelectionInstruction &insn);
    virtual void emitSubGroupOpInstruction(const SelectionInstruction &insn);

    static GenRegister unpacked_ud(GenRegister reg, uint32_t offset = 0);

  protected:
    virtual void setA0Content(uint16_t new_a0[16], uint16_t max_offset = 0, int sz = 0);
    virtual void subTimestamps(GenRegister& t0, GenRegister& t1, GenRegister& tmp);
    virtual void addTimestamps(GenRegister& t0, GenRegister& t1, GenRegister& tmp);
    virtual void emitPrintfLongInstruction(GenRegister& addr, GenRegister& data, GenRegister& src, uint32_t bti);
    virtual GenEncoder* generateEncoder(void) {
      return GBE_NEW(Gen8Encoder, this->simdWidth, 8, deviceID);
    }

  private:
    virtual void emitSLMOffset(void);
    virtual void newSelection(void);
    void packLongVec(GenRegister unpacked, GenRegister packed, uint32_t simd);
    void unpackLongVec(GenRegister packed, GenRegister unpacked, uint32_t simd);
    void calculateFullS64MUL(GenRegister src0, GenRegister src1, GenRegister dst_h,
                             GenRegister dst_l, GenRegister s0_abs, GenRegister s1_abs,
                             GenRegister tmp0, GenRegister tmp1, GenRegister sign, GenRegister flagReg);
    virtual void calculateFullU64MUL(GenRegister src0, GenRegister src1, GenRegister dst_h,
                                           GenRegister dst_l, GenRegister s0l_s1h, GenRegister s0h_s1l);
  };

  class ChvContext : public Gen8Context
  {
  public:
    virtual ~ChvContext(void) { }
    ChvContext(const ir::Unit &unit, const std::string &name, uint32_t deviceID, bool relaxMath = false)
            : Gen8Context(unit, name, deviceID, relaxMath) {
    };
    virtual void emitI64MULInstruction(const SelectionInstruction &insn);

  protected:
    virtual void setA0Content(uint16_t new_a0[16], uint16_t max_offset = 0, int sz = 0);

  private:
    virtual void newSelection(void);
    virtual void calculateFullU64MUL(GenRegister src0, GenRegister src1, GenRegister dst_h,
                                           GenRegister dst_l, GenRegister s0l_s1h, GenRegister s0h_s1l);
    virtual void emitStackPointer(void);
  };
}
#endif /* __GBE_GEN8_CONTEXT_HPP__ */

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
 * \file gen_context.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "backend/gen_context.hpp"
#include "backend/gen_program.hpp"
#include "backend/gen_defs.hpp"
#include "backend/gen_encoder.hpp"
#include "backend/gen_insn_selection.hpp"
#include "backend/gen_reg_allocation.hpp"
#include "ir/function.hpp"
#include "sys/cvar.hpp"
#include <cstring>

namespace gbe
{
  ///////////////////////////////////////////////////////////////////////////
  // GenContext implementation
  ///////////////////////////////////////////////////////////////////////////
  GenContext::GenContext(const ir::Unit &unit, const std::string &name) :
    Context(unit, name)
  {
    this->p = GBE_NEW(GenEncoder, simdWidth, 7); // XXX handle more than Gen7
    this->sel = newSimpleSelection(*this);
    this->ra = GBE_NEW(GenRegAllocator, *this);
  }

  GenContext::~GenContext(void) {
    GBE_DELETE(this->ra);
    GBE_DELETE(this->sel);
    GBE_DELETE(this->p);
  }

  bool GenContext::isScalarOrBool(ir::Register reg) const {
    if (this->isScalarReg(reg))
      return true;
    else {
      const ir::RegisterFamily family = fn.getRegisterFamily(reg);
      return family == ir::FAMILY_BOOL;
    }
  }

  /*! XXX Make both structures the same! */
  INLINE void setInstructionState(GenInstructionState &dst,
                                  const SelectionState &src)
  {
    dst.execWidth = src.execWidth;
    dst.quarterControl = src.quarterControl;
    dst.noMask = src.noMask;
    dst.flag = src.flag;
    dst.subFlag = src.subFlag;
    dst.predicate = src.predicate;
    dst.inversePredicate = src.inversePredicate;
  }

  void GenContext::emitInstructionStream(void) {
    // Emit Gen ISA
    sel->foreachInstruction([&](const SelectionInstruction &insn) {
      const uint32_t opcode = insn.opcode;
      p->push();
      setInstructionState(p->curr, insn.state);
      switch (opcode) {
#define DECL_SELECTION_IR(OPCODE, FAMILY) \
  case SEL_OP_##OPCODE: this->emit##FAMILY(insn); break;
#include "backend/gen_insn_selection.hxx"
#undef DECL_INSN
      }
      p->pop();
    });
  }

  void GenContext::patchBranches(void) {
    using namespace ir;
    for (auto pair : branchPos2) {
      const LabelIndex label = pair.first;
      const int32_t insnID = pair.second;
      const int32_t targetID = labelPos.find(label)->second;
      p->patchJMPI(insnID, (targetID-insnID-1) * 2);
    }
  }

  void GenContext::emitStackPointer(void) {
    using namespace ir;

    // Only emit stack pointer computation if we use a stack
    if (kernel->getCurbeOffset(GBE_CURBE_STACK_POINTER, 0) <= 0)
      return;

    // Check that everything is consistent in the kernel code
    const uint32_t perLaneSize = kernel->getStackSize();
    const uint32_t perThreadSize = perLaneSize * this->simdWidth;
    const int32_t offset = kernel->getCurbeOffset(GBE_CURBE_EXTRA_ARGUMENT, GBE_STACK_BUFFER);
    GBE_ASSERT(perLaneSize > 0);
    GBE_ASSERT(isPowerOf<2>(perLaneSize) == true);
    GBE_ASSERT(isPowerOf<2>(perThreadSize) == true);

    // Use shifts rather than muls which are limited to 32x16 bit sources
    const uint32_t perLaneShift = logi2(perLaneSize);
    const uint32_t perThreadShift = logi2(perThreadSize);
    const SelectionReg selStatckPtr = this->simdWidth == 8 ?
      SelectionReg::ud8grf(ir::ocl::stackptr) :
      SelectionReg::ud16grf(ir::ocl::stackptr);
    const GenReg stackptr = ra->genReg(selStatckPtr);
    const uint32_t nr = offset / GEN_REG_SIZE;
    const uint32_t subnr = (offset % GEN_REG_SIZE) / sizeof(uint32_t);
    const GenReg bufferptr = GenReg::ud1grf(nr, subnr);

    // We compute the per-lane stack pointer here
    p->push();
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->SHR(GenReg::ud1grf(126,0), GenReg::ud1grf(0,5), GenReg::immud(10));
      p->curr.execWidth = this->simdWidth;
      p->SHL(stackptr, stackptr, GenReg::immud(perLaneShift));
      p->curr.execWidth = 1;
      p->SHL(GenReg::ud1grf(126,0), GenReg::ud1grf(126,0), GenReg::immud(perThreadShift));
      p->curr.execWidth = this->simdWidth;
      p->ADD(stackptr, stackptr, bufferptr);
      p->ADD(stackptr, stackptr, GenReg::ud1grf(126,0));
    p->pop();
  }

  void GenContext::emitLabelInstruction(const SelectionInstruction &insn) {
    const ir::LabelIndex label(insn.index);
    this->labelPos.insert(std::make_pair(label, p->store.size()));
  }

  void GenContext::emitUnaryInstruction(const SelectionInstruction &insn) {
    const GenReg dst = ra->genReg(insn.dst[0]);
    const GenReg src = ra->genReg(insn.src[0]);
    switch (insn.opcode) {
      case SEL_OP_MOV: p->MOV(dst, src); break;
      default: NOT_IMPLEMENTED;
    }
  }

  void GenContext::emitBinaryInstruction(const SelectionInstruction &insn) { 
    const GenReg dst = ra->genReg(insn.dst[0]);
    const GenReg src0 = ra->genReg(insn.src[0]);
    const GenReg src1 = ra->genReg(insn.src[1]);
    switch (insn.opcode) {
      case SEL_OP_AND:  p->AND(dst, src0, src1); break;
      case SEL_OP_OR:   p->OR(dst, src0, src1);  break;
      case SEL_OP_XOR:  p->XOR(dst, src0, src1); break;
      case SEL_OP_SHR:  p->SHR(dst, src0, src1); break;
      case SEL_OP_SHL:  p->SHL(dst, src0, src1); break;
      case SEL_OP_RSR:  p->RSR(dst, src0, src1); break;
      case SEL_OP_RSL:  p->RSL(dst, src0, src1); break;
      case SEL_OP_ASR:  p->ASR(dst, src0, src1); break;
      case SEL_OP_ADD:  p->ADD(dst, src0, src1); break;
      case SEL_OP_MUL:  p->MUL(dst, src0, src1); break;
      case SEL_OP_MACH: p->MACH(dst, src0, src1); break;
      default: NOT_IMPLEMENTED;
    }
  }

  void GenContext::emitSelectInstruction(const SelectionInstruction &insn) {
    NOT_IMPLEMENTED;
  }

  void GenContext::emitNoOpInstruction(const SelectionInstruction &insn) {
    NOT_IMPLEMENTED;
  }

  void GenContext::emitWaitInstruction(const SelectionInstruction &insn) {
    NOT_IMPLEMENTED;
  }

  void GenContext::emitMathInstruction(const SelectionInstruction &insn) {
    const GenReg dst = ra->genReg(insn.dst[0]);
    const GenReg src0 = ra->genReg(insn.src[0]);
    const GenReg src1 = ra->genReg(insn.src[1]);
    const uint32_t function = insn.function;
    p->MATH(dst, function, src0, src1);
  }

  void GenContext::emitCompareInstruction(const SelectionInstruction &insn) {
    const GenReg src0 = ra->genReg(insn.src[0]);
    const GenReg src1 = ra->genReg(insn.src[1]);
    p->CMP(insn.function, src0, src1);
  }

  void GenContext::emitJumpInstruction(const SelectionInstruction &insn) {
    const ir::LabelIndex label(insn.index);
    const GenReg src = ra->genReg(insn.src[0]);
    this->branchPos2.push_back(std::make_pair(label, p->store.size()));
    p->JMPI(src);
  }

  void GenContext::emitEotInstruction(const SelectionInstruction &insn) {
    p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.execWidth = 8;
      p->curr.noMask = 1;
      p->MOV(GenReg::f8grf(127,0), GenReg::f8grf(0,0));
      p->EOT(127);
    p->pop();
  }

  void GenContext::emitUntypedReadInstruction(const SelectionInstruction &insn) {
    const GenReg dst = ra->genReg(insn.dst[0]);
    const GenReg src = ra->genReg(insn.src[0]);
    const uint32_t bti = insn.function;
    const uint32_t elemNum = insn.elem;
    p->UNTYPED_READ(dst, src, bti, elemNum);
  }

  void GenContext::emitUntypedWriteInstruction(const SelectionInstruction &insn) {
    const GenReg src = ra->genReg(insn.src[0]);
    const uint32_t bti = insn.function;
    const uint32_t elemNum = insn.elem;
    p->UNTYPED_WRITE(src, bti, elemNum);
  }

  void GenContext::emitByteGatherInstruction(const SelectionInstruction &insn) {
    const GenReg dst = ra->genReg(insn.dst[0]);
    const GenReg src = ra->genReg(insn.src[0]);
    const uint32_t bti = insn.function;
    const uint32_t elemSize = insn.elem;
    p->BYTE_GATHER(dst, src, bti, elemSize);
  }

  void GenContext::emitByteScatterInstruction(const SelectionInstruction &insn) {
    const GenReg src = ra->genReg(insn.src[0]);
    const uint32_t bti = insn.function;
    const uint32_t elemSize = insn.elem;
    p->BYTE_SCATTER(src, bti, elemSize);
  }

  BVAR(OCL_OUTPUT_ASM, false);
  void GenContext::emitCode(void) {
    GenKernel *genKernel = static_cast<GenKernel*>(this->kernel);
    sel->select();
    ra->allocate(*this->sel);
    this->emitStackPointer();
    this->emitInstructionStream();
    this->patchBranches();
    genKernel->insnNum = p->store.size();
    genKernel->insns = GBE_NEW_ARRAY(GenInstruction, genKernel->insnNum);
    std::memcpy(genKernel->insns, &p->store[0], genKernel->insnNum * sizeof(GenInstruction));
    if (OCL_OUTPUT_ASM) {
      FILE *f = fopen("asm.dump", "wb");
      fwrite(genKernel->insns, 1, genKernel->insnNum * sizeof(GenInstruction), f);
      fclose(f);
    }
  }

  Kernel *GenContext::allocateKernel(void) {
    return GBE_NEW(GenKernel, name);
  }

} /* namespace gbe */


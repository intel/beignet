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
#include "backend/gen/gen_mesa_disasm.h"
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
    this->sel = newSelection(*this);
    this->ra = GBE_NEW(GenRegAllocator, *this);
  }

  GenContext::~GenContext(void) {
    GBE_DELETE(this->ra);
    GBE_DELETE(this->sel);
    GBE_DELETE(this->p);
  }

  void GenContext::emitInstructionStream(void) {
    // Emit Gen ISA
    sel->foreachInstruction([&](const SelectionInstruction &insn) {
      const uint32_t opcode = insn.opcode;
      p->push();
      // no more virtual register here in that part of the code generation
      GBE_ASSERT(insn.state.physicalFlag);
      p->curr = insn.state;
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
    const int32_t offset = GEN_REG_SIZE + kernel->getCurbeOffset(GBE_CURBE_EXTRA_ARGUMENT, GBE_STACK_BUFFER);
    GBE_ASSERT(perLaneSize > 0);
    GBE_ASSERT(isPowerOf<2>(perLaneSize) == true);
    GBE_ASSERT(isPowerOf<2>(perThreadSize) == true);

    // Use shifts rather than muls which are limited to 32x16 bit sources
    const uint32_t perLaneShift = logi2(perLaneSize);
    const uint32_t perThreadShift = logi2(perThreadSize);
    const GenRegister selStatckPtr = this->simdWidth == 8 ?
      GenRegister::ud8grf(ir::ocl::stackptr) :
      GenRegister::ud16grf(ir::ocl::stackptr);
    const GenRegister stackptr = ra->genReg(selStatckPtr);
    const uint32_t nr = offset / GEN_REG_SIZE;
    const uint32_t subnr = (offset % GEN_REG_SIZE) / sizeof(uint32_t);
    const GenRegister bufferptr = GenRegister::ud1grf(nr, subnr);

    // We compute the per-lane stack pointer here
    p->push();
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->SHR(GenRegister::ud1grf(126,0), GenRegister::ud1grf(0,5), GenRegister::immud(10));
      p->curr.execWidth = this->simdWidth;
      p->SHL(stackptr, stackptr, GenRegister::immud(perLaneShift));
      p->curr.execWidth = 1;
      p->SHL(GenRegister::ud1grf(126,0), GenRegister::ud1grf(126,0), GenRegister::immud(perThreadShift));
      p->curr.execWidth = this->simdWidth;
      p->ADD(stackptr, stackptr, bufferptr);
      p->ADD(stackptr, stackptr, GenRegister::ud1grf(126,0));
    p->pop();
  }

  void GenContext::emitLabelInstruction(const SelectionInstruction &insn) {
    const ir::LabelIndex label(insn.index);
    this->labelPos.insert(std::make_pair(label, p->store.size()));
  }

  void GenContext::emitUnaryInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    switch (insn.opcode) {
      case SEL_OP_MOV: p->MOV(dst, src); break;
      case SEL_OP_NOT: p->NOT(dst, src); break;
      default: NOT_IMPLEMENTED;
    }
  }

  void GenContext::emitBinaryInstruction(const SelectionInstruction &insn) { 
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src0 = ra->genReg(insn.src(0));
    const GenRegister src1 = ra->genReg(insn.src(1));
    switch (insn.opcode) {
      case SEL_OP_SEL:  p->SEL(dst, src0, src1); break;
      case SEL_OP_AND:  p->AND(dst, src0, src1); break;
      case SEL_OP_OR:   p->OR (dst, src0, src1);  break;
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

  void GenContext::emitTernaryInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src0 = ra->genReg(insn.src(0));
    const GenRegister src1 = ra->genReg(insn.src(1));
    const GenRegister src2 = ra->genReg(insn.src(2));
    switch (insn.opcode) {
      case SEL_OP_MAD:  p->MAD(dst, src0, src1, src2); break;
      default: NOT_IMPLEMENTED;
    }
  }

  void GenContext::emitNoOpInstruction(const SelectionInstruction &insn) {
    NOT_IMPLEMENTED;
  }

  void GenContext::emitWaitInstruction(const SelectionInstruction &insn) {
    NOT_IMPLEMENTED;
  }

  void GenContext::emitMathInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src0 = ra->genReg(insn.src(0));
    const GenRegister src1 = ra->genReg(insn.src(1));
    const uint32_t function = insn.extra.function;
    p->MATH(dst, function, src0, src1);
  }

  void GenContext::emitCompareInstruction(const SelectionInstruction &insn) {
    const GenRegister src0 = ra->genReg(insn.src(0));
    const GenRegister src1 = ra->genReg(insn.src(1));
    p->CMP(insn.extra.function, src0, src1);
  }

  void GenContext::emitJumpInstruction(const SelectionInstruction &insn) {
    const ir::LabelIndex label(insn.index);
    const GenRegister src = ra->genReg(insn.src(0));
    this->branchPos2.push_back(std::make_pair(label, p->store.size()));
    p->JMPI(src);
  }

  void GenContext::emitEotInstruction(const SelectionInstruction &insn) {
    p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.execWidth = 8;
      p->curr.noMask = 1;
      p->MOV(GenRegister::f8grf(127,0), GenRegister::f8grf(0,0));
      p->EOT(127);
    p->pop();
  }

  void GenContext::emitUntypedReadInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t bti = insn.extra.function;
    const uint32_t elemNum = insn.extra.elem;
    p->UNTYPED_READ(dst, src, bti, elemNum);
  }

  void GenContext::emitUntypedWriteInstruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t bti = insn.extra.function;
    const uint32_t elemNum = insn.extra.elem;
    p->UNTYPED_WRITE(src, bti, elemNum);
  }

  void GenContext::emitByteGatherInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t bti = insn.extra.function;
    const uint32_t elemSize = insn.extra.elem;
    p->BYTE_GATHER(dst, src, bti, elemSize);
  }

  void GenContext::emitByteScatterInstruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t bti = insn.extra.function;
    const uint32_t elemSize = insn.extra.elem;
    p->BYTE_SCATTER(src, bti, elemSize);
  }

  void GenContext::emitRegionInstruction(const SelectionInstruction &insn) {
    GBE_ASSERT(insn.dst(0).width == GEN_WIDTH_8 ||
               insn.dst(0).width == GEN_WIDTH_16);
    const GenRegister src = ra->genReg(insn.src(0));
    const GenRegister dst = ra->genReg(insn.dst(1));
    const GenRegister final = ra->genReg(insn.dst(0));

    // Region dimensions
    const uint32_t offset = insn.extra.offset;
    const uint32_t width = insn.extra.width;
    const uint32_t height = simdWidth / insn.extra.width;
    const uint32_t vstride = insn.extra.vstride;
    const uint32_t hstride = insn.extra.hstride;

    // Region spanning in the grf
    const uint32_t start = src.nr * GEN_REG_SIZE + src.subnr + offset * sizeof(int);
    const uint32_t end = start + insn.srcNum * simdWidth * sizeof(int);
    GBE_ASSERT(simdWidth % width == 0);

    // Right now we simply emit scalar MOVs instead of the region
    p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.execWidth = 1;
      p->curr.noMask = 1;
      uint32_t dstOffset = dst.nr * GEN_REG_SIZE + dst.subnr;
      for (uint32_t y = 0; y < height; ++y) {
        uint32_t srcOffset = start + sizeof(int) * vstride * y;
        for (uint32_t x = 0; x < width; ++x,
             srcOffset += sizeof(int) * hstride,
             dstOffset += sizeof(int))
        {
          const uint32_t dstnr = dstOffset / GEN_REG_SIZE;
          const uint32_t dstsubnr = (dstOffset % GEN_REG_SIZE) / sizeof(int);
          const GenRegister dstReg = GenRegister::f1grf(dstnr, dstsubnr);
          if (srcOffset + sizeof(int) > end)
            p->MOV(dstReg, GenRegister::immf(0.f));
          else {
            GBE_ASSERT(srcOffset % sizeof(int) == 0);
            const uint32_t srcnr = srcOffset / GEN_REG_SIZE;
            const uint32_t srcsubnr = (srcOffset % GEN_REG_SIZE) / sizeof(int);
            const GenRegister srcReg = GenRegister::f1grf(srcnr, srcsubnr);
            p->MOV(dstReg, srcReg);
          }
        }
      }
    p->pop();
    p->MOV(GenRegister::retype(final, GEN_TYPE_F), GenRegister::retype(dst, GEN_TYPE_F));
  }

  void GenContext::emitRGatherInstruction(const SelectionInstruction &insn) {
    const GenRegister index0 = GenRegister::retype(ra->genReg(insn.src(0)), GEN_TYPE_UW);
    const GenRegister dst0 = GenRegister::retype(ra->genReg(insn.dst(0)), GEN_TYPE_F);
    const GenRegister src = ra->genReg(insn.src(1));
    const uint32_t offset = src.nr * GEN_REG_SIZE + src.subnr;
    p->push();
      p->curr.execWidth = 8;
      p->SHL(GenRegister::addr8(0), index0, GenRegister::immuw(2));
      p->ADD(GenRegister::addr8(0), GenRegister::addr8(0), GenRegister::immuw(offset));
      p->MOV(dst0, GenRegister::indirect(GEN_TYPE_F, 0, GEN_WIDTH_8));
    p->pop();

    if (simdWidth == 16) {
      const GenRegister dst1 = GenRegister::Qn(dst0, 1);
      const GenRegister index1 = GenRegister::Qn(index0, 1);
      p->push();
        p->curr.execWidth = 8;
        p->curr.quarterControl = GEN_COMPRESSION_Q2;
        p->SHL(GenRegister::addr8(0), index1, GenRegister::immuw(2));
        p->ADD(GenRegister::addr8(0), GenRegister::addr8(0), GenRegister::immuw(offset));
        p->MOV(dst1, GenRegister::indirect(GEN_TYPE_F, 0, GEN_WIDTH_8));
      p->pop();
    }
  }

  void GenContext::emitOBReadInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister addr = ra->genReg(insn.src(0));
    const GenRegister first = GenRegister::ud1grf(addr.nr,addr.subnr/sizeof(float));
    GenRegister header;
    if (simdWidth == 8)
      header = GenRegister::retype(ra->genReg(insn.src(1)), GEN_TYPE_F);
    else
      header = GenRegister::retype(GenRegister::Qn(ra->genReg(insn.src(1)),1), GEN_TYPE_F);

    p->push();
      // Copy r0 into the header first
      p->curr.execWidth = 8;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->MOV(header, GenRegister::f8grf(0,0));

      // Update the header with the current address
      p->curr.execWidth = 1;
      const uint32_t nr = header.nr;
      const uint32_t subnr = header.subnr / sizeof(float);
      p->SHR(GenRegister::ud1grf(nr, subnr+2), first, GenRegister::immud(4));

      // Put zero in the general state base address
      p->MOV(GenRegister::f1grf(nr, subnr+5), GenRegister::immf(0));

      // Now read the data
      p->OBREAD(dst, header, insn.extra.function, insn.extra.elem);
    p->pop();
  }

  void GenContext::emitOBWriteInstruction(const SelectionInstruction &insn) {
    const GenRegister addr = ra->genReg(insn.src(2));
    const GenRegister first = GenRegister::ud1grf(addr.nr,addr.subnr/sizeof(float));
    GenRegister header;
    if (simdWidth == 8)
      header = GenRegister::retype(ra->genReg(insn.src(0)), GEN_TYPE_F);
    else
      header = GenRegister::retype(GenRegister::Qn(ra->genReg(insn.src(0)),1), GEN_TYPE_F);

    p->push();
      // Copy r0 into the header first
      p->curr.execWidth = 8;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->MOV(header, GenRegister::f8grf(0,0));

      // Update the header with the current address
      p->curr.execWidth = 1;
      const uint32_t nr = header.nr;
      const uint32_t subnr = header.subnr / sizeof(float);
      p->SHR(GenRegister::ud1grf(nr, subnr+2), first, GenRegister::immud(4));

      // Put zero in the general state base address
      p->MOV(GenRegister::f1grf(nr, subnr+5), GenRegister::immf(0));

      // Now read the data
      p->OBWRITE(header, insn.extra.function, insn.extra.elem);
    p->pop();
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
    if (OCL_OUTPUT_ASM)
      for (uint32_t insnID = 0; insnID < genKernel->insnNum; ++insnID)
        gen_disasm(stdout, &p->store[insnID]);
  }

  Kernel *GenContext::allocateKernel(void) {
    return GBE_NEW(GenKernel, name);
  }

} /* namespace gbe */


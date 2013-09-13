/*
 * Copyright © 2012 Intel Corporatin
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
#include "backend/gen_insn_scheduling.hpp"
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
  GenContext::GenContext(const ir::Unit &unit,
                         const std::string &name,
                         bool limitRegisterPressure) :
    Context(unit, name), limitRegisterPressure(limitRegisterPressure)
  {
    this->p = GBE_NEW(GenEncoder, simdWidth, 7); // XXX handle more than Gen7
    this->sel = GBE_NEW(Selection, *this);
    this->ra = GBE_NEW(GenRegAllocator, *this);
  }

  GenContext::~GenContext(void) {
    GBE_DELETE(this->ra);
    GBE_DELETE(this->sel);
    GBE_DELETE(this->p);
  }

  void GenContext::emitInstructionStream(void) {
    // Emit Gen ISA
    for (auto &block : *sel->blockList)
    for (auto &insn : block.insnList) {
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
    }
    /* per spec, pad the instruction stream with 8 nop to avoid
	instruction prefetcher prefetch into an invalide page */
    for(int i = 0; i < 8; i++)
	p->NOP();
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

  void GenContext::clearFlagRegister(void) {
    // when group size not aligned to simdWidth, flag register need clear to
    // make prediction(any8/16h) work correctly
    p->push();
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.noMask = 1;
    p->curr.execWidth = 1;
    p->MOV(GenRegister::retype(GenRegister::flag(0,0), GEN_TYPE_UD), GenRegister::immud(0x0));
    p->MOV(GenRegister::retype(GenRegister::flag(1,0), GEN_TYPE_UD), GenRegister::immud(0x0));
    p->pop();
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
      p->AND(GenRegister::ud1grf(126,0), GenRegister::ud1grf(0,5), GenRegister::immud(0x1ff));
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
      case SEL_OP_FBH: p->FBH(dst, src); break;
      case SEL_OP_FBL: p->FBL(dst, src); break;
      case SEL_OP_NOT: p->NOT(dst, src); break;
      case SEL_OP_RNDD: p->RNDD(dst, src); break;
      case SEL_OP_RNDU: p->RNDU(dst, src); break;
      case SEL_OP_RNDE: p->RNDE(dst, src); break;
      case SEL_OP_RNDZ: p->RNDZ(dst, src); break;
      case SEL_OP_LOAD_INT64_IMM: p->LOAD_INT64_IMM(dst, src.value.i64); break;
      case SEL_OP_CONVI64_TO_I:
       {
        int execWidth = p->curr.execWidth;
        GenRegister xsrc = src.bottom_half(), xdst = dst;
        p->push();
        p->curr.execWidth = 8;
        for(int i = 0; i < execWidth/4; i ++) {
          p->curr.chooseNib(i);
          p->MOV(xdst, xsrc);
          xdst = GenRegister::suboffset(xdst, 4);
          xsrc = GenRegister::suboffset(xsrc, 8);
        }
        p->pop();
        break;
       }
      default: NOT_IMPLEMENTED;
    }
  }

  void GenContext::emitUnaryWithTempInstruction(const SelectionInstruction &insn) {
    GenRegister dst = ra->genReg(insn.dst(0));
    GenRegister src = ra->genReg(insn.src(0));
    GenRegister tmp = ra->genReg(insn.dst(1));
    switch (insn.opcode) {
      case SEL_OP_LOAD_DF_IMM:
        p->LOAD_DF_IMM(dst, tmp, src.value.df);
        break;
      case SEL_OP_MOV_DF:
        p->MOV_DF(dst, src, tmp);
        break;
      case SEL_OP_CONVI_TO_I64: {
        GenRegister middle;
        if (src.type == GEN_TYPE_B || src.type == GEN_TYPE_D) {
          middle = tmp;
          middle.type = src.is_signed_int() ? GEN_TYPE_D : GEN_TYPE_UD;
          p->MOV(middle, src);
        } else {
          middle = src;
        }
        int execWidth = p->curr.execWidth;
        p->push();
        p->curr.execWidth = 8;
        for (int nib = 0; nib < execWidth / 4; nib ++) {
          p->curr.chooseNib(nib);
          p->MOV(dst.bottom_half(), middle);
          if(middle.is_signed_int())
            p->ASR(dst.top_half(), middle, GenRegister::immud(31));
          else
            p->MOV(dst.top_half(), GenRegister::immd(0));
          dst = GenRegister::suboffset(dst, 4);
          middle = GenRegister::suboffset(middle, 4);
        }
        p->pop();
        break;
      }
      default:
        NOT_IMPLEMENTED;
    }
  }

  void GenContext::emitBinaryWithTempInstruction(const SelectionInstruction &insn) {
    GenRegister dst = ra->genReg(insn.dst(0));
    GenRegister src0 = ra->genReg(insn.src(0));
    GenRegister src1 = ra->genReg(insn.src(1));
    GenRegister tmp = ra->genReg(insn.dst(1));
    switch (insn.opcode) {
      case SEL_OP_I64ADD: {
        GenRegister x = GenRegister::retype(tmp, GEN_TYPE_UD),
                    y = GenRegister::suboffset(x, p->curr.execWidth);
        loadBottomHalf(x, src0);
        loadBottomHalf(y, src1);
        addWithCarry(x, x, y);
        storeBottomHalf(dst, x);
        loadTopHalf(x, src0);
        p->ADD(x, x, y);
        loadTopHalf(y, src1);
        p->ADD(x, x, y);
        storeTopHalf(dst, x);
        break;
      }
      case SEL_OP_I64SUB: {
        GenRegister x = GenRegister::retype(tmp, GEN_TYPE_UD),
                    y = GenRegister::suboffset(x, p->curr.execWidth);
        loadBottomHalf(x, src0);
        loadBottomHalf(y, src1);
        subWithBorrow(x, x, y);
        storeBottomHalf(dst, x);
        loadTopHalf(x, src0);
        subWithBorrow(x, x, y);
        loadTopHalf(y, src1);
        subWithBorrow(x, x, y);
        storeTopHalf(dst, x);
        break;
      }
      case SEL_OP_MUL_HI: {
        int w = p->curr.execWidth;
        p->push();
        p->curr.execWidth = 8;
        for (int i = 0; i < w / 8; i ++) {
          p->push();
          p->curr.predicate = GEN_PREDICATE_NONE;
          p->MUL(GenRegister::retype(GenRegister::acc(), GEN_TYPE_UD), src0, src1);
          p->curr.accWrEnable = 1;
          p->MACH(tmp, src0, src1);
          p->pop();
          p->curr.quarterControl = i;
          p->MOV(dst, tmp);
          dst = GenRegister::Qn(dst, 1);
          src0 = GenRegister::Qn(src0, 1);
          src1 = GenRegister::Qn(src1, 1);
        }
        p->pop();
        break;
       }
     case SEL_OP_HADD: {
        int w = p->curr.execWidth;
        p->push();
        p->curr.execWidth = 8;
        for (int i = 0; i < w / 8; i ++) {
          p->curr.quarterControl = i;
          p->ADDC(dst, src0, src1);
          p->SHR(dst, dst, GenRegister::immud(1));
          p->SHL(tmp, GenRegister::retype(GenRegister::acc(), GEN_TYPE_D), GenRegister::immud(31));
          p->OR(dst, dst, tmp);
          dst = GenRegister::Qn(dst, 1);
          src0 = GenRegister::Qn(src0, 1);
          src1 = GenRegister::Qn(src1, 1);
        }
        p->pop();
        break;
       }
      case SEL_OP_RHADD: {
        int w = p->curr.execWidth;
        p->push();
        p->curr.execWidth = 8;
        for (int i = 0; i < w / 8; i ++) {
          p->curr.quarterControl = i;
          p->ADDC(dst, src0, src1);
          p->ADD(dst, dst, GenRegister::immud(1));
          p->SHR(dst, dst, GenRegister::immud(1));
          p->SHL(tmp, GenRegister::retype(GenRegister::acc(), GEN_TYPE_D), GenRegister::immud(31));
          p->OR(dst, dst, tmp);
          dst = GenRegister::Qn(dst, 1);
          src0 = GenRegister::Qn(src0, 1);
          src1 = GenRegister::Qn(src1, 1);
        }
        p->pop();
        break;
       }
      default:
        NOT_IMPLEMENTED;
    }
  }

  void GenContext::emitBinaryInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src0 = ra->genReg(insn.src(0));
    const GenRegister src1 = ra->genReg(insn.src(1));
    switch (insn.opcode) {
      case SEL_OP_SEL:  p->SEL(dst, src0, src1); break;
      case SEL_OP_SEL_INT64:
        {
          GenRegister xdst = GenRegister::retype(dst, GEN_TYPE_UL),
                      xsrc0 = GenRegister::retype(src0, GEN_TYPE_UL),
                      xsrc1 = GenRegister::retype(src1, GEN_TYPE_UL);
          int execWidth = p->curr.execWidth;
          p->push();
          p->curr.execWidth = 8;
          for (int nib = 0; nib < execWidth / 4; nib ++) {
            p->curr.chooseNib(nib);
            p->SEL(xdst.bottom_half(), xsrc0.bottom_half(), xsrc1.bottom_half());
            p->SEL(xdst.top_half(), xsrc0.top_half(), xsrc1.top_half());
            xdst = GenRegister::suboffset(xdst, 4);
            xsrc0 = GenRegister::suboffset(xsrc0, 4);
            xsrc1 = GenRegister::suboffset(xsrc1, 4);
          }
          p->pop();
        }
        break;
      case SEL_OP_AND:  p->AND(dst, src0, src1); break;
      case SEL_OP_OR:   p->OR (dst, src0, src1);  break;
      case SEL_OP_XOR:  p->XOR(dst, src0, src1); break;
      case SEL_OP_I64AND:
        {
          GenRegister xdst = GenRegister::retype(dst, GEN_TYPE_UL),
                      xsrc0 = GenRegister::retype(src0, GEN_TYPE_UL),
                      xsrc1 = GenRegister::retype(src1, GEN_TYPE_UL);
          int execWidth = p->curr.execWidth;
          p->push();
          p->curr.execWidth = 8;
          for (int nib = 0; nib < execWidth / 4; nib ++) {
            p->curr.chooseNib(nib);
            p->AND(xdst.bottom_half(), xsrc0.bottom_half(), xsrc1.bottom_half());
            p->AND(xdst.top_half(), xsrc0.top_half(), xsrc1.top_half());
            xdst = GenRegister::suboffset(xdst, 4),
            xsrc0 = GenRegister::suboffset(xsrc0, 4),
            xsrc1 = GenRegister::suboffset(xsrc1, 4);
          }
          p->pop();
        }
        break;
      case SEL_OP_I64OR:
        {
          GenRegister xdst = GenRegister::retype(dst, GEN_TYPE_UL),
                      xsrc0 = GenRegister::retype(src0, GEN_TYPE_UL),
                      xsrc1 = GenRegister::retype(src1, GEN_TYPE_UL);
          int execWidth = p->curr.execWidth;
          p->push();
          p->curr.execWidth = 8;
          for (int nib = 0; nib < execWidth / 4; nib ++) {
            p->curr.chooseNib(nib);
            p->OR(xdst.bottom_half(), xsrc0.bottom_half(), xsrc1.bottom_half());
            p->OR(xdst.top_half(), xsrc0.top_half(), xsrc1.top_half());
            xdst = GenRegister::suboffset(xdst, 4),
            xsrc0 = GenRegister::suboffset(xsrc0, 4),
            xsrc1 = GenRegister::suboffset(xsrc1, 4);
          }
          p->pop();
        }
        break;
      case SEL_OP_I64XOR:
        {
          GenRegister xdst = GenRegister::retype(dst, GEN_TYPE_UL),
                      xsrc0 = GenRegister::retype(src0, GEN_TYPE_UL),
                      xsrc1 = GenRegister::retype(src1, GEN_TYPE_UL);
          int execWidth = p->curr.execWidth;
          p->push();
          p->curr.execWidth = 8;
          for (int nib = 0; nib < execWidth / 4; nib ++) {
            p->curr.chooseNib(nib);
            p->XOR(xdst.bottom_half(), xsrc0.bottom_half(), xsrc1.bottom_half());
            p->XOR(xdst.top_half(), xsrc0.top_half(), xsrc1.top_half());
            xdst = GenRegister::suboffset(xdst, 4),
            xsrc0 = GenRegister::suboffset(xsrc0, 4),
            xsrc1 = GenRegister::suboffset(xsrc1, 4);
          }
          p->pop();
        }
        break;
      case SEL_OP_SHR:  p->SHR(dst, src0, src1); break;
      case SEL_OP_SHL:  p->SHL(dst, src0, src1); break;
      case SEL_OP_RSR:  p->RSR(dst, src0, src1); break;
      case SEL_OP_RSL:  p->RSL(dst, src0, src1); break;
      case SEL_OP_ASR:  p->ASR(dst, src0, src1); break;
      case SEL_OP_ADD:  p->ADD(dst, src0, src1); break;
      case SEL_OP_MUL:  p->MUL(dst, src0, src1); break;
      case SEL_OP_MACH: p->MACH(dst, src0, src1); break;
      case SEL_OP_UPSAMPLE_SHORT: p->UPSAMPLE_SHORT(dst, src0, src1); break;
      case SEL_OP_UPSAMPLE_INT: p->UPSAMPLE_INT(dst, src0, src1); break;
      case SEL_OP_UPSAMPLE_LONG:
        {
          GenRegister xdst = GenRegister::retype(dst, GEN_TYPE_UL),
                      xsrc0 = GenRegister::retype(src0, GEN_TYPE_UL),
                      xsrc1 = GenRegister::retype(src1, GEN_TYPE_UL);
          int execWidth = p->curr.execWidth;
          p->push();
          p->curr.execWidth = 8;
          for (int nib = 0; nib < execWidth / 4; nib ++) {
            p->curr.chooseNib(nib);
            p->MOV(xdst.top_half(), xsrc0.bottom_half());
            p->MOV(xdst.bottom_half(), xsrc1.bottom_half());
            xdst = GenRegister::suboffset(xdst, 4);
            xsrc0 = GenRegister::suboffset(xsrc0, 4);
            xsrc1 = GenRegister::suboffset(xsrc1, 4);
          }
          p->pop();
        }
        break;
      default: NOT_IMPLEMENTED;
    }
  }

  void GenContext::collectShifter(GenRegister dest, GenRegister src) {
    int execWidth = p->curr.execWidth;
    p->push();
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.execWidth = 8;
    for (int nib = 0; nib < execWidth / 4; nib ++) {
      p->AND(dest, src.bottom_half(), GenRegister::immud(63));
      dest = GenRegister::suboffset(dest, 4);
      src = GenRegister::suboffset(src, 4);
    }
    p->pop();
  }

  void GenContext::emitI64HADDInstruction(const SelectionInstruction &insn) {
    GenRegister dest = ra->genReg(insn.dst(0));
    GenRegister x = ra->genReg(insn.src(0));
    GenRegister y = ra->genReg(insn.src(1));
    GenRegister a = ra->genReg(insn.dst(1));
    GenRegister b = ra->genReg(insn.dst(2));
    GenRegister c = ra->genReg(insn.dst(3));
    GenRegister d = ra->genReg(insn.dst(4));
    a.type = b.type = c.type = d.type = GEN_TYPE_UD;
    loadBottomHalf(a, x);
    loadBottomHalf(b, y);
    loadTopHalf(c, x);
    loadTopHalf(d, y);
    addWithCarry(a, a, b);
    addWithCarry(c, c, b);
    addWithCarry(c, c, d);
    p->ADD(b, b, d);
    p->SHR(a, a, GenRegister::immud(1));
    p->SHL(d, c, GenRegister::immud(31));
    p->OR(a, a, d);
    p->SHR(c, c, GenRegister::immud(1));
    p->SHL(d, b, GenRegister::immud(31));
    p->OR(c, c, d);
    storeBottomHalf(dest, a);
    storeTopHalf(dest, c);
  }

  void GenContext::emitI64ShiftInstruction(const SelectionInstruction &insn) {
    GenRegister dest = ra->genReg(insn.dst(0));
    GenRegister x = ra->genReg(insn.src(0));
    GenRegister y = ra->genReg(insn.src(1));
    GenRegister a = ra->genReg(insn.dst(1));
    GenRegister b = ra->genReg(insn.dst(2));
    GenRegister c = ra->genReg(insn.dst(3));
    GenRegister d = ra->genReg(insn.dst(4));
    GenRegister e = ra->genReg(insn.dst(5));
    GenRegister f = ra->genReg(insn.dst(6));
    a.type = b.type = c.type = d.type = e.type = f.type = GEN_TYPE_UD;
    GenRegister zero = GenRegister::immud(0);
    switch(insn.opcode) {
      case SEL_OP_I64SHL:
        p->push();
        p->curr.predicate = GEN_PREDICATE_NONE;
        collectShifter(a, y);
        loadBottomHalf(e, x);
        loadTopHalf(f, x);
        p->SHR(b, e, GenRegister::negate(a));
        p->SHL(c, e, a);
        p->SHL(d, f, a);
        p->OR(e, d, b);
        p->MOV(GenRegister::flag(1, 1), GenRegister::immuw(0xFFFF));
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.physicalFlag = 1, p->curr.flag = 1, p->curr.subFlag = 1;
        p->CMP(GEN_CONDITIONAL_Z, a, zero);
        p->SEL(d, d, e);
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->AND(a, a, GenRegister::immud(32));
        p->MOV(GenRegister::flag(1, 1), GenRegister::immuw(0xFFFF));
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.physicalFlag = 1, p->curr.flag = 1, p->curr.subFlag = 1;
        p->CMP(GEN_CONDITIONAL_Z, a, zero);
        p->SEL(d, d, c);
        p->SEL(c, c, zero);
        p->pop();
        storeBottomHalf(dest, c);
        storeTopHalf(dest, d);
        break;
      case SEL_OP_I64SHR:
        p->push();
        p->curr.predicate = GEN_PREDICATE_NONE;
        collectShifter(a, y);
        loadBottomHalf(e, x);
        loadTopHalf(f, x);
        p->SHL(b, f, GenRegister::negate(a));
        p->SHR(c, f, a);
        p->SHR(d, e, a);
        p->OR(e, d, b);
        p->MOV(GenRegister::flag(1, 1), GenRegister::immuw(0xFFFF));
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.physicalFlag = 1, p->curr.flag = 1, p->curr.subFlag = 1;
        p->CMP(GEN_CONDITIONAL_Z, a, zero);
        p->SEL(d, d, e);
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->AND(a, a, GenRegister::immud(32));
        p->MOV(GenRegister::flag(1, 1), GenRegister::immuw(0xFFFF));
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.physicalFlag = 1, p->curr.flag = 1, p->curr.subFlag = 1;
        p->CMP(GEN_CONDITIONAL_Z, a, zero);
        p->SEL(d, d, c);
        p->SEL(c, c, zero);
        p->pop();
        storeBottomHalf(dest, d);
        storeTopHalf(dest, c);
        break;
      case SEL_OP_I64ASR:
        f.type = GEN_TYPE_D;
        p->push();
        p->curr.predicate = GEN_PREDICATE_NONE;
        collectShifter(a, y);
        loadBottomHalf(e, x);
        loadTopHalf(f, x);
        p->SHL(b, f, GenRegister::negate(a));
        p->ASR(c, f, a);
        p->SHR(d, e, a);
        p->OR(e, d, b);
        p->MOV(GenRegister::flag(1, 1), GenRegister::immuw(0xFFFF));
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.physicalFlag = 1, p->curr.flag = 1, p->curr.subFlag = 1;
        p->CMP(GEN_CONDITIONAL_Z, a, zero);
        p->SEL(d, d, e);
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->AND(a, a, GenRegister::immud(32));
        p->MOV(GenRegister::flag(1, 1), GenRegister::immuw(0xFFFF));
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.physicalFlag = 1, p->curr.flag = 1, p->curr.subFlag = 1;
        p->CMP(GEN_CONDITIONAL_Z, a, zero);
        p->SEL(d, d, c);
        p->SEL(c, c, GenRegister::immd(-1));
        p->pop();
        storeBottomHalf(dest, d);
        storeTopHalf(dest, c);
        break;
      default:
        NOT_IMPLEMENTED;
    }
  }

  void GenContext::saveFlag(GenRegister dest, int flag, int subFlag) {
    p->push();
    p->curr.execWidth = 1;
    p->MOV(dest, GenRegister::flag(flag, subFlag));
    p->pop();
  }

  void GenContext::UnsignedI64ToFloat(GenRegister dst, GenRegister high, GenRegister low, GenRegister tmp) {
    p->MOV(dst, high);
    p->MUL(dst, dst, GenRegister::immf(65536.f * 65536.f));
    tmp.type = GEN_TYPE_F;
    p->MOV(tmp, low);
    p->ADD(dst, dst, tmp);
  }

  void GenContext::emitI64ToFloatInstruction(const SelectionInstruction &insn) {
    GenRegister src = ra->genReg(insn.src(0));
    GenRegister dest = ra->genReg(insn.dst(0));
    GenRegister high = ra->genReg(insn.dst(1));
    GenRegister low = ra->genReg(insn.dst(2));
    GenRegister tmp = ra->genReg(insn.dst(3));
    loadTopHalf(high, src);
    loadBottomHalf(low, src);
    if(!src.is_signed_int()) {
      UnsignedI64ToFloat(dest, high, low, tmp);
    } else {
      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.physicalFlag = 1;
      p->curr.flag = 1;
      p->curr.subFlag = 0;
      p->CMP(GEN_CONDITIONAL_GE, high, GenRegister::immud(0x80000000));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->NOT(high, high);
      p->NOT(low, low);
      p->MOV(tmp, GenRegister::immud(1));
      addWithCarry(low, low, tmp);
      p->ADD(high, high, tmp);
      p->pop();
      UnsignedI64ToFloat(dest, high, low, tmp);
      p->push();
      p->curr.physicalFlag = 1;
      p->curr.flag = 1;
      p->curr.subFlag = 0;
      dest.type = GEN_TYPE_UD;
      p->OR(dest, dest, GenRegister::immud(0x80000000));
      p->pop();
    }
  }

  void GenContext::emitI64CompareInstruction(const SelectionInstruction &insn) {
    GenRegister src0 = ra->genReg(insn.src(0));
    GenRegister src1 = ra->genReg(insn.src(1));
    GenRegister tmp0 = ra->genReg(insn.dst(0));
    GenRegister tmp1 = ra->genReg(insn.dst(1));
    GenRegister tmp2 = ra->genReg(insn.dst(2));
    tmp0.type = (src0.type == GEN_TYPE_L) ? GEN_TYPE_D : GEN_TYPE_UD;
    tmp1.type = (src1.type == GEN_TYPE_L) ? GEN_TYPE_D : GEN_TYPE_UD;
    int flag = p->curr.flag, subFlag = p->curr.subFlag;
    GenRegister f1 = GenRegister::retype(tmp2, GEN_TYPE_UW),
                f2 = GenRegister::suboffset(f1, 1),
                f3 = GenRegister::suboffset(f1, 2),
                f4 = GenRegister::suboffset(f1, 3);
    p->push();
    p->curr.predicate = GEN_PREDICATE_NONE;
    saveFlag(f4, flag, subFlag);
    loadTopHalf(tmp0, src0);
    loadTopHalf(tmp1, src1);
    switch(insn.extra.function) {
      case GEN_CONDITIONAL_L:
      case GEN_CONDITIONAL_LE:
      case GEN_CONDITIONAL_G:
      case GEN_CONDITIONAL_GE:
        {
          int cmpTopHalf = insn.extra.function;
          if(insn.extra.function == GEN_CONDITIONAL_LE)
            cmpTopHalf = GEN_CONDITIONAL_L;
          if(insn.extra.function == GEN_CONDITIONAL_GE)
            cmpTopHalf = GEN_CONDITIONAL_G;
          p->CMP(cmpTopHalf, tmp0, tmp1);
        }
        saveFlag(f1, flag, subFlag);
        p->CMP(GEN_CONDITIONAL_EQ, tmp0, tmp1);
        saveFlag(f2, flag, subFlag);
        tmp0.type = tmp1.type = GEN_TYPE_UD;
        loadBottomHalf(tmp0, src0);
        loadBottomHalf(tmp1, src1);
        p->CMP(insn.extra.function, tmp0, tmp1);
        saveFlag(f3, flag, subFlag);
        p->push();
        p->curr.execWidth = 1;
        p->AND(f2, f2, f3);
        p->OR(f1, f1, f2);
        p->pop();
        break;
      case GEN_CONDITIONAL_EQ:
        p->CMP(GEN_CONDITIONAL_EQ, tmp0, tmp1);
        saveFlag(f1, flag, subFlag);
        tmp0.type = tmp1.type = GEN_TYPE_UD;
        loadBottomHalf(tmp0, src0);
        loadBottomHalf(tmp1, src1);
        p->CMP(GEN_CONDITIONAL_EQ, tmp0, tmp1);
        saveFlag(f2, flag, subFlag);
        p->push();
        p->curr.execWidth = 1;
        p->AND(f1, f1, f2);
        p->pop();
        break;
      case GEN_CONDITIONAL_NEQ:
        p->CMP(GEN_CONDITIONAL_NEQ, tmp0, tmp1);
        saveFlag(f1, flag, subFlag);
        tmp0.type = tmp1.type = GEN_TYPE_UD;
        loadBottomHalf(tmp0, src0);
        loadBottomHalf(tmp1, src1);
        p->CMP(GEN_CONDITIONAL_NEQ, tmp0, tmp1);
        saveFlag(f2, flag, subFlag);
        p->push();
        p->curr.execWidth = 1;
        p->OR(f1, f1, f2);
        p->pop();
        break;
      default:
        NOT_IMPLEMENTED;
    }
    p->curr.execWidth = 1;
    p->AND(f1, f1, f4);
    p->MOV(GenRegister::flag(flag, subFlag), f1);
    p->pop();
  }

  void GenContext::loadTopHalf(GenRegister dest, GenRegister src) {
    int execWidth = p->curr.execWidth;
    src = src.top_half();
    p->push();
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.execWidth = 8;
    p->MOV(dest, src);
    p->MOV(GenRegister::suboffset(dest, 4), GenRegister::suboffset(src, 8));
    if (execWidth == 16) {
      p->MOV(GenRegister::suboffset(dest, 8), GenRegister::suboffset(src, 16));
      p->MOV(GenRegister::suboffset(dest, 12), GenRegister::suboffset(src, 24));
    }
    p->pop();
  }

  void GenContext::storeTopHalf(GenRegister dest, GenRegister src) {
    int execWidth = p->curr.execWidth;
    dest = dest.top_half();
    p->push();
    p->curr.execWidth = 8;
    p->MOV(dest, src);
    p->curr.nibControl = 1;
    p->MOV(GenRegister::suboffset(dest, 8), GenRegister::suboffset(src, 4));
    if (execWidth == 16) {
      p->curr.quarterControl = 1;
      p->curr.nibControl = 0;
      p->MOV(GenRegister::suboffset(dest, 16), GenRegister::suboffset(src, 8));
      p->curr.nibControl = 1;
      p->MOV(GenRegister::suboffset(dest, 24), GenRegister::suboffset(src, 12));
    }
    p->pop();
  }

  void GenContext::loadBottomHalf(GenRegister dest, GenRegister src) {
    int execWidth = p->curr.execWidth;
    src = src.bottom_half();
    p->push();
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.execWidth = 8;
    p->MOV(dest, src);
    p->MOV(GenRegister::suboffset(dest, 4), GenRegister::suboffset(src, 8));
    if (execWidth == 16) {
      p->MOV(GenRegister::suboffset(dest, 8), GenRegister::suboffset(src, 16));
      p->MOV(GenRegister::suboffset(dest, 12), GenRegister::suboffset(src, 24));
    }
    p->pop();
  }

  void GenContext::storeBottomHalf(GenRegister dest, GenRegister src) {
    int execWidth = p->curr.execWidth;
    dest = dest.bottom_half();
    p->push();
    p->curr.execWidth = 8;
    p->MOV(dest, src);
    p->curr.nibControl = 1;
    p->MOV(GenRegister::suboffset(dest, 8), GenRegister::suboffset(src, 4));
    if (execWidth == 16) {
      p->curr.quarterControl = 1;
      p->curr.nibControl = 0;
      p->MOV(GenRegister::suboffset(dest, 16), GenRegister::suboffset(src, 8));
      p->curr.nibControl = 1;
      p->MOV(GenRegister::suboffset(dest, 24), GenRegister::suboffset(src, 12));
    }
    p->pop();
  }

  void GenContext::addWithCarry(GenRegister dest, GenRegister src0, GenRegister src1) {
    int execWidth = p->curr.execWidth;
    GenRegister acc0 = GenRegister::retype(GenRegister::acc(), GEN_TYPE_D);
    p->push();
    p->curr.execWidth = 8;
    p->ADDC(dest, src0, src1);
    p->MOV(src1, acc0);
    if (execWidth == 16) {
      p->curr.quarterControl = 1;
      p->ADDC(GenRegister::suboffset(dest, 8),
              GenRegister::suboffset(src0, 8),
              GenRegister::suboffset(src1, 8));
      p->MOV(GenRegister::suboffset(src1, 8), acc0);
    }
    p->pop();
  }

  void GenContext::subWithBorrow(GenRegister dest, GenRegister src0, GenRegister src1) {
    int execWidth = p->curr.execWidth;
    GenRegister acc0 = GenRegister::retype(GenRegister::acc(), GEN_TYPE_D);
    p->push();
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.execWidth = 8;
    p->SUBB(dest, src0, src1);
    p->MOV(src1, acc0);
    if (execWidth == 16) {
      p->SUBB(GenRegister::suboffset(dest, 8),
              GenRegister::suboffset(src0, 8),
              GenRegister::suboffset(src1, 8));
      p->MOV(GenRegister::suboffset(src1, 8), acc0);
    }
    p->pop();
  }

  void GenContext::I32FullMult(GenRegister high, GenRegister low, GenRegister src0, GenRegister src1) {
    GenRegister acc = GenRegister::retype(GenRegister::acc(), GEN_TYPE_UD);
    int execWidth = p->curr.execWidth;
    p->push();
    p->curr.execWidth = 8;
    for(int i = 0; i < execWidth; i += 8) {
      p->MUL(acc, src0, src1);
      p->curr.accWrEnable = 1;
      p->MACH(high, src0, src1);
      p->curr.accWrEnable = 0;
      p->MOV(low, acc);
      src0 = GenRegister::suboffset(src0, 8);
      src1 = GenRegister::suboffset(src1, 8);
      high = GenRegister::suboffset(high, 8);
      low = GenRegister::suboffset(low, 8);
    }
    p->pop();
  }

  void GenContext::emitI64MULInstruction(const SelectionInstruction &insn) {
    GenRegister dest = ra->genReg(insn.dst(0));
    GenRegister x = ra->genReg(insn.src(0));
    GenRegister y = ra->genReg(insn.src(1));
    GenRegister a = ra->genReg(insn.dst(1));
    GenRegister b = ra->genReg(insn.dst(2));
    GenRegister c = ra->genReg(insn.dst(3));
    GenRegister d = ra->genReg(insn.dst(4));
    GenRegister e = ra->genReg(insn.dst(5));
    GenRegister f = ra->genReg(insn.dst(6));
    a.type = b.type = c.type = d.type = e.type = f.type = GEN_TYPE_UD;
    loadTopHalf(a, x);
    loadBottomHalf(b, x);
    loadTopHalf(c, y);
    loadBottomHalf(d, y);
    p->push();
    p->curr.predicate = GEN_PREDICATE_NONE;
    I32FullMult(GenRegister::null(), e, b, c);
    I32FullMult(GenRegister::null(), f, a, d);
    p->ADD(e, e, f);
    I32FullMult(f, a, b, d);
    p->ADD(e, e, f);
    p->pop();
    storeTopHalf(dest, e);
    storeBottomHalf(dest, a);
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
    p->WAIT();
  }

  void GenContext::emitBarrierInstruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.src(0));
    p->BARRIER(src);
  }

  void GenContext::emitFenceInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    p->FENCE(dst);
    p->MOV(dst, dst);
  }

  void GenContext::emitMathInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src0 = ra->genReg(insn.src(0));
    const uint32_t function = insn.extra.function;
    if (insn.srcNum == 2) {
      const GenRegister src1 = ra->genReg(insn.src(1));
      p->MATH(dst, function, src0, src1);
    } else
      p->MATH(dst, function, src0);
  }

  void GenContext::emitCompareInstruction(const SelectionInstruction &insn) {
    const GenRegister src0 = ra->genReg(insn.src(0));
    const GenRegister src1 = ra->genReg(insn.src(1));
    if (insn.opcode == SEL_OP_CMP)
      p->CMP(insn.extra.function, src0, src1);
    else {
      GBE_ASSERT(insn.opcode == SEL_OP_SEL_CMP);
      const GenRegister dst = ra->genReg(insn.dst(0));
      p->SEL_CMP(insn.extra.function, dst, src0, src1);
    }
  }

  void GenContext::emitAtomicInstruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.src(0));
    const GenRegister dst = ra->genReg(insn.dst(0));
    const uint32_t function = insn.extra.function;
    const uint32_t bti = insn.extra.elem;

    p->ATOMIC(dst, function, src, bti, insn.srcNum);
  }

  void GenContext::emitIndirectMoveInstruction(const SelectionInstruction &insn) {
    GenRegister src = ra->genReg(insn.src(0));
    if(isScalarReg(src.reg()))
      src = GenRegister::retype(src, GEN_TYPE_UW);
    else
      src = GenRegister::unpacked_uw(src.nr, src.subnr / typeSize(GEN_TYPE_UW));

    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister a0 = GenRegister::addr8(0);
    uint32_t simdWidth = p->curr.execWidth;

    p->push();
      p->curr.execWidth = 8;
      p->curr.quarterControl = GEN_COMPRESSION_Q1;
      p->MOV(a0, src);
      p->MOV(dst, GenRegister::indirect(dst.type, 0, GEN_WIDTH_8));
    p->pop();

    if (simdWidth == 16) {
      p->push();
        p->curr.execWidth = 8;
        p->curr.quarterControl = GEN_COMPRESSION_Q2;

        const GenRegister nextDst = GenRegister::Qn(dst, 1);
        const GenRegister nextSrc = GenRegister::Qn(src, 1);
        p->MOV(a0, nextSrc);
        p->MOV(nextDst, GenRegister::indirect(dst.type, 0, GEN_WIDTH_8));
      p->pop();
    }
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
      p->curr.noMask = 1;
      p->MOV(GenRegister::ud8grf(112, 0), GenRegister::ud8grf(0, 0));
      p->curr.execWidth = 8;
      p->EOT(112);
    p->pop();
  }

  void GenContext::emitSpillRegInstruction(const SelectionInstruction &insn) {
    uint32_t simdWidth = p->curr.execWidth;
    uint32_t scratchOffset = insn.extra.scratchOffset;
    const uint32_t header = insn.extra.scratchMsgHeader;
    p->push();

    const GenRegister msg = GenRegister::ud8grf(header, 0);
    const GenRegister src = ra->genReg(insn.src(0));
    GenRegister payload = src;
    payload.nr = header + 1;
    payload.subnr = 0;

    p->MOV(payload, src);
    uint32_t regType = insn.src(0).type;
    uint32_t size = typeSize(regType);
    assert(size <= 4);
    uint32_t regNum = (stride(src.hstride)*size*simdWidth) > 32 ? 2 : 1;
    this->scratchWrite(msg, scratchOffset, regNum, regType, GEN_SCRATCH_CHANNEL_MODE_DWORD);
    p->pop();
  }

  void GenContext::emitUnSpillRegInstruction(const SelectionInstruction &insn) {
    uint32_t scratchOffset = insn.extra.scratchOffset;
    const GenRegister dst = insn.dst(0);
    uint32_t regType = dst.type;
    uint32_t simdWidth = p->curr.execWidth;
    const uint32_t header = insn.extra.scratchMsgHeader;
    uint32_t size = typeSize(regType);
    assert(size <= 4);
    uint32_t regNum = (stride(dst.hstride)*size*simdWidth) > 32 ? 2 : 1;
    const GenRegister msg = GenRegister::ud8grf(header, 0);
    this->scratchRead(GenRegister::retype(dst, GEN_TYPE_UD), msg, scratchOffset, regNum, regType, GEN_SCRATCH_CHANNEL_MODE_DWORD);
  }

  //  For SIMD8, we allocate 2*elemNum temporary registers from dst(0), and
  //  then follow the real destination registers.
  //  For SIMD16, we allocate elemNum temporary registers from dst(0).
  void GenContext::emitRead64Instruction(const SelectionInstruction &insn) {
    const uint32_t elemNum = insn.extra.elem;
    const uint32_t tmpRegSize = (p->curr.execWidth == 8) ? elemNum * 2 : elemNum;
    const GenRegister tempAddr = ra->genReg(insn.dst(0));
    const GenRegister dst = ra->genReg(insn.dst(tmpRegSize + 1));
    const GenRegister tmp = ra->genReg(insn.dst(1));
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t bti = insn.extra.function;
    p->READ64(dst, tmp, tempAddr, src, bti, elemNum);
  }

  void GenContext::emitUntypedReadInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t bti = insn.extra.function;
    const uint32_t elemNum = insn.extra.elem;
    p->UNTYPED_READ(dst, src, bti, elemNum);
  }

  //  For SIMD8, we allocate 2*elemNum temporary registers from dst(0), and
  //  then follow the real destination registers.
  //  For SIMD16, we allocate elemNum temporary registers from dst(0).
  void GenContext::emitWrite64Instruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.dst(0));
    const uint32_t elemNum = insn.extra.elem;
    const GenRegister addr = ra->genReg(insn.src(0)); //tmpRegSize + 1));
    const GenRegister data = ra->genReg(insn.src(1));
    const uint32_t bti = insn.extra.function;
    p->MOV(src, addr);
    p->WRITE64(src, data, bti, elemNum);
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

  void GenContext::emitDWordGatherInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t bti = insn.extra.function;
    p->DWORD_GATHER(dst, src, bti);
  }

  void GenContext::emitSampleInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister msgPayload = GenRegister::retype(ra->genReg(insn.src(0)), GEN_TYPE_F);
    const unsigned char bti = insn.extra.function;
    const unsigned char sampler = insn.extra.elem;
    const GenRegister ucoord = ra->genReg(insn.src(4));
    const GenRegister vcoord = ra->genReg(insn.src(5));
    const GenRegister wcoord = ra->genReg(insn.src(6));
    uint32_t simdWidth = p->curr.execWidth;
    p->push();
    const uint32_t nr = msgPayload.nr;
    // prepare mesg desc and move to a0.0.
    // desc = bti | (sampler << 8) | (0 << 12) | (2 << 16) | (0 << 18) | (0 << 19) | (4 << 20) | (1 << 25) | (0 < 29) | (0 << 31)
    /* Prepare message payload. */
    p->MOV(GenRegister::f8grf(nr , 0), ucoord);
    p->MOV(GenRegister::f8grf(nr + (simdWidth/8), 0), vcoord);
    if (insn.src(8).reg() != 0)
      p->MOV(GenRegister::f8grf(nr + (simdWidth/4), 0), wcoord);
    p->SAMPLE(dst, msgPayload, false, bti, sampler, simdWidth, -1, 0);
    p->pop();
  }

  void GenContext::scratchWrite(const GenRegister header, uint32_t offset, uint32_t reg_num, uint32_t reg_type, uint32_t channel_mode) {
    p->push();
    uint32_t simdWidth = p->curr.execWidth;
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.noMask = 1;

    p->curr.execWidth = 8;
    p->MOV(header, GenRegister::ud8grf(0,0));
    p->pop();

    int size = typeSize(reg_type)*simdWidth;
    p->push();
    p->SCRATCH_WRITE(header, offset/32, size, reg_num, channel_mode);
    p->pop();
  }

  void GenContext::scratchRead(const GenRegister dst, const GenRegister header, uint32_t offset, uint32_t reg_num, uint32_t reg_type, uint32_t channel_mode) {
    p->push();
    uint32_t simdWidth = p->curr.execWidth;
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.noMask = 1;
    p->curr.execWidth = 8;
    p->MOV(header, GenRegister::ud8grf(0,0));
    p->pop();

    int size = typeSize(reg_type)*simdWidth;
    p->push();
    p->SCRATCH_READ(dst, header, offset/32, size, reg_num, channel_mode);
    p->pop();
  }

  void GenContext::emitTypedWriteInstruction(const SelectionInstruction &insn) {
    const GenRegister header = GenRegister::retype(ra->genReg(insn.src(0)), GEN_TYPE_UD);
    const GenRegister ucoord = ra->genReg(insn.src(insn.extra.elem));
    const GenRegister vcoord = ra->genReg(insn.src(1 + insn.extra.elem));
    const GenRegister wcoord = ra->genReg(insn.src(2 + insn.extra.elem));
    const GenRegister R = ra->genReg(insn.src(3 + insn.extra.elem));
    const GenRegister G = ra->genReg(insn.src(4 + insn.extra.elem));
    const GenRegister B = ra->genReg(insn.src(5 + insn.extra.elem));
    const GenRegister A = ra->genReg(insn.src(6 + insn.extra.elem));
    const unsigned char bti = insn.extra.function;

    p->push();
    uint32_t simdWidth = p->curr.execWidth;
    const uint32_t nr = header.nr;
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.noMask = 1;
    p->MOV(header, GenRegister::immud(0x0));
    p->curr.execWidth = 1;

    // prepare mesg desc and move to a0.0.
    // desc = bti | (msg_type << 14) | (header_present << 19))
    // prepare header, we need to enable all the 8 planes.
    p->MOV(GenRegister::ud8grf(nr, 7), GenRegister::immud(0xffff));
    p->curr.execWidth = 8;
    // Typed write only support SIMD8.
    // Prepare message payload U + V + R(ignored) + LOD(0) + RGBA.
    // Currently, we don't support non-zero lod, so we clear all lod to
    // zero for both quarters thus save one instruction here.
    // Thus we must put this instruction in noMask and no predication state.
    p->MOV(GenRegister::ud8grf(nr + 4, 0), GenRegister::immud(0)); //LOD
    p->pop();
    p->push();
    p->curr.execWidth = 8;
    // TYPED WRITE send instruction only support SIMD8, if we are SIMD16, we
    // need to call it twice.
    uint32_t quarterNum = (simdWidth == 8) ? 1 : 2;

    for( uint32_t quarter = 0; quarter < quarterNum; quarter++)
    {
#define QUARTER_MOV0(dst_nr, src) p->MOV(GenRegister::ud8grf(dst_nr, 0), \
                                        GenRegister::retype(GenRegister::QnPhysical(src, quarter), src.type))
#define QUARTER_MOV1(dst_nr, src) p->MOV(GenRegister::retype(GenRegister::ud8grf(dst_nr, 0), src.type), \
                                        GenRegister::retype(GenRegister::QnPhysical(src,quarter), src.type))
      if (quarter == 1)
        p->curr.quarterControl = GEN_COMPRESSION_Q2;
      QUARTER_MOV0(nr + 1, ucoord);
      QUARTER_MOV0(nr + 2, vcoord);
      if (insn.src(3 + insn.extra.elem).reg() != 0)
        QUARTER_MOV0(nr + 3, wcoord);
      QUARTER_MOV1(nr + 5, R);
      QUARTER_MOV1(nr + 6, G);
      QUARTER_MOV1(nr + 7, B);
      QUARTER_MOV1(nr + 8, A);
#undef QUARTER_MOV
      p->TYPED_WRITE(header, true, bti);
    }
    p->pop();
  }

  void GenContext::emitGetImageInfoInstruction(const SelectionInstruction &insn) {
    const unsigned char bti = insn.extra.function;
    const unsigned char type = insn.extra.elem;
    const uint32_t dstNum = ir::GetImageInfoInstruction::getDstNum4Type(type);
    ir::ImageInfoKey key;
    key.index = bti;
    key.type = type;

    uint32_t offset = this->getImageInfoCurbeOffset(key, dstNum * 4) + GEN_REG_SIZE;
    for(uint32_t i = 0; i < dstNum; i++) {
      const uint32_t nr = offset / GEN_REG_SIZE;
      const uint32_t subnr = (offset % GEN_REG_SIZE) / sizeof(uint32_t);
      p->MOV(ra->genReg(insn.dst(i)), GenRegister::ud1grf(nr, subnr));
      offset += 32;
    }
  }

  BVAR(OCL_OUTPUT_REG_ALLOC, false);
  BVAR(OCL_OUTPUT_ASM, false);
  bool GenContext::emitCode(void) {
    GenKernel *genKernel = static_cast<GenKernel*>(this->kernel);
    sel->select();
    schedulePreRegAllocation(*this, *this->sel);
    if (UNLIKELY(ra->allocate(*this->sel) == false))
      return false;
    schedulePostRegAllocation(*this, *this->sel);
    if (OCL_OUTPUT_REG_ALLOC)
      ra->outputAllocation();
    this->clearFlagRegister();
    this->emitStackPointer();
    this->emitInstructionStream();
    this->patchBranches();
    genKernel->insnNum = p->store.size();
    genKernel->insns = GBE_NEW_ARRAY_NO_ARG(GenInstruction, genKernel->insnNum);
    std::memcpy(genKernel->insns, &p->store[0], genKernel->insnNum * sizeof(GenInstruction));
    if (OCL_OUTPUT_ASM)
      for (uint32_t insnID = 0; insnID < genKernel->insnNum; ++insnID)
        gen_disasm(stdout, &p->store[insnID]);
    return true;
  }

  Kernel *GenContext::allocateKernel(void) {
    return GBE_NEW(GenKernel, name);
  }

} /* namespace gbe */


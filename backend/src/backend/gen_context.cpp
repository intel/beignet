/*
 * Copyright © 2012 Intel Corporatin
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
#include "ir/value.hpp"
#include "sys/cvar.hpp"
#include <cstring>
#include <iostream>
#include <iomanip>

namespace gbe
{
  ///////////////////////////////////////////////////////////////////////////
  // GenContext implementation
  ///////////////////////////////////////////////////////////////////////////
  GenContext::GenContext(const ir::Unit &unit, const std::string &name, uint32_t deviceID,
	     bool relaxMath) :
    Context(unit, name), deviceID(deviceID), relaxMath(relaxMath)
  {
    this->p = NULL;
    this->sel = NULL;
    this->ra = NULL;
    this->ifEndifFix = false;
    this->regSpillTick = 0;
  }

  GenContext::~GenContext(void) {
    GBE_DELETE(this->ra);
    GBE_DELETE(this->sel);
    GBE_DELETE(this->p);
  }

  void GenContext::startNewCG(uint32_t simdWidth, uint32_t reservedSpillRegs, bool limitRegisterPressure) {
    this->limitRegisterPressure = limitRegisterPressure;
    this->reservedSpillRegs = reservedSpillRegs;
    Context::startNewCG(simdWidth);
    GBE_SAFE_DELETE(ra);
    GBE_SAFE_DELETE(sel);
    GBE_SAFE_DELETE(p);
    this->p = generateEncoder();
    this->newSelection();
    this->ra = GBE_NEW(GenRegAllocator, *this);
    this->branchPos2.clear();
    this->branchPos3.clear();
    this->labelPos.clear();
    this->errCode = NO_ERROR;
    this->regSpillTick = 0;
  }

  void GenContext::newSelection(void) {
    this->sel = GBE_NEW(Selection, *this);
  }

  uint32_t GenContext::alignScratchSize(uint32_t size){
    uint32_t i = 0;
    while(i < size) i+=1024;
    return i;
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

  bool GenContext::patchBranches(void) {
    using namespace ir;
    for (auto pair : branchPos2) {
      const LabelIndex label = pair.first;
      const int32_t insnID = pair.second;
      const int32_t targetID = labelPos.find(label)->second;
      p->patchJMPI(insnID, (targetID - insnID), 0);
    }
    for (auto pair : branchPos3) {
      const LabelPair labelPair = pair.first;
      const int32_t insnID = pair.second;
      const int32_t jip = labelPos.find(labelPair.l0)->second;
      const int32_t uip = labelPos.find(labelPair.l1)->second;
      if (((jip - insnID) > 32767 || (jip - insnID) < -32768) ||
          ((uip - insnID) > 32768 || (uip - insnID) < -32768)) {
        // The only possible error instruction is if/endif here.
        errCode = OUT_OF_RANGE_IF_ENDIF; 
        return false;
      }
      p->patchJMPI(insnID, jip - insnID, uip - insnID);
    }
    return true;
  }

  void GenContext::clearFlagRegister(void) {
    // when group size not aligned to simdWidth, flag register need clear to
    // make prediction(any8/16h) work correctly
    const GenRegister blockip = ra->genReg(GenRegister::uw8grf(ir::ocl::blockip));
    const GenRegister zero = ra->genReg(GenRegister::uw1grf(ir::ocl::zero));
    const GenRegister one = ra->genReg(GenRegister::uw1grf(ir::ocl::one));
    p->push();
      p->curr.noMask = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->MOV(blockip, GenRegister::immuw(GEN_MAX_LABEL));
      p->curr.noMask = 0;
      p->MOV(blockip, GenRegister::immuw(0));
      p->curr.execWidth = 1;
      // FIXME, need to get the final use set of zero/one, if there is no user,
      // no need to generate the following two instructions.
      p->MOV(zero, GenRegister::immuw(0));
      p->MOV(one, GenRegister::immw(-1));
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
    const GenRegister selStackBuffer = GenRegister::ud1grf(ir::ocl::stackbuffer);
    const GenRegister bufferptr = ra->genReg(selStackBuffer);

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
      case SEL_OP_MOV: p->MOV(dst, src, insn.extra.function); break;
      case SEL_OP_READ_ARF: p->MOV(dst, src); break;
      case SEL_OP_FBH: p->FBH(dst, src); break;
      case SEL_OP_FBL: p->FBL(dst, src); break;
      case SEL_OP_CBIT: p->CBIT(dst, src); break;
      case SEL_OP_NOT: p->NOT(dst, src); break;
      case SEL_OP_RNDD: p->RNDD(dst, src); break;
      case SEL_OP_RNDU: p->RNDU(dst, src); break;
      case SEL_OP_RNDE: p->RNDE(dst, src); break;
      case SEL_OP_RNDZ: p->RNDZ(dst, src); break;
      case SEL_OP_F16TO32: p->F16TO32(dst, src); break;
      case SEL_OP_F32TO16: p->F32TO16(dst, src); break;
      case SEL_OP_LOAD_INT64_IMM: p->LOAD_INT64_IMM(dst, src.value.i64); break;
      case SEL_OP_CONVI64_TO_I:
       {
        p->MOV(dst, src.bottom_half());
        break;
       }
      case SEL_OP_BRC:
        {
          const ir::LabelIndex label0(insn.index), label1(insn.index1);
          const LabelPair labelPair(label0, label1);
          const GenRegister src = ra->genReg(insn.src(0));
          this->branchPos3.push_back(std::make_pair(labelPair, p->store.size()));
          p->BRC(src);
        }
        break;
      case SEL_OP_BRD:
        insertJumpPos(insn);
        p->BRD(src);
        break;
      case SEL_OP_ENDIF:
        insertJumpPos(insn);
        p->ENDIF(src);
        break;
      case SEL_OP_IF:
        {
          const ir::LabelIndex label0(insn.index), label1(insn.index1);
          const LabelPair labelPair(label0, label1);
          const GenRegister src = ra->genReg(insn.src(0));
          this->branchPos3.push_back(std::make_pair(labelPair, p->store.size()));
          p->IF(src);
        }
        break;
      case SEL_OP_ELSE:
        {
          insertJumpPos(insn);
          /*
          const ir::LabelIndex label(insn.index), label1(insn.index);
          const LabelPair labelPair(label, label1);
          const GenRegister src = ra->genReg(insn.src(0));
          this->branchPos3.push_back(std::make_pair(labelPair, p->store.size()));*/
          p->ELSE(src);
        }
        break;
      case SEL_OP_WHILE:
        {
          /*const ir::LabelIndex label0(insn.index), label1(insn.index1);
          const LabelPair labelPair(label0, label1);
          const GenRegister src = ra->genReg(insn.src(0));
          this->branchPos3.push_back(std::make_pair(labelPair, p->store.size()));*/
          insertJumpPos(insn);
          p->WHILE(src);
        }
        break;
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
        GenRegister middle = src;
        if(src.type == GEN_TYPE_B || src.type == GEN_TYPE_W) {
          middle = tmp;
          middle.type = GEN_TYPE_D;
          p->MOV(middle, src);
        }

        p->MOV(dst.bottom_half(), middle);
        if(src.is_signed_int())
          p->ASR(dst.top_half(this->simdWidth), middle, GenRegister::immud(31));
        else
          p->MOV(dst.top_half(this->simdWidth), GenRegister::immud(0));
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
        tmp = GenRegister::retype(tmp, GEN_TYPE_UL);
        GenRegister x = tmp.bottom_half();
        GenRegister y = tmp.top_half(this->simdWidth);

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
        tmp = GenRegister::retype(tmp, GEN_TYPE_UL);
        GenRegister x = tmp.bottom_half();
        GenRegister y = tmp.top_half(this->simdWidth);

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
          p->curr.noMask = 1;
          p->MUL(GenRegister::retype(GenRegister::acc(), GEN_TYPE_UD), src0,
                     GenRegister::h2(GenRegister::retype(src1, GEN_TYPE_UW)));
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
          p->SEL(dst.bottom_half(), src0.bottom_half(), src1.bottom_half());
          p->SEL(dst.top_half(this->simdWidth), src0.top_half(this->simdWidth), src1.top_half(this->simdWidth));
        }
        break;
      case SEL_OP_AND:  p->AND(dst, src0, src1, insn.extra.function); break;
      case SEL_OP_OR:   p->OR (dst, src0, src1, insn.extra.function);  break;
      case SEL_OP_XOR:  p->XOR(dst, src0, src1, insn.extra.function); break;
      case SEL_OP_I64AND:
        {
          p->AND(dst.bottom_half(), src0.bottom_half(), src1.bottom_half());
          p->AND(dst.top_half(this->simdWidth), src0.top_half(this->simdWidth), src1.top_half(this->simdWidth));
        }
        break;
      case SEL_OP_I64OR:
        {
          p->OR(dst.bottom_half(), src0.bottom_half(), src1.bottom_half());
          p->OR(dst.top_half(this->simdWidth), src0.top_half(this->simdWidth), src1.top_half(this->simdWidth));
        }
        break;
      case SEL_OP_I64XOR:
        {
          p->XOR(dst.bottom_half(), src0.bottom_half(), src1.bottom_half());
          p->XOR(dst.top_half(this->simdWidth), src0.top_half(this->simdWidth), src1.top_half(this->simdWidth));
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
      case SEL_OP_UPSAMPLE_LONG:
        {
          GenRegister xdst = GenRegister::retype(dst, GEN_TYPE_UL),
                      xsrc0 = GenRegister::retype(src0, GEN_TYPE_UL),
                      xsrc1 = GenRegister::retype(src1, GEN_TYPE_UL);
          p->MOV(xdst.top_half(this->simdWidth), xsrc0.bottom_half());
          p->MOV(xdst.bottom_half(), xsrc1.bottom_half());
        }
        break;
      default: NOT_IMPLEMENTED;
    }
  }

  void GenContext::collectShifter(GenRegister dest, GenRegister src) {
    p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
    p->AND(dest, src.bottom_half(), GenRegister::immud(63));
    p->pop();
  }

  void GenContext::I64FullAdd(GenRegister high1, GenRegister low1, GenRegister high2, GenRegister low2) {
    addWithCarry(low1, low1, low2);
    addWithCarry(high1, high1, high2);
    p->ADD(high1, high1, low2);
  }

  void GenContext::I64FullMult(GenRegister dst1, GenRegister dst2, GenRegister dst3, GenRegister dst4, GenRegister x_high, GenRegister x_low, GenRegister y_high, GenRegister y_low) {
    GenRegister &e = dst1, &f = dst2, &g = dst3, &h = dst4,
                &a = x_high, &b = x_low, &c = y_high, &d = y_low;
    I32FullMult(e, h, b, d);
    I32FullMult(f, g, a, d);
    addWithCarry(g, g, e);
    addWithCarry(f, f, e);
    I32FullMult(e, d, b, c);
    I64FullAdd(f, g, e, d);
    I32FullMult(b, d, a, c);
    I64FullAdd(e, f, b, d);
  }

  void GenContext::I64Neg(GenRegister high, GenRegister low, GenRegister tmp) {
    p->NOT(high, high);
    p->NOT(low, low);
    p->MOV(tmp, GenRegister::immud(1));
    addWithCarry(low, low, tmp);
    p->ADD(high, high, tmp);
  }

  void GenContext::I64ABS(GenRegister sign, GenRegister high, GenRegister low, GenRegister tmp, GenRegister flagReg) {
    p->SHR(sign, high, GenRegister::immud(31));
    p->push();
    p->curr.noMask = 1;
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
    p->CMP(GEN_CONDITIONAL_NZ, sign, GenRegister::immud(0));
    p->curr.predicate = GEN_PREDICATE_NORMAL;
    I64Neg(high, low, tmp);
    p->pop();
  }

  void GenContext::emitI64MULHIInstruction(const SelectionInstruction &insn) {
    GenRegister dest = ra->genReg(insn.dst(0));
    GenRegister x = ra->genReg(insn.src(0));
    GenRegister y = ra->genReg(insn.src(1));
    GenRegister a = ra->genReg(insn.dst(1));
    GenRegister b = ra->genReg(insn.dst(2));
    GenRegister c = ra->genReg(insn.dst(3));
    GenRegister d = ra->genReg(insn.dst(4));
    GenRegister e = ra->genReg(insn.dst(5));
    GenRegister f = ra->genReg(insn.dst(6));
    GenRegister g = ra->genReg(insn.dst(7));
    GenRegister h = ra->genReg(insn.dst(8));
    GenRegister i = ra->genReg(insn.dst(9));
    GBE_ASSERT(insn.state.flag == 0 && insn.state.subFlag == 1);
    GenRegister flagReg = GenRegister::flag(insn.state.flag, insn.state.subFlag);
    loadTopHalf(a, x);
    loadBottomHalf(b, x);
    loadTopHalf(c, y);
    loadBottomHalf(d, y);
    if(x.type == GEN_TYPE_UL) {
      I64FullMult(e, f, g, h, a, b, c, d);
    } else {
      I64ABS(e, a, b, i, flagReg);
      I64ABS(f, c, d, i, flagReg);
      p->XOR(i, e, f);
      I64FullMult(e, f, g, h, a, b, c, d);
      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_NZ, i, GenRegister::immud(0));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->NOT(e, e);
      p->NOT(f, f);
      p->NOT(g, g);
      p->NOT(h, h);
      p->MOV(i, GenRegister::immud(1));
      addWithCarry(h, h, i);
      addWithCarry(g, g, i);
      addWithCarry(f, f, i);
      p->ADD(e, e, i);
      p->pop();
    }
    storeTopHalf(dest, e);
    storeBottomHalf(dest, f);
  }

  void GenContext::emitI64MADSATInstruction(const SelectionInstruction &insn) {
    GenRegister dest = ra->genReg(insn.dst(0));
    GenRegister x = ra->genReg(insn.src(0));
    GenRegister y = ra->genReg(insn.src(1));
    GenRegister z = ra->genReg(insn.src(2));
    GenRegister a = ra->genReg(insn.dst(1));
    GenRegister b = ra->genReg(insn.dst(2));
    GenRegister c = ra->genReg(insn.dst(3));
    GenRegister d = ra->genReg(insn.dst(4));
    GenRegister e = ra->genReg(insn.dst(5));
    GenRegister f = ra->genReg(insn.dst(6));
    GenRegister g = ra->genReg(insn.dst(7));
    GenRegister h = ra->genReg(insn.dst(8));
    GenRegister i = ra->genReg(insn.dst(9));
    GBE_ASSERT(insn.state.flag == 0 && insn.state.subFlag == 1);
    GenRegister flagReg = GenRegister::flag(insn.state.flag, insn.state.subFlag);
    GenRegister zero = GenRegister::immud(0), one = GenRegister::immud(1);
    loadTopHalf(a, x);
    loadBottomHalf(b, x);
    loadTopHalf(c, y);
    loadBottomHalf(d, y);
    if(x.type == GEN_TYPE_UL) {
      I64FullMult(e, f, g, h, a, b, c, d);
      loadTopHalf(c, z);
      loadBottomHalf(d, z);
      addWithCarry(h, h, d);
      addWithCarry(g, g, d);
      addWithCarry(f, f, d);
      p->ADD(e, e, d);
      addWithCarry(g, g, c);
      addWithCarry(f, f, c);
      p->ADD(e, e, c);
      p->OR(a, e, f);
      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_NZ, a, zero);
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(g, GenRegister::immd(-1));
      p->MOV(h, GenRegister::immd(-1));
      p->pop();
    } else {
      I64ABS(e, a, b, i, flagReg);
      I64ABS(f, c, d, i, flagReg);
      p->XOR(i, e, f);
      I64FullMult(e, f, g, h, a, b, c, d);
      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_NZ, i, zero);
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->NOT(e, e);
      p->NOT(f, f);
      p->NOT(g, g);
      p->NOT(h, h);
      p->MOV(i, one);
      addWithCarry(h, h, i);
      addWithCarry(g, g, i);
      addWithCarry(f, f, i);
      p->ADD(e, e, i);
      p->pop();
      loadTopHalf(c, z);
      loadBottomHalf(d, z);
      p->ASR(GenRegister::retype(b, GEN_TYPE_D), GenRegister::retype(c, GEN_TYPE_D), GenRegister::immd(31));
      p->MOV(a, b);
      addWithCarry(h, h, d);
      addWithCarry(g, g, d);
      addWithCarry(f, f, d);
      p->ADD(e, e, d);
      addWithCarry(g, g, c);
      addWithCarry(f, f, c);
      p->ADD(e, e, c);
      addWithCarry(f, f, b);
      p->ADD(e, e, b);
      p->ADD(e, e, a);
      p->MOV(b, zero);
      p->push();
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->CMP(GEN_CONDITIONAL_NZ, e, zero);
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(b, one);
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->CMP(GEN_CONDITIONAL_NZ, f, zero);
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(b, one);
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->CMP(GEN_CONDITIONAL_G, g, GenRegister::immud(0x7FFFFFFF));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(b, one);
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->SHR(a, e, GenRegister::immud(31));
      p->CMP(GEN_CONDITIONAL_NZ, a, zero);
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(b, zero);
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->CMP(GEN_CONDITIONAL_NZ, b, zero);
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(g, GenRegister::immud(0x7FFFFFFF));
      p->MOV(h, GenRegister::immud(0xFFFFFFFFu));
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->MOV(b, zero);
      p->CMP(GEN_CONDITIONAL_NEQ, e, GenRegister::immud(0xFFFFFFFFu));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(b, one);
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->CMP(GEN_CONDITIONAL_NEQ, f, GenRegister::immud(0xFFFFFFFFu));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(b, one);
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->CMP(GEN_CONDITIONAL_LE, g, GenRegister::immud(0x7FFFFFFF));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(b, one);
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->CMP(GEN_CONDITIONAL_Z, a, zero);
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(b, zero);
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->CMP(GEN_CONDITIONAL_NZ, b, zero);
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(g, GenRegister::immud(0x80000000u));
      p->MOV(h, zero);
      p->pop();
    }
    storeTopHalf(dest, g);
    storeBottomHalf(dest, h);
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

  void GenContext::emitI64RHADDInstruction(const SelectionInstruction &insn) {
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
    addWithCarry(a, a, b);
    p->MOV(c, GenRegister::immud(1));
    addWithCarry(a, a, c);
    p->ADD(b, b, c);
    loadTopHalf(c, x);
    loadTopHalf(d, y);
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
    GBE_ASSERT(insn.state.flag == 0 && insn.state.subFlag == 1);
    GenRegister flagReg = GenRegister::flag(insn.state.flag, insn.state.subFlag);
    GenRegister zero = GenRegister::immud(0);
    switch(insn.opcode) {
      case SEL_OP_I64SHL:
        p->push();
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->curr.noMask = 1;
        collectShifter(a, y);
        loadBottomHalf(e, x);
        loadTopHalf(f, x);
        p->SHR(b, e, GenRegister::negate(a));
        p->SHL(c, e, a);
        p->SHL(d, f, a);
        p->OR(e, d, b);
        setFlag(flagReg, GenRegister::immuw(0xFFFF));
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
        p->CMP(GEN_CONDITIONAL_Z, a, zero);
        p->SEL(d, d, e);
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->AND(a, a, GenRegister::immud(32));
        setFlag(flagReg, GenRegister::immuw(0xFFFF));
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
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
        p->curr.noMask = 1;
        collectShifter(a, y);
        loadBottomHalf(e, x);
        loadTopHalf(f, x);
        p->SHL(b, f, GenRegister::negate(a));
        p->SHR(c, f, a);
        p->SHR(d, e, a);
        p->OR(e, d, b);
        setFlag(flagReg, GenRegister::immuw(0xFFFF));
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
        p->CMP(GEN_CONDITIONAL_Z, a, zero);
        p->SEL(d, d, e);
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->AND(a, a, GenRegister::immud(32));
        setFlag(flagReg, GenRegister::immuw(0xFFFF));
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
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
        p->curr.noMask = 1;
        collectShifter(a, y);
        loadBottomHalf(e, x);
        loadTopHalf(f, x);
        p->SHL(b, f, GenRegister::negate(a));
        p->ASR(c, f, a);
        p->SHR(d, e, a);
        p->OR(e, d, b);
        setFlag(flagReg, GenRegister::immuw(0xFFFF));
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
        p->CMP(GEN_CONDITIONAL_Z, a, zero);
        p->SEL(d, d, e);
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->AND(a, a, GenRegister::immud(32));
        p->ASR(f, f, GenRegister::immd(31));
        setFlag(flagReg, GenRegister::immuw(0xFFFF));
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
        p->CMP(GEN_CONDITIONAL_Z, a, zero);
        p->SEL(d, d, c);
        p->SEL(c, c, f);
        p->pop();
        storeBottomHalf(dest, d);
        storeTopHalf(dest, c);
        break;
      default:
        NOT_IMPLEMENTED;
    }
  }
  void GenContext::setFlag(GenRegister flagReg, GenRegister src) {
    p->push();
    p->curr.noMask = 1;
    p->curr.execWidth = 1;
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->MOV(flagReg, src);
    p->pop();
  }

  void GenContext::saveFlag(GenRegister dest, int flag, int subFlag) {
    p->push();
    p->curr.execWidth = 1;
    p->MOV(dest, GenRegister::flag(flag, subFlag));
    p->pop();
  }

  void GenContext::UnsignedI64ToFloat(GenRegister dst, GenRegister high, GenRegister low, GenRegister exp,
                                            GenRegister mantissa, GenRegister tmp, GenRegister flag) {
    uint32_t jip0, jip1;
    GenRegister dst_ud = GenRegister::retype(dst, GEN_TYPE_UD);
    p->push();
      p->curr.noMask = 1;
      p->MOV(exp, GenRegister::immud(32)); // make sure the inactive lane is 1 when check ALL8H/ALL16H condition latter.
    p->pop();
    p->FBH(exp, high);
    p->ADD(exp, GenRegister::negate(exp), GenRegister::immud(31));  //exp = 32 when high == 0
    p->push();
      p->curr.useFlag(flag.flag_nr(), flag.flag_subnr());
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->CMP(GEN_CONDITIONAL_EQ, exp, GenRegister::immud(32));   //high == 0
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->curr.noMask = 0;
      p->MOV(dst, low);
      p->push();
        if (simdWidth == 8)
          p->curr.predicate = GEN_PREDICATE_ALIGN1_ALL8H;
        else if (simdWidth == 16)
          p->curr.predicate = GEN_PREDICATE_ALIGN1_ALL16H;
        else
          NOT_IMPLEMENTED;
        p->curr.execWidth = 1;
        p->curr.noMask = 1;
        jip0 = p->n_instruction();
        p->JMPI(GenRegister::immud(0));
      p->pop();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->CMP(GEN_CONDITIONAL_G, exp, GenRegister::immud(23));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->CMP(GEN_CONDITIONAL_L, exp, GenRegister::immud(32));  //exp>23 && high!=0
      p->ADD(tmp, exp, GenRegister::immud(-23));
      p->SHR(mantissa, high, tmp);
      p->AND(mantissa, mantissa, GenRegister::immud(0x7fffff));
      p->SHR(dst_ud, low, tmp);   //dst is temp regitster here
      p->ADD(tmp, GenRegister::negate(tmp), GenRegister::immud(32));
      p->SHL(high, high, tmp);
      p->OR(high, high, dst_ud);
      p->SHL(low, low, tmp);
      p->push();
        if (simdWidth == 8)
          p->curr.predicate = GEN_PREDICATE_ALIGN1_ALL8H;
        else if (simdWidth == 16)
          p->curr.predicate = GEN_PREDICATE_ALIGN1_ALL16H;
        else
          NOT_IMPLEMENTED;
        p->curr.execWidth = 1;
        p->curr.noMask = 1;
        jip1 = p->n_instruction();
        p->JMPI(GenRegister::immud(0));
      p->pop();

      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->CMP(GEN_CONDITIONAL_EQ, exp, GenRegister::immud(23));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(dst_ud, GenRegister::immud(0));   //exp==9, SHR == 0

      p->curr.predicate = GEN_PREDICATE_NONE;
      p->CMP(GEN_CONDITIONAL_L, exp, GenRegister::immud(23));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->ADD(tmp, exp, GenRegister::immud(9));
      p->SHR(dst_ud, low, tmp);   //dst is temp regitster here

      p->curr.predicate = GEN_PREDICATE_NONE;
      p->CMP(GEN_CONDITIONAL_LE, exp, GenRegister::immud(23));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->ADD(tmp, GenRegister::negate(exp), GenRegister::immud(23));
      p->SHL(mantissa, high, tmp);
      p->OR(mantissa, mantissa, dst_ud);
      p->AND(mantissa, mantissa, GenRegister::immud(0x7fffff));
      p->SHL(high, low, tmp);
      p->MOV(low, GenRegister::immud(0));

      p->patchJMPI(jip1, (p->n_instruction() - jip1), 0);
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->CMP(GEN_CONDITIONAL_LE, exp, GenRegister::immud(31));  //update dst where high != 0
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->ADD(exp, exp, GenRegister::immud(159));
      p->SHL(exp, exp, GenRegister::immud(23));
      p->OR(dst_ud, exp, mantissa);

      p->CMP(GEN_CONDITIONAL_GE, high, GenRegister::immud(0x80000000));
      p->ADD(dst_ud, dst_ud, GenRegister::immud(1));

      p->CMP(GEN_CONDITIONAL_EQ, high, GenRegister::immud(0x80000000));
      p->CMP(GEN_CONDITIONAL_EQ, low, GenRegister::immud(0x0));
      p->AND(dst_ud, dst_ud, GenRegister::immud(0xfffffffe));
      p->patchJMPI(jip0, (p->n_instruction() - jip0), 0);

    p->pop();

  }

  void GenContext::emitI64ToFloatInstruction(const SelectionInstruction &insn) {
    GenRegister src = ra->genReg(insn.src(0));
    GenRegister dest = ra->genReg(insn.dst(0));
    GenRegister high = ra->genReg(insn.dst(1));
    GenRegister low = ra->genReg(insn.dst(2));
    GenRegister exp = ra->genReg(insn.dst(3));
    GenRegister mantissa = ra->genReg(insn.dst(4));
    GenRegister tmp = ra->genReg(insn.dst(5));
    GenRegister tmp_high = ra->genReg(insn.dst(6));
    GBE_ASSERT(insn.state.flag == 0 && insn.state.subFlag == 1);
    GenRegister flagReg = GenRegister::flag(insn.state.flag, insn.state.subFlag);
    loadTopHalf(high, src);
    loadBottomHalf(low, src);
    if(!src.is_signed_int()) {
      UnsignedI64ToFloat(dest, high, low, exp, mantissa, tmp, flagReg);
    } else {
      p->MOV(tmp_high, high);
      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_GE, tmp_high, GenRegister::immud(0x80000000));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->NOT(high, high);
      p->NOT(low, low);
      p->MOV(tmp, GenRegister::immud(1));
      addWithCarry(low, low, tmp);
      p->ADD(high, high, tmp);
      p->pop();
      UnsignedI64ToFloat(dest, high, low, exp, mantissa, tmp, flagReg);
      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_GE, tmp_high, GenRegister::immud(0x80000000));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      dest.type = GEN_TYPE_UD;
      p->OR(dest, dest, GenRegister::immud(0x80000000));
      p->pop();
    }
  }


  void GenContext::emitFloatToI64Instruction(const SelectionInstruction &insn) {
    GenRegister src = ra->genReg(insn.src(0));
    GenRegister dst = ra->genReg(insn.dst(0));
    GenRegister high = ra->genReg(insn.dst(1));
    GenRegister tmp = ra->genReg(insn.dst(2));
    GBE_ASSERT(insn.state.flag == 0 && insn.state.subFlag == 1);
    GenRegister flagReg = GenRegister::flag(insn.state.flag, insn.state.subFlag);

    if(dst.is_signed_int())
      high = GenRegister::retype(high, GEN_TYPE_D);
    GenRegister low = GenRegister::retype(tmp, GEN_TYPE_UD);
    float c = (1.f / 65536.f) * (1.f / 65536.f);
    p->MUL(tmp, src, GenRegister::immf(c));
    p->RNDZ(tmp, tmp);
    p->MOV(high, tmp);
    c = 65536.f * 65536.f;
    p->MOV(tmp, high);  //result may not equal to tmp
    //mov float to int/uint is sat, so must sub high*0xffffffff
    p->MUL(tmp, tmp, GenRegister::immf(c));
    p->ADD(tmp, src, GenRegister::negate(tmp));
    p->MOV(low, GenRegister::abs(tmp));
    if(dst.is_signed_int()) {
      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_L, src, GenRegister::immf(0x0));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->CMP(GEN_CONDITIONAL_NEQ, low, GenRegister::immud(0x0));
      p->ADD(high, high, GenRegister::immd(-1));
      p->NOT(low, low);
      p->ADD(low, low, GenRegister::immud(1));
      p->pop();
    }
    storeTopHalf(dst, high);
    storeBottomHalf(dst, low);
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
    GenRegister f1 = GenRegister::retype(tmp2, GEN_TYPE_UW);
                f1.width = GEN_WIDTH_1;
    GenRegister f2 = GenRegister::suboffset(f1, 1);
    GenRegister f3 = GenRegister::suboffset(f1, 2);

    p->push();
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.noMask = 1;
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
    p->MOV(GenRegister::flag(flag, subFlag), f1);
    p->pop();
  }

  void GenContext::emitI64SATADDInstruction(const SelectionInstruction &insn) {
    GenRegister x = ra->genReg(insn.src(0));
    GenRegister y = ra->genReg(insn.src(1));
    GenRegister dst = ra->genReg(insn.dst(0));
    GenRegister a = ra->genReg(insn.dst(1));
    GenRegister b = ra->genReg(insn.dst(2));
    GenRegister c = ra->genReg(insn.dst(3));
    GenRegister d = ra->genReg(insn.dst(4));
    GenRegister e = ra->genReg(insn.dst(5));
    GBE_ASSERT(insn.state.flag == 0 && insn.state.subFlag == 1);
    GenRegister flagReg = GenRegister::flag(insn.state.flag, insn.state.subFlag);
    loadTopHalf(a, x);
    loadBottomHalf(b, x);
    loadTopHalf(c, y);
    loadBottomHalf(d, y);
    if(dst.is_signed_int())
      p->SHR(e, a, GenRegister::immud(31));
    addWithCarry(b, b, d);
    addWithCarry(a, a, d);
    addWithCarry(a, a, c);
    p->ADD(c, c, d);
    p->push();
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.noMask = 1;
    p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
    if(! dst.is_signed_int()) {
      p->CMP(GEN_CONDITIONAL_NZ, c, GenRegister::immud(0));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(a, GenRegister::immud(0xFFFFFFFFu));
      p->MOV(b, GenRegister::immud(0xFFFFFFFFu));
    } else {
      p->CMP(GEN_CONDITIONAL_EQ, e, GenRegister::immud(1));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->CMP(GEN_CONDITIONAL_L, a, GenRegister::immud(0x80000000u));
      p->MOV(a, GenRegister::immud(0x80000000u));
      p->MOV(b, GenRegister::immud(0));
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->CMP(GEN_CONDITIONAL_EQ, e, GenRegister::immud(0));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->CMP(GEN_CONDITIONAL_GE, a, GenRegister::immud(0x80000000u));
      p->MOV(a, GenRegister::immud(0x7FFFFFFFu));
      p->MOV(b, GenRegister::immud(0xFFFFFFFFu));
    }
    p->pop();
    storeTopHalf(dst, a);
    storeBottomHalf(dst, b);
  }

  void GenContext::emitI64SATSUBInstruction(const SelectionInstruction &insn) {
    GenRegister x = ra->genReg(insn.src(0));
    GenRegister y = ra->genReg(insn.src(1));
    GenRegister dst = ra->genReg(insn.dst(0));
    GenRegister a = ra->genReg(insn.dst(1));
    GenRegister b = ra->genReg(insn.dst(2));
    GenRegister c = ra->genReg(insn.dst(3));
    GenRegister d = ra->genReg(insn.dst(4));
    GenRegister e = ra->genReg(insn.dst(5));
    GBE_ASSERT(insn.state.flag == 0 && insn.state.subFlag == 1);
    GenRegister flagReg = GenRegister::flag(insn.state.flag, insn.state.subFlag);
    loadTopHalf(a, x);
    loadBottomHalf(b, x);
    loadTopHalf(c, y);
    loadBottomHalf(d, y);
    if(dst.is_signed_int())
      p->SHR(e, a, GenRegister::immud(31));
    subWithBorrow(b, b, d);
    subWithBorrow(a, a, d);
    subWithBorrow(a, a, c);
    p->ADD(c, c, d);
    p->push();
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.noMask = 1;
    p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
    if(! dst.is_signed_int()) {
      p->CMP(GEN_CONDITIONAL_NZ, c, GenRegister::immud(0));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(a, GenRegister::immud(0));
      p->MOV(b, GenRegister::immud(0));
    } else {
      p->CMP(GEN_CONDITIONAL_EQ, e, GenRegister::immud(1));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->CMP(GEN_CONDITIONAL_L, a, GenRegister::immud(0x80000000u));
      p->MOV(a, GenRegister::immud(0x80000000u));
      p->MOV(b, GenRegister::immud(0));
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->CMP(GEN_CONDITIONAL_EQ, e, GenRegister::immud(0));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->CMP(GEN_CONDITIONAL_GE, a, GenRegister::immud(0x80000000u));
      p->MOV(a, GenRegister::immud(0x7FFFFFFFu));
      p->MOV(b, GenRegister::immud(0xFFFFFFFFu));
    }
    p->pop();
    storeTopHalf(dst, a);
    storeBottomHalf(dst, b);
  }

  void GenContext::loadTopHalf(GenRegister dest, GenRegister src) {
    p->MOV(dest, src.top_half(this->simdWidth));
  }

  void GenContext::storeTopHalf(GenRegister dest, GenRegister src) {
    p->MOV(dest.top_half(this->simdWidth), src);
  }

  void GenContext::loadBottomHalf(GenRegister dest, GenRegister src) {
    p->MOV(dest, src.bottom_half());
  }

  void GenContext::storeBottomHalf(GenRegister dest, GenRegister src) {
    p->MOV(dest.bottom_half(), src);
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
    p->curr.execWidth = 8;
    p->SUBB(dest, src0, src1);
    p->MOV(src1, acc0);
    if (execWidth == 16) {
      p->curr.quarterControl = 1;
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
      p->MUL(acc, src0, GenRegister::h2(GenRegister::retype(src1, GEN_TYPE_UW)));
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
    p->curr.noMask = 1;
    I32FullMult(GenRegister::retype(GenRegister::null(), GEN_TYPE_D), e, b, c);
    I32FullMult(GenRegister::retype(GenRegister::null(), GEN_TYPE_D), f, a, d);
    p->ADD(e, e, f);
    I32FullMult(f, a, b, d);
    p->ADD(e, e, f);
    p->pop();
    storeTopHalf(dest, e);
    storeBottomHalf(dest, a);
  }

  void GenContext::emitI64DIVREMInstruction(const SelectionInstruction &insn) {
    GenRegister dest = ra->genReg(insn.dst(0));
    GenRegister x = ra->genReg(insn.src(0));
    GenRegister y = ra->genReg(insn.src(1));
    GenRegister a = ra->genReg(insn.dst(1));
    GenRegister b = ra->genReg(insn.dst(2));
    GenRegister c = ra->genReg(insn.dst(3));
    GenRegister d = ra->genReg(insn.dst(4));
    GenRegister e = ra->genReg(insn.dst(5));
    GenRegister f = ra->genReg(insn.dst(6));
    GenRegister g = ra->genReg(insn.dst(7));
    GenRegister h = ra->genReg(insn.dst(8));
    GenRegister i = ra->genReg(insn.dst(9));
    GenRegister j = ra->genReg(insn.dst(10));
    GenRegister k = ra->genReg(insn.dst(11));
    GenRegister l = ra->genReg(insn.dst(12));
    GenRegister m = ra->genReg(insn.dst(13));
    GBE_ASSERT(insn.state.flag == 0 && insn.state.subFlag == 1);
    GenRegister flagReg = GenRegister::flag(insn.state.flag, insn.state.subFlag);
    GenRegister zero = GenRegister::immud(0),
                one = GenRegister::immud(1),
                imm31 = GenRegister::immud(31);
    uint32_t jip0;
    // (a,b) <- x
    loadTopHalf(a, x);
    loadBottomHalf(b, x);
    // (c,d) <- y
    loadTopHalf(c, y);
    loadBottomHalf(d, y);
    // k <- sign_of_result
    if(x.is_signed_int()) {
      GBE_ASSERT(y.is_signed_int());
      GBE_ASSERT(dest.is_signed_int());
      I64ABS(k, a, b, e, flagReg);
      I64ABS(l, c, d, e, flagReg);
      if(insn.opcode == SEL_OP_I64DIV)
        p->XOR(k, k, l);
    }
    // (e,f) <- 0
    p->MOV(e, zero);
    p->MOV(f, zero);
    // (g,h) <- 2**63
    p->MOV(g, GenRegister::immud(0x80000000));
    p->MOV(h, zero);
    // (i,j) <- 0
    p->MOV(i, zero);
    p->MOV(j, zero);
    // m <- 0
    p->MOV(m, zero);
    {
      uint32_t loop_start = p->n_instruction();
      // (c,d,e,f) <- (c,d,e,f) / 2
      p->SHR(f, f, one);
      p->SHL(l, e, imm31);
      p->OR(f, f, l);
      p->SHR(e, e, one);
      p->SHL(l, d, imm31);
      p->OR(e, e, l);
      p->SHR(d, d, one);
      p->SHL(l, c, imm31);
      p->OR(d, d, l);
      p->SHR(c, c, one);
      // condition <- (c,d)==0 && (a,b)>=(e,f)
      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->MOV(l, zero);
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_EQ, a, e);
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->CMP(GEN_CONDITIONAL_GE, b, f);
      p->MOV(l, one);
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->CMP(GEN_CONDITIONAL_G, a, e);
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(l, one);
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->CMP(GEN_CONDITIONAL_NEQ, l, zero);
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->CMP(GEN_CONDITIONAL_EQ, c, zero);
      p->CMP(GEN_CONDITIONAL_EQ, d, zero);
      // under condition, (a,b) <- (a,b) - (e,f)
      p->MOV(l, f);
      subWithBorrow(b, b, l);
      subWithBorrow(a, a, l);
      p->MOV(l, e);
      subWithBorrow(a, a, l);
      // under condition, (i,j) <- (i,j) | (g,h)
      p->OR(i, i, g);
      p->OR(j, j, h);
      p->pop();
      // (g,h) /= 2
      p->SHR(h, h, one);
      p->SHL(l, g, imm31);
      p->OR(h, h, l);
      p->SHR(g, g, one);
      // condition: m < 64
      p->ADD(m, m, one);

      p->push();
      p->curr.noMask = 1;
      p->curr.execWidth = 1;
      p->MOV(flagReg, zero);
      p->pop();

      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 0;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_L, m, GenRegister::immud(64));

      p->curr.execWidth = 1;
      p->curr.noMask = 1;
      // under condition, jump back to start point
      if (simdWidth == 8)
        p->curr.predicate = GEN_PREDICATE_ALIGN1_ANY8H;
      else if (simdWidth == 16)
        p->curr.predicate = GEN_PREDICATE_ALIGN1_ANY16H;
      else
        NOT_IMPLEMENTED;
      int distance = -(int)(p->n_instruction() - loop_start );
      p->curr.noMask = 1;
      jip0 = p->n_instruction();
      p->JMPI(zero);
      p->patchJMPI(jip0, distance, 0);
      p->pop();
      // end of loop
    }
    // adjust sign of result
    if(x.is_signed_int()) {
      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_NEQ, k, zero);
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      if(insn.opcode == SEL_OP_I64DIV)
        I64Neg(i, j, l);
      else
        I64Neg(a, b, l);
      p->pop();
    }
    // write dest
    if(insn.opcode == SEL_OP_I64DIV) {
      storeTopHalf(dest, i);
      storeBottomHalf(dest, j);
    } else {
      GBE_ASSERT(insn.opcode == SEL_OP_I64REM);
      storeTopHalf(dest, a);
      storeBottomHalf(dest, b);
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
   p->NOP();
  }

  void GenContext::emitWaitInstruction(const SelectionInstruction &insn) {
    p->WAIT();
  }

  void GenContext::emitBarrierInstruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.src(0));
    const GenRegister fenceDst = ra->genReg(insn.dst(0));
    uint32_t barrierType = insn.extra.barrierType;
    const GenRegister barrierId = ra->genReg(GenRegister::ud1grf(ir::ocl::barrierid));

    if (barrierType == ir::syncGlobalBarrier) {
      p->FENCE(fenceDst);
      p->MOV(fenceDst, fenceDst);
    }
    p->push();
      // As only the payload.2 is used and all the other regions are ignored
      // SIMD8 mode here is safe.
      p->curr.execWidth = 8;
      p->curr.physicalFlag = 0;
      p->curr.noMask = 1;
      // Copy barrier id from r0.
      p->AND(src, barrierId, GenRegister::immud(0x0f000000));
      // A barrier is OK to start the thread synchronization *and* SLM fence
      p->BARRIER(src);
      p->curr.execWidth = 1;
      // Now we wait for the other threads
      p->WAIT();
    p->pop();
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
    const GenRegister dst = ra->genReg(insn.dst(0));
    if (insn.opcode == SEL_OP_CMP)
      p->CMP(insn.extra.function, src0, src1, dst);
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
    const uint32_t bti = insn.getbti();

    p->ATOMIC(dst, function, src, bti, insn.srcNum);
  }

  void GenContext::emitIndirectMoveInstruction(const SelectionInstruction &insn) {
    GenRegister src = ra->genReg(insn.src(0));
    if(sel->isScalarReg(src.reg()))
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

 void GenContext::insertJumpPos(const SelectionInstruction &insn) {
    const ir::LabelIndex label(insn.index);
    this->branchPos2.push_back(std::make_pair(label, p->store.size()));
 }

  void GenContext::emitJumpInstruction(const SelectionInstruction &insn) {
    insertJumpPos(insn);
    const GenRegister src = ra->genReg(insn.src(0));
    p->JMPI(src, insn.extra.longjmp);
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

    GBE_ASSERT(src.subnr == 0);
    uint32_t regType = insn.src(0).type;
    uint32_t size = typeSize(regType);
    uint32_t regSize = stride(src.hstride)*size;

    GBE_ASSERT(regSize == 4 || regSize == 8);
    if(regSize == 4) {
      if (payload.nr != src.nr)
        p->MOV(payload, src);
      uint32_t regNum = (regSize*simdWidth) > 32 ? 2 : 1;
      this->scratchWrite(msg, scratchOffset, regNum, GEN_TYPE_UD, GEN_SCRATCH_CHANNEL_MODE_DWORD);
    }
    else { //size == 8
      payload.type = GEN_TYPE_UD;
      GBE_ASSERT(payload.hstride == GEN_HORIZONTAL_STRIDE_1);
      loadBottomHalf(payload, src);
      uint32_t regNum = (regSize/2*simdWidth) > 32 ? 2 : 1;
      this->scratchWrite(msg, scratchOffset, regNum, GEN_TYPE_UD, GEN_SCRATCH_CHANNEL_MODE_DWORD);
      loadTopHalf(payload, src);
      this->scratchWrite(msg, scratchOffset + 4*simdWidth, regNum, GEN_TYPE_UD, GEN_SCRATCH_CHANNEL_MODE_DWORD);
    }
    p->pop();
  }

  void GenContext::emitUnSpillRegInstruction(const SelectionInstruction &insn) {
    uint32_t scratchOffset = insn.extra.scratchOffset;
    const GenRegister dst = insn.dst(0);
    uint32_t regType = dst.type;
    uint32_t simdWidth = p->curr.execWidth;
    const uint32_t header = insn.extra.scratchMsgHeader;
    uint32_t size = typeSize(regType);
    uint32_t regSize = stride(dst.hstride)*size;

    const GenRegister msg = GenRegister::ud8grf(header, 0);
    GenRegister payload = msg;
    payload.nr = header + 1;

    p->push();
    assert(regSize == 4 || regSize == 8);
    if(regSize == 4) {
      uint32_t regNum = (regSize*simdWidth) > 32 ? 2 : 1;
      this->scratchRead(GenRegister::ud8grf(dst.nr, dst.subnr), msg, scratchOffset, regNum, GEN_TYPE_UD, GEN_SCRATCH_CHANNEL_MODE_DWORD);
    } else {
      uint32_t regNum = (regSize/2*simdWidth) > 32 ? 2 : 1;
      this->scratchRead(payload, msg, scratchOffset, regNum, GEN_TYPE_UD, GEN_SCRATCH_CHANNEL_MODE_DWORD);
      storeBottomHalf(dst, payload);
      this->scratchRead(payload, msg, scratchOffset + 4*simdWidth, regNum, GEN_TYPE_UD, GEN_SCRATCH_CHANNEL_MODE_DWORD);
      storeTopHalf(dst, payload);
    }
    p->pop();
  }

  void GenContext::emitRead64Instruction(const SelectionInstruction &insn) {
    const uint32_t elemNum = insn.extra.elem;
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t bti = insn.getbti();
    p->UNTYPED_READ(dst, src, bti, elemNum*2);
  }

  void GenContext::emitUntypedReadInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t bti = insn.getbti();
    const uint32_t elemNum = insn.extra.elem;
    p->UNTYPED_READ(dst, src, bti, elemNum);
  }

  void GenContext::emitWrite64Instruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.dst(0));
    const uint32_t elemNum = insn.extra.elem;
    const uint32_t bti = insn.getbti();
    p->UNTYPED_WRITE(src, bti, elemNum*2);
  }

  void GenContext::emitUntypedWriteInstruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t bti = insn.getbti();
    const uint32_t elemNum = insn.extra.elem;
    p->UNTYPED_WRITE(src, bti, elemNum);
  }

  void GenContext::emitByteGatherInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t bti = insn.getbti();
    const uint32_t elemSize = insn.extra.elem;
    p->BYTE_GATHER(dst, src, bti, elemSize);
  }

  void GenContext::emitByteScatterInstruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t bti = insn.getbti();
    const uint32_t elemSize = insn.extra.elem;
    p->BYTE_SCATTER(src, bti, elemSize);
  }

  void GenContext::emitUnpackByteInstruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.src(0));
    for(uint32_t i = 0; i < insn.dstNum; i++) {
      p->MOV(ra->genReg(insn.dst(i)), GenRegister::splitReg(src, insn.extra.elem, i));
    }
  }

  void GenContext::emitPackByteInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    p->push();
    if(simdWidth == 8) {
      for(uint32_t i = 0; i < insn.srcNum; i++)
        p->MOV(GenRegister::splitReg(dst, insn.extra.elem, i), ra->genReg(insn.src(i)));
    } else {
      // when destination expands two registers, the source must span two registers.
      p->curr.execWidth = 8;
      for(uint32_t i = 0; i < insn.srcNum; i++) {
        GenRegister dsti = GenRegister::splitReg(dst, insn.extra.elem, i);
        GenRegister src = ra->genReg(insn.src(i));

        p->curr.quarterControl = 0;
        p->MOV(dsti, src);
        p->curr.quarterControl = 1;
        p->MOV(GenRegister::Qn(dsti,1), GenRegister::Qn(src, 1));
      }
    }
    p->pop();
  }

  void GenContext::emitDWordGatherInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t bti = insn.getbti();
    p->DWORD_GATHER(dst, src, bti);
  }

  void GenContext::emitSampleInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister msgPayload = GenRegister::retype(ra->genReg(insn.src(0)), GEN_TYPE_F);
    const unsigned char bti = insn.getbti();
    const unsigned char sampler = insn.extra.sampler;
    const unsigned int msgLen = insn.extra.rdmsglen;
    uint32_t simdWidth = p->curr.execWidth;
    p->SAMPLE(dst, msgPayload, msgLen, false, bti, sampler, simdWidth, -1, 0, insn.extra.isLD, insn.extra.isUniform);
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
    const uint32_t bti = insn.getbti();
    p->TYPED_WRITE(header, true, bti);
  }

  BVAR(OCL_OUTPUT_REG_ALLOC, false);
  BVAR(OCL_OUTPUT_ASM, false);

  void GenContext::allocCurbeReg(ir::Register reg, gbe_curbe_type value, uint32_t subValue) {
    uint32_t regSize;
    regSize = this->ra->getRegSize(reg);
    insertCurbeReg(reg, newCurbeEntry(value, subValue, regSize));
  }

  void GenContext::buildPatchList(void) {
    const uint32_t ptrSize = this->getPointerSize();
    kernel->curbeSize = 0u;
    auto &stackUse = dag->getUse(ir::ocl::stackptr);

    // We insert the block IP mask first
    using namespace ir::ocl;
    allocCurbeReg(blockip, GBE_CURBE_BLOCK_IP);
    allocCurbeReg(lid0, GBE_CURBE_LOCAL_ID_X);
    allocCurbeReg(lid1, GBE_CURBE_LOCAL_ID_Y);
    allocCurbeReg(lid2, GBE_CURBE_LOCAL_ID_Z);
    allocCurbeReg(zero, GBE_CURBE_ZERO);
    allocCurbeReg(one, GBE_CURBE_ONE);
    if (stackUse.size() != 0)
      allocCurbeReg(stackbuffer, GBE_CURBE_EXTRA_ARGUMENT, GBE_STACK_BUFFER);
    allocSLMOffsetCurbe();
    // Go over the arguments and find the related patch locations
    const uint32_t argNum = fn.argNum();
    for (uint32_t argID = 0u; argID < argNum; ++argID) {
      const ir::FunctionArgument &arg = fn.getArg(argID);
      // For pointers and values, we have nothing to do. We just push the values
      if (arg.type == ir::FunctionArgument::GLOBAL_POINTER ||
          arg.type == ir::FunctionArgument::LOCAL_POINTER ||
          arg.type == ir::FunctionArgument::CONSTANT_POINTER)
        this->insertCurbeReg(arg.reg, this->newCurbeEntry(GBE_CURBE_KERNEL_ARGUMENT, argID, ptrSize, ptrSize));
      if (arg.type == ir::FunctionArgument::VALUE ||
          arg.type == ir::FunctionArgument::STRUCTURE ||
          arg.type == ir::FunctionArgument::IMAGE ||
          arg.type == ir::FunctionArgument::SAMPLER)
        this->insertCurbeReg(arg.reg, this->newCurbeEntry(GBE_CURBE_KERNEL_ARGUMENT, argID, arg.size, arg.size));
    }

    // Go over all the instructions and find the special register we need
    // to push
    #define INSERT_REG(SPECIAL_REG, PATCH) \
    if (reg == ir::ocl::SPECIAL_REG) { \
      if (curbeRegs.find(reg) != curbeRegs.end()) continue; \
      allocCurbeReg(reg, GBE_CURBE_##PATCH); \
    } else
  
    fn.foreachInstruction([&](ir::Instruction &insn) {
      const uint32_t srcNum = insn.getSrcNum();
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        const ir::Register reg = insn.getSrc(srcID);
        if (insn.getOpcode() == ir::OP_GET_IMAGE_INFO) {
          if (srcID != 0) continue;
          const unsigned char bti = ir::cast<ir::GetImageInfoInstruction>(insn).getImageIndex();
          const unsigned char type =  ir::cast<ir::GetImageInfoInstruction>(insn).getInfoType();;
          ir::ImageInfoKey key(bti, type);
          const ir::Register imageInfo = insn.getSrc(0);
          if (curbeRegs.find(imageInfo) == curbeRegs.end()) {
            uint32_t offset = this->getImageInfoCurbeOffset(key, 4);
            insertCurbeReg(imageInfo, offset);
          }
          continue;
        }
        if (fn.isSpecialReg(reg) == false) continue;
        if (curbeRegs.find(reg) != curbeRegs.end()) continue;
        if (reg == ir::ocl::stackptr) GBE_ASSERT(stackUse.size() > 0);
        INSERT_REG(lsize0, LOCAL_SIZE_X)
        INSERT_REG(lsize1, LOCAL_SIZE_Y)
        INSERT_REG(lsize2, LOCAL_SIZE_Z)
        INSERT_REG(gsize0, GLOBAL_SIZE_X)
        INSERT_REG(gsize1, GLOBAL_SIZE_Y)
        INSERT_REG(gsize2, GLOBAL_SIZE_Z)
        INSERT_REG(goffset0, GLOBAL_OFFSET_X)
        INSERT_REG(goffset1, GLOBAL_OFFSET_Y)
        INSERT_REG(goffset2, GLOBAL_OFFSET_Z)
        INSERT_REG(workdim, WORK_DIM)
        INSERT_REG(numgroup0, GROUP_NUM_X)
        INSERT_REG(numgroup1, GROUP_NUM_Y)
        INSERT_REG(numgroup2, GROUP_NUM_Z)
        INSERT_REG(stackptr, STACK_POINTER)
        INSERT_REG(printfbptr, PRINTF_BUF_POINTER)
        INSERT_REG(printfiptr, PRINTF_INDEX_POINTER)
        do {} while(0);
      }
    });
#undef INSERT_REG


    // After this point the vector is immutable. Sorting it will make
    // research faster
    std::sort(kernel->patches.begin(), kernel->patches.end());

    kernel->curbeSize = ALIGN(kernel->curbeSize, GEN_REG_SIZE);
  }

  bool GenContext::emitCode(void) {
    GenKernel *genKernel = static_cast<GenKernel*>(this->kernel);
    buildPatchList();
    sel->select();
    schedulePreRegAllocation(*this, *this->sel);
    if (UNLIKELY(ra->allocate(*this->sel) == false))
      return false;
    schedulePostRegAllocation(*this, *this->sel);
    if (OCL_OUTPUT_REG_ALLOC)
      ra->outputAllocation();
    this->clearFlagRegister();
    this->emitStackPointer();
    this->emitSLMOffset();
    this->emitInstructionStream();
    if (this->patchBranches() == false)
      return false;
    genKernel->insnNum = p->store.size();
    genKernel->insns = GBE_NEW_ARRAY_NO_ARG(GenInstruction, genKernel->insnNum);
    std::memcpy(genKernel->insns, &p->store[0], genKernel->insnNum * sizeof(GenInstruction));
    if (OCL_OUTPUT_ASM) {
      std::cout << genKernel->getName() << "'s disassemble begin:" << std::endl;
      ir::LabelIndex curLabel = (ir::LabelIndex)0;
      GenCompactInstruction * pCom = NULL;
      GenInstruction insn[2];
      std::cout << "  L0:" << std::endl;
      for (uint32_t insnID = 0; insnID < genKernel->insnNum; ) {
        if (labelPos.find((ir::LabelIndex)(curLabel + 1))->second == insnID &&
            curLabel < this->getFunction().labelNum()) {
          std::cout << "  L" << curLabel + 1 << ":" << std::endl;
          curLabel = (ir::LabelIndex)(curLabel + 1);
          while(labelPos.find((ir::LabelIndex)(curLabel + 1))->second == insnID) {
            std::cout << "  L" << curLabel + 1 << ":" << std::endl;
            curLabel = (ir::LabelIndex)(curLabel + 1);
          }
        }
        std::cout << "    (" << std::setw(8) << insnID << ")  ";
        pCom = (GenCompactInstruction*)&p->store[insnID];
        if(pCom->bits1.cmpt_control == 1) {
          decompactInstruction(pCom, &insn);
          gen_disasm(stdout, &insn, deviceID, 1);
          insnID++;
        } else {
          gen_disasm(stdout, &p->store[insnID], deviceID, 0);
          insnID = insnID + 2;
        }
      }
      std::cout << genKernel->getName() << "'s disassemble end." << std::endl;
    }
    return true;
  }

  Kernel *GenContext::allocateKernel(void) {
    return GBE_NEW(GenKernel, name, deviceID);
  }

} /* namespace gbe */


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
      case SEL_OP_RNDD: p->RNDD(dst, src); break;
      case SEL_OP_RNDU: p->RNDU(dst, src); break;
      case SEL_OP_RNDE: p->RNDE(dst, src); break;
      case SEL_OP_RNDZ: p->RNDZ(dst, src); break;
      default: NOT_IMPLEMENTED;
    }
  }

  void GenContext::emitBinaryInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src0 = ra->genReg(insn.src(0));
    const GenRegister src1 = ra->genReg(insn.src(1));
    switch (insn.opcode) {
      case SEL_OP_LOAD_DF_IMM: p->LOAD_DF_IMM(dst, src1, src0.value.df); break;
      case SEL_OP_MOV_DF: p->MOV_DF(dst, src0, src1); break;
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
      p->curr.execWidth = 8;
      p->curr.noMask = 1;
      p->EOT(0);
    p->pop();
  }

  void GenContext::emitReadFloat64Instruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t bti = insn.extra.function;
    const uint32_t elemNum = insn.extra.elem;
    p->READ_FLOAT64(dst, src, bti, elemNum);
  }

  void GenContext::emitUntypedReadInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t bti = insn.extra.function;
    const uint32_t elemNum = insn.extra.elem;
    p->UNTYPED_READ(dst, src, bti, elemNum);
  }

  void GenContext::emitWriteFloat64Instruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t bti = insn.extra.function;
    const uint32_t elemNum = insn.extra.elem;
    p->WRITE_FLOAT64(src, bti, elemNum);
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


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
#include "backend/gen_insn_selection_output.hpp"
#include "backend/gen_reg_allocation.hpp"
#include "backend/gen/gen_mesa_disasm.h"
#include "ir/function.hpp"
#include "ir/value.hpp"
#include "ir/profiling.hpp"
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
    this->asmFileName = NULL;
    this->ifEndifFix = false;
    this->regSpillTick = 0;
    this->inProfilingMode = false;
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

  void GenContext::setASMFileName(const char* asmFname) {
    this->asmFileName = asmFname;
  }

  void GenContext::newSelection(void) {
    this->sel = GBE_NEW(Selection, *this);
  }

  uint32_t GenContext::alignScratchSize(uint32_t size){
    uint32_t i = 0;
    while(i < size) i+=1024;
    return i;
  }

  extern bool OCL_DEBUGINFO; // first defined by calling BVAR in program.cpp
#define SET_GENINSN_DBGINFO(I) \
  if(OCL_DEBUGINFO) p->DBGInfo = I.DBGInfo;
      
  void GenContext::emitInstructionStream(void) {
    // Emit Gen ISA
    for (auto &block : *sel->blockList)
    for (auto &insn : block.insnList) {
      const uint32_t opcode = insn.opcode;
      p->push();
      // no more virtual register here in that part of the code generation
      GBE_ASSERT(insn.state.physicalFlag);
      p->curr = insn.state;
      SET_GENINSN_DBGINFO(insn);
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
#undef SET_GENINSN_DBGINFO

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

  /* Get proper block ip register according to current label width. */
  GenRegister GenContext::getBlockIP(void) {
    GenRegister blockip;
    if (!isDWLabel())
      blockip = ra->genReg(GenRegister::uw8grf(ir::ocl::blockip));
    else
      blockip = ra->genReg(GenRegister::ud8grf(ir::ocl::dwblockip));
    return blockip;
  }

  /* Set current block ip register to a specified constant label value. */
  void GenContext::setBlockIP(GenRegister blockip, uint32_t label) {
    if (!isDWLabel())
      p->MOV(blockip, GenRegister::immuw(label));
    else
      p->MOV(blockip, GenRegister::immud(label));
  }

  void GenContext::clearFlagRegister(void) {
    // when group size not aligned to simdWidth, flag register need clear to
    // make prediction(any8/16h) work correctly
    const GenRegister blockip = getBlockIP();
    p->push();
      p->curr.noMask = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      setBlockIP(blockip, getMaxLabel());
      p->curr.noMask = 0;
      setBlockIP(blockip, 0);
      p->curr.execWidth = 1;
      if (ra->isAllocated(ir::ocl::zero))
        p->MOV(ra->genReg(GenRegister::uw1grf(ir::ocl::zero)), GenRegister::immuw(0));
      if (ra->isAllocated(ir::ocl::one))
        p->MOV(ra->genReg(GenRegister::uw1grf(ir::ocl::one)), GenRegister::immw(-1));
    p->pop();
  }

  void GenContext::loadLaneID(GenRegister dst) {
    const GenRegister laneID = GenRegister::immv(0x76543210);
    GenRegister dst_;
    if (dst.type == GEN_TYPE_UW)
      dst_ = dst;
    else if (dst.type == GEN_TYPE_UD)
      dst_ = GenRegister::retype(dst, GEN_TYPE_UW);
    p->push();
      uint32_t execWidth = p->curr.execWidth;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      if (execWidth == 8)
        p->MOV(dst_, laneID);
      else {
        p->curr.execWidth = 8;
        p->MOV(dst_, laneID);
        //Packed Unsigned Half-Byte Integer Vector does not work
        //have to mock by adding 8 to the singed vector
        const GenRegister eight = GenRegister::immuw(8);
        p->ADD(GenRegister::offset(dst_, 0, 16), dst_, eight);
        p->curr.execWidth = 16;
      }
      if (dst.type != GEN_TYPE_UW)
        p->MOV(dst, dst_);
    p->pop();
  }

  void GenContext::emitStackPointer(void) {
    using namespace ir;

    // Only emit stack pointer computation if we use a stack
    if (kernel->getStackSize() == 0)
      return;

    // Check that everything is consistent in the kernel code
    const uint32_t perLaneSize = kernel->getStackSize();
    GBE_ASSERT(perLaneSize > 0);

    const GenRegister selStatckPtr = this->simdWidth == 8 ?
      GenRegister::ud8grf(ir::ocl::stackptr) :
      GenRegister::ud16grf(ir::ocl::stackptr);
    const GenRegister stackptr = ra->genReg(selStatckPtr);
    // borrow block ip as temporary register as we will
    // initialize block ip latter.
    const GenRegister tmpReg = GenRegister::retype(GenRegister::vec1(getBlockIP()), GEN_TYPE_UW);
    const GenRegister tmpReg_ud = GenRegister::retype(tmpReg, GEN_TYPE_UD);

    loadLaneID(stackptr);

    // We compute the per-lane stack pointer here
    // threadId * perThreadSize + laneId*perLaneSize or
    // (threadId * simdWidth + laneId)*perLaneSize
    // let private address start from zero
    //p->MOV(stackptr, GenRegister::immud(0));
    p->push();
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->AND(tmpReg, GenRegister::ud1grf(0,5), GenRegister::immuw(0x1ff)); //threadId
      p->MUL(tmpReg, tmpReg, GenRegister::immuw(this->simdWidth));  //threadId * simdWidth
      p->curr.execWidth = this->simdWidth;
      p->ADD(stackptr, GenRegister::unpacked_uw(stackptr), tmpReg);  //threadId * simdWidth + laneId, must < 64K
      p->curr.execWidth = 1;
      p->MOV(tmpReg_ud, GenRegister::immud(perLaneSize));
      p->curr.execWidth = this->simdWidth;
      p->MUL(stackptr, tmpReg_ud, stackptr); // (threadId * simdWidth + laneId)*perLaneSize
      if (fn.getPointerFamily() == ir::FAMILY_QWORD) {
        const GenRegister selStatckPtr2 = this->simdWidth == 8 ?
          GenRegister::ul8grf(ir::ocl::stackptr) :
          GenRegister::ul16grf(ir::ocl::stackptr);
        const GenRegister stackptr2 = ra->genReg(selStatckPtr2);
        int simdWidth = p->curr.execWidth;
        if (simdWidth == 16) {
          // we need do second quarter first, because the dst type is QW,
          // while the src is DW. If we do first quater first, the 1st
          // quarter's dst would contain the 2nd quarter's src.
          p->curr.execWidth = 8;
          p->curr.quarterControl = GEN_COMPRESSION_Q2;
          p->MOV(GenRegister::Qn(stackptr2, 1), GenRegister::Qn(stackptr,1));
        }
        p->curr.quarterControl = GEN_COMPRESSION_Q1;
        p->MOV(stackptr2, stackptr);
      }
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
      case SEL_OP_LZD: p->LZD(dst, src); break;
      case SEL_OP_NOT: p->NOT(dst, src); break;
      case SEL_OP_RNDD: p->RNDD(dst, src); break;
      case SEL_OP_RNDU: p->RNDU(dst, src); break;
      case SEL_OP_RNDE: p->RNDE(dst, src); break;
      case SEL_OP_RNDZ: p->RNDZ(dst, src); break;
      case SEL_OP_F16TO32: p->F16TO32(dst, src); break;
      case SEL_OP_F32TO16: p->F32TO16(dst, src); break;
      case SEL_OP_LOAD_INT64_IMM: p->LOAD_INT64_IMM(dst, src); break;
      case SEL_OP_BFREV: p->BFREV(dst, src); break;
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
      case SEL_OP_BSWAP: {
        uint32_t simd = p->curr.execWidth;
        GBE_ASSERT(simd == 8 || simd == 16 || simd == 1);
        uint16_t new_a0[16];
        memset(new_a0, 0, sizeof(new_a0));

        GBE_ASSERT(src.type == dst.type);
        uint32_t start_addr = src.nr*32 + src.subnr;

        if (simd == 1) {
          GBE_ASSERT(src.hstride == GEN_HORIZONTAL_STRIDE_0
              && dst.hstride == GEN_HORIZONTAL_STRIDE_0);
          if (src.type == GEN_TYPE_UD || src.type == GEN_TYPE_D) {
            GBE_ASSERT(start_addr >= 0);
            new_a0[0] = start_addr + 3;
            new_a0[1] = start_addr + 2;
            new_a0[2] = start_addr + 1;
            new_a0[3] = start_addr;
            this->setA0Content(new_a0, 0, 4);

            p->push();
            p->curr.execWidth = 4;
            p->curr.predicate = GEN_PREDICATE_NONE;
            p->curr.noMask = 1;
            GenRegister ind_src = GenRegister::to_indirect1xN(GenRegister::retype(src, GEN_TYPE_UB), new_a0[0], 0);
            GenRegister dst_ = dst;
            dst_.type = GEN_TYPE_UB;
            dst_.hstride = GEN_HORIZONTAL_STRIDE_1;
            dst_.width = GEN_WIDTH_4;
            dst_.vstride = GEN_VERTICAL_STRIDE_4;
            p->MOV(dst_, ind_src);
            p->pop();
          } else if (src.type == GEN_TYPE_UW || src.type == GEN_TYPE_W) {
            p->MOV(GenRegister::retype(dst, GEN_TYPE_UB),
                GenRegister::retype(GenRegister::offset(src, 0, 1), GEN_TYPE_UB));
            p->MOV(GenRegister::retype(GenRegister::offset(dst, 0, 1), GEN_TYPE_UB),
                GenRegister::retype(src, GEN_TYPE_UB));
          } else {
            GBE_ASSERT(0);
          }
        } else {
          if (src.type == GEN_TYPE_UD || src.type == GEN_TYPE_D) {
            bool uniform_src = (src.hstride == GEN_HORIZONTAL_STRIDE_0);
            GBE_ASSERT(uniform_src || src.subnr == 0);
            GBE_ASSERT(dst.subnr == 0);
            GBE_ASSERT(tmp.subnr == 0);
            GBE_ASSERT(start_addr >= 0);
            new_a0[0] = start_addr + 3;
            new_a0[1] = start_addr + 2;
            new_a0[2] = start_addr + 1;
            new_a0[3] = start_addr;
            if (!uniform_src) {
              new_a0[4] = start_addr + 7;
              new_a0[5] = start_addr + 6;
              new_a0[6] = start_addr + 5;
              new_a0[7] = start_addr + 4;
            } else {
              new_a0[4] = start_addr + 3;
              new_a0[5] = start_addr + 2;
              new_a0[6] = start_addr + 1;
              new_a0[7] = start_addr;
            }
            this->setA0Content(new_a0, 56);

            p->push();
            p->curr.execWidth = 8;
            p->curr.predicate = GEN_PREDICATE_NONE;
            p->curr.noMask = 1;
            GenRegister ind_src = GenRegister::to_indirect1xN(GenRegister::retype(src, GEN_TYPE_UB), new_a0[0], 0);
            p->MOV(GenRegister::retype(tmp, GEN_TYPE_UB), ind_src);
            for (int i = 1; i < 4; i++) {
              if (!uniform_src)
                ind_src.addr_imm += 8;
              p->MOV(GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_UB), 0, 8*i), ind_src);
            }
            if (simd == 16) {
              for (int i = 0; i < 4; i++) {
                if (!uniform_src)
                  ind_src.addr_imm += 8;
                p->MOV(GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_UB), 1, 8*i), ind_src);
              }
            }
            p->pop();

            p->MOV(dst, tmp);
          } else if (src.type == GEN_TYPE_UW || src.type == GEN_TYPE_W) {
            bool uniform_src = (src.hstride == GEN_HORIZONTAL_STRIDE_0);
            GBE_ASSERT(uniform_src || src.subnr == 0 || src.subnr == 16);
            GBE_ASSERT(dst.subnr == 0 || dst.subnr == 16);
            GBE_ASSERT(tmp.subnr == 0 || tmp.subnr == 16);
            GBE_ASSERT(start_addr >= 0);
            new_a0[0] = start_addr + 1;
            new_a0[1] = start_addr;
            if (!uniform_src) {
              new_a0[2] = start_addr + 3;
              new_a0[3] = start_addr + 2;
              new_a0[4] = start_addr + 5;
              new_a0[5] = start_addr + 4;
              new_a0[6] = start_addr + 7;
              new_a0[7] = start_addr + 6;
            } else {
              new_a0[2] = start_addr + 1;
              new_a0[3] = start_addr;
              new_a0[4] = start_addr + 1;
              new_a0[5] = start_addr;
              new_a0[6] = start_addr + 1;
              new_a0[7] = start_addr;
            }
            this->setA0Content(new_a0, 56);

            p->push();
            p->curr.execWidth = 8;
            p->curr.predicate = GEN_PREDICATE_NONE;
            p->curr.noMask = 1;
            GenRegister ind_src = GenRegister::to_indirect1xN(GenRegister::retype(src, GEN_TYPE_UB), new_a0[0], 0);
            p->MOV(GenRegister::retype(tmp, GEN_TYPE_UB), ind_src);
            for (int i = 1; i < (simd == 8 ? 2 : 4); i++) {
              if (!uniform_src)
                ind_src.addr_imm += 8;
              p->MOV(GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_UB), 0, 8*i), ind_src);
            }
            p->pop();

            p->MOV(dst, tmp);
          }else if (src.type == GEN_TYPE_UL || src.type == GEN_TYPE_L) {
            bool uniform_src = (src.hstride == GEN_HORIZONTAL_STRIDE_0);
            GBE_ASSERT(uniform_src || src.subnr == 0);
            GBE_ASSERT(dst.subnr == 0);
            GBE_ASSERT(tmp.subnr == 0);
            GBE_ASSERT(start_addr >= 0);
            if (!uniform_src) {
              new_a0[0] = start_addr + 3;
              new_a0[1] = start_addr + 2;
              new_a0[2] = start_addr + 1;
              new_a0[3] = start_addr;
              new_a0[4] = start_addr + 7;
              new_a0[5] = start_addr + 6;
              new_a0[6] = start_addr + 5;
              new_a0[7] = start_addr + 4;
            } else {
              new_a0[0] = start_addr + 7;
              new_a0[1] = start_addr + 6;
              new_a0[2] = start_addr + 5;
              new_a0[3] = start_addr + 4;
              new_a0[4] = start_addr + 3;
              new_a0[5] = start_addr + 2;
              new_a0[6] = start_addr + 1;
              new_a0[7] = start_addr;
            }
            this->setA0Content(new_a0, 56);

            if (!uniform_src) {
              p->push();
              p->curr.execWidth = 8;
              p->curr.predicate = GEN_PREDICATE_NONE;
              p->curr.noMask = 1;
              GenRegister ind_src = GenRegister::to_indirect1xN(GenRegister::retype(src, GEN_TYPE_UB), new_a0[0], 0);
              p->MOV(GenRegister::retype(tmp, GEN_TYPE_UB), ind_src);
              for (int i = 1; i < 4; i++) {
                if (!uniform_src)
                  ind_src.addr_imm += 8;
                p->MOV(GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_UB), 0, 8*i), ind_src);
              }
              for (int i = 0; i < 4; i++) {
                if (!uniform_src)
                  ind_src.addr_imm += 8;
                p->MOV(GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_UB), 1, 8*i), ind_src);
              }
              if (simd == 16) {
                for (int i = 0; i < 4; i++) {
                  if (!uniform_src)
                    ind_src.addr_imm += 8;
                  p->MOV(GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_UB), 2, 8*i), ind_src);
                }
                for (int i = 0; i < 4; i++) {
                  if (!uniform_src)
                    ind_src.addr_imm += 8;
                  p->MOV(GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_UB), 3, 8*i), ind_src);
                }
              }
              p->pop();

              p->push();
              p->curr.execWidth = 8;
              p->curr.predicate = GEN_PREDICATE_NONE;
              p->curr.noMask = 1;
              if (simd == 8) {
                p->MOV(GenRegister::offset(GenRegister::retype(dst, GEN_TYPE_D), 1, 0),
                    GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_D), 0, 0));
                p->MOV(GenRegister::offset(GenRegister::retype(dst, GEN_TYPE_D), 0, 0),
                    GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_D), 1, 0));
              }else if(simd == 16) {
                p->MOV(GenRegister::offset(GenRegister::retype(dst, GEN_TYPE_D), 2, 0),
                    GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_D), 0, 0));
                p->MOV(GenRegister::offset(GenRegister::retype(dst, GEN_TYPE_D), 3, 0),
                    GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_D), 1, 0));
                p->MOV(GenRegister::offset(GenRegister::retype(dst, GEN_TYPE_D), 0, 0),
                    GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_D), 2, 0));
                p->MOV(GenRegister::offset(GenRegister::retype(dst, GEN_TYPE_D), 1, 0),
                    GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_D), 3, 0));
              }
              p->pop();
            } else {
                p->push();
                p->curr.execWidth = 8;
                p->curr.predicate = GEN_PREDICATE_NONE;
                p->curr.noMask = 1;
                GenRegister ind_src = GenRegister::to_indirect1xN(GenRegister::retype(src, GEN_TYPE_UB), new_a0[0], 0);
                p->MOV(GenRegister::retype(tmp, GEN_TYPE_UB), ind_src);
                p->pop();

                p->push();
                p->curr.execWidth = 8;
                p->curr.predicate = GEN_PREDICATE_NONE;
                p->curr.noMask = 1;
                GenRegister x = GenRegister::ud1grf(tmp.nr, 0);
                GenRegister y = GenRegister::ud1grf(tmp.nr, 1);
                GenRegister dst_ = dst;
                dst_.type = GEN_TYPE_UD;
                dst_.hstride = GEN_HORIZONTAL_STRIDE_1;
                dst_.width = GEN_WIDTH_8;
                dst_.vstride = GEN_VERTICAL_STRIDE_8;

                if (simd == 8) {
                  p->MOV(GenRegister::offset(GenRegister::retype(dst_, GEN_TYPE_D), 0, 0), x);
                  p->MOV(GenRegister::offset(GenRegister::retype(dst_, GEN_TYPE_D), 1, 0), y);
                }else if(simd == 16) {
                  p->MOV(GenRegister::offset(GenRegister::retype(dst_, GEN_TYPE_D), 0, 0), x);
                  p->MOV(GenRegister::offset(GenRegister::retype(dst_, GEN_TYPE_D), 1, 0), x);
                  p->MOV(GenRegister::offset(GenRegister::retype(dst_, GEN_TYPE_D), 2, 0), y);
                  p->MOV(GenRegister::offset(GenRegister::retype(dst_, GEN_TYPE_D), 3, 0), y);
                }
                p->pop();
            }
          } else {
            GBE_ASSERT(0);
          }
        }
      }
      break;
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

  void GenContext::emitSimdShuffleInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src0 = ra->genReg(insn.src(0));
    const GenRegister src1 = ra->genReg(insn.src(1));
    assert(insn.opcode == SEL_OP_SIMD_SHUFFLE);
    assert (src1.file != GEN_IMMEDIATE_VALUE);

    uint32_t base = src0.nr * 32 + src0.subnr;
    GenRegister baseReg = GenRegister::immuw(base);
    const GenRegister a0 = GenRegister::addr8(0);
    uint32_t simd = p->curr.execWidth;
    p->push();
      if (simd == 8) {
        p->ADD(a0, GenRegister::unpacked_uw(src1.nr, src1.subnr / typeSize(GEN_TYPE_UW)), baseReg);
        GenRegister indirect = GenRegister::to_indirect1xN(src0, 0, 0);
        p->MOV(dst, indirect);
      } else if (simd == 16) {
        p->curr.execWidth = 8;
        p->ADD(a0, GenRegister::unpacked_uw(src1.nr, src1.subnr / typeSize(GEN_TYPE_UW)), baseReg);
        GenRegister indirect = GenRegister::to_indirect1xN(src0, 0, 0);
        p->MOV(dst, indirect);

        p->curr.quarterControl = 1;
        p->ADD(a0, GenRegister::unpacked_uw(src1.nr+1, src1.subnr / typeSize(GEN_TYPE_UW)), baseReg);
        p->MOV(GenRegister::offset(dst, 0, 8 * typeSize(src0.type)), indirect);
      } else
        NOT_IMPLEMENTED;
    p->pop();
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

  void GenContext::emitF64DIVInstruction(const SelectionInstruction &insn) {
    GBE_ASSERT(0); // No support for double on Gen7
  }

  void GenContext::emitTernaryInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src0 = ra->genReg(insn.src(0));
    const GenRegister src1 = ra->genReg(insn.src(1));
    const GenRegister src2 = ra->genReg(insn.src(2));
    switch (insn.opcode) {
      case SEL_OP_MAD:  p->MAD(dst, src0, src1, src2); break;
      case SEL_OP_LRP:  p->LRP(dst, src0, src1, src2); break;
      default: NOT_IMPLEMENTED;
    }
  }

  void GenContext::emitNoOpInstruction(const SelectionInstruction &insn) {
   p->NOP();
  }

  void GenContext::emitWaitInstruction(const SelectionInstruction &insn) {
    p->WAIT(insn.extra.waitType);
  }

  void GenContext::emitBarrierInstruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.src(0));
    const GenRegister fenceDst = ra->genReg(insn.dst(0));
    uint32_t barrierType = insn.extra.barrierType;
    const GenRegister barrierId = ra->genReg(GenRegister::ud1grf(ir::ocl::barrierid));
    bool imageFence = barrierType & ir::SYNC_IMAGE_FENCE;

    if (barrierType & ir::SYNC_GLOBAL_READ_FENCE) {
      p->FENCE(fenceDst, imageFence);
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
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->WAIT();
    p->pop();
    if (imageFence) {
      p->FLUSH_SAMPLERCACHE(fenceDst);
      p->MOV(fenceDst, fenceDst);
    }
  }

  void GenContext::emitFenceInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    p->FENCE(dst, false);
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
    const GenRegister addr = ra->genReg(insn.src(0));
    const GenRegister dst = ra->genReg(insn.dst(0));
    const uint32_t function = insn.extra.function;
    unsigned srcNum = insn.extra.elem;

    GenRegister data = addr;
    if (srcNum > 1)
      data = ra->genReg(insn.src(1));

    const GenRegister bti = ra->genReg(insn.src(srcNum));

    if (bti.file == GEN_IMMEDIATE_VALUE) {
      p->ATOMIC(dst, function, addr, data, bti, srcNum, insn.extra.splitSend);
    } else {
      GenRegister flagTemp = ra->genReg(insn.dst(1));
      GenRegister btiTmp = ra->genReg(insn.dst(2));

      unsigned desc = 0;
      if (insn.extra.splitSend)
        desc = p->generateAtomicMessageDesc(function, 0, 1);
      else
        desc = p->generateAtomicMessageDesc(function, 0, srcNum);

      unsigned jip0 = beforeMessage(insn, bti, flagTemp, btiTmp, desc);
      p->push();
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(insn.state.flag, insn.state.subFlag);
        p->ATOMIC(dst, function, addr, data, GenRegister::addr1(0), srcNum, insn.extra.splitSend);
      p->pop();
      afterMessage(insn, bti, flagTemp, btiTmp, jip0);
    }
  }

  void GenContext::emitIndirectMoveInstruction(const SelectionInstruction &insn) {
    GenRegister baseReg = ra->genReg(insn.src(0));
    GenRegister offset = ra->genReg(insn.src(1));
    uint32_t immoffset = insn.extra.indirect_offset;

    const GenRegister dst = ra->genReg(insn.dst(0));
    GenRegister tmp = ra->genReg(insn.dst(1));
    const GenRegister a0 = GenRegister::addr8(0);
    uint32_t simdWidth = p->curr.execWidth;
    GenRegister indirect_src;

    if(sel->isScalarReg(offset.reg()))
      offset = GenRegister::retype(offset, GEN_TYPE_UW);
    else
      offset = GenRegister::unpacked_uw(offset);
    uint32_t baseRegOffset = GenRegister::grfOffset(baseReg);
    //There is a restrict that: lower 5 bits indirect reg SubRegNum and
    //the lower 5 bits of indirect imm SubRegNum cannot exceed 5 bits.
    //So can't use AddrImm field, need a add.
    p->ADD(tmp, offset, GenRegister::immuw(baseRegOffset + immoffset));
    indirect_src = GenRegister::indirect(dst.type, 0, GEN_WIDTH_1,
                                         GEN_VERTICAL_STRIDE_ONE_DIMENSIONAL, GEN_HORIZONTAL_STRIDE_0);

    p->push();
      p->curr.execWidth = 8;
      p->curr.quarterControl = GEN_COMPRESSION_Q1;
      p->MOV(a0, tmp);
      p->MOV(dst, indirect_src);
    p->pop();

    if (simdWidth == 16) {
      p->push();
        p->curr.execWidth = 8;
        p->curr.quarterControl = GEN_COMPRESSION_Q2;

        const GenRegister nextDst = GenRegister::Qn(dst, 1);
        const GenRegister nextOffset = GenRegister::Qn(tmp, 1);
        p->MOV(a0, nextOffset);
        p->MOV(nextDst, indirect_src);
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
      loadBottomHalf(payload, src.isdf()? GenRegister::retype(src, GEN_TYPE_UL) : src );
      uint32_t regNum = (regSize/2*simdWidth) > 32 ? 2 : 1;
      this->scratchWrite(msg, scratchOffset, regNum, GEN_TYPE_UD, GEN_SCRATCH_CHANNEL_MODE_DWORD);
      loadTopHalf(payload, src.isdf() ? GenRegister::retype(src, GEN_TYPE_UL) : src);
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
      storeBottomHalf(GenRegister::ul8grf(dst.nr, dst.subnr), payload);
      this->scratchRead(payload, msg, scratchOffset + 4*simdWidth, regNum, GEN_TYPE_UD, GEN_SCRATCH_CHANNEL_MODE_DWORD);
      storeTopHalf(GenRegister::ul8grf(dst.nr, dst.subnr), payload);
    }
    p->pop();
  }

  void GenContext::emitRead64Instruction(const SelectionInstruction &insn) {
    const uint32_t elemNum = insn.extra.elem * 2;
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    const GenRegister bti = ra->genReg(insn.src(1));

    if (bti.file == GEN_IMMEDIATE_VALUE) {
      p->UNTYPED_READ(dst, src, bti, elemNum);
    } else {
      const GenRegister tmp = ra->genReg(insn.dst(insn.extra.elem));
      const GenRegister btiTmp = ra->genReg(insn.dst(insn.extra.elem + 1));
      unsigned desc = p->generateUntypedReadMessageDesc(0, elemNum);

      unsigned jip0 = beforeMessage(insn, bti, tmp, btiTmp, desc);

      //predicated load
      p->push();
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(insn.state.flag, insn.state.subFlag);
        p->UNTYPED_READ(dst, src, GenRegister::retype(GenRegister::addr1(0), GEN_TYPE_UD), elemNum);
      p->pop();
      afterMessage(insn, bti, tmp, btiTmp, jip0);
    }
  }
  unsigned GenContext::beforeMessage(const SelectionInstruction &insn, GenRegister bti, GenRegister tmp, GenRegister btiTmp, unsigned desc) {
      const GenRegister flagReg = GenRegister::flag(insn.state.flag, insn.state.subFlag);
      setFlag(flagReg, GenRegister::immuw(0));
      p->CMP(GEN_CONDITIONAL_NZ, flagReg, GenRegister::immuw(1));

      GenRegister btiUD = GenRegister::retype(btiTmp, GEN_TYPE_UD);
      GenRegister btiUW = GenRegister::retype(btiTmp, GEN_TYPE_UW);
      GenRegister btiUB = GenRegister::retype(btiTmp, GEN_TYPE_UB);
      unsigned jip0 = p->n_instruction();
      p->push();
        p->curr.execWidth = 1;
        p->curr.noMask = 1;
        p->AND(btiUD, flagReg, GenRegister::immud(0xffffffff));
        p->LZD(btiUD, btiUD);
        p->ADD(btiUW, GenRegister::negate(btiUW), GenRegister::immuw(0x1f));
        p->MUL(btiUW, btiUW, GenRegister::immuw(0x4));
        p->ADD(GenRegister::addr1(0), btiUW, GenRegister::immud(bti.nr*32));
        p->MOV(btiUD, GenRegister::indirect(GEN_TYPE_UD, 0, GEN_WIDTH_1, GEN_VERTICAL_STRIDE_ONE_DIMENSIONAL, GEN_HORIZONTAL_STRIDE_0));
        //save flag
        p->MOV(tmp, flagReg);
      p->pop();

      p->CMP(GEN_CONDITIONAL_Z, bti, btiUD);
      p->push();
        p->curr.execWidth = 1;
        p->curr.noMask = 1;
        p->OR(GenRegister::retype(GenRegister::addr1(0), GEN_TYPE_UD), btiUB, GenRegister::immud(desc));
      p->pop();
      return jip0;
  }
  void GenContext::afterMessage(const SelectionInstruction &insn, GenRegister bti, GenRegister tmp, GenRegister btiTmp, unsigned jip0) {
    const GenRegister btiUD = GenRegister::retype(btiTmp, GEN_TYPE_UD);
      //restore flag
      setFlag(GenRegister::flag(insn.state.flag, insn.state.subFlag), tmp);
      // get active channel
      p->push();
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(insn.state.flag, insn.state.subFlag);
        p->CMP(GEN_CONDITIONAL_NZ, bti, btiUD);
        unsigned jip1 = p->n_instruction();
        p->WHILE(GenRegister::immud(0));
      p->pop();
      p->patchJMPI(jip1, jip0 - jip1, 0);
  }

  void GenContext::emitUntypedReadInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    const GenRegister bti = ra->genReg(insn.src(1));

    const uint32_t elemNum = insn.extra.elem;
    if (bti.file == GEN_IMMEDIATE_VALUE) {
      p->UNTYPED_READ(dst, src, bti, elemNum);
    } else {
      const GenRegister tmp = ra->genReg(insn.dst(elemNum));
      const GenRegister btiTmp = ra->genReg(insn.dst(elemNum + 1));
      unsigned desc = p->generateUntypedReadMessageDesc(0, elemNum);

      unsigned jip0 = beforeMessage(insn, bti, tmp, btiTmp, desc);

      //predicated load
      p->push();
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(insn.state.flag, insn.state.subFlag);
        p->UNTYPED_READ(dst, src, GenRegister::retype(GenRegister::addr1(0), GEN_TYPE_UD), elemNum);
      p->pop();
      afterMessage(insn, bti, tmp, btiTmp, jip0);
    }
  }

  void GenContext::emitWrite64Instruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.dst(0));
    const uint32_t elemNum = insn.extra.elem;
    const GenRegister bti = ra->genReg(insn.src(elemNum+1));

    if (bti.file == GEN_IMMEDIATE_VALUE) {
      p->UNTYPED_WRITE(src, src, bti, elemNum*2, false);
    } else {
      const GenRegister tmp = ra->genReg(insn.dst(0));
      const GenRegister btiTmp = ra->genReg(insn.dst(1));
      unsigned desc = p->generateUntypedWriteMessageDesc(0, elemNum*2);

      unsigned jip0 = beforeMessage(insn, bti, tmp, btiTmp, desc);

      //predicated load
      p->push();
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(insn.state.flag, insn.state.subFlag);
        p->UNTYPED_WRITE(src, src, GenRegister::addr1(0), elemNum*2, false);
      p->pop();
      afterMessage(insn, bti, tmp, btiTmp, jip0);
    }
  }

  void GenContext::emitUntypedWriteInstruction(const SelectionInstruction &insn) {
    const GenRegister addr = ra->genReg(insn.src(0));
    GenRegister data = ra->genReg(insn.src(1));
    const uint32_t elemNum = insn.extra.elem;
    const GenRegister bti = ra->genReg(insn.src(elemNum+1));
    if (bti.file == GEN_IMMEDIATE_VALUE) {
      p->UNTYPED_WRITE(addr, data, bti, elemNum, insn.extra.splitSend);
    } else {
      const GenRegister tmp = ra->genReg(insn.dst(0));
      const GenRegister btiTmp = ra->genReg(insn.dst(1));
      unsigned desc = 0;
      if (insn.extra.splitSend)
        desc = p->generateUntypedWriteSendsMessageDesc(0, elemNum);
      else
        desc = p->generateUntypedWriteMessageDesc(0, elemNum);

      unsigned jip0 = beforeMessage(insn, bti, tmp, btiTmp, desc);

      //predicated load
      p->push();
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(insn.state.flag, insn.state.subFlag);
        p->UNTYPED_WRITE(addr, data, GenRegister::addr1(0), elemNum, insn.extra.splitSend);
      p->pop();
      afterMessage(insn, bti, tmp, btiTmp, jip0);
    }
  }

  void GenContext::emitByteGatherInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    const GenRegister bti = ra->genReg(insn.src(1));
    const uint32_t elemSize = insn.extra.elem;

    if (bti.file == GEN_IMMEDIATE_VALUE) {
      p->BYTE_GATHER(dst, src, bti, elemSize);
    } else {
      const GenRegister tmp = ra->genReg(insn.dst(1));
      const GenRegister btiTmp = ra->genReg(insn.dst(2));
      unsigned desc = p->generateByteGatherMessageDesc(0, elemSize);

      unsigned jip0 = beforeMessage(insn, bti, tmp, btiTmp, desc);

      //predicated load
      p->push();
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(insn.state.flag, insn.state.subFlag);
        p->BYTE_GATHER(dst, src, GenRegister::addr1(0), elemSize);
      p->pop();
      afterMessage(insn, bti, tmp, btiTmp, jip0);
    }
  }

  void GenContext::emitByteScatterInstruction(const SelectionInstruction &insn) {
    const GenRegister addr = ra->genReg(insn.src(0));
    GenRegister data = ra->genReg(insn.src(1));
    const uint32_t elemSize = insn.extra.elem;
    const GenRegister bti = ra->genReg(insn.src(2));

    if (bti.file == GEN_IMMEDIATE_VALUE) {
      p->BYTE_SCATTER(addr, data, bti, elemSize, insn.extra.splitSend);
    } else {
      const GenRegister tmp = ra->genReg(insn.dst(0));
      const GenRegister btiTmp = ra->genReg(insn.dst(1));
      unsigned desc = 0;
      if (insn.extra.splitSend)
        desc = p->generateByteScatterSendsMessageDesc(0, elemSize);
      else
        desc = p->generateByteScatterMessageDesc(0, elemSize);

      unsigned jip0 = beforeMessage(insn, bti, tmp, btiTmp, desc);

      //predicated load
      p->push();
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(insn.state.flag, insn.state.subFlag);
        p->BYTE_SCATTER(addr, data, GenRegister::addr1(0), elemSize, insn.extra.splitSend);
      p->pop();
      afterMessage(insn, bti, tmp, btiTmp, jip0);
    }

  }

  void GenContext::emitUntypedReadA64Instruction(const SelectionInstruction &insn) {
    assert(0);
  }

  void GenContext::emitUntypedWriteA64Instruction(const SelectionInstruction &insn) {
    assert(0);
  }

  void GenContext::emitByteGatherA64Instruction(const SelectionInstruction &insn) {
    assert(0);
  }

  void GenContext::emitByteScatterA64Instruction(const SelectionInstruction &insn) {
    assert(0);
  }
  void GenContext::emitRead64A64Instruction(const SelectionInstruction &insn) {
    assert(0);
  }
  void GenContext::emitWrite64A64Instruction(const SelectionInstruction &insn) {
    assert(0);
  }
  void GenContext::emitAtomicA64Instruction(const SelectionInstruction &insn) {
    assert(0);
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

  void GenContext::emitUnpackLongInstruction(const SelectionInstruction &insn) {
    GBE_ASSERT(0);
  }

  void GenContext::emitPackLongInstruction(const SelectionInstruction &insn) {
    GBE_ASSERT(0);
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

  void GenContext::emitVmeInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const unsigned int msg_type = insn.extra.msg_type;

    GBE_ASSERT(msg_type == 1);
    int rsp_len;
    if(msg_type == 1)
      rsp_len = 6;
    uint32_t execWidth_org = p->curr.execWidth;
    p->push();
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.noMask = 1;
    p->curr.execWidth = 1;
    /* Use MOV to Setup bits of payload: mov payload value stored in insn.src(x) to
     * 5 consecutive payload grf.
     * In simd8 mode, one virtual grf register map to one physical grf register. But
     * in simd16 mode, one virtual grf register map to two physical grf registers.
     * So we should treat them differently.
     * */
    if(execWidth_org == 8){
      for(int i=0; i < 5; i++){
        GenRegister payload_grf = ra->genReg(insn.dst(rsp_len+i));
        payload_grf.vstride = GEN_VERTICAL_STRIDE_0;
        payload_grf.width = GEN_WIDTH_1;
        payload_grf.hstride = GEN_HORIZONTAL_STRIDE_0;
        payload_grf.subphysical = 1;
        for(int j=0; j < 8; j++){
          payload_grf.subnr = (7 - j) * typeSize(GEN_TYPE_UD);
          GenRegister payload_val = ra->genReg(insn.src(i*8+j));
          payload_val.vstride = GEN_VERTICAL_STRIDE_0;
          payload_val.width = GEN_WIDTH_1;
          payload_val.hstride = GEN_HORIZONTAL_STRIDE_0;

          p->MOV(payload_grf, payload_val);
        }
      }
    }
    else if(execWidth_org == 16){
      for(int i=0; i < 2; i++){
        for(int k = 0; k < 2; k++){
          GenRegister payload_grf = ra->genReg(insn.dst(rsp_len+i));
          payload_grf.nr += k;
          payload_grf.vstride = GEN_VERTICAL_STRIDE_0;
          payload_grf.width = GEN_WIDTH_1;
          payload_grf.hstride = GEN_HORIZONTAL_STRIDE_0;
          payload_grf.subphysical = 1;
          for(int j=0; j < 8; j++){
            payload_grf.subnr = (7 - j) * typeSize(GEN_TYPE_UD);
            GenRegister payload_val = ra->genReg(insn.src(i*16+k*8+j));
            payload_val.vstride = GEN_VERTICAL_STRIDE_0;
            payload_val.width = GEN_WIDTH_1;
            payload_val.hstride = GEN_HORIZONTAL_STRIDE_0;

            p->MOV(payload_grf, payload_val);
          }
        }
      }
      {
        int i = 2;
        GenRegister payload_grf = ra->genReg(insn.dst(rsp_len+i));
        payload_grf.vstride = GEN_VERTICAL_STRIDE_0;
        payload_grf.width = GEN_WIDTH_1;
        payload_grf.hstride = GEN_HORIZONTAL_STRIDE_0;
        payload_grf.subphysical = 1;
        for(int j=0; j < 8; j++){
          payload_grf.subnr = (7 - j) * typeSize(GEN_TYPE_UD);
          GenRegister payload_val = ra->genReg(insn.src(i*16+j));
          payload_val.vstride = GEN_VERTICAL_STRIDE_0;
          payload_val.width = GEN_WIDTH_1;
          payload_val.hstride = GEN_HORIZONTAL_STRIDE_0;

          p->MOV(payload_grf, payload_val);
        }
      }
    }
    p->pop();

    p->push();
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.noMask = 1;
    p->curr.execWidth = 1;
    GenRegister payload_did = GenRegister::retype(ra->genReg(insn.dst(rsp_len)), GEN_TYPE_UB);
    payload_did.vstride = GEN_VERTICAL_STRIDE_0;
    payload_did.width = GEN_WIDTH_1;
    payload_did.hstride = GEN_HORIZONTAL_STRIDE_0;
    payload_did.subphysical = 1;
    payload_did.subnr = 20 * typeSize(GEN_TYPE_UB);
    GenRegister grf0 = GenRegister::ub1grf(0, 20);
    p->MOV(payload_did, grf0);
    p->pop();

    const GenRegister msgPayload = ra->genReg(insn.dst(rsp_len));
    const unsigned char bti = insn.getbti();
    const unsigned int vme_search_path_lut = insn.extra.vme_search_path_lut;
    const unsigned int lut_sub = insn.extra.lut_sub;
    p->VME(bti, dst, msgPayload, msg_type, vme_search_path_lut, lut_sub);
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
    GenRegister data = ra->genReg(insn.src(5));
    const uint32_t bti = insn.getbti();
    p->TYPED_WRITE(header, data, true, bti, insn.extra.typedWriteSplitSend);
  }

  static void calcGID(GenRegister& reg, GenRegister& tmp, int flag, int subFlag, int dim, GenContext *gc)
  {
    GenRegister flagReg = GenRegister::flag(flag, subFlag);
    GenRegister gstart = GenRegister::offset(reg, 0, 8 + dim*8);
    GenRegister gend = GenRegister::offset(gstart, 0, 4);
    GenRegister lid, localsz, gid, goffset;
    if (dim == 0) {
      lid = GenRegister::toUniform(gc->ra->genReg(GenRegister::ud16grf(ir::ocl::lid0)), GEN_TYPE_UD);
      localsz = GenRegister::toUniform(gc->ra->genReg(GenRegister::ud1grf(ir::ocl::lsize0)), GEN_TYPE_UD);
      gid = GenRegister::toUniform(gc->ra->genReg(GenRegister::ud1grf(ir::ocl::groupid0)), GEN_TYPE_UD);
      goffset = GenRegister::toUniform(gc->ra->genReg(GenRegister::ud1grf(ir::ocl::goffset0)), GEN_TYPE_UD);
    } else if (dim == 1) {
      lid = GenRegister::toUniform(gc->ra->genReg(GenRegister::ud16grf(ir::ocl::lid1)), GEN_TYPE_UD);
      localsz = GenRegister::toUniform(gc->ra->genReg(GenRegister::ud1grf(ir::ocl::lsize1)), GEN_TYPE_UD);
      gid = GenRegister::toUniform(gc->ra->genReg(GenRegister::ud1grf(ir::ocl::groupid1)), GEN_TYPE_UD);
      goffset = GenRegister::toUniform(gc->ra->genReg(GenRegister::ud1grf(ir::ocl::goffset1)), GEN_TYPE_UD);
    } else {
      lid = GenRegister::toUniform(gc->ra->genReg(GenRegister::ud16grf(ir::ocl::lid2)), GEN_TYPE_UD);
      localsz = GenRegister::toUniform(gc->ra->genReg(GenRegister::ud1grf(ir::ocl::lsize2)), GEN_TYPE_UD);
      gid = GenRegister::toUniform(gc->ra->genReg(GenRegister::ud1grf(ir::ocl::groupid2)), GEN_TYPE_UD);
      goffset = GenRegister::toUniform(gc->ra->genReg(GenRegister::ud1grf(ir::ocl::goffset2)), GEN_TYPE_UD);
    }

    gc->p->MUL(gstart, localsz, gid);
    gc->p->ADD(gstart, gstart, lid);
    gc->p->ADD(gstart, gstart, goffset);

    GenRegister ip;
    gc->p->MOV(flagReg, GenRegister::immuw(0x0));
    gc->p->curr.useFlag(flag, subFlag);
    gc->p->curr.predicate = GEN_PREDICATE_NONE;
    if (gc->getSimdWidth() == 16)
      gc->p->curr.execWidth = 16;
    else
      gc->p->curr.execWidth = 8;

    if (!gc->isDWLabel()) {
      ip = gc->ra->genReg(GenRegister::uw16grf(ir::ocl::blockip));
      gc->p->CMP(GEN_CONDITIONAL_EQ, ip, GenRegister::immuw(0xffff));
    } else {
      ip = gc->ra->genReg(GenRegister::ud16grf(ir::ocl::dwblockip));
      gc->p->CMP(GEN_CONDITIONAL_EQ, ip, GenRegister::immud(0xffffffff));
    }
    gc->p->curr.execWidth = 1;
    gc->p->MOV(GenRegister::retype(tmp, GEN_TYPE_UW), flagReg);

    if (gc->getSimdWidth() == 16)
      gc->p->OR(tmp, tmp, GenRegister::immud(0xffff0000));
    else
      gc->p->OR(tmp, tmp, GenRegister::immud(0xffffff00));

    gc->p->FBL(tmp, tmp);
    gc->p->ADD(tmp, tmp, GenRegister::negate(GenRegister::immud(0x1)));
    gc->p->MUL(tmp, tmp, GenRegister::immud(4));
    gc->p->ADD(tmp, tmp, GenRegister::immud(lid.nr*32));
    gc->p->MOV(GenRegister::addr1(0), GenRegister::retype(tmp, GEN_TYPE_UW));
    GenRegister dimEnd = GenRegister::to_indirect1xN(lid, 0, 0);
    gc->p->MOV(tmp, dimEnd);
    gc->p->MUL(gend, localsz, gid);
    gc->p->ADD(gend, gend, tmp);
    gc->p->ADD(gend, gend, goffset);
  }

  void GenContext::calcGlobalXYZRange(GenRegister& reg, GenRegister& tmp, int flag, int subFlag)
  {
    p->push(); {
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      calcGID(reg, tmp, flag, subFlag, 0, this);
      calcGID(reg, tmp, flag, subFlag, 1, this);
      calcGID(reg, tmp, flag, subFlag, 2, this);
    } p->pop();
  }

  void GenContext::profilingProlog(void) {
    // record the prolog, globalXYZ and lasttimestamp at the very beginning.
    GenRegister profilingReg2, profilingReg3, profilingReg4;
    GenRegister tmArf = GenRegister::tm0();
    if (this->simdWidth == 16) {
      profilingReg2 = ra->genReg(GenRegister::ud16grf(ir::ocl::profilingts1));
      profilingReg3 = GenRegister::offset(profilingReg2, 1);
      profilingReg4 = ra->genReg(GenRegister::ud16grf(ir::ocl::profilingts2));
    } else {
      GBE_ASSERT(this->simdWidth == 8);
      profilingReg2 = ra->genReg(GenRegister::ud8grf(ir::ocl::profilingts2));
      profilingReg3 = ra->genReg(GenRegister::ud8grf(ir::ocl::profilingts3));
      profilingReg4 = ra->genReg(GenRegister::ud8grf(ir::ocl::profilingts4));
    }

    /* MOV(4)   prolog<1>:UW   arf_tm<4,4,1>:UW  */
    /* MOV(4)   lastTsReg<1>:UW  prolog<4,4,1>:UW  */
    GenRegister prolog = profilingReg2;
    prolog.type = GEN_TYPE_UW;
    prolog.hstride = GEN_HORIZONTAL_STRIDE_1;
    prolog.vstride = GEN_VERTICAL_STRIDE_4;
    prolog.width = GEN_WIDTH_4;
    prolog = GenRegister::offset(prolog, 0, 4*sizeof(uint32_t));

    GenRegister lastTsReg = GenRegister::toUniform(profilingReg3, GEN_TYPE_UL);
    lastTsReg = GenRegister::offset(lastTsReg, 0, 2*sizeof(uint64_t));
    lastTsReg.type = GEN_TYPE_UW;
    lastTsReg.hstride = GEN_HORIZONTAL_STRIDE_1;
    lastTsReg.vstride = GEN_VERTICAL_STRIDE_4;
    lastTsReg.width = GEN_WIDTH_4;

    GenRegister gids = GenRegister::toUniform(profilingReg4, GEN_TYPE_UD);
    GenRegister tmp = GenRegister::toUniform(profilingReg4, GEN_TYPE_UD);

    // X Y and Z
    this->calcGlobalXYZRange(gids, tmp, 0, 1);

    p->push(); {
      p->curr.execWidth = 4;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->MOV(prolog, tmArf);
      p->MOV(lastTsReg, tmArf);
    } p->pop();

    p->NOP();
    p->NOP();
    return;
  }

  void GenContext::subTimestamps(GenRegister& t0, GenRegister& t1, GenRegister& tmp)
  {
    p->push(); {
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->SUBB(GenRegister::retype(t0, GEN_TYPE_UD),
          GenRegister::retype(t0, GEN_TYPE_UD), GenRegister::retype(t1, GEN_TYPE_UD));
      /* FIXME We can not get the acc register's value correctly by set simd = 1. */
      p->curr.execWidth = 8;
      p->MOV(tmp, GenRegister::retype(GenRegister::acc(), GEN_TYPE_UD));
      p->curr.execWidth = 1;
      p->ADD(GenRegister::retype(GenRegister::offset(t0, 0, sizeof(uint32_t)), GEN_TYPE_UD),
          GenRegister::retype(GenRegister::offset(t0, 0, sizeof(uint32_t)), GEN_TYPE_UD),
          GenRegister::negate(GenRegister::toUniform(tmp, GEN_TYPE_UD)));
      p->ADD(GenRegister::retype(GenRegister::offset(t0, 0, sizeof(uint32_t)), GEN_TYPE_UD),
          GenRegister::retype(GenRegister::offset(t0, 0, sizeof(uint32_t)), GEN_TYPE_UD),
          GenRegister::negate(GenRegister::retype(GenRegister::offset(t1, 0, sizeof(uint32_t)), GEN_TYPE_UD)));
    } p->pop();
  }

  void GenContext::addTimestamps(GenRegister& t0, GenRegister& t1, GenRegister& tmp)
  {
    p->push(); {
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->ADDC(GenRegister::retype(t0, GEN_TYPE_UD),
          GenRegister::retype(t0, GEN_TYPE_UD), GenRegister::retype(t1, GEN_TYPE_UD));
      p->curr.execWidth = 8;
      p->MOV(tmp, GenRegister::retype(GenRegister::acc(), GEN_TYPE_UD));
      p->curr.execWidth = 1;
      p->ADD(GenRegister::retype(GenRegister::offset(t0, 0, sizeof(uint32_t)), GEN_TYPE_UD),
          GenRegister::retype(GenRegister::offset(t0, 0, sizeof(uint32_t)), GEN_TYPE_UD),
          GenRegister::offset(GenRegister::toUniform(tmp, GEN_TYPE_UD), 0, 6*sizeof(uint32_t)));
      p->ADD(GenRegister::retype(GenRegister::offset(t0, 0, sizeof(uint32_t)), GEN_TYPE_UD),
          GenRegister::retype(GenRegister::offset(t0, 0, sizeof(uint32_t)), GEN_TYPE_UD),
          GenRegister::retype(GenRegister::offset(t1, 0, sizeof(uint32_t)), GEN_TYPE_UD));
    } p->pop();
  }

  /* We will record at most 20 timestamps, each one is 16bits. We also will record the
     prolog and epilog timestamps in 64 bits. So the format of the curbe timestamp reg is:
     ---------------------------------------------------------
     | ts0  | ts1  | ts2  | ts3  | ts4  | ts5  | ts6  | ts7	|  profilingReg0
     | ts8  | ts9  | ts10 | ts11 | ts12 | ts13 | ts14 | ts15 |  profilingReg1
     | ts16 | ts17 | ts18 | ts19 |	 prolog   |    epilog	|  profilingReg2
     ---------------------------------------------------------
     |    tmp0     |    tmp1     |lasttimestamp|  real clock |  profilingReg3
     ---------------------------------------------------------
     |	      | gX s | gX e | gY s | gY e | gZ s | gZ e |  profilingReg4
     ---------------------------------------------------------
     */
  void GenContext::emitCalcTimestampInstruction(const SelectionInstruction &insn)
  {
    uint32_t pointNum = insn.extra.pointNum;
    uint32_t tsType = insn.extra.timestampType;
    GenRegister flagReg = GenRegister::flag(insn.state.flag, insn.state.subFlag);

    (void) tsType;
    GBE_ASSERT(tsType == 1);
    GenRegister tmArf = GenRegister::tm0();
    GenRegister profilingReg[5];
    GenRegister tmp;
    if (p->curr.execWidth == 16) {
      profilingReg[0] = GenRegister::retype(ra->genReg(insn.src(0)), GEN_TYPE_UD);
      profilingReg[1] = GenRegister::offset(profilingReg[0], 1);
      profilingReg[2] = GenRegister::retype(ra->genReg(insn.src(1)), GEN_TYPE_UD);
      profilingReg[3] = GenRegister::offset(profilingReg[2], 1);
      profilingReg[4] = GenRegister::retype(ra->genReg(insn.src(2)), GEN_TYPE_UD);
      if (insn.dstNum == 4) {
        tmp = GenRegister::retype(ra->genReg(insn.dst(3)), GEN_TYPE_UD);
      } else {
        GBE_ASSERT(insn.dstNum == 3);
        tmp = GenRegister::toUniform(profilingReg[4], GEN_TYPE_UL);
      }
    } else {
      GBE_ASSERT(p->curr.execWidth == 8);
      profilingReg[0] = GenRegister::retype(ra->genReg(insn.src(0)), GEN_TYPE_UD);
      profilingReg[1] = GenRegister::retype(ra->genReg(insn.src(1)), GEN_TYPE_UD);
      profilingReg[2] = GenRegister::retype(ra->genReg(insn.src(2)), GEN_TYPE_UD);
      profilingReg[3] = GenRegister::retype(ra->genReg(insn.src(3)), GEN_TYPE_UD);
      profilingReg[4] = GenRegister::retype(ra->genReg(insn.src(4)), GEN_TYPE_UD);
      if (insn.dstNum == 6) {
        tmp = GenRegister::retype(ra->genReg(insn.dst(5)), GEN_TYPE_UD);
      } else {
        GBE_ASSERT(insn.dstNum == 5);
        tmp = GenRegister::toUniform(profilingReg[4], GEN_TYPE_UL);
      }
    }
    GenRegister tmp0 = GenRegister::toUniform(profilingReg[3], GEN_TYPE_UL);
    GenRegister lastTsReg = GenRegister::toUniform(profilingReg[3], GEN_TYPE_UL);
    lastTsReg = GenRegister::offset(lastTsReg, 0, 2*sizeof(uint64_t));
    GenRegister realClock = GenRegister::offset(lastTsReg, 0, sizeof(uint64_t));

    /* MOV(4)   tmp0<1>:UW	   arf_tm<4,4,1>:UW  */
    p->push(); {
      p->curr.execWidth = 4;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      GenRegister _tmp0 = tmp0;
      _tmp0.type = GEN_TYPE_UW;
      _tmp0.hstride = GEN_HORIZONTAL_STRIDE_1;
      _tmp0.vstride = GEN_VERTICAL_STRIDE_4;
      _tmp0.width = GEN_WIDTH_4;
      p->MOV(_tmp0, tmArf);
    } p->pop();

    /* Calc the time elapsed. */
    // SUB(1)  tmp0<1>:UL  tmp0<1>:UL   lastTS<0,1,0>
    subTimestamps(tmp0, lastTsReg, tmp);

    /* Update the real clock
       ADD(1)   realclock<1>:UL  realclock<1>:UL  tmp0<1>:UL */
    addTimestamps(realClock, tmp0, tmp);

    /* We just record timestamp of the first time this point is reached. If the this point is
       in loop, it can be reached many times. We will not record the later timestamps. The 32bits
       timestamp can represent about 3.2s, one each kernel's execution time should never exceed
       3s. So we just record the low 32 bits.
       CMP.EQ(1)flag0.1	  NULL		tsReg_n<1>:UD  0x0
       (+flag0.1) MOV(1)   tsReg_n<1>:UD  realclock<1>:UD  Just record the low 32bits
       */
    GenRegister tsReg = GenRegister::toUniform(profilingReg[pointNum/8], GEN_TYPE_UD);
    tsReg = GenRegister::offset(tsReg, 0, (pointNum%8)*sizeof(uint32_t));

    p->push(); {
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_EQ, tsReg, GenRegister::immud(0));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->curr.inversePredicate = 0;
      p->MOV(tsReg, GenRegister::retype(GenRegister::retype(realClock, GEN_TYPE_UD), GEN_TYPE_UD));
    } p->pop();

    /* Store the timestamp for next point use.
       MOV(4)   lastTS<1>:UW     arf_tm<4,4,1>:UW  */
    p->push(); {
      p->curr.execWidth = 4;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      GenRegister _lastTsReg = lastTsReg;
      _lastTsReg.type = GEN_TYPE_UW;
      _lastTsReg.hstride = GEN_HORIZONTAL_STRIDE_1;
      _lastTsReg.vstride = GEN_VERTICAL_STRIDE_4;
      _lastTsReg.width = GEN_WIDTH_4;
      p->MOV(_lastTsReg, tmArf);
    } p->pop();
  }

  void GenContext::emitStoreProfilingInstruction(const SelectionInstruction &insn) {
    uint32_t simdType;
    if (this->simdWidth == 16) {
      simdType = ir::ProfilingInfo::ProfilingSimdType16;
    } else if (this->simdWidth == 8) {
      simdType = ir::ProfilingInfo::ProfilingSimdType8;
    } else {
      simdType = ir::ProfilingInfo::ProfilingSimdType1;
      GBE_ASSERT(0);
    }

    p->NOP();
    p->NOP();

    GenRegister tmArf = GenRegister::tm0();
    GenRegister profilingReg[5];
    if (p->curr.execWidth == 16) {
      profilingReg[0] = GenRegister::retype(ra->genReg(insn.src(0)), GEN_TYPE_UD);
      profilingReg[1] = GenRegister::offset(profilingReg[0], 1);
      profilingReg[2] = GenRegister::retype(ra->genReg(insn.src(1)), GEN_TYPE_UD);
      profilingReg[3] = GenRegister::offset(profilingReg[2], 1);
      profilingReg[4] = GenRegister::retype(ra->genReg(insn.src(2)), GEN_TYPE_UD);
    } else {
      GBE_ASSERT(p->curr.execWidth == 8);
      profilingReg[0] = GenRegister::retype(ra->genReg(insn.src(0)), GEN_TYPE_UD);
      profilingReg[1] = GenRegister::retype(ra->genReg(insn.src(1)), GEN_TYPE_UD);
      profilingReg[2] = GenRegister::retype(ra->genReg(insn.src(2)), GEN_TYPE_UD);
      profilingReg[3] = GenRegister::retype(ra->genReg(insn.src(3)), GEN_TYPE_UD);
      profilingReg[4] = GenRegister::retype(ra->genReg(insn.src(4)), GEN_TYPE_UD);
    }
    GenRegister tmp = ra->genReg(insn.dst(0));
    uint32_t profilingType = insn.extra.profilingType;
    uint32_t bti = insn.extra.profilingBTI;
    (void) profilingType;
    GBE_ASSERT(profilingType == 1);
    GenRegister flagReg = GenRegister::flag(insn.state.flag, insn.state.subFlag);
    GenRegister lastTsReg = GenRegister::toUniform(profilingReg[3], GEN_TYPE_UL);
    lastTsReg = GenRegister::offset(lastTsReg, 0, 2*sizeof(uint64_t));
    GenRegister realClock = GenRegister::offset(lastTsReg, 0, sizeof(uint64_t));
    GenRegister tmp0 = GenRegister::toUniform(profilingReg[3], GEN_TYPE_UL);

    /* MOV(4)   tmp0<1>:UW	 arf_tm<4,4,1>:UW  */
    p->push(); {
      p->curr.execWidth = 4;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      GenRegister _tmp0 = tmp0;
      _tmp0.type = GEN_TYPE_UW;
      _tmp0.hstride = GEN_HORIZONTAL_STRIDE_1;
      _tmp0.vstride = GEN_VERTICAL_STRIDE_4;
      _tmp0.width = GEN_WIDTH_4;
      p->MOV(_tmp0, tmArf);
    } p->pop();

    /* Calc the time elapsed. */
    subTimestamps(tmp0, lastTsReg, tmp);
    /* Update the real clock */
    addTimestamps(realClock, tmp0, tmp);

    //the epilog, record the last timestamp and return.
    /* MOV(1)   epilog<1>:UL   realclock<0,1,0>:UL  */
    /* ADD(1)   epilog<1>:UL   prolog<0,1,0>:UL  */
    GenRegister prolog = GenRegister::toUniform(profilingReg[2], GEN_TYPE_UD);
    prolog = GenRegister::offset(prolog, 0, 4*sizeof(uint32_t));
    GenRegister epilog = GenRegister::offset(prolog, 0, 2*sizeof(uint32_t));
    p->push(); {
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->MOV(epilog, GenRegister::retype(realClock, GEN_TYPE_UD));
      p->MOV(GenRegister::offset(epilog, 0, sizeof(uint32_t)),
          GenRegister::offset(GenRegister::retype(realClock, GEN_TYPE_UD), 0, sizeof(uint32_t)));
      addTimestamps(epilog, prolog, tmp);
    } p->pop();

    /* Now, begin to write the results out. */
    // Inc the log items number.
    p->push(); {
      //ptr[0] is the total count of the log items.
      GenRegister sndMsg = GenRegister::retype(tmp, GEN_TYPE_UD);
      sndMsg.width = GEN_WIDTH_8;
      sndMsg.hstride = GEN_HORIZONTAL_STRIDE_1;
      sndMsg.vstride = GEN_VERTICAL_STRIDE_8;
      p->curr.execWidth = 8;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->MOV(sndMsg, GenRegister::immud(0x0));

      GenRegister incRes = GenRegister::offset(sndMsg, 1);
      p->push();
      {
        p->curr.execWidth = 1;
        p->MOV(flagReg, GenRegister::immuw(0x01));
      }
      p->pop();
      p->curr.useFlag(insn.state.flag, insn.state.subFlag);
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->ATOMIC(incRes, GEN_ATOMIC_OP_INC, sndMsg, sndMsg, GenRegister::immud(bti), 1, false);
    } p->pop();

    // Calculate the final addr
    GenRegister addr = GenRegister::retype(tmp, GEN_TYPE_UD);
    addr.width = GEN_WIDTH_8;
    addr.hstride = GEN_HORIZONTAL_STRIDE_1;
    addr.vstride = GEN_VERTICAL_STRIDE_8;
    p->push(); {
      GenRegister offset = GenRegister::offset(addr, 1);

      p->curr.execWidth = 8;
      p->curr.noMask = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->MUL(addr, GenRegister::toUniform(offset, GEN_TYPE_UD),
          GenRegister::immud(sizeof(ir::ProfilingInfo::ProfilingReportItem)));
      p->ADD(addr, addr, GenRegister::immud(4)); // for the counter
      p->curr.execWidth = 1;
      for (int i = 1; i < 8; i++) {
        p->ADD(GenRegister::toUniform(GenRegister::offset(addr, 0, i*sizeof(uint32_t)), GEN_TYPE_UD),
            GenRegister::toUniform(GenRegister::offset(addr, 0, i*sizeof(uint32_t)), GEN_TYPE_UD),
            GenRegister::immud(i*sizeof(uint32_t)));
      }
    } p->pop();

    GenRegister data = GenRegister::offset(addr, 1);
    p->push(); {
      p->curr.execWidth = 8;
      p->curr.noMask = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->MOV(data, profilingReg[4]);
    } p->pop();

    // Write the result out
    p->push(); {
      GenRegister ffid = GenRegister::toUniform(data, GEN_TYPE_UD);
      GenRegister tmp = GenRegister::toUniform(profilingReg[3], GEN_TYPE_UD);
      GenRegister stateReg =  GenRegister::sr(0, 0);
      p->curr.noMask = 1;
      p->curr.execWidth = 1;
      p->MOV(ffid, stateReg);
      p->SHR(ffid, ffid, GenRegister::immud(24));
      p->AND(ffid, ffid, GenRegister::immud(0x0ff));
      p->OR(ffid, ffid, GenRegister::immud(simdType << 4));

      GenRegister genInfo = GenRegister::offset(ffid, 0, 4);
      p->MOV(genInfo, stateReg);
      p->AND(genInfo, genInfo, GenRegister::immud(0x0ff07));
      //The dispatch mask
      stateReg = GenRegister::sr(0, 2);
      p->MOV(tmp, stateReg);
      p->AND(tmp, tmp, GenRegister::immud(0x0000ffff));
      p->SHL(tmp, tmp, GenRegister::immud(16));
      p->OR(genInfo, genInfo, tmp);

      // Write it out.
      p->curr.execWidth = 8;
      p->curr.noMask = 1;
      p->UNTYPED_WRITE(addr, addr, GenRegister::immud(bti), 1, false);
      p->ADD(addr, addr, GenRegister::immud(32));

      // time stamps
      for (int i = 0; i < 3; i++) {
        p->curr.execWidth = 8;
        p->MOV(data, GenRegister::retype(profilingReg[i], GEN_TYPE_UD));
        p->UNTYPED_WRITE(addr, addr, GenRegister::immud(bti), 1, false);
        p->ADD(addr, addr, GenRegister::immud(32));
      }
    } p->pop();
  }

  /* Init value according to WORKGROUP OP
   * Emit assert is invalid combination operation - datatype */
  static void wgOpInitValue(GenEncoder *p, GenRegister dataReg, uint32_t wg_op)
  {

    if (wg_op == ir::WORKGROUP_OP_ALL)
    {
      if (dataReg.type == GEN_TYPE_D
          || dataReg.type == GEN_TYPE_UD)
        p->MOV(dataReg, GenRegister::immd(0xFFFFFFFF));
      else if(dataReg.type == GEN_TYPE_L ||
          dataReg.type == GEN_TYPE_UL)
        p->MOV(dataReg, GenRegister::immint64(0xFFFFFFFFFFFFFFFFL));
      else
        GBE_ASSERT(0); /* unsupported data-type */
    }

    else if(wg_op == ir::WORKGROUP_OP_ANY
      || wg_op == ir::WORKGROUP_OP_REDUCE_ADD
      || wg_op == ir::WORKGROUP_OP_INCLUSIVE_ADD
      || wg_op == ir::WORKGROUP_OP_EXCLUSIVE_ADD)
    {
      if (dataReg.type == GEN_TYPE_D)
        p->MOV(dataReg, GenRegister::immd(0x0));
      else if (dataReg.type == GEN_TYPE_UD)
        p->MOV(dataReg, GenRegister::immud(0x0));
      else if (dataReg.type == GEN_TYPE_F)
        p->MOV(dataReg, GenRegister::immf(0x0));
      else if (dataReg.type == GEN_TYPE_L)
        p->MOV(dataReg, GenRegister::immint64(0x0));
      else if (dataReg.type == GEN_TYPE_UL)
        p->MOV(dataReg, GenRegister::immuint64(0x0));
      else if (dataReg.type == GEN_TYPE_W)
        p->MOV(dataReg, GenRegister::immw(0x0));
      else if (dataReg.type == GEN_TYPE_UW)
        p->MOV(dataReg, GenRegister::immuw(0x0));
      else
        GBE_ASSERT(0); /* unsupported data-type */
    }

    else if(wg_op == ir::WORKGROUP_OP_REDUCE_MIN
      || wg_op == ir::WORKGROUP_OP_INCLUSIVE_MIN
      || wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MIN)
    {
      if (dataReg.type == GEN_TYPE_D)
        p->MOV(dataReg, GenRegister::immd(0x7FFFFFFF));
      else if (dataReg.type == GEN_TYPE_UD)
        p->MOV(dataReg, GenRegister::immud(0xFFFFFFFF));
      else if (dataReg.type == GEN_TYPE_F)
        p->MOV(GenRegister::retype(dataReg, GEN_TYPE_UD), GenRegister::immud(0x7F800000));
      else if (dataReg.type == GEN_TYPE_L)
        p->MOV(dataReg, GenRegister::immint64(0x7FFFFFFFFFFFFFFFL));
      else if (dataReg.type == GEN_TYPE_UL)
        p->MOV(dataReg, GenRegister::immuint64(0xFFFFFFFFFFFFFFFFL));
      else if (dataReg.type == GEN_TYPE_W)
        p->MOV(dataReg, GenRegister::immw(0x7FFF));
      else if (dataReg.type == GEN_TYPE_UW)
        p->MOV(dataReg, GenRegister::immuw(0xFFFF));
      else
        GBE_ASSERT(0); /* unsupported data-type */
    }

    else if(wg_op == ir::WORKGROUP_OP_REDUCE_MAX
      || wg_op == ir::WORKGROUP_OP_INCLUSIVE_MAX
      || wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MAX)
    {
      if (dataReg.type == GEN_TYPE_D)
        p->MOV(dataReg, GenRegister::immd(0x80000000));
      else if (dataReg.type == GEN_TYPE_UD)
        p->MOV(dataReg, GenRegister::immud(0x0));
      else if (dataReg.type == GEN_TYPE_F)
        p->MOV(GenRegister::retype(dataReg, GEN_TYPE_UD), GenRegister::immud(0xFF800000));
      else if (dataReg.type == GEN_TYPE_L)
        p->MOV(dataReg, GenRegister::immint64(0x8000000000000000L));
      else if (dataReg.type == GEN_TYPE_UL)
        p->MOV(dataReg, GenRegister::immuint64(0x0));
      else if (dataReg.type == GEN_TYPE_W)
        p->MOV(dataReg, GenRegister::immw(0x8000));
      else if (dataReg.type == GEN_TYPE_UW)
        p->MOV(dataReg, GenRegister::immuw(0x0));
      else
        GBE_ASSERT(0); /* unsupported data-type */
    }

    /* unsupported operation */
    else
      GBE_ASSERT(0);
  }

  /* Perform WORKGROUP OP on 2 input elements (registers) */
  static void wgOpPerform(GenRegister dst,
                         GenRegister src1,
                         GenRegister src2,
                         uint32_t wg_op,
                         GenEncoder *p)
  {
    /* perform OP REDUCE on 2 elements */
    if (wg_op == ir::WORKGROUP_OP_ANY)
      p->OR(dst, src1, src2);
    else if (wg_op == ir::WORKGROUP_OP_ALL)
      p->AND(dst, src1, src2);
    else if(wg_op == ir::WORKGROUP_OP_REDUCE_ADD)
      p->ADD(dst, src1, src2);
    else if(wg_op == ir::WORKGROUP_OP_REDUCE_MIN)
      p->SEL_CMP(GEN_CONDITIONAL_LE, dst, src1, src2);
    else if(wg_op == ir::WORKGROUP_OP_REDUCE_MAX)
      p->SEL_CMP(GEN_CONDITIONAL_GE, dst, src1, src2);

    /* perform OP SCAN INCLUSIVE on 2 elements */
    else if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_ADD)
      p->ADD(dst, src1, src2);
    else if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_MIN)
      p->SEL_CMP(GEN_CONDITIONAL_LE, dst, src1, src2);
    else if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_MAX)
      p->SEL_CMP(GEN_CONDITIONAL_GE, dst, src1, src2);

    /* perform OP SCAN EXCLUSIVE on 2 elements */
    else if(wg_op == ir::WORKGROUP_OP_EXCLUSIVE_ADD)
      p->ADD(dst, src1, src2);
    else if(wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MIN)
      p->SEL_CMP(GEN_CONDITIONAL_LE, dst, src1, src2);
    else if(wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MAX)
      p->SEL_CMP(GEN_CONDITIONAL_GE, dst, src1, src2);

    else
      GBE_ASSERT(0);
  }

  static void wgOpPerformThread(GenRegister threadDst,
                                  GenRegister inputVal,
                                  GenRegister threadExchangeData,
                                   GenRegister resultVal,
                                   uint32_t simd,
                                   uint32_t wg_op,
                                   GenEncoder *p)
  {
   p->push();
   p->curr.predicate = GEN_PREDICATE_NONE;
   p->curr.noMask = 1;
   p->curr.execWidth = 1;

   /* setting the type */
   resultVal = GenRegister::retype(resultVal, inputVal.type);
   threadDst = GenRegister::retype(threadDst, inputVal.type);
   threadExchangeData = GenRegister::retype(threadExchangeData, inputVal.type);

   vector<GenRegister> input;
   vector<GenRegister> result;

   /* for workgroup all and any we can use simd_all/any for each thread */
   if (wg_op == ir::WORKGROUP_OP_ALL || wg_op == ir::WORKGROUP_OP_ANY) {
     GenRegister constZero = GenRegister::immuw(0);
     GenRegister flag01 = GenRegister::flag(0, 1);

     p->push();
     {
       p->curr.predicate = GEN_PREDICATE_NONE;
       p->curr.noMask = 1;
       p->curr.execWidth = simd;
       p->MOV(resultVal, GenRegister::immud(1));
       p->curr.execWidth = 1;
       if (wg_op == ir::WORKGROUP_OP_ALL)
         p->MOV(flag01, GenRegister::immw(-1));
       else
         p->MOV(flag01, constZero);

       p->curr.execWidth = simd;
       p->curr.noMask = 0;

       p->curr.flag = 0;
       p->curr.subFlag = 1;
       p->CMP(GEN_CONDITIONAL_NEQ, inputVal, constZero);

       if (p->curr.execWidth == 16)
         if (wg_op == ir::WORKGROUP_OP_ALL)
           p->curr.predicate = GEN_PREDICATE_ALIGN1_ALL16H;
         else
           p->curr.predicate = GEN_PREDICATE_ALIGN1_ANY16H;
       else if (p->curr.execWidth == 8)
         if (wg_op == ir::WORKGROUP_OP_ALL)
           p->curr.predicate = GEN_PREDICATE_ALIGN1_ALL8H;
         else
          p->curr.predicate = GEN_PREDICATE_ALIGN1_ANY8H;
       else
         NOT_IMPLEMENTED;
       p->SEL(threadDst, resultVal, constZero);
       p->SEL(threadExchangeData, resultVal, constZero);
     }
     p->pop();
   } else {
     if (inputVal.hstride == GEN_HORIZONTAL_STRIDE_0) {
       p->MOV(threadExchangeData, inputVal);
       p->pop();
       return;
     }

     /* init thread data to min/max/null values */
     p->push(); {
       p->curr.execWidth = simd;
       wgOpInitValue(p, threadExchangeData, wg_op);
       p->MOV(resultVal, inputVal);
     } p->pop();

     GenRegister resultValSingle = resultVal;
     resultValSingle.hstride = GEN_HORIZONTAL_STRIDE_0;
     resultValSingle.vstride = GEN_VERTICAL_STRIDE_0;
     resultValSingle.width = GEN_WIDTH_1;

     GenRegister inputValSingle = inputVal;
     inputValSingle.hstride = GEN_HORIZONTAL_STRIDE_0;
     inputValSingle.vstride = GEN_VERTICAL_STRIDE_0;
     inputValSingle.width = GEN_WIDTH_1;


     /* make an array of registers for easy accesing */
     for(uint32_t i = 0; i < simd; i++){
       /* add all resultVal offset reg positions from list */
       result.push_back(resultValSingle);
       input.push_back(inputValSingle);

       /* move to next position */
       resultValSingle.subnr += typeSize(resultValSingle.type);
       if (resultValSingle.subnr == 32) {
           resultValSingle.subnr = 0;
           resultValSingle.nr++;
       }
       /* move to next position */
       inputValSingle.subnr += typeSize(inputValSingle.type);
       if (inputValSingle.subnr == 32) {
           inputValSingle.subnr = 0;
           inputValSingle.nr++;
       }
     }

     uint32_t start_i = 0;
     if( wg_op == ir::WORKGROUP_OP_REDUCE_ADD ||
         wg_op == ir::WORKGROUP_OP_REDUCE_MIN ||
         wg_op == ir::WORKGROUP_OP_REDUCE_MAX ||
         wg_op == ir::WORKGROUP_OP_INCLUSIVE_ADD ||
         wg_op == ir::WORKGROUP_OP_INCLUSIVE_MIN ||
         wg_op == ir::WORKGROUP_OP_INCLUSIVE_MAX) {
       p->MOV(result[0], input[0]);
       start_i = 1;
     }

     else if(wg_op == ir::WORKGROUP_OP_EXCLUSIVE_ADD ||
         wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MIN ||
         wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MAX) {
       p->MOV(result[1], input[0]);
       start_i = 2;
     }

     /* algorithm workgroup */
     for (uint32_t i = start_i; i < simd; i++)
     {
       if( wg_op == ir::WORKGROUP_OP_REDUCE_ADD ||
           wg_op == ir::WORKGROUP_OP_REDUCE_MIN ||
           wg_op == ir::WORKGROUP_OP_REDUCE_MAX)
         wgOpPerform(result[0], result[0], input[i], wg_op, p);

       else if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_ADD ||
           wg_op == ir::WORKGROUP_OP_INCLUSIVE_MIN ||
           wg_op == ir::WORKGROUP_OP_INCLUSIVE_MAX)
         wgOpPerform(result[i], result[i - 1], input[i], wg_op, p);

       else if(wg_op == ir::WORKGROUP_OP_EXCLUSIVE_ADD ||
           wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MIN ||
           wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MAX)
         wgOpPerform(result[i], result[i - 1], input[i - 1], wg_op, p);

       else
         GBE_ASSERT(0);
     }
   }

   if( wg_op == ir::WORKGROUP_OP_REDUCE_ADD ||
       wg_op == ir::WORKGROUP_OP_REDUCE_MIN ||
       wg_op == ir::WORKGROUP_OP_REDUCE_MAX)
   {
     p->curr.execWidth = simd;
     /* value exchanged with other threads */
     p->MOV(threadExchangeData, result[0]);
     /* partial result thread */
     p->MOV(threadDst, result[0]);
   }
   else if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_ADD ||
       wg_op == ir::WORKGROUP_OP_INCLUSIVE_MIN ||
       wg_op == ir::WORKGROUP_OP_INCLUSIVE_MAX)
   {
     p->curr.execWidth = simd;
     /* value exchanged with other threads */
     p->MOV(threadExchangeData, result[simd - 1]);
     /* partial result thread */
     p->MOV(threadDst, resultVal);
   }
   else if(wg_op == ir::WORKGROUP_OP_EXCLUSIVE_ADD ||
       wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MIN ||
       wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MAX)
   {
     p->curr.execWidth = 1;
     /* set result[0] to min/max/null */
     wgOpInitValue(p, result[0], wg_op);

     p->curr.execWidth = simd;
     /* value exchanged with other threads */
     wgOpPerform(threadExchangeData, result[simd - 1], input[simd - 1], wg_op, p);
     /* partial result thread */
     p->MOV(threadDst, resultVal);
   }

   p->pop();
 }

/**
 * WORKGROUP OP: ALL, ANY, REDUCE, SCAN INCLUSIVE, SCAN EXCLUSIVE
 *
 * Implementation:
 * 1. All the threads first perform the workgroup op value for the
 * allocated work-items. SIMD16=> 16 work-items allocated for each thread
 * 2. Each thread writes the partial result in shared local memory using threadId
 * 3. After a barrier, each thread will read in chunks of 1-4 elements,
 * the shared local memory region, using a loop based on the thread num value (threadN)
 * 4. Each thread computes the final value individually
 *
 * Optimizations:
 * Performance is given by chunk read. If threads read in chunks of 4 elements
 * the performance is increase 2-3x times compared to chunks of 1 element.
 */
  void GenContext::emitWorkGroupOpInstruction(const SelectionInstruction &insn){
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister tmp = GenRegister::retype(ra->genReg(insn.dst(1)), dst.type);
    const GenRegister theVal = GenRegister::retype(ra->genReg(insn.src(2)), dst.type);
    GenRegister threadData = ra->genReg(insn.src(3));
    GenRegister partialData = GenRegister::toUniform(threadData, dst.type);
    GenRegister threadId = ra->genReg(insn.src(0));
    GenRegister threadLoop = ra->genReg(insn.src(1));
    GenRegister barrierId = ra->genReg(GenRegister::ud1grf(ir::ocl::barrierid));
    GenRegister localBarrier = ra->genReg(insn.src(5));

    uint32_t wg_op = insn.extra.wgop.workgroupOp;
    uint32_t simd = p->curr.execWidth;
    int32_t jip0, jip1;

    /* masked elements should be properly set to init value */
    p->push(); {
      p->curr.noMask = 1;
      wgOpInitValue(p, tmp, wg_op);
      p->curr.noMask = 0;
      p->MOV(tmp, theVal);
      p->curr.noMask = 1;
      p->MOV(theVal, tmp);
    } p->pop();

    threadId = GenRegister::toUniform(threadId, GEN_TYPE_UD);

    /* use of continuous GRF allocation from insn selection */
    GenRegister msg = GenRegister::retype(ra->genReg(insn.dst(2)), dst.type);
    GenRegister msgSlmOff = GenRegister::retype(ra->genReg(insn.src(4)), GEN_TYPE_UD);
    GenRegister msgAddr = GenRegister::retype(msg, GEN_TYPE_UD);
    GenRegister msgData = GenRegister::retype(ra->genReg(insn.dst(3)), dst.type);

    /* do some calculation within each thread */
    wgOpPerformThread(dst, theVal, threadData, tmp, simd, wg_op, p);

    p->curr.execWidth = simd;
    p->MOV(theVal, dst);
    threadData = GenRegister::toUniform(threadData, dst.type);

    /* store thread count for future use on read/write to SLM */
    if (wg_op == ir::WORKGROUP_OP_ANY ||
      wg_op == ir::WORKGROUP_OP_ALL ||
      wg_op == ir::WORKGROUP_OP_REDUCE_ADD ||
      wg_op == ir::WORKGROUP_OP_REDUCE_MIN ||
      wg_op == ir::WORKGROUP_OP_REDUCE_MAX)
    {
      threadLoop = GenRegister::retype(tmp, GEN_TYPE_D);
      p->MOV(threadLoop, ra->genReg(GenRegister::ud1grf(ir::ocl::threadn)));
    }
    else if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_ADD ||
      wg_op == ir::WORKGROUP_OP_INCLUSIVE_MIN ||
      wg_op == ir::WORKGROUP_OP_INCLUSIVE_MAX ||
      wg_op == ir::WORKGROUP_OP_EXCLUSIVE_ADD ||
      wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MIN ||
      wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MAX)
    {
      threadLoop = GenRegister::retype(tmp, GEN_TYPE_D);
      p->MOV(threadLoop, ra->genReg(GenRegister::ud1grf(ir::ocl::threadid)));
    }

    /* all threads write the partial results to SLM memory */
    if(dst.type == GEN_TYPE_UL || dst.type == GEN_TYPE_L)
    {
      GenRegister threadDataL = GenRegister::retype(threadData, GEN_TYPE_D);
      GenRegister threadDataH = threadDataL.offset(threadDataL, 0, 4);
      GenRegister msgDataL = GenRegister::retype(msgData, GEN_TYPE_D);
      GenRegister msgDataH = msgDataL.offset(msgDataL, 1);
      p->curr.execWidth = 8;
      p->MOV(msgDataL, threadDataL);
      p->MOV(msgDataH, threadDataH);

      p->MUL(msgAddr, threadId, GenRegister::immd(0x8));
      p->ADD(msgAddr, msgAddr, msgSlmOff);
      p->UNTYPED_WRITE(msg, msg, GenRegister::immw(0xFE), 2, false);
    }
    else
    {
      p->curr.execWidth = 8;
      p->MOV(msgData, threadData);
      p->MUL(msgAddr, threadId, GenRegister::immd(0x4));
      p->ADD(msgAddr, msgAddr, msgSlmOff);
      p->UNTYPED_WRITE(msg, msg, GenRegister::immw(0xFE), 1, false);
    }

    /* init partialData register, it will hold the final result */
    wgOpInitValue(p, partialData, wg_op);

    /* add call to barrier */
    p->push();
      p->curr.execWidth = 8;
      p->curr.physicalFlag = 0;
      p->curr.noMask = 1;
      p->AND(localBarrier, barrierId, GenRegister::immud(0x0f000000));
      p->BARRIER(localBarrier);
      p->curr.execWidth = 1;
      p->WAIT();
    p->pop();

    /* perform a loop, based on thread count (which is now multiple of 4) */
    p->push();{
      jip0 = p->n_instruction();

      /* read in chunks of 4 to optimize SLM reads and reduce SEND messages */
      if(dst.type == GEN_TYPE_UL || dst.type == GEN_TYPE_L)
      {
        p->curr.execWidth = 8;
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->ADD(threadLoop, threadLoop, GenRegister::immd(-1));
        p->MUL(msgAddr, threadLoop, GenRegister::immd(0x8));
        p->ADD(msgAddr, msgAddr, msgSlmOff);
        p->UNTYPED_READ(msgData, msgAddr, GenRegister::immw(0xFE), 2);

        GenRegister msgDataL = msgData.retype(msgData.offset(msgData, 0, 4), GEN_TYPE_D);
        GenRegister msgDataH = msgData.retype(msgData.offset(msgData, 1, 4), GEN_TYPE_D);
        msgDataL.hstride = 2;
        msgDataH.hstride = 2;
        p->MOV(msgDataL, msgDataH);

        /* perform operation, partialData will hold result */
        wgOpPerform(partialData, partialData, msgData.offset(msgData, 0), wg_op, p);
      }
      else
      {
        p->curr.execWidth = 8;
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->ADD(threadLoop, threadLoop, GenRegister::immd(-1));
        p->MUL(msgAddr, threadLoop, GenRegister::immd(0x4));
        p->ADD(msgAddr, msgAddr, msgSlmOff);
        p->UNTYPED_READ(msgData, msgAddr, GenRegister::immw(0xFE), 1);

        /* perform operation, partialData will hold result */
        wgOpPerform(partialData, partialData, msgData.offset(msgData, 0), wg_op, p);
      }

      /* while threadN is not 0, cycle read SLM / update value */
      p->curr.noMask = 1;
      p->curr.flag = 0;
      p->curr.subFlag = 1;
      p->CMP(GEN_CONDITIONAL_G, threadLoop, GenRegister::immd(0x0));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      jip1 = p->n_instruction();
      p->JMPI(GenRegister::immud(0));
      p->patchJMPI(jip1, jip0 - jip1, 0);
    } p->pop();

    if(wg_op == ir::WORKGROUP_OP_ANY ||
      wg_op == ir::WORKGROUP_OP_ALL ||
      wg_op == ir::WORKGROUP_OP_REDUCE_ADD ||
      wg_op == ir::WORKGROUP_OP_REDUCE_MIN ||
      wg_op == ir::WORKGROUP_OP_REDUCE_MAX)
    {
      /* save result to final register location dst */
      p->curr.execWidth = simd;
      p->MOV(dst, partialData);
    }
    else
    {
      /* save result to final register location dst */
      p->curr.execWidth = simd;

      if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_ADD
          || wg_op == ir::WORKGROUP_OP_EXCLUSIVE_ADD)
        p->ADD(dst, dst, partialData);
      else if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_MIN
        || wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MIN)
      {
        /* workaround QW datatype on CMP */
        if(dst.type == GEN_TYPE_UL || dst.type == GEN_TYPE_L){
          p->push();
            p->curr.execWidth = 8;
            p->SEL_CMP(GEN_CONDITIONAL_LE, dst, dst, partialData);
            if (simd == 16) {
              p->curr.execWidth = 8;
              p->curr.quarterControl = GEN_COMPRESSION_Q2;
              p->SEL_CMP(GEN_CONDITIONAL_LE, GenRegister::Qn(dst, 1),
                         GenRegister::Qn(dst, 1), GenRegister::Qn(partialData, 1));
            }
          p->pop();
        } else
          p->SEL_CMP(GEN_CONDITIONAL_LE, dst, dst, partialData);
      }
      else if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_MAX
        || wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MAX)
      {
        /* workaround QW datatype on CMP */
        if(dst.type == GEN_TYPE_UL || dst.type == GEN_TYPE_L){
          p->push();
            p->curr.execWidth = 8;
            p->SEL_CMP(GEN_CONDITIONAL_GE, dst, dst, partialData);
            if (simd == 16) {
              p->curr.execWidth = 8;
              p->curr.quarterControl = GEN_COMPRESSION_Q2;
              p->SEL_CMP(GEN_CONDITIONAL_GE, GenRegister::Qn(dst, 1),
                         GenRegister::Qn(dst, 1), GenRegister::Qn(partialData, 1));
            }
          p->pop();
        } else
          p->SEL_CMP(GEN_CONDITIONAL_GE, dst, dst, partialData);
      }
    }

    /* corner cases for threads 0 */
    if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_ADD ||
      wg_op == ir::WORKGROUP_OP_INCLUSIVE_MIN ||
      wg_op == ir::WORKGROUP_OP_INCLUSIVE_MAX ||
      wg_op == ir::WORKGROUP_OP_EXCLUSIVE_ADD ||
      wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MIN ||
      wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MAX)
    {
      p->push();{
        p->curr.flag = 0;
        p->curr.subFlag = 1;
        p->CMP(GEN_CONDITIONAL_EQ, threadId, GenRegister::immd(0x0));
        p->curr.predicate = GEN_PREDICATE_NORMAL;

        p->curr.execWidth = simd;
        p->MOV(dst, theVal);
      } p->pop();
    }
  }

  void GenContext::emitSubGroupOpInstruction(const SelectionInstruction &insn){
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister tmp = GenRegister::retype(ra->genReg(insn.dst(1)), dst.type);
    const GenRegister theVal = GenRegister::retype(ra->genReg(insn.src(0)), dst.type);
    GenRegister threadData = ra->genReg(insn.src(1));

    uint32_t wg_op = insn.extra.wgop.workgroupOp;
    uint32_t simd = p->curr.execWidth;

    /* masked elements should be properly set to init value */
    p->push(); {
      p->curr.noMask = 1;
      wgOpInitValue(p, tmp, wg_op);
      p->curr.noMask = 0;
      p->MOV(tmp, theVal);
      p->curr.noMask = 1;
      p->MOV(theVal, tmp);
    } p->pop();

    /* do some calculation within each thread */
    wgOpPerformThread(dst, theVal, threadData, tmp, simd, wg_op, p);
  }

  void GenContext::emitPrintfLongInstruction(GenRegister& addr, GenRegister& data,
                                             GenRegister& src, uint32_t bti, bool useSends) {
    p->MOV(GenRegister::retype(data, GEN_TYPE_UD), src.bottom_half());
    p->UNTYPED_WRITE(addr, data, GenRegister::immud(bti), 1, useSends);
    p->ADD(addr, addr, GenRegister::immud(sizeof(uint32_t)));

    p->MOV(GenRegister::retype(data, GEN_TYPE_UD), src.top_half(this->simdWidth));
    p->UNTYPED_WRITE(addr, data, GenRegister::immud(bti), 1, useSends);
    p->ADD(addr, addr, GenRegister::immud(sizeof(uint32_t)));
  }

  void GenContext::emitPrintfInstruction(const SelectionInstruction &insn) {
    const GenRegister tmp0 = ra->genReg(insn.dst(0));
    const GenRegister tmp1 = ra->genReg(insn.dst(1));
    GenRegister src;
    uint32_t srcNum = insn.srcNum;

    GenRegister addr = GenRegister::retype(tmp0, GEN_TYPE_UD);
    GenRegister data = GenRegister::retype(tmp1, GEN_TYPE_UD);
    bool useSends = insn.extra.printfSplitSend;

    if (!insn.extra.continueFlag) {
      p->push(); {
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->curr.noMask = 1;
        //ptr[0] is the total count of the log size.
        p->MOV(addr, GenRegister::immud(0));
        p->MOV(data, GenRegister::immud(insn.extra.printfSize + 12));
      } p->pop();

      p->ATOMIC(addr, GEN_ATOMIC_OP_ADD, addr, data, GenRegister::immud(insn.extra.printfBTI), 2, useSends);
      /* Write out the header. */
      p->MOV(data, GenRegister::immud(0xAABBCCDD));
      p->UNTYPED_WRITE(addr, data, GenRegister::immud(insn.extra.printfBTI), 1, useSends);

      p->ADD(addr, addr, GenRegister::immud(sizeof(uint32_t)));
      p->MOV(data, GenRegister::immud(insn.extra.printfSize + 12));
      p->UNTYPED_WRITE(addr, data, GenRegister::immud(insn.extra.printfBTI), 1, useSends);

      p->ADD(addr, addr, GenRegister::immud(sizeof(uint32_t)));
      p->MOV(data, GenRegister::immud(insn.extra.printfNum));
      p->UNTYPED_WRITE(addr, data, GenRegister::immud(insn.extra.printfBTI), 1, useSends);

      p->ADD(addr, addr, GenRegister::immud(sizeof(uint32_t)));
    }

    // Now, store out every parameter.
    for(uint32_t i = 0; i < srcNum; i++) {
      src = ra->genReg(insn.src(i));
      if (src.type == GEN_TYPE_UD || src.type == GEN_TYPE_D || src.type == GEN_TYPE_F) {
        p->MOV(GenRegister::retype(data, src.type), src);
        p->UNTYPED_WRITE(addr, data, GenRegister::immud(insn.extra.printfBTI), 1, useSends);
        p->ADD(addr, addr, GenRegister::immud(sizeof(uint32_t)));
      } else if (src.type == GEN_TYPE_B || src.type == GEN_TYPE_UB ) {
        p->MOV(GenRegister::retype(data, GEN_TYPE_UD), src);
        p->UNTYPED_WRITE(addr, data, GenRegister::immud(insn.extra.printfBTI), 1, useSends);
        p->ADD(addr, addr, GenRegister::immud(sizeof(uint32_t)));
      } else if (src.type == GEN_TYPE_L || src.type == GEN_TYPE_UL ) {
        emitPrintfLongInstruction(addr, data, src, insn.extra.printfBTI, useSends);
      }
    }
  }

  void GenContext::setA0Content(uint16_t new_a0[16], uint16_t max_offset, int sz) {
    if (sz == 0)
      sz = 8;
    GBE_ASSERT(sz%4 == 0);
    GBE_ASSERT(new_a0[0] >= 0 && new_a0[0] < 4096);

    p->push();
    p->curr.execWidth = 1;
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.noMask = 1;
    for (int i = 0; i < sz/2; i++) {
      p->MOV(GenRegister::retype(GenRegister::addr1(i*2), GEN_TYPE_UD),
             GenRegister::immud(new_a0[i*2 + 1] << 16 | new_a0[i*2]));
    }
    p->pop();
  }

  void GenContext::emitOBReadInstruction(const SelectionInstruction &insn) {
    const GenRegister header = ra->genReg(insn.src(0));
    const GenRegister tmp = ra->genReg(insn.dst(0));
    const uint32_t bti = insn.getbti();
    const uint32_t ow_size = insn.extra.elem;
    bool isA64 = bti == 255;
    if (isA64)
       p->OBREADA64(tmp, header, bti, ow_size);
    else
       p->OBREAD(tmp, header, bti, ow_size);
  }

  void GenContext::emitOBWriteInstruction(const SelectionInstruction &insn) {
    const GenRegister header = ra->genReg(insn.src(0));
    const GenRegister data = ra->genReg(insn.src(1));
    const uint32_t bti = insn.getbti();
    const uint32_t ow_size = insn.extra.elem;
    bool isA64 = bti == 255;
    if (isA64)
       p->OBWRITEA64(header, bti, ow_size);
    else
       p->OBWRITE(header, data, bti, ow_size, insn.extra.splitSend);
  }

  void GenContext::emitMBReadInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister header = ra->genReg(insn.src(0));
    const size_t response_size = insn.extra.elem;
    p->MBREAD(dst, header, insn.getbti(), response_size);
  }

  void GenContext::emitMBWriteInstruction(const SelectionInstruction &insn) {
    const GenRegister header = ra->genReg(insn.dst(0));
    const GenRegister data = ra->genReg(insn.dst(1));
    const size_t data_size = insn.extra.elem;
    p->MBWRITE(header, data, insn.getbti(), data_size, insn.extra.splitSend);
  }

  BVAR(OCL_OUTPUT_REG_ALLOC, false);
  BVAR(OCL_OUTPUT_ASM, false);

  void GenContext::allocCurbeReg(ir::Register reg) {
    uint32_t regSize;
    gbe_curbe_type curbeType;
    int subType;
    this->getRegPayloadType(reg, curbeType, subType);
    regSize = this->ra->getRegSize(reg);
    insertCurbeReg(reg, newCurbeEntry(curbeType, subType, regSize));
    /* Need to patch the image information registers. */
    if (curbeType == GBE_CURBE_IMAGE_INFO) {
      std::sort(kernel->patches.begin(), kernel->patches.end());
      uint32_t offset = kernel->getCurbeOffset(GBE_CURBE_IMAGE_INFO, subType);
      fn.getImageSet()->appendInfo(static_cast<ir::ImageInfoKey>(subType), offset);
    }
  }

  void GenContext::buildPatchList() {

    // After this point the vector is immutable. Sorting it will make
    // research faster
    std::sort(kernel->patches.begin(), kernel->patches.end());
    kernel->curbeSize = ALIGN(kernel->curbeSize, GEN_REG_SIZE);
  }

  BVAR(OCL_OUTPUT_SEL_IR, false);
  BVAR(OCL_OPTIMIZE_SEL_IR, true);
  bool GenContext::emitCode(void) {
    GenKernel *genKernel = static_cast<GenKernel*>(this->kernel);
    sel->select();
    if (OCL_OPTIMIZE_SEL_IR)
      sel->optimize();
    sel->addID();
    if (OCL_OUTPUT_SEL_IR)
      outputSelectionIR(*this, this->sel, genKernel->getName());
    schedulePreRegAllocation(*this, *this->sel);
    sel->addID();
    if (UNLIKELY(ra->allocate(*this->sel) == false))
      return false;
    schedulePostRegAllocation(*this, *this->sel);
    if (OCL_OUTPUT_REG_ALLOC)
      ra->outputAllocation();
    if (inProfilingMode) { // add the profiling prolog before do anything.
      this->profilingProlog();
    }
    this->emitStackPointer();
    this->clearFlagRegister();
    this->emitSLMOffset();
    this->emitInstructionStream();
    if (this->patchBranches() == false)
      return false;
    genKernel->insnNum = p->store.size();
    genKernel->insns = GBE_NEW_ARRAY_NO_ARG(GenInstruction, genKernel->insnNum);
    std::memcpy(genKernel->insns, &p->store[0], genKernel->insnNum * sizeof(GenInstruction));
    if (OCL_OUTPUT_ASM)
      outputAssembly(stdout, genKernel);

    if (OCL_DEBUGINFO)
      outputAssembly(stdout, genKernel);

    if (this->asmFileName) {
      FILE *asmDumpStream = fopen(this->asmFileName, "a");
      if (asmDumpStream) {
        outputAssembly(asmDumpStream, genKernel);
        fclose(asmDumpStream);
      }
    }

    return true;
  }

  Kernel *GenContext::allocateKernel(void) {
    return GBE_NEW(GenKernel, name, deviceID);
  }

  void GenContext::outputAssembly(FILE *file, GenKernel* genKernel) {
    /* get gen version for the instruction compact */
    uint32_t insn_version = 0;
    if (IS_GEN7(deviceID) || IS_GEN75(deviceID))
      insn_version = 7;
    else if (IS_GEN8(deviceID) || IS_GEN9(deviceID))
      insn_version = 8;
    fprintf(file, "%s's disassemble begin:\n", genKernel->getName());
    ir::LabelIndex curLabel = (ir::LabelIndex)0;
    GenCompactInstruction * pCom = NULL;
    GenInstruction insn[2];
    fprintf(file, "  L0:\n");
    for (uint32_t insnID = 0; insnID < genKernel->insnNum; ) {
      if (labelPos.find((ir::LabelIndex)(curLabel + 1))->second == insnID &&
          curLabel < this->getFunction().labelNum()) {
        fprintf(file, "  L%i:\n", curLabel + 1);
        curLabel = (ir::LabelIndex)(curLabel + 1);
        while(labelPos.find((ir::LabelIndex)(curLabel + 1))->second == insnID) {
          fprintf(file, "  L%i:\n", curLabel + 1);
          curLabel = (ir::LabelIndex)(curLabel + 1);
        }
      }

      if (OCL_DEBUGINFO)
        fprintf(file, "[%3i,%3i]", p->storedbg[insnID].line, p->storedbg[insnID].col);

      fprintf(file, "    (%8i)  ", insnID);
      pCom = (GenCompactInstruction*)&p->store[insnID];
      if(pCom->bits1.cmpt_control == 1) {
        decompactInstruction(pCom, &insn, insn_version);
        gen_disasm(file, &insn, deviceID, 1);
        insnID++;
      } else {
        gen_disasm(file, &p->store[insnID], deviceID, 0);
        insnID = insnID + 2;
      }
    }
    fprintf(file, "%s's disassemble end.\n", genKernel->getName());
  }

} /* namespace gbe */


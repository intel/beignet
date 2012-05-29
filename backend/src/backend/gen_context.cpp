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
#include "backend/gen_eu.hpp"
#include "backend/gen_insn_selection.hpp"
#include "backend/gen_reg_allocation.hpp"
#include "ir/function.hpp"
#include "sys/cvar.hpp"
#include <cstring>

namespace gbe
{
  ///////////////////////////////////////////////////////////////////////////
  // Various helper functions
  ///////////////////////////////////////////////////////////////////////////

  INLINE uint32_t getGenCompare(ir::Opcode opcode) {
    using namespace ir;
    switch (opcode) {
      case OP_LE: return GEN_CONDITIONAL_LE;
      case OP_LT: return GEN_CONDITIONAL_L;
      case OP_GE: return GEN_CONDITIONAL_GE;
      case OP_GT: return GEN_CONDITIONAL_G;
      case OP_EQ: return GEN_CONDITIONAL_EQ;
      case OP_NE: return GEN_CONDITIONAL_NEQ;
      default: NOT_SUPPORTED; return 0u;
    };
  }

  ///////////////////////////////////////////////////////////////////////////
  // GenContext implementation
  ///////////////////////////////////////////////////////////////////////////
  GenContext::GenContext(const ir::Unit &unit, const std::string &name) :
    Context(unit, name)
  {
    this->p = GBE_NEW(GenEmitter, simdWidth, 7); // XXX handle more than Gen7
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

  void GenContext::emitUnaryInstruction(const ir::UnaryInstruction &insn) {
    GBE_ASSERT(insn.getOpcode() == ir::OP_MOV);
    p->MOV(ra->genReg(insn.getDst(0)),
           ra->genReg(insn.getSrc(0)));
  }

  void GenContext::emitIntMul32x32(const ir::Instruction &insn,
                                   GenReg dst, GenReg src0, GenReg src1)
  {
    using namespace ir;
    const uint32_t width = p->curr.execWidth;
    p->push();

      // Either left part of the 16-wide register or just a simd 8 register
      dst  = GenReg::retype(dst,  GEN_TYPE_D);
      src0 = GenReg::retype(src0, GEN_TYPE_D);
      src1 = GenReg::retype(src1, GEN_TYPE_D);
      p->curr.execWidth = 8;
      p->curr.quarterControl = GEN_COMPRESSION_Q1;
      p->MUL(GenReg::retype(GenReg::acc(), GEN_TYPE_D), src0, src1);
      p->MACH(GenReg::retype(GenReg::null(), GEN_TYPE_D), src0, src1);
      p->MOV(GenReg::retype(dst, GEN_TYPE_F), GenReg::acc());

      // Right part of the 16-wide register now
      if (width == 16) {
        p->curr.noMask = 1;
        const GenReg nextSrc0 = ra->genRegQn(insn.getSrc(0), 2, TYPE_S32);
        const GenReg nextSrc1 = ra->genRegQn(insn.getSrc(1), 2, TYPE_S32);
        p->MUL(GenReg::retype(GenReg::acc(), GEN_TYPE_D), nextSrc0, nextSrc1);
        p->MACH(GenReg::retype(GenReg::null(), GEN_TYPE_D), nextSrc0, nextSrc1);
        p->curr.quarterControl = GEN_COMPRESSION_Q2;
        p->MOV(GenReg::f8grf(116,0), GenReg::acc());
        p->curr.noMask = 0;
        p->MOV(GenReg::retype(GenReg::next(dst), GEN_TYPE_F), GenReg::f8grf(116,0));
      }

    p->pop();
  }

  void GenContext::emitForwardBranch(const ir::BranchInstruction &insn,
                                     ir::LabelIndex dst,
                                     ir::LabelIndex src)
  {
    using namespace ir;
    const GenReg ip = ra->genReg(ocl::blockip, TYPE_U16);
    const LabelIndex jip = JIPs.find(&insn)->second;

    // We will not emit any jump if we must go the next block anyway
    const BasicBlock *curr = insn.getParent();
    const BasicBlock *next = curr->getNextBlock();
    const LabelIndex nextLabel = next->getLabelIndex();

    // Inefficient code. If the instruction is predicated, we build the flag
    // register from the boolean vector
    if (insn.isPredicated() == true) {
      const GenReg pred = ra->genReg(insn.getPredicateIndex(), TYPE_U16);

      // Reset the flag register
      p->push();
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->curr.execWidth = 1;
        p->curr.noMask = 1;
        p->MOV(GenReg::flag(0,1), pred);
      p->pop();

      // Update the PcIPs
      p->push();
        p->curr.flag = 0;
        p->curr.subFlag = 1;
        p->MOV(ip, GenReg::immuw(uint16_t(dst)));
      p->pop();

      if (nextLabel == jip) return;

      // It is slightly more complicated than for backward jump. We check that
      // all PcIPs are greater than the next block IP to be sure that we can
      // jump
      p->push();
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->curr.flag = 0;
        p->curr.subFlag = 1;
        p->CMP(GEN_CONDITIONAL_G, ip, GenReg::immuw(nextLabel));

        // Branch to the jump target
        this->branchPos.insert(std::make_pair(&insn, p->insnNum));
        if (simdWidth == 8)
          p->curr.predicate = GEN_PREDICATE_ALIGN1_ALL8H;
        else if (simdWidth == 16)
          p->curr.predicate = GEN_PREDICATE_ALIGN1_ALL16H;
        else
          NOT_SUPPORTED;
        p->curr.execWidth = 1;
        p->curr.noMask = 1;
        p->JMPI(GenReg::immd(0));
      p->pop();

    } else {
      // Update the PcIPs
      p->MOV(ip, GenReg::immuw(uint16_t(dst)));

      // Do not emit branch when we go to the next block anyway
      if (nextLabel == jip) return;
      this->branchPos.insert(std::make_pair(&insn, p->insnNum));
      p->push();
        p->curr.execWidth = 1;
        p->curr.noMask = 1;
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->JMPI(GenReg::immd(0));
      p->pop();
    }
  }

  void GenContext::emitBackwardBranch(const ir::BranchInstruction &insn,
                                      ir::LabelIndex dst,
                                      ir::LabelIndex src)
  {
    using namespace ir;
    const GenReg ip = ra->genReg(ocl::blockip, TYPE_U16);
    const BasicBlock &bb = fn.getBlock(src);
    GBE_ASSERT(bb.getNextBlock() != NULL);

    // Inefficient code: we make a GRF to flag conversion
    if (insn.isPredicated() == true) {
      const GenReg pred = ra->genReg(insn.getPredicateIndex(), TYPE_U16);

      // Update the PcIPs for all the branches. Just put the IPs of the next
      // block. Next instruction will properly reupdate the IPs of the lanes
      // that actually take the branch
      const LabelIndex next = bb.getNextBlock()->getLabelIndex();
      p->MOV(ip, GenReg::immuw(uint16_t(next)));

      // Rebuild the flag register by comparing the boolean with 1s
      p->push();
        p->curr.noMask = 1;
        p->curr.execWidth = 1;
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->MOV(GenReg::flag(0,1), pred);
      p->pop();

      p->push();
        p->curr.flag = 0;
        p->curr.subFlag = 1;

        // Re-update the PcIPs for the branches that takes the backward jump
        p->MOV(ip, GenReg::immuw(uint16_t(dst)));

        // Branch to the jump target
        this->branchPos.insert(std::make_pair(&insn, p->insnNum));
        if (simdWidth == 8)
          p->curr.predicate = GEN_PREDICATE_ALIGN1_ANY8H;
        else if (simdWidth == 16)
          p->curr.predicate = GEN_PREDICATE_ALIGN1_ANY16H;
        else
          NOT_SUPPORTED;
        p->curr.execWidth = 1;
        p->curr.noMask = 1;
        p->JMPI(GenReg::immd(0));
      p->pop();

    } else {

      // Update the PcIPs
      p->MOV(ip, GenReg::immuw(uint16_t(dst)));

      // Branch to the jump target
      this->branchPos.insert(std::make_pair(&insn, p->insnNum));
      p->push();
        p->curr.execWidth = 1;
        p->curr.noMask = 1;
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->JMPI(GenReg::immd(0));
      p->pop();
    }
  }

  void GenContext::emitBinaryInstruction(const ir::BinaryInstruction &insn) {
    using namespace ir;
    const Opcode opcode = insn.getOpcode();
    const Type type = insn.getType();
    GenReg dst  = ra->genReg(insn.getDst(0), type);
    GenReg src0 = ra->genReg(insn.getSrc(0), type);
    GenReg src1 = ra->genReg(insn.getSrc(1), type);

    p->push();

    // Boolean values use scalars
    if (this->isScalarOrBool(insn.getDst(0)) == true) {
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
    }

    // Output the binary instruction
    switch (opcode) {
      case OP_ADD: p->ADD(dst, src0, src1); break;
      case OP_SUB: p->ADD(dst, src0, GenReg::negate(src1)); break;
      case OP_AND: p->AND(dst, src0, src1); break;
      case OP_XOR: p->XOR(dst, src0, src1); break;
      case OP_OR:  p->OR(dst, src0,  src1); break;
      case OP_SHL: p->SHL(dst, src0, src1); break;
      case OP_MUL: 
      {
        if (type == TYPE_FLOAT)
          p->MUL(dst, src0, src1);
        else if (type == TYPE_U32 || type == TYPE_S32)
          this->emitIntMul32x32(insn, dst, src0, src1);
        else
          NOT_IMPLEMENTED;
      }
      break;
      case OP_DIV:
      {
        GBE_ASSERT(type == TYPE_FLOAT);
        p->MATH(dst, GEN_MATH_FUNCTION_FDIV, src0, src1);
      }
      break;
      default: NOT_IMPLEMENTED;
    }
    p->pop();
  }

  void GenContext::emitTernaryInstruction(const ir::TernaryInstruction &insn) {
    NOT_IMPLEMENTED;
  }
  void GenContext::emitSelectInstruction(const ir::SelectInstruction &insn) {
    NOT_IMPLEMENTED;
  }
  void GenContext::emitSampleInstruction(const ir::SampleInstruction &insn) {
    NOT_IMPLEMENTED;
  }
  void GenContext::emitTypedWriteInstruction(const ir::TypedWriteInstruction &insn) {
    NOT_IMPLEMENTED;
  }

  void GenContext::emitCompareInstruction(const ir::CompareInstruction &insn) {
    using namespace ir;
    const Opcode opcode = insn.getOpcode();
    const Type type = insn.getType();
    const uint32_t genCmp = getGenCompare(opcode);
    const GenReg dst  = ra->genReg(insn.getDst(0), TYPE_BOOL);
    const GenReg src0 = ra->genReg(insn.getSrc(0), type);
    const GenReg src1 = ra->genReg(insn.getSrc(1), type);

    // Copy the predicate to save it basically
    p->push();
      p->curr.noMask = 1;
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->MOV(GenReg::flag(0,1), GenReg::flag(0,0));
    p->pop();

    // Emit the compare instruction itself
    p->push();
      p->curr.flag = 0;
      p->curr.subFlag = 1;
      p->CMP(genCmp, src0, src1);
    p->pop();

    // We emit an unoptimized code where we store the resulting mask in a GRF
    p->push();
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->MOV(dst, GenReg::flag(0,1));
    p->pop();
  }

  void GenContext::emitConvertInstruction(const ir::ConvertInstruction &insn) {
    using namespace ir;
    const Type dstType = insn.getDstType();
    const Type srcType = insn.getSrcType();
    const RegisterFamily dstFamily = getFamily(dstType);
    const RegisterFamily srcFamily = getFamily(srcType);
    const GenReg dst = ra->genReg(insn.getDst(0), dstType);
    const GenReg src = ra->genReg(insn.getSrc(0), srcType);

    GBE_ASSERT(dstFamily != FAMILY_QWORD && srcFamily != FAMILY_QWORD);

    // We need two instructions to make the conversion
    if (dstFamily != FAMILY_DWORD && srcFamily == FAMILY_DWORD) {
      GenReg unpacked;
      if (dstFamily == FAMILY_WORD) {
        const uint32_t type = TYPE_U16 ? GEN_TYPE_UW : GEN_TYPE_W;
        unpacked = GenReg::unpacked_uw(112, 0);
        unpacked = GenReg::retype(unpacked, type);
      } else {
        const uint32_t type = TYPE_U8 ? GEN_TYPE_UB : GEN_TYPE_B;
        unpacked = GenReg::unpacked_ub(112, 0);
        unpacked = GenReg::retype(unpacked, type);
      }
      p->MOV(unpacked, src);
      p->MOV(dst, unpacked);
    } else
      p->MOV(dst, src);
  }

  void GenContext::emitBranchInstruction(const ir::BranchInstruction &insn) {
    using namespace ir;
    const Opcode opcode = insn.getOpcode();
    if (opcode == OP_RET) {
      p->push();
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->curr.execWidth = 8;
        p->curr.noMask = 1;
        p->MOV(GenReg::f8grf(127,0), GenReg::f8grf(0,0));
        p->EOT(127);
      p->pop();
    } else if (opcode == OP_BRA) {
      const LabelIndex dst = insn.getLabelIndex();
      const LabelIndex src = insn.getParent()->getLabelIndex();

      // We handle foward and backward branches differently
      if (uint32_t(dst) <= uint32_t(src))
        this->emitBackwardBranch(insn, dst, src);
      else
        this->emitForwardBranch(insn, dst, src);
    } else
      NOT_IMPLEMENTED;
  }

  void GenContext::emitLoadImmInstruction(const ir::LoadImmInstruction &insn) {
    using namespace ir;
    const Type type = insn.getType();
    const Immediate imm = insn.getImmediate();
    const GenReg dst = ra->genReg(insn.getDst(0), type);

    switch (type) {
      case TYPE_U32: p->MOV(dst, GenReg::immud(imm.data.u32)); break;
      case TYPE_S32: p->MOV(dst, GenReg::immd(imm.data.s32)); break;
      case TYPE_U16: p->MOV(dst, GenReg::immuw(imm.data.u16)); break;
      case TYPE_S16: p->MOV(dst, GenReg::immw(imm.data.s16)); break;
      case TYPE_U8:  p->MOV(dst, GenReg::immuw(imm.data.u8)); break;
      case TYPE_S8:  p->MOV(dst, GenReg::immw(imm.data.s8)); break;
      case TYPE_FLOAT: p->MOV(dst, GenReg::immf(imm.data.f32)); break;
      default: NOT_SUPPORTED;
    }
  }

  void GenContext::emitUntypedRead(const ir::LoadInstruction &insn, GenReg address)
  {
    using namespace ir;
    const uint32_t valueNum = insn.getValueNum();
    GenReg src;

    // A scalar address register requires to be aligned
    if (isScalarReg(insn.getAddress()) == true) {
      if (this->simdWidth == 8)
        src = GenReg::f8grf(112, 0);
      else
        src = GenReg::f16grf(112, 0);
      p->MOV(src, GenReg::retype(address, GEN_TYPE_F));
    } else
      src = address;

    // Gather of integer is simpler since we may not need to move the
    // destination
    if (valueNum == 1) {
      const GenReg value = ra->genReg(insn.getValue(0));
      p->UNTYPED_READ(value, src, 0, 1);
    }
    // Right now, we just move everything to registers when dealing with
    // int2/3/4
    else {
      p->UNTYPED_READ(GenReg::f8grf(114, 0), src, 0, valueNum);
      if (this->simdWidth == 8) {
        for (uint32_t value = 0; value < valueNum; ++value) {
          const GenReg dst = ra->genReg(insn.getValue(value), TYPE_FLOAT);
          p->MOV(dst, GenReg::f8grf(114+value, 0));
        }
      } else if (this->simdWidth == 16) {
        for (uint32_t value = 0; value < valueNum; ++value) {
          const GenReg dst = ra->genReg(insn.getValue(value), TYPE_FLOAT);
          p->MOV(dst, GenReg::f16grf(114+2*value, 0));
        }
      } else
        NOT_SUPPORTED;
    }
  }

  INLINE uint32_t getByteScatterGatherSize(ir::Type type) {
    using namespace ir;
    switch (type) {
      case TYPE_FLOAT:
      case TYPE_U32:
      case TYPE_S32:
        return GEN_BYTE_SCATTER_DWORD;
      case TYPE_U16:
      case TYPE_S16:
        return GEN_BYTE_SCATTER_WORD;
      case TYPE_U8:
      case TYPE_S8:
        return GEN_BYTE_SCATTER_BYTE;
      default: NOT_SUPPORTED;
        return GEN_BYTE_SCATTER_BYTE;
    }
  }

  void GenContext::emitByteGather(const ir::LoadInstruction &insn,
                                  GenReg address,
                                  GenReg value)
  {
    using namespace ir;
    GBE_ASSERT(insn.getValueNum() == 1);
    const Type type = insn.getValueType();
    const uint32_t elemSize = getByteScatterGatherSize(type);

    // We need a temporary register if we read bytes or words
    GenReg dst = value;
    if (elemSize == GEN_BYTE_SCATTER_WORD ||
        elemSize == GEN_BYTE_SCATTER_BYTE) {
      if (this->simdWidth == 8)
        dst = GenReg::f8grf(113, 0);
      else if (this->simdWidth == 16)
        dst = GenReg::f16grf(114, 0);
      else
        NOT_IMPLEMENTED;
    }

    // Emit the byte gather itself
    if (isScalarReg(insn.getAddress()) == true) {
      if (this->simdWidth == 8) {
        p->MOV(GenReg::f8grf(112, 0), GenReg::retype(address, GEN_TYPE_F));
        p->BYTE_GATHER(dst, GenReg::f8grf(112, 0), 0, elemSize);
      } else if (this->simdWidth == 16) {
        p->MOV(GenReg::f16grf(112, 0), GenReg::retype(address, GEN_TYPE_F));
        p->BYTE_GATHER(dst, GenReg::f16grf(112, 0), 0, elemSize);
      }
    } else
      p->BYTE_GATHER(dst, address, 0, elemSize);

    // Repack bytes or words using a converting mov instruction
    if (elemSize == GEN_BYTE_SCATTER_WORD)
      p->MOV(GenReg::retype(value, GEN_TYPE_UW), GenReg::unpacked_uw(dst.nr, 0));
    else if (elemSize == GEN_BYTE_SCATTER_BYTE)
      p->MOV(GenReg::retype(value, GEN_TYPE_UB), GenReg::unpacked_ub(dst.nr, 0));
  }

  void GenContext::emitLoadInstruction(const ir::LoadInstruction &insn) {
    using namespace ir;
    const GenReg address = ra->genReg(insn.getAddress());
    GBE_ASSERT(insn.getAddressSpace() == MEM_GLOBAL ||
               insn.getAddressSpace() == MEM_PRIVATE);
    GBE_ASSERT(this->isScalarReg(insn.getValue(0)) == false);
    if (insn.isAligned() == true)
      this->emitUntypedRead(insn, address);
    else {
      const GenReg value = ra->genReg(insn.getValue(0));
      this->emitByteGather(insn, address, value);
    }
  }

  void GenContext::emitUntypedWrite(const ir::StoreInstruction &insn)
  {
    using namespace ir;
    const uint32_t srcNum = insn.getSrcNum();
    const uint32_t valueNum = insn.getValueNum();

    // We do it stupidly right now. We just move everything to temporaries
    if (this->simdWidth == 8)
      for (uint32_t src = 0; src < srcNum; ++src) {
        const GenReg reg = ra->genReg(insn.getSrc(src), TYPE_FLOAT);
        p->MOV(GenReg::f8grf(112+src, 0), reg);
      }
    else if (this->simdWidth == 16)
      for (uint32_t src = 0; src < srcNum; ++src) {
        const GenReg reg = ra->genReg(insn.getSrc(src), TYPE_FLOAT);
        p->MOV(GenReg::f16grf(112+2*src, 0), reg);
      }
    else
      NOT_IMPLEMENTED;
    p->UNTYPED_WRITE(GenReg::f8grf(112, 0), 0, valueNum);
  }

  void GenContext::emitByteScatter(const ir::StoreInstruction &insn,
                                   GenReg address,
                                   GenReg value)
  {
    using namespace ir;
    const Type type = insn.getValueType();
    const uint32_t elemSize = getByteScatterGatherSize(type);

    GBE_ASSERT(insn.getValueNum() == 1);
    if (this->simdWidth == 8) {
      p->MOV(GenReg::f8grf(112, 0), GenReg::retype(address, GEN_TYPE_F));
      if (elemSize == GEN_BYTE_SCATTER_DWORD)
        p->MOV(GenReg::f8grf(113, 0), GenReg::retype(value, GEN_TYPE_F));
      else if (elemSize == GEN_BYTE_SCATTER_WORD)
        p->MOV(GenReg::ud8grf(113, 0), GenReg::retype(value, GEN_TYPE_UW));
      else if (elemSize == GEN_BYTE_SCATTER_BYTE)
        p->MOV(GenReg::ud8grf(113, 0), GenReg::retype(value, GEN_TYPE_UB));
      p->BYTE_SCATTER(GenReg::f8grf(112, 0), 1, elemSize);
    } else if (this->simdWidth == 16) {
      p->MOV(GenReg::f16grf(112, 0), GenReg::retype(address, GEN_TYPE_F));
      if (elemSize == GEN_BYTE_SCATTER_DWORD)
        p->MOV(GenReg::f16grf(114, 0), GenReg::retype(value, GEN_TYPE_F));
      else if (elemSize == GEN_BYTE_SCATTER_WORD)
        p->MOV(GenReg::ud16grf(114, 0), GenReg::retype(value, GEN_TYPE_UW));
      else if (elemSize == GEN_BYTE_SCATTER_BYTE)
        p->MOV(GenReg::ud16grf(114, 0), GenReg::retype(value, GEN_TYPE_UB));
      p->BYTE_SCATTER(GenReg::f16grf(112, 0), 1, elemSize);
    } else
      NOT_IMPLEMENTED;
  }

  void GenContext::emitStoreInstruction(const ir::StoreInstruction &insn) {
    using namespace ir;
    //GBE_ASSERT(insn.getAddressSpace() == MEM_GLOBAL);
    if (insn.isAligned() == true)
      this->emitUntypedWrite(insn);
    else {
      const GenReg address = ra->genReg(insn.getAddress());
      const GenReg value = ra->genReg(insn.getValue(0));
      this->emitByteScatter(insn, address, value);
    }
  }

  void GenContext::emitFenceInstruction(const ir::FenceInstruction &insn) {}
  void GenContext::emitLabelInstruction(const ir::LabelInstruction &insn) {
    using namespace ir;
    const LabelIndex label = insn.getLabelIndex();
    const GenReg src0 = ra->genReg(ocl::blockip);
    const GenReg src1 = GenReg::immuw(label);

    // Labels are branch targets. We save the position of each label in the
    // stream
    this->labelPos.insert(std::make_pair(label, p->insnNum));

    // Emit the mask computation at the head of each basic block
    p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.flag = 0;
      p->CMP(GEN_CONDITIONAL_LE, GenReg::retype(src0, GEN_TYPE_UW), src1);
    p->pop();

    // If it is required, insert a JUMP to bypass the block
    auto it = JIPs.find(&insn);
    if (it != JIPs.end()) {
      p->push();
        this->branchPos.insert(std::make_pair(&insn, p->insnNum));
        if (simdWidth == 8)
          p->curr.predicate = GEN_PREDICATE_ALIGN1_ANY8H;
        else if (simdWidth == 16)
          p->curr.predicate = GEN_PREDICATE_ALIGN1_ANY16H;
        else
          NOT_IMPLEMENTED;
        p->curr.inversePredicate = 1;
        p->curr.execWidth = 1;
        p->curr.flag = 0;
        p->curr.subFlag = 0;
        p->curr.noMask = 1;
        p->JMPI(GenReg::immd(0));
      p->pop();
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
    const GenReg stackptr = ra->genReg(ir::ocl::stackptr, TYPE_U32);
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

  void GenContext::emitInstructionStream(void) {
    using namespace ir;

    // XXX we push 0 in a scalar register to make select(pred, 0, 1) faster
    p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.execWidth = 1;
      p->MOV(GenReg::uw16grf(127,0), GenReg::immuw(1));
    p->pop();

    // Emit all the other instructions
    fn.foreachInstruction([&](const Instruction &insn) {
      const Opcode opcode = insn.getOpcode();
      switch (opcode) {
#define DECL_INSN(OPCODE, FAMILY) \
  case OP_##OPCODE: this->emit##FAMILY(cast<FAMILY>(insn)); break;
#include "ir/instruction.hxx"
#undef DECL_INSN
      }
    });
  }

  void GenContext::patchBranches(void) {
    using namespace ir;
    for (auto pair : branchPos) {
      const Instruction *insn = pair.first;
      const LabelIndex label = JIPs.find(insn)->second;
      const int32_t insnID = pair.second;
      const int32_t targetID = labelPos.find(label)->second;
      p->patchJMPI(insnID, (targetID-insnID-1) * 2);
    }
  }

  ///////////////////// XXX ///////////////////////
  void GenContext::emitLabelInstruction(const SelectionInstruction &insn) {
    const ir::LabelIndex label(insn.index);
    this->labelPos.insert(std::make_pair(label, p->insnNum));
  }

  void GenContext::emitUnaryInstruction(const SelectionInstruction &insn) {

  }

  void GenContext::emitBinaryInstruction(const SelectionInstruction &insn){}
  void GenContext::emitSelectInstruction(const SelectionInstruction &insn){}
  void GenContext::emitCompareInstruction(const SelectionInstruction &insn){}
  void GenContext::emitJumpInstruction(const SelectionInstruction &insn){}
  void GenContext::emitEotInstruction(const SelectionInstruction &insn){}
  void GenContext::emitNoOpInstruction(const SelectionInstruction &insn){}
  void GenContext::emitWaitInstruction(const SelectionInstruction &insn){}
  void GenContext::emitMathInstruction(const SelectionInstruction &insn){}
  void GenContext::emitUntypedReadInstruction(const SelectionInstruction &insn){}
  void GenContext::emitUntypedWriteInstruction(const SelectionInstruction &insn){}
  void GenContext::emitByteGatherInstruction(const SelectionInstruction &insn){}
  void GenContext::emitByteScatterInstruction(const SelectionInstruction &insn){}

  BVAR(OCL_OUTPUT_ASM, false);
  void GenContext::emitCode(void) {
    GenKernel *genKernel = static_cast<GenKernel*>(this->kernel);
    sel->select();
    ra->allocate(*this->sel);
    this->emitStackPointer();
    this->emitInstructionStream();
    this->patchBranches();
    genKernel->insnNum = p->insnNum;
    genKernel->insns = GBE_NEW_ARRAY(GenInstruction, genKernel->insnNum);
    std::memcpy(genKernel->insns, p->store, genKernel->insnNum * sizeof(GenInstruction));
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


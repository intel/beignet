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
#include "ir/function.hpp"
#include "sys/cvar.hpp"
#include <cstring>

namespace gbe
{
  ///////////////////////////////////////////////////////////////////////////
  // Various helper functions
  ///////////////////////////////////////////////////////////////////////////

  INLINE uint32_t getGenType(ir::Type type) {
    using namespace ir;
    switch (type) {
      case TYPE_BOOL: return GEN_TYPE_UW;
      case TYPE_S8: return GEN_TYPE_B;
      case TYPE_U8: return GEN_TYPE_UB;
      case TYPE_S16: return GEN_TYPE_W;
      case TYPE_U16: return GEN_TYPE_UW;
      case TYPE_S32: return GEN_TYPE_D;
      case TYPE_U32: return GEN_TYPE_UD;
      case TYPE_FLOAT: return GEN_TYPE_F;
      default: NOT_SUPPORTED; return GEN_TYPE_F;
    }
  }

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
    p = GBE_NEW(GenEmitter, simdWidth, 7); // XXX handle more than gen7
  }

  GenContext::~GenContext(void) { GBE_DELETE(p); }

  GenReg GenContext::genReg(ir::Register reg, ir::Type type) {
    const uint32_t genType = getGenType(type);
    auto it = RA.find(reg);
    GBE_ASSERT(it != RA.end());
    return GenReg::retype(it->second, genType);
  }

  GenReg GenContext::genRegQn(ir::Register reg, uint32_t quarter, ir::Type type) {
    GBE_ASSERT(quarter == 2 || quarter == 3 || quarter == 4);
    GenReg genReg = this->genReg(reg, type);
    if (this->isScalarReg(reg) == true)
      return genReg;
    else {
      const uint32_t elemSz = typeSize(genReg.file);
      const uint32_t grfOffset = genReg.nr*GEN_REG_SIZE + elemSz*genReg.subnr;
      const uint32_t nextOffset = grfOffset + 8*(quarter-1)*elemSz;
      genReg.nr = nextOffset / GEN_REG_SIZE;
      genReg.subnr = (nextOffset % GEN_REG_SIZE) / elemSz;
      return genReg;
    }
  }

  // Per-lane block IPs are always pre-allocated and used for branches. We just
  // 0xffff as a fake register for them
  static const ir::Register blockIPReg(0xffff);

  void GenContext::allocatePayloadReg(gbe_curbe_type value,
                                      uint32_t subValue,
                                      const ir::Register &reg)
  {
    const int32_t offset = kernel->getCurbeOffset(value, subValue);
    if (offset >= 0) {
      const ir::RegisterData data = fn.getRegisterData(reg);
      const uint32_t typeSize = data.getSize();
      const uint32_t nr = (offset + GEN_REG_SIZE) / GEN_REG_SIZE;
      const uint32_t subnr = ((offset + GEN_REG_SIZE) % GEN_REG_SIZE) / typeSize;
      GBE_ASSERT(data.family == ir::FAMILY_DWORD); // XXX support the rest
      if (this->isScalarReg(reg) == true)
        RA.insert(std::make_pair(reg, GenReg::f1grf(nr, subnr)));
      else if (this->simdWidth == 8)
        RA.insert(std::make_pair(reg, GenReg::f8grf(nr, subnr)));
      else if (this->simdWidth == 16)
        RA.insert(std::make_pair(reg, GenReg::f16grf(nr, subnr)));
    }
  }

  void GenContext::allocateRegister(void) {
    using namespace ir;
    GBE_ASSERT(fn.getProfile() == PROFILE_OCL);

    // Allocate the special registers (only those which are actually used)
    allocatePayloadReg(GBE_CURBE_LOCAL_ID_X, 0, ocl::lid0);
    allocatePayloadReg(GBE_CURBE_LOCAL_ID_Y, 0, ocl::lid1);
    allocatePayloadReg(GBE_CURBE_LOCAL_ID_Z, 0, ocl::lid2);
    allocatePayloadReg(GBE_CURBE_LOCAL_SIZE_X, 0, ocl::lsize0);
    allocatePayloadReg(GBE_CURBE_LOCAL_SIZE_Y, 0, ocl::lsize1);
    allocatePayloadReg(GBE_CURBE_LOCAL_SIZE_Z, 0, ocl::lsize2);
    allocatePayloadReg(GBE_CURBE_GLOBAL_SIZE_X, 0, ocl::gsize0);
    allocatePayloadReg(GBE_CURBE_GLOBAL_SIZE_Y, 0, ocl::gsize1);
    allocatePayloadReg(GBE_CURBE_GLOBAL_SIZE_Z, 0, ocl::gsize2);
    allocatePayloadReg(GBE_CURBE_GLOBAL_OFFSET_X, 0, ocl::goffset0);
    allocatePayloadReg(GBE_CURBE_GLOBAL_OFFSET_Y, 0, ocl::goffset1);
    allocatePayloadReg(GBE_CURBE_GLOBAL_OFFSET_Z, 0, ocl::goffset2);
    allocatePayloadReg(GBE_CURBE_GROUP_NUM_X, 0, ocl::numgroup0);
    allocatePayloadReg(GBE_CURBE_GROUP_NUM_Y, 0, ocl::numgroup1);
    allocatePayloadReg(GBE_CURBE_GROUP_NUM_Z, 0, ocl::numgroup2);

    // Group IDs are always allocated by the hardware in r0
    RA.insert(std::make_pair(ocl::groupid0, GenReg::f1grf(0, 1)));
    RA.insert(std::make_pair(ocl::groupid1, GenReg::f1grf(0, 6)));
    RA.insert(std::make_pair(ocl::groupid2, GenReg::f1grf(0, 7)));

    // block IP used to handle the mask in SW is always allocated
    int32_t blockIPOffset = GEN_REG_SIZE + kernel->getCurbeOffset(GBE_CURBE_BLOCK_IP,0);
    GBE_ASSERT(blockIPOffset >= 0 && blockIPOffset % GEN_REG_SIZE == 0);
    blockIPOffset /= GEN_REG_SIZE;
    if (simdWidth == 8)
      RA.insert(std::make_pair(blockIPReg, GenReg::uw8grf(blockIPOffset, 0)));
    else if (simdWidth == 16)
      RA.insert(std::make_pair(blockIPReg, GenReg::uw16grf(blockIPOffset, 0)));
    else
      NOT_SUPPORTED;

    // Allocate all input parameters
    const uint32_t inputNum = fn.inputNum();
    for (uint32_t inputID = 0; inputID < inputNum; ++inputID) {
      const FunctionInput &input = fn.getInput(inputID);
      GBE_ASSERT(input.type == FunctionInput::GLOBAL_POINTER ||
                 input.type == FunctionInput::CONSTANT_POINTER);
      allocatePayloadReg(GBE_CURBE_KERNEL_ARGUMENT, inputID, input.reg);
    }

    // First we build the set of all used registers
    set<Register> usedRegs;
    fn.foreachInstruction([&usedRegs](const Instruction &insn) {
      const uint32_t srcNum = insn.getSrcNum(), dstNum = insn.getDstNum();
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID)
        usedRegs.insert(insn.getSrc(srcID));
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID)
        usedRegs.insert(insn.getDst(dstID));
    });

    // Allocate all used registers. Just crash when we run out-of-registers
    uint32_t grfOffset = kernel->getCurbeSize() + GEN_REG_SIZE;
    GBE_ASSERT(simdWidth != 32); // XXX a bit more complicated see later
    if (simdWidth == 16) grfOffset = ALIGN(grfOffset, 2*GEN_REG_SIZE);
    for (auto reg : usedRegs) {
      if (fn.isSpecialReg(reg) == true) continue; // already done
      if (fn.getInput(reg) != NULL) continue; // already done
      const RegisterData regData = fn.getRegisterData(reg);
      const uint32_t typeSize = regData.getSize();
      const uint32_t regSize = simdWidth*typeSize;
      grfOffset = ALIGN(grfOffset, regSize);
      if (grfOffset + regSize <= GEN_GRF_SIZE) {
        const uint32_t nr = grfOffset / GEN_REG_SIZE;
        const uint32_t subnr = (grfOffset % GEN_REG_SIZE) / typeSize;
        if (simdWidth == 16)
          RA.insert(std::make_pair(reg, GenReg::f16grf(nr, subnr)));
        else if (simdWidth == 8)
          RA.insert(std::make_pair(reg, GenReg::f8grf(nr, subnr)));
        else
          NOT_SUPPORTED;
        grfOffset += simdWidth * typeSize;
      } else
        NOT_IMPLEMENTED;
    }
  }

  void GenContext::emitUnaryInstruction(const ir::UnaryInstruction &insn) {
    GBE_ASSERT(insn.getOpcode() == ir::OP_MOV);
    p->MOV(genReg(insn.getDst(0)), genReg(insn.getSrc(0)));
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
        const GenReg nextSrc0 = this->genRegQn(insn.getSrc(0), 2, TYPE_S32);
        const GenReg nextSrc1 = this->genRegQn(insn.getSrc(1), 2, TYPE_S32);
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
    const GenReg ip = this->genReg(blockIPReg, TYPE_U16);

    // Inefficient code. If the instruction is predicated, we build the flag
    // register from the boolean vector
    if (insn.isPredicated() == true) {
      const GenReg pred = this->genReg(insn.getPredicateIndex(), TYPE_U16);
      p->push();
        p->curr.noMask = 1;
        p->curr.execWidth = 1;
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->MOV(GenReg::flag(0,1), GenReg::flag(0,0));
      p->pop();

      // Rebuild the flag register by comparing the boolean with 1s
      p->push();
        p->curr.flag = 0;
        p->curr.subFlag = 1;
        p->CMP(GenReg::null(), GEN_CONDITIONAL_EQ, pred, GenReg::immuw(1));

        // Update the PcIPs
        p->MOV(ip, GenReg::immuw(uint16_t(dst)));
      p->pop();

      // We do not emit any jump if we must jump to the next instruction
      const BasicBlock *curr = insn.getParent();
      const BasicBlock *next = curr->getNextBlock();
      const LabelIndex nextLabel = next->getLabelIndex();
      if (nextLabel == dst) return;

      // It is slightly more complicated than for backward jump. We check that
      // all PcIPs are greater than the next block IP to be sure that we can
      // jump
      p->push();
        p->curr.flag = 0;
        p->curr.subFlag = 1;
        p->CMP(GenReg::null(), GEN_CONDITIONAL_G, ip, GenReg::immuw(nextLabel));

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

  void GenContext::emitBackwardBranch(const ir::BranchInstruction &insn,
                                      ir::LabelIndex dst,
                                      ir::LabelIndex src)
  {
    using namespace ir;
    const GenReg ip = this->genReg(blockIPReg, TYPE_U16);

    // Inefficient code. If the instruction is predicated, we build the flag
    // register from the boolean vector
    if (insn.isPredicated() == true) {
      const GenReg pred = this->genReg(insn.getPredicateIndex(), TYPE_U16);
      p->push();
        p->curr.noMask = 1;
        p->curr.execWidth = 1;
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->MOV(GenReg::flag(0,1), GenReg::flag(0,0));
      p->pop();

      // Rebuild the flag register by comparing the boolean with 1s
      p->push();
        p->curr.flag = 0;
        p->curr.subFlag = 1;
        p->CMP(GenReg::null(), GEN_CONDITIONAL_EQ, pred, GenReg::immuw(1));

        // Update the PcIPs
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
    GenReg dst  = this->genReg(insn.getDst(0), type);
    GenReg src0 = this->genReg(insn.getSrc(0), type);
    GenReg src1 = this->genReg(insn.getSrc(1), type);

    // Output the binary instruction
    switch (opcode) {
      case OP_ADD: p->ADD(dst, src0, src1); break;
      case OP_SUB: p->ADD(dst, src0, GenReg::negate(src1)); break;
      case OP_MUL: 
      {
        if (type == TYPE_FLOAT)
          p->MUL(dst, src0, src1);
        else if (type == TYPE_U32 || type == TYPE_S32)
          this->emitIntMul32x32(insn, dst, src0, src1);
        else
          NOT_IMPLEMENTED;
        break;
      }
      default: NOT_IMPLEMENTED;
    }
  }

  void GenContext::emitTernaryInstruction(const ir::TernaryInstruction &insn) {}
  void GenContext::emitSelectInstruction(const ir::SelectInstruction &insn) {}

  void GenContext::emitCompareInstruction(const ir::CompareInstruction &insn) {
    using namespace ir;
    const Opcode opcode = insn.getOpcode();
    const Type type = insn.getType();
    const uint32_t typeSize = getSize(type);
    const uint32_t genCmp = getGenCompare(opcode);
    const GenReg dst  = this->genReg(insn.getDst(0), TYPE_BOOL);
    const GenReg src0 = this->genReg(insn.getSrc(0), type);
    const GenReg src1 = this->genReg(insn.getSrc(1), type);

    // Copy the predicate to save it basically
    p->push();
      p->curr.noMask = 1;
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->MOV(GenReg::flag(0,1), GenReg::flag(0,0));
    p->pop();

    // Emit the compare instruction now. dwords require to push two SIMD8
    // instructions
    GBE_ASSERT(typeSize == 2 || typeSize == 4);
    p->push();
      if (this->simdWidth == 8 || typeSize == 2) {
        p->curr.flag = 0;
        p->curr.subFlag = 1;
        p->CMP(GenReg::null(), genCmp, src0, src1);
      } else if (this->simdWidth == 16) {
        const GenReg nextSrc0 = this->genRegQn(insn.getSrc(0), 2, type);
        const GenReg nextSrc1 = this->genRegQn(insn.getSrc(1), 2, type);
        p->curr.flag = 0;
        p->curr.subFlag = 1;
        p->curr.execWidth = 8;
        p->curr.quarterControl = GEN_COMPRESSION_Q1;
        p->CMP(GenReg::null(), genCmp, src0, src1);
        p->curr.quarterControl = GEN_COMPRESSION_Q2;
        p->CMP(GenReg::null(), genCmp, nextSrc0, nextSrc1);
      } else
        NOT_SUPPORTED;
    p->pop();

    // We emit a very unoptimized code where we store the resulting mask in a GRF
    p->push();
      p->curr.flag = 0;
      p->curr.subFlag = 1;
      p->SEL(dst, GenReg::uw1grf(127,0), GenReg::immuw(0));
    p->pop();
  }

  void GenContext::emitConvertInstruction(const ir::ConvertInstruction &insn) {}
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
  void GenContext::emitTextureInstruction(const ir::TextureInstruction &insn) {}

  void GenContext::emitLoadImmInstruction(const ir::LoadImmInstruction &insn) {
    using namespace ir;
    const Type type = insn.getType();
    const Immediate imm = insn.getImmediate();
    const GenReg dst = this->genReg(insn.getDst(0), type);

    switch (type) {
      case TYPE_U32: p->MOV(dst, GenReg::immud(imm.data.u32)); break;
      case TYPE_S32: p->MOV(dst, GenReg::immd(imm.data.s32)); break;
      case TYPE_FLOAT: p->MOV(dst, GenReg::immf(imm.data.f32)); break;
      default: NOT_SUPPORTED;
    }
  }

  void GenContext::emitLoadInstruction(const ir::LoadInstruction &insn) {
    using namespace ir;
    GBE_ASSERT(insn.getAddressSpace() == MEM_GLOBAL);
    GBE_ASSERT(insn.getValueNum() == 1);
    GBE_ASSERT(insn.isAligned() == true);
    GBE_ASSERT(this->simdWidth <= 16);
    const GenReg address = this->genReg(insn.getAddress());
    const GenReg value = this->genReg(insn.getValue(0));
    if (this->simdWidth == 8 || this->simdWidth == 16) {
      if (isScalarReg(insn.getAddress()) == true) {
        if (this->simdWidth == 8) {
          p->MOV(GenReg::f8grf(112, 0), GenReg::retype(address, GEN_TYPE_F));
          p->UNTYPED_READ(value, GenReg::f8grf(112, 0), 0, 1);
        } else if (this->simdWidth == 16) {
          p->MOV(GenReg::f16grf(112, 0), GenReg::retype(address, GEN_TYPE_F));
          p->UNTYPED_READ(value, GenReg::f16grf(112, 0), 0, 1);
        }
      } else
        p->UNTYPED_READ(value, address, 0, 1);
    } else
      NOT_IMPLEMENTED;
  }

  void GenContext::emitStoreInstruction(const ir::StoreInstruction &insn) {
    using namespace ir;
    GBE_ASSERT(insn.getAddressSpace() == MEM_GLOBAL);
    GBE_ASSERT(insn.getValueNum() == 1);
    GBE_ASSERT(insn.isAligned() == true);
    GBE_ASSERT(this->simdWidth <= 16);
    const GenReg address = this->genReg(insn.getAddress());
    const GenReg value = this->genReg(insn.getValue(0));
    // XXX remove that later. Now we just copy everything to GRFs to make it
    // contiguous
    if (this->simdWidth == 8) {
      p->MOV(GenReg::f8grf(112, 0), GenReg::retype(address, GEN_TYPE_F));
      p->MOV(GenReg::f8grf(113, 0), GenReg::retype(value, GEN_TYPE_F));
      p->UNTYPED_WRITE(GenReg::f8grf(112, 0), 0, 1);
    } else if (this->simdWidth == 16) {
      p->MOV(GenReg::f16grf(112, 0), GenReg::retype(address, GEN_TYPE_F));
      p->MOV(GenReg::f16grf(114, 0), GenReg::retype(value, GEN_TYPE_F));
      p->UNTYPED_WRITE(GenReg::f16grf(112, 0), 0, 1);
    } else
      NOT_IMPLEMENTED;
  }
  void GenContext::emitFenceInstruction(const ir::FenceInstruction &insn) {}
  void GenContext::emitLabelInstruction(const ir::LabelInstruction &insn) {
    const ir::LabelIndex label = insn.getLabelIndex();
    const GenReg dst = GenReg::retype(GenReg::null(), GEN_TYPE_UW);
    const GenReg src0 = this->genReg(blockIPReg);
    const GenReg src1 = GenReg::immuw(label);

    // Labels are branch targets. We save the position of each label in the
    // stream
    this->labelPos.insert(std::make_pair(label, p->insnNum));

    // Emit the mask computation at the head of each basic block
    p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.flag = 0;
      p->CMP(dst, GEN_CONDITIONAL_LE, GenReg::retype(src0, GEN_TYPE_UW), src1);
    p->pop();
  }

  void GenContext::emitInstructionStream(void) {
    using namespace ir;

    // We push 0 in a scalar register to make select(pred, 0, 1) faster
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
      const BranchInstruction *insn = cast<BranchInstruction>(pair.first);
      const LabelIndex label = insn->getLabelIndex();
      const int32_t insnID = pair.second;
      const int32_t targetID = labelPos.find(label)->second;
      p->patchJMPI(insnID, (targetID-insnID-1) * 2);
    }
  }

  BVAR(OCL_OUTPUT_ASM, false);
  void GenContext::emitCode(void) {
    GenKernel *genKernel = static_cast<GenKernel*>(this->kernel);
    this->allocateRegister();
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


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
 * \file gen_insn_selection.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "backend/gen_insn_selection.hpp"
#include "backend/gen_context.hpp"
#include "ir/function.hpp"
#include "sys/cvar.hpp"

namespace gbe
{
  ///////////////////////////////////////////////////////////////////////////
  // SelectionInstruction
  ///////////////////////////////////////////////////////////////////////////

  void SelectionInstruction::appendBefore(const SelectionInstruction &other) {
    Selection *selection = this->parent->parent;
    SelectionInstruction *toAppend = selection->newSelectionInstruction(other);
    SelectionInstruction *prev = this->prev;
    this->prev = toAppend;
    toAppend->next = this;
    if (prev) {
      toAppend->prev = prev;
      prev->next = toAppend;
    } else {
      GBE_ASSERT (this == this->parent->insnHead);
      this->parent->insnHead = toAppend;
    }
  }

  void SelectionInstruction::appendAfter(const SelectionInstruction &other) {
    Selection *selection = this->parent->parent;
    SelectionInstruction *toAppend = selection->newSelectionInstruction(other);
    SelectionInstruction *next = this->next;
    this->next = toAppend;
    toAppend->prev = this;
    if (next) {
      toAppend->next = next;
      next->prev = toAppend;
    } else {
      GBE_ASSERT (this == this->parent->insnTail);
      this->parent->insnTail = toAppend;
    }
  }

  ///////////////////////////////////////////////////////////////////////////
  // Selection
  ///////////////////////////////////////////////////////////////////////////

  Selection::Selection(GenContext &ctx) :
    ctx(ctx), blockHead(NULL), blockTail(NULL), block(NULL),
    curr(ctx.getSimdWidth()), file(ctx.getFunction().getRegisterFile()),
    stateNum(0), vectorNum(0) {}

  Selection::~Selection(void) {
    while (this->blockHead) {
      SelectionBlock *next = this->blockHead->next;
      while (this->blockHead->vector) {
        SelectionVector *next = this->blockHead->vector->next;
        this->deleteSelectionVector(this->blockHead->vector);
        this->blockHead->vector = next;
      }
      this->deleteSelectionBlock(this->blockHead);
      this->blockHead = next;
    }
  }

  void Selection::appendBlock(const ir::BasicBlock &bb) {
    this->block = this->newSelectionBlock(&bb);
    this->block->parent = this;
    if (this->blockTail != NULL)
      this->blockTail->next = this->block;
    if (this->blockHead == NULL)
      this->blockHead = this->block;
    this->blockTail = this->block;
  }

  SelectionInstruction *Selection::appendInsn(void) {
    GBE_ASSERT(this->block != NULL);
    SelectionInstruction *insn = this->newSelectionInstruction();
    this->block->append(insn);
    return insn;
  }

  SelectionVector *Selection::appendVector(void) {
    GBE_ASSERT(this->block != NULL && this->block->insnTail != NULL);
    SelectionVector *vector = this->newSelectionVector();
    vector->insn = this->block->insnTail;
    this->block->append(vector);
    this->vectorNum++;
    return vector;
  }

  ir::Register Selection::replaceSrc(SelectionInstruction *insn, uint32_t regID) {
    SelectionBlock *block = insn->parent;
    const uint32_t simdWidth = ctx.getSimdWidth();
    ir::Register tmp;

    // This will append the temporary register in the instruction block
    this->block = block;
    tmp = this->reg(ir::FAMILY_DWORD);

    // Generate the MOV instruction and replace the register in the instruction
    SelectionInstruction mov;
    mov.opcode = SEL_OP_MOV;
    mov.src[0] = GenRegister::retype(insn->src[regID], GEN_TYPE_F);
    mov.state = GenInstructionState(simdWidth);
    mov.dstNum = mov.srcNum = 1;
    insn->src[regID] = mov.dst[0] = GenRegister::fxgrf(simdWidth, tmp);
    insn->appendBefore(mov);

    return tmp;
  }

  ir::Register Selection::replaceDst(SelectionInstruction *insn, uint32_t regID) {
    SelectionBlock *block = insn->parent;
    const uint32_t simdWidth = ctx.getSimdWidth();
    ir::Register tmp;

    // This will append the temporary register in the instruction block
    this->block = block;
    tmp = this->reg(ir::FAMILY_DWORD);

    // Generate the MOV instruction and replace the register in the instruction
    SelectionInstruction mov;
    mov.opcode = SEL_OP_MOV;
    mov.dst[0] = GenRegister::retype(insn->dst[regID], GEN_TYPE_F);
    mov.state = GenInstructionState(simdWidth);
    mov.dstNum = mov.srcNum = 1;
    insn->dst[regID] = mov.src[0] = GenRegister::fxgrf(simdWidth, tmp);
    insn->appendAfter(mov);

    return tmp;
  }

  bool Selection::isScalarOrBool(ir::Register reg) const {
    if (ctx.isScalarReg(reg))
      return true;
    else {
      const ir::RegisterFamily family = file.get(reg).family;
      return family == ir::FAMILY_BOOL;
    }
  }

#define SEL_REG(SIMD16, SIMD8, SIMD1) \
  if (ctx.sel->isScalarOrBool(reg) == true) \
    return GenRegister::retype(GenRegister::SIMD1(reg), genType); \
  else if (simdWidth == 8) \
    return GenRegister::retype(GenRegister::SIMD8(reg), genType); \
  else { \
    GBE_ASSERT (simdWidth == 16); \
    return GenRegister::retype(GenRegister::SIMD16(reg), genType); \
  }

  GenRegister Selection::selReg(ir::Register reg, ir::Type type) {
    using namespace ir;
    const uint32_t genType = getGenType(type);
    const uint32_t simdWidth = ctx.getSimdWidth();
    const RegisterData data = file.get(reg);
    const RegisterFamily family = data.family;
    switch (family) {
      case FAMILY_BOOL: SEL_REG(uw1grf, uw1grf, uw1grf); break;
      case FAMILY_WORD: SEL_REG(uw16grf, uw8grf, uw1grf); break;
      case FAMILY_BYTE: SEL_REG(ub16grf, ub8grf, ub1grf); break;
      case FAMILY_DWORD: SEL_REG(f16grf, f8grf, f1grf); break;
      default: NOT_SUPPORTED;
    }
    GBE_ASSERT(false);
    return GenRegister();
  }

#undef SEL_REG

  GenRegister Selection::selRegQn(ir::Register reg, uint32_t q, ir::Type type) {
    GenRegister sreg = this->selReg(reg, type);
    sreg.quarter = q;
    return sreg;
  }

  /*! Syntactic sugar for method declaration */
  typedef const GenRegister &Reg;

  void Selection::LABEL(ir::LabelIndex index) {
    SelectionInstruction *insn = this->appendInsn();
    insn->opcode = SEL_OP_LABEL;
    insn->state = this->curr;
    insn->index = uint16_t(index);
    insn->srcNum = insn->dstNum = 0;
  }

  void Selection::JMPI(Reg src, ir::LabelIndex index) {
    SelectionInstruction *insn = this->appendInsn();
    insn->src[0] = src;
    insn->opcode = SEL_OP_JMPI;
    insn->state = this->curr;
    insn->index = uint16_t(index);
    insn->srcNum = 1;
    insn->dstNum = 0;
  }

  void Selection::CMP(uint32_t conditional, Reg src0, Reg src1) {
    SelectionInstruction *insn = this->appendInsn();
    insn->src[0] = src0;
    insn->src[1] = src1;
    insn->extra.function = conditional;
    insn->opcode = SEL_OP_CMP;
    insn->state = this->curr;
    insn->srcNum = 2;
    insn->dstNum = 0;
  }

  void Selection::EOT(void) {
    SelectionInstruction *insn = this->appendInsn();
    insn->opcode = SEL_OP_EOT;
    insn->state = this->curr;
    insn->srcNum = 0;
    insn->dstNum = 0;
  }

  void Selection::NOP(void) {
    SelectionInstruction *insn = this->appendInsn();
    insn->opcode = SEL_OP_NOP;
    insn->state = this->curr;
    insn->srcNum = insn->dstNum = 0;
  }

  void Selection::WAIT(void) {
    SelectionInstruction *insn = this->appendInsn();
    insn->opcode = SEL_OP_WAIT;
    insn->state = this->curr;
    insn->srcNum = insn->dstNum = 0;
  }

  void Selection::UNTYPED_READ(Reg addr,
                               const GenRegister *dst,
                               uint32_t elemNum,
                               uint32_t bti)
  {
    SelectionInstruction *insn = this->appendInsn();
    SelectionVector *srcVector = this->appendVector();
    SelectionVector *dstVector = this->appendVector();

    // Regular instruction to encode
    insn->opcode = SEL_OP_UNTYPED_READ;
    for (uint32_t elemID = 0; elemID < elemNum; ++elemID)
      insn->dst[elemID] = dst[elemID];
    insn->src[0] = addr;
    insn->extra.function = bti;
    insn->extra.elem = elemNum;
    insn->state = this->curr;
    insn->srcNum = 1;
    insn->dstNum = elemNum;

    // Sends require contiguous allocation
    dstVector->regNum = elemNum;
    dstVector->isSrc = 0;
    dstVector->reg = insn->dst;

    // Source cannot be scalar (yet)
    srcVector->regNum = 1;
    srcVector->isSrc = 1;
    srcVector->reg = insn->src;
  }

  void Selection::UNTYPED_WRITE(Reg addr,
                                const GenRegister *src,
                                uint32_t elemNum,
                                uint32_t bti)
  {
    SelectionInstruction *insn = this->appendInsn();
    SelectionVector *vector = this->appendVector();

    // Regular instruction to encode
    insn->opcode = SEL_OP_UNTYPED_WRITE;
    insn->src[0] = addr;
    for (uint32_t elemID = 0; elemID < elemNum; ++elemID)
      insn->src[elemID+1] = src[elemID];
    insn->extra.function = bti;
    insn->extra.elem = elemNum;
    insn->state = this->curr;
    insn->srcNum = elemNum+1;
    insn->dstNum = 0;

    // Sends require contiguous allocation for the sources
    vector->regNum = elemNum+1;
    vector->reg = insn->src;
    vector->isSrc = 1;
  }

  void Selection::BYTE_GATHER(Reg dst, Reg addr, uint32_t elemSize, uint32_t bti) {
    SelectionInstruction *insn = this->appendInsn();
    SelectionVector *srcVector = this->appendVector();
    SelectionVector *dstVector = this->appendVector();

    // Instruction to encode
    insn->opcode = SEL_OP_BYTE_GATHER;
    insn->src[0] = addr;
    insn->dst[0] = dst;
    insn->extra.function = bti;
    insn->extra.elem = elemSize;
    insn->state = this->curr;
    insn->srcNum = 1;
    insn->dstNum = 1;

    // byte gather requires vector in the sense that scalar are not allowed
    // (yet)
    dstVector->regNum = 1;
    dstVector->isSrc = 0;
    dstVector->reg = insn->dst;
    srcVector->regNum = 1;
    srcVector->isSrc = 1;
    srcVector->reg = insn->src;
  }

  void Selection::BYTE_SCATTER(Reg addr, Reg src, uint32_t elemSize, uint32_t bti) {
    SelectionInstruction *insn = this->appendInsn();
    SelectionVector *vector = this->appendVector();

    // Instruction to encode
    insn->opcode = SEL_OP_BYTE_SCATTER;
    insn->src[0] = addr;
    insn->src[1] = src;
    insn->extra.function = bti;
    insn->extra.elem = elemSize;
    insn->state = this->curr;
    insn->srcNum = 2;
    insn->dstNum = 0;

    // value and address are contiguous in the send
    vector->regNum = 2;
    vector->isSrc = 1;
    vector->reg = insn->src;
  }

  void Selection::MATH(Reg dst, uint32_t function, Reg src0, Reg src1) {
    SelectionInstruction *insn = this->appendInsn();
    insn->opcode = SEL_OP_MATH;
    insn->dst[0] = dst;
    insn->src[0] = src0;
    insn->src[1] = src1;
    insn->extra.function = function;
    insn->state = this->curr;
    insn->srcNum = 2;
    insn->dstNum = 1;
  }

  void Selection::ALU1(uint32_t opcode, Reg dst, Reg src) {
    SelectionInstruction *insn = this->appendInsn();
    insn->opcode = opcode;
    insn->dst[0] = dst;
    insn->src[0] = src;
    insn->state = this->curr;
    insn->srcNum = 1;
    insn->dstNum = 1;
  }

  void Selection::ALU2(uint32_t opcode, Reg dst, Reg src0, Reg src1) {
    SelectionInstruction *insn = this->appendInsn();
    insn->opcode = opcode;
    insn->dst[0] = dst;
    insn->src[0] = src0;
    insn->src[1] = src1;
    insn->state = this->curr;
    insn->srcNum = 2;
    insn->dstNum = 1;
  }

  void Selection::REGION(Reg dst0, Reg dst1, const GenRegister *src,
                         uint32_t offset, uint32_t vstride,
                         uint32_t width, uint32_t hstride,
                         uint32_t srcNum)
  {
    SelectionInstruction *insn = this->appendInsn();
    SelectionVector *vector = this->appendVector();

    // Instruction to encode
    insn->opcode = SEL_OP_REGION;
    insn->dst[0] = dst0;
    insn->dst[1] = dst1;
    GBE_ASSERT(srcNum <= SelectionInstruction::MAX_SRC_NUM);
    for (uint32_t srcID = 0; srcID < srcNum; ++srcID)
      insn->src[srcID] = src[srcID];
    insn->state = this->curr;
    insn->srcNum = srcNum;
    insn->dstNum = 2;
    insn->extra.vstride = vstride;
    insn->extra.width = width;
    insn->extra.offset = offset;
    insn->extra.hstride = hstride;

    // Regioning requires contiguous allocation for the sources
    vector->regNum = srcNum;
    vector->reg = insn->src;
    vector->isSrc = 1;
  }

  void Selection::RGATHER(Reg dst, const GenRegister *src, uint32_t srcNum)
  {
    SelectionInstruction *insn = this->appendInsn();
    SelectionVector *vector = this->appendVector();

    // Instruction to encode
    insn->opcode = SEL_OP_RGATHER;
    insn->dst[0] = dst;
    GBE_ASSERT(srcNum <= SelectionInstruction::MAX_SRC_NUM);
    for (uint32_t srcID = 0; srcID < srcNum; ++srcID)
      insn->src[srcID] = src[srcID];
    insn->state = this->curr;
    insn->srcNum = srcNum;
    insn->dstNum = 1;

    // Regioning requires contiguous allocation for the sources
    vector->regNum = srcNum;
    vector->reg = insn->src;
    vector->isSrc = 1;
  }

  void Selection::OBREAD(Reg dst, Reg addr, Reg header, uint32_t bti, uint32_t size) {
    SelectionInstruction *insn = this->appendInsn();
    insn->opcode = SEL_OP_OBREAD;
    insn->dst[0] = dst;
    insn->src[0] = addr;
    insn->src[1] = header;
    insn->state = this->curr;
    insn->srcNum = 2;
    insn->dstNum = 1;
    insn->extra.function = bti;
    insn->extra.elem = size / sizeof(int[4]); // number of owords
  }

  void Selection::OBWRITE(Reg addr, Reg value, Reg header, uint32_t bti, uint32_t size) {
    SelectionInstruction *insn = this->appendInsn();
    SelectionVector *vector = this->appendVector();
    insn->opcode = SEL_OP_OBWRITE;
    insn->src[0] = header;
    insn->src[1] = value;
    insn->src[2] = addr;
    insn->state = this->curr;
    insn->srcNum = 3;
    insn->dstNum = 0;
    insn->extra.function = bti;
    insn->extra.elem = size / sizeof(int[4]); // number of owords

    // We need to put the header and the data together
    vector->regNum = 2;
    vector->reg = insn->src;
    vector->isSrc = 1;
  }

  ///////////////////////////////////////////////////////////////////////////
  // SimpleSelection
  ///////////////////////////////////////////////////////////////////////////

  /*! This is a simplistic one-to-many instruction selection engine */
  class SimpleSelection : public Selection
  {
  public:
    SimpleSelection(GenContext &ctx);
    virtual ~SimpleSelection(void);
    /*! Implements the base class */
    virtual void select(void);
  private:
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
    void emitIntMul32x32(const ir::Instruction &insn, GenRegister dst, GenRegister src0, GenRegister src1);
    /*! Use untyped writes and reads for everything aligned on 4 bytes */
    void emitUntypedRead(const ir::LoadInstruction &insn, GenRegister address);
    void emitUntypedWrite(const ir::StoreInstruction &insn);
    /*! Use byte scatters and gathers for everything not aligned on 4 bytes */
    void emitByteGather(const ir::LoadInstruction &insn, GenRegister address, GenRegister value);
    void emitByteScatter(const ir::StoreInstruction &insn, GenRegister address, GenRegister value);
    /*! Backward and forward branches are handled slightly differently */
    void emitForwardBranch(const ir::BranchInstruction&, ir::LabelIndex dst, ir::LabelIndex src);
    void emitBackwardBranch(const ir::BranchInstruction&, ir::LabelIndex dst, ir::LabelIndex src);

    // Gen OCL extensions
    void emitRegionInstruction(const ir::RegionInstruction &insn);
    void emitVoteInstruction(const ir::VoteInstruction &insn);
    void emitRGatherInstruction(const ir::RGatherInstruction &insn);
    void emitOBReadInstruction(const ir::OBReadInstruction &insn);
    void emitOBWriteInstruction(const ir::OBWriteInstruction &insn);
  };

  SimpleSelection::SimpleSelection(GenContext &ctx) :
    Selection(ctx) {}
  SimpleSelection::~SimpleSelection(void) {}

  void SimpleSelection::select(void) {
    using namespace ir;
    const Function &fn = ctx.getFunction();
    fn.foreachBlock([&](const BasicBlock &bb) {
      this->appendBlock(bb);
      bb.foreach([&](const Instruction &insn) {
        const Opcode opcode = insn.getOpcode();
        switch (opcode) {
#define DECL_INSN(OPCODE, FAMILY) \
    case OP_##OPCODE: this->emit##FAMILY(cast<FAMILY>(insn)); break;
#include "ir/instruction.hxx"
#undef DECL_INSN
        }
      });
    });
  }

  void SimpleSelection::emitUnaryInstruction(const ir::UnaryInstruction &insn) {
    GBE_ASSERT(insn.getOpcode() == ir::OP_MOV);
    this->MOV(selReg(insn.getDst(0)), selReg(insn.getSrc(0)));
  }

  void SimpleSelection::emitIntMul32x32(const ir::Instruction &insn,
                                        GenRegister dst,
                                        GenRegister src0,
                                        GenRegister src1)
  {
    using namespace ir;
    const uint32_t simdWidth = this->curr.execWidth;
    this->push();

    // Either left part of the 16-wide register or just a simd 8 register
    dst  = GenRegister::retype(dst,  GEN_TYPE_D);
    src0 = GenRegister::retype(src0, GEN_TYPE_D);
    src1 = GenRegister::retype(src1, GEN_TYPE_D);
    this->curr.execWidth = 8;
    this->curr.quarterControl = GEN_COMPRESSION_Q1;
    this->MUL(GenRegister::retype(GenRegister::acc(), GEN_TYPE_D), src0, src1);
    this->MACH(GenRegister::retype(GenRegister::null(), GEN_TYPE_D), src0, src1);
    this->MOV(GenRegister::retype(dst, GEN_TYPE_F), GenRegister::acc());

    // Right part of the 16-wide register now
    if (simdWidth == 16) {
      this->curr.noMask = 1;
      const GenRegister nextSrc0 = this->selRegQn(insn.getSrc(0), 1, TYPE_S32);
      const GenRegister nextSrc1 = this->selRegQn(insn.getSrc(1), 1, TYPE_S32);
      this->MUL(GenRegister::retype(GenRegister::acc(), GEN_TYPE_D), nextSrc0, nextSrc1);
      this->MACH(GenRegister::retype(GenRegister::null(), GEN_TYPE_D), nextSrc0, nextSrc1);
      this->curr.quarterControl = GEN_COMPRESSION_Q2;
      const ir::Register reg = this->reg(FAMILY_DWORD);
      this->MOV(GenRegister::f8grf(reg), GenRegister::acc());
      this->curr.noMask = 0;
      this->MOV(GenRegister::retype(GenRegister::next(dst), GEN_TYPE_F),
                GenRegister::f8grf(reg));
    }

    this->pop();
  }

  void SimpleSelection::emitBinaryInstruction(const ir::BinaryInstruction &insn) {
    using namespace ir;
    const Opcode opcode = insn.getOpcode();
    const Type type = insn.getType();
    GenRegister dst  = this->selReg(insn.getDst(0), type);
    GenRegister src0 = this->selReg(insn.getSrc(0), type);
    GenRegister src1 = this->selReg(insn.getSrc(1), type);

    this->push();

    // Boolean values use scalars
    if (ctx.sel->isScalarOrBool(insn.getDst(0)) == true) {
      this->curr.execWidth = 1;
      this->curr.predicate = GEN_PREDICATE_NONE;
      this->curr.noMask = 1;
    }

    // Output the binary instruction
    switch (opcode) {
      case OP_ADD: this->ADD(dst, src0, src1); break;
      case OP_SUB: this->ADD(dst, src0, GenRegister::negate(src1)); break;
      case OP_AND: this->AND(dst, src0, src1); break;
      case OP_XOR: this->XOR(dst, src0, src1); break;
      case OP_OR:  this->OR(dst, src0,  src1); break;
      case OP_SHL: this->SHL(dst, src0, src1); break;
      case OP_MUL: 
      {
        if (type == TYPE_FLOAT)
          this->MUL(dst, src0, src1);
        else if (type == TYPE_U32 || type == TYPE_S32)
          this->emitIntMul32x32(insn, dst, src0, src1);
        else
          NOT_IMPLEMENTED;
      }
      break;
      case OP_DIV:
      {
        GBE_ASSERT(type == TYPE_FLOAT);
        this->MATH(dst, GEN_MATH_FUNCTION_FDIV, src0, src1);
      }
      break;
      default: NOT_IMPLEMENTED;
    }
    this->pop();
  }

  void SimpleSelection::emitSelectInstruction(const ir::SelectInstruction &insn) {
    using namespace ir;

    // Get all registers for the instruction
    const Type type = insn.getType();
    const GenRegister dst  = this->selReg(insn.getDst(0), type);
    const GenRegister src0 = this->selReg(insn.getSrc(SelectInstruction::src0Index), type);
    const GenRegister src1 = this->selReg(insn.getSrc(SelectInstruction::src1Index), type);

    // Since we cannot predicate the select instruction with our current mask,
    // we need to perform the selection in two steps (one to select, one to
    // update the destination register)
    const RegisterFamily family = getFamily(type);
    const GenRegister tmp = this->selReg(this->reg(family), type);
    const uint32_t simdWidth = ctx.getSimdWidth();
    const Register pred = insn.getPredicate();
    this->push();
      this->curr.predicate = GEN_PREDICATE_NORMAL;
      this->curr.execWidth = simdWidth;
      this->curr.physicalFlag = 0;
      this->curr.flagIndex = uint16_t(pred);
      this->curr.noMask = 0;
      this->SEL(tmp, src0, src1);
    this->pop();

    // Update the destination register properly now
    this->MOV(dst, tmp);
  }

  void SimpleSelection::emitTernaryInstruction(const ir::TernaryInstruction &insn) {
    NOT_IMPLEMENTED;
  }
  void SimpleSelection::emitSampleInstruction(const ir::SampleInstruction &insn) {
    NOT_IMPLEMENTED;
  }
  void SimpleSelection::emitTypedWriteInstruction(const ir::TypedWriteInstruction &insn) {
    NOT_IMPLEMENTED;
  }

  void SimpleSelection::emitLoadImmInstruction(const ir::LoadImmInstruction &insn) {
    using namespace ir;
    const Type type = insn.getType();
    const Immediate imm = insn.getImmediate();
    const GenRegister dst = this->selReg(insn.getDst(0), type);

    switch (type) {
      case TYPE_U32: this->MOV(dst, GenRegister::immud(imm.data.u32)); break;
      case TYPE_S32: this->MOV(dst, GenRegister::immd(imm.data.s32)); break;
      case TYPE_U16: this->MOV(dst, GenRegister::immuw(imm.data.u16)); break;
      case TYPE_S16: this->MOV(dst, GenRegister::immw(imm.data.s16)); break;
      case TYPE_U8:  this->MOV(dst, GenRegister::immuw(imm.data.u8)); break;
      case TYPE_S8:  this->MOV(dst, GenRegister::immw(imm.data.s8)); break;
      case TYPE_FLOAT: this->MOV(dst, GenRegister::immf(imm.data.f32)); break;
      default: NOT_SUPPORTED;
    }
  }

  void SimpleSelection::emitUntypedRead(const ir::LoadInstruction &insn, GenRegister addr)
  {
    using namespace ir;
    const uint32_t valueNum = insn.getValueNum();
    GenRegister dst[valueNum];
    for (uint32_t dstID = 0; dstID < valueNum; ++dstID)
      dst[dstID] = GenRegister::retype(this->selReg(insn.getValue(dstID)), GEN_TYPE_F);
    this->UNTYPED_READ(addr, dst, valueNum, 0);
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

  void SimpleSelection::emitByteGather(const ir::LoadInstruction &insn,
                                       GenRegister address,
                                       GenRegister value)
  {
    using namespace ir;
    GBE_ASSERT(insn.getValueNum() == 1);
    const Type type = insn.getValueType();
    const uint32_t elemSize = getByteScatterGatherSize(type);
    const uint32_t simdWidth = ctx.getSimdWidth();

    // We need a temporary register if we read bytes or words
    Register dst = value.reg;
    if (elemSize == GEN_BYTE_SCATTER_WORD ||
        elemSize == GEN_BYTE_SCATTER_BYTE) {
      dst = this->reg(FAMILY_DWORD);
      this->BYTE_GATHER(GenRegister::fxgrf(simdWidth, dst), address, elemSize, 0);
    }

    // Repack bytes or words using a converting mov instruction
    if (elemSize == GEN_BYTE_SCATTER_WORD)
      this->MOV(GenRegister::retype(value, GEN_TYPE_UW), GenRegister::unpacked_uw(dst));
    else if (elemSize == GEN_BYTE_SCATTER_BYTE)
      this->MOV(GenRegister::retype(value, GEN_TYPE_UB), GenRegister::unpacked_ub(dst));
  }

  void SimpleSelection::emitLoadInstruction(const ir::LoadInstruction &insn) {
    using namespace ir;
    const GenRegister address = this->selReg(insn.getAddress());
    GBE_ASSERT(insn.getAddressSpace() == MEM_GLOBAL ||
               insn.getAddressSpace() == MEM_PRIVATE);
    GBE_ASSERT(ctx.isScalarReg(insn.getValue(0)) == false);
    if (insn.isAligned() == true)
      this->emitUntypedRead(insn, address);
    else {
      const GenRegister value = this->selReg(insn.getValue(0));
      this->emitByteGather(insn, address, value);
    }
  }

  void SimpleSelection::emitUntypedWrite(const ir::StoreInstruction &insn)
  {
    using namespace ir;
    const uint32_t valueNum = insn.getValueNum();
    const uint32_t addrID = ir::StoreInstruction::addressIndex;
    GenRegister addr, value[valueNum];

    addr = GenRegister::retype(this->selReg(insn.getSrc(addrID)), GEN_TYPE_F);;
    for (uint32_t valueID = 0; valueID < valueNum; ++valueID)
      value[valueID] = GenRegister::retype(this->selReg(insn.getValue(valueID)), GEN_TYPE_F);
    this->UNTYPED_WRITE(addr, value, valueNum, 0);
  }

  void SimpleSelection::emitByteScatter(const ir::StoreInstruction &insn,
                                        GenRegister addr,
                                        GenRegister value)
  {
    using namespace ir;
    const Type type = insn.getValueType();
    const uint32_t elemSize = getByteScatterGatherSize(type);
    const uint32_t simdWidth = ctx.getSimdWidth();
    const GenRegister dst = value;

    GBE_ASSERT(insn.getValueNum() == 1);
    if (elemSize == GEN_BYTE_SCATTER_WORD) {
      value = GenRegister::udxgrf(simdWidth, this->reg(FAMILY_DWORD));
      this->MOV(value, GenRegister::retype(dst, GEN_TYPE_UW));
    } else if (elemSize == GEN_BYTE_SCATTER_BYTE) {
      value = GenRegister::udxgrf(simdWidth, this->reg(FAMILY_DWORD));
      this->MOV(value, GenRegister::retype(dst, GEN_TYPE_UB));
    }
    this->BYTE_SCATTER(addr, value, elemSize, 1);
  }

  void SimpleSelection::emitStoreInstruction(const ir::StoreInstruction &insn) {
    using namespace ir;
    if (insn.isAligned() == true)
      this->emitUntypedWrite(insn);
    else {
      const GenRegister address = this->selReg(insn.getAddress());
      const GenRegister value = this->selReg(insn.getValue(0));
      this->emitByteScatter(insn, address, value);
    }
  }

  void SimpleSelection::emitForwardBranch(const ir::BranchInstruction &insn,
                                          ir::LabelIndex dst,
                                          ir::LabelIndex src)
  {
    using namespace ir;
    const GenRegister ip = this->selReg(ocl::blockip, TYPE_U16);
    const LabelIndex jip = ctx.getLabelIndex(&insn);
    const uint32_t simdWidth = ctx.getSimdWidth();

    // We will not emit any jump if we must go the next block anyway
    const BasicBlock *curr = insn.getParent();
    const BasicBlock *next = curr->getNextBlock();
    const LabelIndex nextLabel = next->getLabelIndex();

    // Inefficient code. If the instruction is predicated, we build the flag
    // register from the boolean vector
    if (insn.isPredicated() == true) {
      const Register pred = insn.getPredicateIndex();

      // Update the PcIPs
      this->push();
        this->curr.physicalFlag = 0;
        this->curr.flagIndex = uint16_t(pred);
        this->MOV(ip, GenRegister::immuw(uint16_t(dst)));
      this->pop();

      if (nextLabel == jip) return;

      // It is slightly more complicated than for backward jump. We check that
      // all PcIPs are greater than the next block IP to be sure that we can
      // jump
      this->push();
        this->curr.physicalFlag = 0;
        this->curr.flagIndex = uint16_t(pred);
        this->curr.predicate = GEN_PREDICATE_NONE;
        this->CMP(GEN_CONDITIONAL_G, ip, GenRegister::immuw(nextLabel));

        // Branch to the jump target
        if (simdWidth == 8)
          this->curr.predicate = GEN_PREDICATE_ALIGN1_ALL8H;
        else if (simdWidth == 16)
          this->curr.predicate = GEN_PREDICATE_ALIGN1_ALL16H;
        else
          NOT_SUPPORTED;
        this->curr.execWidth = 1;
        this->curr.noMask = 1;
        this->JMPI(GenRegister::immd(0), jip);
      this->pop();

    } else {
      // Update the PcIPs
      this->MOV(ip, GenRegister::immuw(uint16_t(dst)));

      // Do not emit branch when we go to the next block anyway
      if (nextLabel == jip) return;
      this->push();
        this->curr.execWidth = 1;
        this->curr.noMask = 1;
        this->curr.predicate = GEN_PREDICATE_NONE;
        this->JMPI(GenRegister::immd(0), jip);
      this->pop();
    }
  }

  void SimpleSelection::emitBackwardBranch(const ir::BranchInstruction &insn,
                                           ir::LabelIndex dst,
                                           ir::LabelIndex src)
  {
    using namespace ir;
    const GenRegister ip = this->selReg(ocl::blockip, TYPE_U16);
    const Function &fn = ctx.getFunction();
    const BasicBlock &bb = fn.getBlock(src);
    const LabelIndex jip = ctx.getLabelIndex(&insn);
    const uint32_t simdWidth = ctx.getSimdWidth();
    GBE_ASSERT(bb.getNextBlock() != NULL);

    // Inefficient code: we make a GRF to flag conversion
    if (insn.isPredicated() == true) {
      const Register pred = insn.getPredicateIndex();

      // Update the PcIPs for all the branches. Just put the IPs of the next
      // block. Next instruction will properly reupdate the IPs of the lanes
      // that actually take the branch
      const LabelIndex next = bb.getNextBlock()->getLabelIndex();
      this->MOV(ip, GenRegister::immuw(uint16_t(next)));

      this->push();
        // Re-update the PcIPs for the branches that takes the backward jump
        this->curr.physicalFlag = 0;
        this->curr.flagIndex = uint16_t(pred);
        this->MOV(ip, GenRegister::immuw(uint16_t(dst)));

        // Branch to the jump target
        if (simdWidth == 8)
          this->curr.predicate = GEN_PREDICATE_ALIGN1_ANY8H;
        else if (simdWidth == 16)
          this->curr.predicate = GEN_PREDICATE_ALIGN1_ANY16H;
        else
          NOT_SUPPORTED;
        this->curr.execWidth = 1;
        this->curr.noMask = 1;
        this->JMPI(GenRegister::immd(0), jip);
      this->pop();

    } else {

      // Update the PcIPs
      this->MOV(ip, GenRegister::immuw(uint16_t(dst)));

      // Branch to the jump target
      this->push();
        this->curr.execWidth = 1;
        this->curr.noMask = 1;
        this->curr.predicate = GEN_PREDICATE_NONE;
        this->JMPI(GenRegister::immd(0), jip);
      this->pop();
    }
  }

  void SimpleSelection::emitCompareInstruction(const ir::CompareInstruction &insn) {
    using namespace ir;
    const Opcode opcode = insn.getOpcode();
    const Type type = insn.getType();
    const uint32_t genCmp = getGenCompare(opcode);
    const Register dst  = insn.getDst(0);
    const GenRegister src0 = this->selReg(insn.getSrc(0), type);
    const GenRegister src1 = this->selReg(insn.getSrc(1), type);

    // Limit the compare to the active lanes. Use the same compare as for f0.0
    this->push();
      const LabelIndex label = insn.getParent()->getLabelIndex();
      const GenRegister blockip = this->selReg(ocl::blockip, TYPE_U16);
      const GenRegister labelReg = GenRegister::immuw(label);
      this->curr.predicate = GEN_PREDICATE_NONE;
      this->curr.physicalFlag = 0;
      this->curr.flagIndex = uint16_t(dst);
      this->CMP(GEN_CONDITIONAL_LE, blockip, labelReg);
    this->pop();

    this->push();
      this->curr.physicalFlag = 0;
      this->curr.flagIndex = uint16_t(dst);
      this->CMP(genCmp, src0, src1);
    this->pop();
  }

  void SimpleSelection::emitConvertInstruction(const ir::ConvertInstruction &insn) {
    using namespace ir;
    const Type dstType = insn.getDstType();
    const Type srcType = insn.getSrcType();
    const RegisterFamily dstFamily = getFamily(dstType);
    const RegisterFamily srcFamily = getFamily(srcType);
    const GenRegister dst = this->selReg(insn.getDst(0), dstType);
    const GenRegister src = this->selReg(insn.getSrc(0), srcType);

    GBE_ASSERT(dstFamily != FAMILY_QWORD && srcFamily != FAMILY_QWORD);

    // We need two instructions to make the conversion
    if (dstFamily != FAMILY_DWORD && srcFamily == FAMILY_DWORD) {
      GenRegister unpacked;
      if (dstFamily == FAMILY_WORD) {
        const uint32_t type = TYPE_U16 ? GEN_TYPE_UW : GEN_TYPE_W;
        unpacked = GenRegister::unpacked_uw(this->reg(FAMILY_DWORD));
        unpacked = GenRegister::retype(unpacked, type);
      } else {
        const uint32_t type = TYPE_U8 ? GEN_TYPE_UB : GEN_TYPE_B;
        unpacked = GenRegister::unpacked_ub(this->reg(FAMILY_DWORD));
        unpacked = GenRegister::retype(unpacked, type);
      }
      this->MOV(unpacked, src);
      this->MOV(dst, unpacked);
    } else
      this->MOV(dst, src);
  }

  void SimpleSelection::emitBranchInstruction(const ir::BranchInstruction &insn) {
    using namespace ir;
    const Opcode opcode = insn.getOpcode();
    if (opcode == OP_RET)
      this->EOT();
    else if (opcode == OP_BRA) {
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

  void SimpleSelection::emitFenceInstruction(const ir::FenceInstruction &insn) {}
  void SimpleSelection::emitLabelInstruction(const ir::LabelInstruction &insn) {
    using namespace ir;
    const LabelIndex label = insn.getLabelIndex();
    const GenRegister src0 = this->selReg(ocl::blockip);
    const GenRegister src1 = GenRegister::immuw(label);
    const uint32_t simdWidth = ctx.getSimdWidth();
    this->LABEL(label);

    // Emit the mask computation at the head of each basic block
    this->push();
      this->curr.predicate = GEN_PREDICATE_NONE;
      this->curr.flag = 0;
      this->curr.subFlag = 0;
      this->CMP(GEN_CONDITIONAL_LE, GenRegister::retype(src0, GEN_TYPE_UW), src1);
    this->pop();

    // If it is required, insert a JUMP to bypass the block
    if (ctx.hasJIP(&insn)) {
      const LabelIndex jip = ctx.getLabelIndex(&insn);
      this->push();
        if (simdWidth == 8)
          this->curr.predicate = GEN_PREDICATE_ALIGN1_ANY8H;
        else if (simdWidth == 16)
          this->curr.predicate = GEN_PREDICATE_ALIGN1_ANY16H;
        else
          NOT_IMPLEMENTED;
        this->curr.inversePredicate = 1;
        this->curr.execWidth = 1;
        this->curr.flag = 0;
        this->curr.subFlag = 0;
        this->curr.noMask = 1;
        this->JMPI(GenRegister::immd(0), jip);
      this->pop();
    }
  }

  void SimpleSelection::emitRegionInstruction(const ir::RegionInstruction &insn) {
    using namespace ir;

    // Two destinations: one is the real destination, one is a temporary
    GenRegister dst0 = this->selReg(insn.getDst(0)), dst1;
    if (ctx.getSimdWidth() == 8)
      dst1 = GenRegister::ud8grf(this->reg(FAMILY_DWORD));
    else
      dst1 = GenRegister::ud16grf(this->reg(FAMILY_DWORD));

    // Get all the sources
    GenRegister src[SelectionInstruction::MAX_SRC_NUM];
    const uint32_t srcNum = insn.getSrcNum();
    GBE_ASSERT(srcNum <= SelectionInstruction::MAX_SRC_NUM);
    for (uint32_t srcID = 0; srcID < insn.getSrcNum(); ++srcID)
      src[srcID] = this->selReg(insn.getSrc(srcID));

    // Get the region parameters
    const uint32_t offset = insn.getOffset();
    const uint32_t vstride = insn.getVStride();
    const uint32_t width = insn.getWidth();
    const uint32_t hstride = insn.getHStride();
    this->REGION(dst0, dst1, src, offset, vstride, width, hstride, srcNum);
  }

  void SimpleSelection::emitVoteInstruction(const ir::VoteInstruction &insn) {
    using namespace ir;
    const uint32_t simdWidth = ctx.getSimdWidth();
    const GenRegister dst = this->selReg(insn.getDst(0), TYPE_U16);
    const GenRegister src = this->selReg(insn.getSrc(0), TYPE_U16);

    // Limit the vote to the active lanes. Use the same compare as for f0.0
    this->push();
      const LabelIndex label = insn.getParent()->getLabelIndex();
      const GenRegister blockip = this->selReg(ocl::blockip, TYPE_U16);
      const GenRegister labelReg = GenRegister::immuw(label);
      this->curr.predicate = GEN_PREDICATE_NONE;
      this->curr.flag = 0;
      this->curr.subFlag = 1;
      this->CMP(GEN_CONDITIONAL_LE, blockip, labelReg);
    this->pop();

    // Emit the compare instruction to get the flag register
    this->push();
      const VotePredicate vote = insn.getVotePredicate();
      const uint32_t genCmp = vote == VOTE_ANY ? GEN_CONDITIONAL_NEQ : GEN_CONDITIONAL_EQ;
      this->curr.flag = 0;
      this->curr.subFlag = 1;
      this->CMP(genCmp, src, GenRegister::immuw(0));
    this->pop();

    // Broadcast the result to the destination
    if (vote == VOTE_ANY)
      this->MOV(dst, GenRegister::flag(0,1));
    else {
      const GenRegister tmp = this->selReg(this->reg(FAMILY_WORD), TYPE_U16);
      this->push();
        // Set all lanes of tmp to zero
        this->curr.predicate = GEN_PREDICATE_NONE;
        this->MOV(tmp, GenRegister::immuw(0));

        // Compute the short values with no mask
        this->curr.flag = 0;
        this->curr.subFlag = 1;
        this->curr.inversePredicate = 1;
        this->curr.predicate = simdWidth == 8 ?
          GEN_PREDICATE_ALIGN1_ANY8H :
          GEN_PREDICATE_ALIGN1_ANY16H;
        this->MOV(tmp, GenRegister::immuw(1));
      this->pop();

      // Update the destination with the proper mask
      this->MOV(dst, tmp);
    }
  }

  void SimpleSelection::emitRGatherInstruction(const ir::RGatherInstruction &insn) {
    using namespace ir;
    // Two destinations: one is the real destination, one is a temporary
    const GenRegister dst = this->selReg(insn.getDst(0)), dst1;

    // Get all the sources
    GenRegister src[SelectionInstruction::MAX_SRC_NUM];
    const uint32_t srcNum = insn.getSrcNum();
    GBE_ASSERT(srcNum <= SelectionInstruction::MAX_SRC_NUM);
    for (uint32_t srcID = 0; srcID < insn.getSrcNum(); ++srcID)
      src[srcID] = this->selReg(insn.getSrc(srcID));

    // Get the region parameters
    this->RGATHER(dst, src, srcNum);
  }

  void SimpleSelection::emitOBReadInstruction(const ir::OBReadInstruction &insn) {
    using namespace ir;
    const GenRegister header = this->selReg(this->reg(FAMILY_DWORD), TYPE_U32);
    const GenRegister addr = this->selReg(insn.getAddress(), TYPE_U32);
    const GenRegister value = this->selReg(insn.getValue(), TYPE_U32);
    const uint32_t simdWidth = ctx.getSimdWidth();
    this->OBREAD(value, addr, header, 0xff, simdWidth * sizeof(int));
  }

  void SimpleSelection::emitOBWriteInstruction(const ir::OBWriteInstruction &insn) {
    using namespace ir;
    const GenRegister header = this->selReg(this->reg(FAMILY_DWORD), TYPE_U32);
    const GenRegister addr = this->selReg(insn.getAddress(), TYPE_U32);
    const GenRegister value = this->selReg(insn.getValue(), TYPE_U32);
    const uint32_t simdWidth = ctx.getSimdWidth();
    this->OBWRITE(addr, value, header, 0xff, simdWidth * sizeof(int));
  }

  Selection *newSimpleSelection(GenContext &ctx) {
    return GBE_NEW(SimpleSelection, ctx);
  }
} /* namespace gbe */


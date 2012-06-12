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
    SelectionInstruction *mov = this->newSelectionInstruction();
    mov->opcode = SEL_OP_MOV;
    mov->src[0] = SelectionReg::retype(insn->src[regID], GEN_TYPE_F);
    mov->state = SelectionState(simdWidth);
    mov->dstNum = mov->srcNum = 1;
    if (simdWidth == 8)
      insn->src[regID] = mov->dst[0] = SelectionReg::f8grf(tmp);
    else
      insn->src[regID] = mov->dst[0] = SelectionReg::f16grf(tmp);

    // Insert the MOV instruction before the current instruction
    SelectionInstruction *prev = insn->prev;
    insn->prev = mov;
    mov->next = insn;
    if (prev) {
      mov->prev = prev;
      prev->next = mov;
    } else {
      GBE_ASSERT (insn == block->insnHead);
      block->insnHead = mov;
    }

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
    SelectionInstruction *mov = this->newSelectionInstruction();
    mov->opcode = SEL_OP_MOV;
    mov->dst[0] = SelectionReg::retype(insn->dst[regID], GEN_TYPE_F);
    mov->state = SelectionState(simdWidth);
    mov->dstNum = mov->srcNum = 1;
    if (simdWidth == 8)
      insn->dst[regID] = mov->src[0] = SelectionReg::f8grf(tmp);
    else
      insn->dst[regID] = mov->src[0] = SelectionReg::f16grf(tmp);

    // Insert the MOV instruction after the current instruction
    SelectionInstruction *next = insn->next;
    insn->next = mov;
    mov->prev = insn;
    if (next) {
      mov->next = next;
      next->prev = mov;
    } else {
      GBE_ASSERT (insn == block->insnTail);
      block->insnTail = mov;
    }

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
  if (ctx.isScalarOrBool(reg) == true) \
    return SelectionReg::retype(SelectionReg::SIMD1(reg), genType); \
  else if (simdWidth == 8) \
    return SelectionReg::retype(SelectionReg::SIMD8(reg), genType); \
  else { \
    GBE_ASSERT (simdWidth == 16); \
    return SelectionReg::retype(SelectionReg::SIMD16(reg), genType); \
  }

  SelectionReg Selection::selReg(ir::Register reg, ir::Type type) {
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
    return SelectionReg();
  }

#undef SEL_REG

  SelectionReg Selection::selRegQn(ir::Register reg, uint32_t q, ir::Type type) {
    SelectionReg sreg = this->selReg(reg, type);
    sreg.quarter = q;
    return sreg;
  }

  /*! Syntactic sugar for method declaration */
  typedef const SelectionReg &Reg;

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
    insn->function = conditional;
    insn->opcode = SEL_OP_CMP;
    insn->state = this->curr;
    insn->srcNum = 2;
    insn->dstNum = 0;
  }

  void Selection::EOT(void) {
    SelectionInstruction *insn = this->appendInsn();
    insn->opcode = SEL_OP_EOT;
    insn->state = this->curr;
    insn->srcNum = 1;
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
                               const SelectionReg *dst,
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
    insn->function = bti;
    insn->elem = elemNum;
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
                               const SelectionReg *src,
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
    insn->function = bti;
    insn->elem = elemNum;
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
    insn->function = bti;
    insn->elem = elemSize;
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
    insn->function = bti;
    insn->elem = elemSize;
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
    insn->function = function;
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
    void emitIntMul32x32(const ir::Instruction &insn, SelectionReg dst, SelectionReg src0, SelectionReg src1);
    /*! Use untyped writes and reads for everything aligned on 4 bytes */
    void emitUntypedRead(const ir::LoadInstruction &insn, SelectionReg address);
    void emitUntypedWrite(const ir::StoreInstruction &insn);
    /*! Use byte scatters and gathers for everything not aligned on 4 bytes */
    void emitByteGather(const ir::LoadInstruction &insn, SelectionReg address, SelectionReg value);
    void emitByteScatter(const ir::StoreInstruction &insn, SelectionReg address, SelectionReg value);
    /*! Backward and forward branches are handled slightly differently */
    void emitForwardBranch(const ir::BranchInstruction&, ir::LabelIndex dst, ir::LabelIndex src);
    void emitBackwardBranch(const ir::BranchInstruction&, ir::LabelIndex dst, ir::LabelIndex src);
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
                                        SelectionReg dst,
                                        SelectionReg src0,
                                        SelectionReg src1)
  {
    using namespace ir;
    const uint32_t width = this->curr.execWidth;
    this->push();

    // Either left part of the 16-wide register or just a simd 8 register
    dst  = SelectionReg::retype(dst,  GEN_TYPE_D);
    src0 = SelectionReg::retype(src0, GEN_TYPE_D);
    src1 = SelectionReg::retype(src1, GEN_TYPE_D);
    this->curr.execWidth = 8;
    this->curr.quarterControl = GEN_COMPRESSION_Q1;
    this->MUL(SelectionReg::retype(SelectionReg::acc(), GEN_TYPE_D), src0, src1);
    this->MACH(SelectionReg::retype(SelectionReg::null(), GEN_TYPE_D), src0, src1);
    this->MOV(SelectionReg::retype(dst, GEN_TYPE_F), SelectionReg::acc());

    // Right part of the 16-wide register now
    if (width == 16) {
      this->curr.noMask = 1;
      const SelectionReg nextSrc0 = this->selRegQn(insn.getSrc(0), 1, TYPE_S32);
      const SelectionReg nextSrc1 = this->selRegQn(insn.getSrc(1), 1, TYPE_S32);
      this->MUL(SelectionReg::retype(SelectionReg::acc(), GEN_TYPE_D), nextSrc0, nextSrc1);
      this->MACH(SelectionReg::retype(SelectionReg::null(), GEN_TYPE_D), nextSrc0, nextSrc1);
      this->curr.quarterControl = GEN_COMPRESSION_Q2;
      const ir::Register reg = this->reg(FAMILY_DWORD);
      this->MOV(SelectionReg::f8grf(reg), SelectionReg::acc());
      this->curr.noMask = 0;
      this->MOV(SelectionReg::retype(SelectionReg::next(dst), GEN_TYPE_F),
                SelectionReg::f8grf(reg));
    }

    this->pop();
  }

  void SimpleSelection::emitBinaryInstruction(const ir::BinaryInstruction &insn) {
    using namespace ir;
    const Opcode opcode = insn.getOpcode();
    const Type type = insn.getType();
    SelectionReg dst  = this->selReg(insn.getDst(0), type);
    SelectionReg src0 = this->selReg(insn.getSrc(0), type);
    SelectionReg src1 = this->selReg(insn.getSrc(1), type);

    this->push();

    // Boolean values use scalars
    if (ctx.isScalarOrBool(insn.getDst(0)) == true) {
      this->curr.execWidth = 1;
      this->curr.predicate = GEN_PREDICATE_NONE;
      this->curr.noMask = 1;
    }

    // Output the binary instruction
    switch (opcode) {
      case OP_ADD: this->ADD(dst, src0, src1); break;
      case OP_SUB: this->ADD(dst, src0, SelectionReg::negate(src1)); break;
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

  void SimpleSelection::emitTernaryInstruction(const ir::TernaryInstruction &insn) {
    NOT_IMPLEMENTED;
  }
  void SimpleSelection::emitSelectInstruction(const ir::SelectInstruction &insn) {
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
    const SelectionReg dst = this->selReg(insn.getDst(0), type);

    switch (type) {
      case TYPE_U32: this->MOV(dst, SelectionReg::immud(imm.data.u32)); break;
      case TYPE_S32: this->MOV(dst, SelectionReg::immd(imm.data.s32)); break;
      case TYPE_U16: this->MOV(dst, SelectionReg::immuw(imm.data.u16)); break;
      case TYPE_S16: this->MOV(dst, SelectionReg::immw(imm.data.s16)); break;
      case TYPE_U8:  this->MOV(dst, SelectionReg::immuw(imm.data.u8)); break;
      case TYPE_S8:  this->MOV(dst, SelectionReg::immw(imm.data.s8)); break;
      case TYPE_FLOAT: this->MOV(dst, SelectionReg::immf(imm.data.f32)); break;
      default: NOT_SUPPORTED;
    }
  }

  void SimpleSelection::emitUntypedRead(const ir::LoadInstruction &insn, SelectionReg addr)
  {
    using namespace ir;
    const uint32_t valueNum = insn.getValueNum();
    SelectionReg dst[valueNum];

      for (uint32_t dstID = 0; dstID < valueNum; ++dstID)
        dst[dstID] = SelectionReg::retype(this->selReg(insn.getValue(dstID)), GEN_TYPE_F);
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
                                    SelectionReg address,
                                    SelectionReg value)
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
      if (simdWidth == 8)
        this->BYTE_GATHER(SelectionReg::f8grf(dst), address, elemSize, 0);
      else if (simdWidth == 16)
        this->BYTE_GATHER(SelectionReg::f16grf(dst), address, elemSize, 0);
      else
        NOT_IMPLEMENTED;
    }

    // Repack bytes or words using a converting mov instruction
    if (elemSize == GEN_BYTE_SCATTER_WORD)
      this->MOV(SelectionReg::retype(value, GEN_TYPE_UW), SelectionReg::unpacked_uw(dst));
    else if (elemSize == GEN_BYTE_SCATTER_BYTE)
      this->MOV(SelectionReg::retype(value, GEN_TYPE_UB), SelectionReg::unpacked_ub(dst));
  }

  void SimpleSelection::emitLoadInstruction(const ir::LoadInstruction &insn) {
    using namespace ir;
    const SelectionReg address = this->selReg(insn.getAddress());
    GBE_ASSERT(insn.getAddressSpace() == MEM_GLOBAL ||
               insn.getAddressSpace() == MEM_PRIVATE);
    GBE_ASSERT(ctx.isScalarReg(insn.getValue(0)) == false);
    if (insn.isAligned() == true)
      this->emitUntypedRead(insn, address);
    else {
      const SelectionReg value = this->selReg(insn.getValue(0));
      this->emitByteGather(insn, address, value);
    }
  }

  void SimpleSelection::emitUntypedWrite(const ir::StoreInstruction &insn)
  {
    using namespace ir;
    const uint32_t valueNum = insn.getValueNum();
    const uint32_t addrID = ir::StoreInstruction::addressIndex;
    SelectionReg addr, value[valueNum];

    addr = SelectionReg::retype(this->selReg(insn.getSrc(addrID)), GEN_TYPE_F);;
    for (uint32_t valueID = 0; valueID < valueNum; ++valueID)
      value[valueID] = SelectionReg::retype(this->selReg(insn.getValue(valueID)), GEN_TYPE_F);
    this->UNTYPED_WRITE(addr, value, valueNum, 0);
  }

  void SimpleSelection::emitByteScatter(const ir::StoreInstruction &insn,
                                        SelectionReg addr,
                                        SelectionReg value)
  {
    using namespace ir;
    const Type type = insn.getValueType();
    const uint32_t elemSize = getByteScatterGatherSize(type);
    const uint32_t simdWidth = ctx.getSimdWidth();
    const SelectionReg dst = value;

    GBE_ASSERT(insn.getValueNum() == 1);
    if (simdWidth == 8) {
      if (elemSize == GEN_BYTE_SCATTER_WORD) {
        value = SelectionReg::ud8grf(this->reg(FAMILY_DWORD));
        this->MOV(value, SelectionReg::retype(dst, GEN_TYPE_UW));
      } else if (elemSize == GEN_BYTE_SCATTER_BYTE) {
        value = SelectionReg::ud8grf(this->reg(FAMILY_DWORD));
        this->MOV(value, SelectionReg::retype(dst, GEN_TYPE_UB));
      }
    } else if (simdWidth == 16) {
      if (elemSize == GEN_BYTE_SCATTER_WORD) {
        value = SelectionReg::ud16grf(this->reg(FAMILY_DWORD));
        this->MOV(value, SelectionReg::retype(dst, GEN_TYPE_UW));
      } else if (elemSize == GEN_BYTE_SCATTER_BYTE) {
        value = SelectionReg::ud16grf(this->reg(FAMILY_DWORD));
        this->MOV(value, SelectionReg::retype(dst, GEN_TYPE_UB));
      }
    } else
      NOT_IMPLEMENTED;

    this->BYTE_SCATTER(addr, value, elemSize, 1);
  }

  void SimpleSelection::emitStoreInstruction(const ir::StoreInstruction &insn) {
    using namespace ir;
    if (insn.isAligned() == true)
      this->emitUntypedWrite(insn);
    else {
      const SelectionReg address = this->selReg(insn.getAddress());
      const SelectionReg value = this->selReg(insn.getValue(0));
      this->emitByteScatter(insn, address, value);
    }
  }

  void SimpleSelection::emitForwardBranch(const ir::BranchInstruction &insn,
                                       ir::LabelIndex dst,
                                       ir::LabelIndex src)
  {
    using namespace ir;
    const SelectionReg ip = this->selReg(ocl::blockip, TYPE_U16);
    const LabelIndex jip = ctx.getLabelIndex(&insn);
    const uint32_t simdWidth = ctx.getSimdWidth();

    // We will not emit any jump if we must go the next block anyway
    const BasicBlock *curr = insn.getParent();
    const BasicBlock *next = curr->getNextBlock();
    const LabelIndex nextLabel = next->getLabelIndex();

    // Inefficient code. If the instruction is predicated, we build the flag
    // register from the boolean vector
    if (insn.isPredicated() == true) {
      const SelectionReg pred = this->selReg(insn.getPredicateIndex(), TYPE_U16);

      // Reset the flag register
      this->push();
        this->curr.predicate = GEN_PREDICATE_NONE;
        this->curr.execWidth = 1;
        this->curr.noMask = 1;
        this->MOV(SelectionReg::flag(0,1), pred);
      this->pop();

      // Update the PcIPs
      this->push();
        this->curr.flag = 0;
        this->curr.subFlag = 1;
        this->MOV(ip, SelectionReg::immuw(uint16_t(dst)));
      this->pop();

      if (nextLabel == jip) return;

      // It is slightly more complicated than for backward jump. We check that
      // all PcIPs are greater than the next block IP to be sure that we can
      // jump
      this->push();
        this->curr.predicate = GEN_PREDICATE_NONE;
        this->curr.flag = 0;
        this->curr.subFlag = 1;
        this->CMP(GEN_CONDITIONAL_G, ip, SelectionReg::immuw(nextLabel));

        // Branch to the jump target
        if (simdWidth == 8)
          this->curr.predicate = GEN_PREDICATE_ALIGN1_ALL8H;
        else if (simdWidth == 16)
          this->curr.predicate = GEN_PREDICATE_ALIGN1_ALL16H;
        else
          NOT_SUPPORTED;
        this->curr.execWidth = 1;
        this->curr.noMask = 1;
        this->JMPI(SelectionReg::immd(0), jip);
      this->pop();

    } else {
      // Update the PcIPs
      this->MOV(ip, SelectionReg::immuw(uint16_t(dst)));

      // Do not emit branch when we go to the next block anyway
      if (nextLabel == jip) return;
      this->push();
        this->curr.execWidth = 1;
        this->curr.noMask = 1;
        this->curr.predicate = GEN_PREDICATE_NONE;
        this->JMPI(SelectionReg::immd(0), jip);
      this->pop();
    }
  }

  void SimpleSelection::emitBackwardBranch(const ir::BranchInstruction &insn,
                                        ir::LabelIndex dst,
                                        ir::LabelIndex src)
  {
    using namespace ir;
    const SelectionReg ip = this->selReg(ocl::blockip, TYPE_U16);
    const Function &fn = ctx.getFunction();
    const BasicBlock &bb = fn.getBlock(src);
    const LabelIndex jip = ctx.getLabelIndex(&insn);
    const uint32_t simdWidth = ctx.getSimdWidth();
    GBE_ASSERT(bb.getNextBlock() != NULL);

    // Inefficient code: we make a GRF to flag conversion
    if (insn.isPredicated() == true) {
      const SelectionReg pred = this->selReg(insn.getPredicateIndex(), TYPE_U16);

      // Update the PcIPs for all the branches. Just put the IPs of the next
      // block. Next instruction will properly reupdate the IPs of the lanes
      // that actually take the branch
      const LabelIndex next = bb.getNextBlock()->getLabelIndex();
      this->MOV(ip, SelectionReg::immuw(uint16_t(next)));

      // Rebuild the flag register by comparing the boolean with 1s
      this->push();
        this->curr.noMask = 1;
        this->curr.execWidth = 1;
        this->curr.predicate = GEN_PREDICATE_NONE;
        this->MOV(SelectionReg::flag(0,1), pred);
      this->pop();

      this->push();
        this->curr.flag = 0;
        this->curr.subFlag = 1;

        // Re-update the PcIPs for the branches that takes the backward jump
        this->MOV(ip, SelectionReg::immuw(uint16_t(dst)));

        // Branch to the jump target
        if (simdWidth == 8)
          this->curr.predicate = GEN_PREDICATE_ALIGN1_ANY8H;
        else if (simdWidth == 16)
          this->curr.predicate = GEN_PREDICATE_ALIGN1_ANY16H;
        else
          NOT_SUPPORTED;
        this->curr.execWidth = 1;
        this->curr.noMask = 1;
        this->JMPI(SelectionReg::immd(0), jip);
      this->pop();

    } else {

      // Update the PcIPs
      this->MOV(ip, SelectionReg::immuw(uint16_t(dst)));

      // Branch to the jump target
      this->push();
        this->curr.execWidth = 1;
        this->curr.noMask = 1;
        this->curr.predicate = GEN_PREDICATE_NONE;
        this->JMPI(SelectionReg::immd(0), jip);
      this->pop();
    }
  }


  void SimpleSelection::emitCompareInstruction(const ir::CompareInstruction &insn) {
    using namespace ir;
    const Opcode opcode = insn.getOpcode();
    const Type type = insn.getType();
    const uint32_t genCmp = getGenCompare(opcode);
    const SelectionReg dst  = this->selReg(insn.getDst(0), TYPE_BOOL);
    const SelectionReg src0 = this->selReg(insn.getSrc(0), type);
    const SelectionReg src1 = this->selReg(insn.getSrc(1), type);

    // Copy the predicate to save it basically
    this->push();
      this->curr.noMask = 1;
      this->curr.execWidth = 1;
      this->curr.predicate = GEN_PREDICATE_NONE;
      this->MOV(SelectionReg::flag(0,1), SelectionReg::flag(0,0));
    this->pop();

    // Emit the compare instruction itself
    this->push();
      this->curr.flag = 0;
      this->curr.subFlag = 1;
      this->CMP(genCmp, src0, src1);
    this->pop();

    // We emit an unoptimized code where we store the resulting mask in a GRF
    this->push();
      this->curr.execWidth = 1;
      this->curr.predicate = GEN_PREDICATE_NONE;
      this->MOV(dst, SelectionReg::flag(0,1));
    this->pop();
  }

  void SimpleSelection::emitConvertInstruction(const ir::ConvertInstruction &insn) {
    using namespace ir;
    const Type dstType = insn.getDstType();
    const Type srcType = insn.getSrcType();
    const RegisterFamily dstFamily = getFamily(dstType);
    const RegisterFamily srcFamily = getFamily(srcType);
    const SelectionReg dst = this->selReg(insn.getDst(0), dstType);
    const SelectionReg src = this->selReg(insn.getSrc(0), srcType);

    GBE_ASSERT(dstFamily != FAMILY_QWORD && srcFamily != FAMILY_QWORD);

    // We need two instructions to make the conversion
    if (dstFamily != FAMILY_DWORD && srcFamily == FAMILY_DWORD) {
      SelectionReg unpacked;
      if (dstFamily == FAMILY_WORD) {
        const uint32_t type = TYPE_U16 ? GEN_TYPE_UW : GEN_TYPE_W;
        unpacked = SelectionReg::unpacked_uw(this->reg(FAMILY_DWORD));
        unpacked = SelectionReg::retype(unpacked, type);
      } else {
        const uint32_t type = TYPE_U8 ? GEN_TYPE_UB : GEN_TYPE_B;
        unpacked = SelectionReg::unpacked_ub(this->reg(FAMILY_DWORD));
        unpacked = SelectionReg::retype(unpacked, type);
      }
      this->MOV(unpacked, src);
      this->MOV(dst, unpacked);
    } else
      this->MOV(dst, src);
  }


  void SimpleSelection::emitBranchInstruction(const ir::BranchInstruction &insn) {
    using namespace ir;
    const Opcode opcode = insn.getOpcode();
    if (opcode == OP_RET) {
#if 0
      this->push();
        this->curr.predicate = GEN_PREDICATE_NONE;
        this->curr.execWidth = 8;
        this->curr.noMask = 1;
        this->MOV(SelectionReg::f8grf(127,0), SelectionReg::f8grf(0,0));
        this->EOT(127);
      this->pop();
#endif
      this->EOT();
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

  void SimpleSelection::emitFenceInstruction(const ir::FenceInstruction &insn) {}
  void SimpleSelection::emitLabelInstruction(const ir::LabelInstruction &insn) {
    using namespace ir;
    const LabelIndex label = insn.getLabelIndex();
    const SelectionReg src0 = this->selReg(ocl::blockip);
    const SelectionReg src1 = SelectionReg::immuw(label);
    const uint32_t simdWidth = ctx.getSimdWidth();
    this->LABEL(label);

    // Emit the mask computation at the head of each basic block
    this->push();
      this->curr.predicate = GEN_PREDICATE_NONE;
      this->curr.flag = 0;
      this->CMP(GEN_CONDITIONAL_LE, SelectionReg::retype(src0, GEN_TYPE_UW), src1);
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
        this->JMPI(SelectionReg::immd(0), jip);
      this->pop();
    }
  }

  Selection *newSimpleSelection(GenContext &ctx) {
    return GBE_NEW(SimpleSelection, ctx);
  }
} /* namespace gbe */

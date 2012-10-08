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
#include "ir/liveness.hpp"
#include "ir/profile.hpp"
#include "sys/cvar.hpp"
#include "sys/vector.hpp"
#include <algorithm>

namespace gbe
{
  ///////////////////////////////////////////////////////////////////////////
  // SelectionInstruction
  ///////////////////////////////////////////////////////////////////////////

  SelectionInstruction::SelectionInstruction(SelectionOpcode op, uint32_t dst, uint32_t src) :
    parent(NULL), opcode(op), dstNum(dst), srcNum(src)
  {}

  void SelectionInstruction::prepend(SelectionInstruction &other) {
    gbe::prepend(&other, this);
    other.parent = this->parent;
  }

  void SelectionInstruction::append(SelectionInstruction &other) {
    gbe::append(&other, this);
    other.parent = this->parent;
  }

  bool SelectionInstruction::isRead(void) const {
    return this->opcode == SEL_OP_OBREAD ||
           this->opcode == SEL_OP_UNTYPED_READ ||
           this->opcode == SEL_OP_BYTE_GATHER;
  }

  bool SelectionInstruction::isWrite(void) const {
    return this->opcode == SEL_OP_OBWRITE ||
           this->opcode == SEL_OP_UNTYPED_WRITE ||
           this->opcode == SEL_OP_BYTE_SCATTER;
  }

  bool SelectionInstruction::isBranch(void) const {
    return this->opcode == SEL_OP_JMPI;
  }

  bool SelectionInstruction::isLabel(void) const {
    return this->opcode == SEL_OP_LABEL;
  }

  ///////////////////////////////////////////////////////////////////////////
  // SelectionVector
  ///////////////////////////////////////////////////////////////////////////

  SelectionVector::SelectionVector(void) :
    insn(NULL), next(NULL), reg(NULL), regNum(0), isSrc(0)
  {}

  ///////////////////////////////////////////////////////////////////////////
  // SelectionBlock
  ///////////////////////////////////////////////////////////////////////////

  SelectionBlock::SelectionBlock(const ir::BasicBlock *bb) :
    vector(NULL), next(NULL), bb(bb)
  {}

  void SelectionBlock::append(ir::Register reg) { tmp.push_back(reg); }

  void SelectionBlock::append(SelectionInstruction *insn) {
    this->insnList.push_back(insn);
    insn->parent = this;
  }

  void SelectionBlock::prepend(SelectionInstruction *insn) {
    this->insnList.push_front(insn);
    insn->parent = this;
  }

  void SelectionBlock::append(SelectionVector *vec) {
    SelectionVector *tmp = this->vector;
    this->vector = vec;
    this->vector->next = tmp;
  }

  ///////////////////////////////////////////////////////////////////////////
  // Maximal munch selection on DAG
  ///////////////////////////////////////////////////////////////////////////

  /*! All instructions in a block are organized into a DAG */
  class SelectionDAG
  {
  public:
    INLINE SelectionDAG(const ir::Instruction &insn) :
      insn(insn), mergeable(0), childNum(insn.getSrcNum()), isRoot(0) {
      for (uint32_t childID = 0; childID < childNum; ++childID)
        this->child[childID] = NULL;
    }
    /*! Mergeable are non-root instructions with valid sources */
    INLINE void setAsMergeable(uint32_t which) { mergeable|=(1<<which); }
    /*! Mergeable are non-root instructions with valid sources */
    INLINE bool isMergeable(uint32_t which) const { return mergeable&(1<<which); }
    /*! Children that need to be matched */
    SelectionDAG *child[ir::Instruction::MAX_SRC_NUM];
    /*! Instruction that needs to be matched */
    const ir::Instruction &insn;
    /*! When sources have been overwritten, a child insn cannot be merged */
    uint32_t mergeable:ir::Instruction::MAX_SRC_NUM;
    /*! Number of children we have in the pattern */
    uint32_t childNum:4;
    /*! A root must be generated, no matter what */
    uint32_t isRoot:1;
  };

  /*! A pattern is a tree to match. This is the general interface for them. For
   *  pattern to be matched, we need to match the complete tree i.e. this node
   *  and its child nodes
   */
  class SelectionPattern
  {
  public:
    SelectionPattern(uint32_t insnNum, uint32_t cost) :
      insnNum(insnNum), cost(cost) {}
    /*! This is an abstract class */
    virtual ~SelectionPattern(void) {}
    /*! Emit Gen code in the selection. Return false if no match */
    virtual bool emit(Selection::Opaque &sel, SelectionDAG &dag) const = 0;
    /*! Directly mark all sources as root (when no match is found) */
    void markAllChildren(SelectionDAG &dag) const {
      // Do not merge anything, so all sources become roots
      for (uint32_t childID = 0; childID < dag.childNum; ++childID)
        if (dag.child[childID]) 
          dag.child[childID]->isRoot = 1;
    }
    /*! All the possible opcodes for this pattern (for fast sort) */
    vector<ir::Opcode> opcodes;
    /*! Number of instruction generated */
    uint32_t insnNum;
    /*! Cost of the pattern */
    uint32_t cost;
  };

  /*! Store and sort all the patterns. This is our global library we use for the
   *  code selection
   */
  class SelectionLibrary
  {
  public:
    /*! Will register all the patterns */
    SelectionLibrary(void);
    /*! Release and destroy all the registered patterns */
    ~SelectionLibrary(void);
    /*! Insert the given pattern for all associated opcodes */
    template <typename PatternType> void insert(void);
    /*! One list of pattern per opcode */
    typedef vector<const SelectionPattern*> PatternList;
    /*! All lists of patterns properly sorted per opcode */
    PatternList patterns[ir::OP_INVALID];
    /*! All patterns to free */
    vector<const SelectionPattern*> toFree;
  };

  ///////////////////////////////////////////////////////////////////////////
  // Code selection internal implementation
  ///////////////////////////////////////////////////////////////////////////

  /*! Actual implementation of the instruction selection engine */
  class Selection::Opaque
  {
  public:
    /*! simdWidth is the default width for the instructions */
    Opaque(GenContext &ctx);
    /*! Release everything */
    virtual ~Opaque(void);
    /*! Implements the instruction selection itself */
    void select(void);
    /*! Start a backward generation (from the end of the block) */
    void startBackwardGeneration(void);
    /*! End backward code generation and output the code in the block */
    void endBackwardGeneration(void);
    /*! Apply the given functor on all selection block */
    template <typename T>
    INLINE void foreach(const T &functor) const {
      SelectionBlock *curr = blockHead;
      while (curr) {
        SelectionBlock *succ = curr->next;
        functor(*curr);
        curr = succ;
      }
    }
    /*! Implement public class */
    uint32_t getLargestBlockSize(void) const;
    /*! Implement public class */
    INLINE uint32_t getVectorNum(void) const { return this->vectorNum; }
    /*! Implement public class */
    INLINE ir::Register replaceSrc(SelectionInstruction *insn, uint32_t regID);
    /*! Implement public class */
    INLINE ir::Register replaceDst(SelectionInstruction *insn, uint32_t regID);
    /*! Implement public class */
    INLINE uint32_t getRegNum(void) const { return file.regNum(); }
    /*! Implements public interface */
    bool isScalarOrBool(ir::Register reg) const;
    /*! Implements public interface */
    INLINE ir::RegisterData getRegisterData(ir::Register reg) const {
      return file.get(reg);
    }
    /*! Implement public class */
    INLINE ir::RegisterFamily getRegisterFamily(ir::Register reg) const {
      return file.get(reg).family;
    }
    /*! Implement public class */
    SelectionInstruction *create(SelectionOpcode, uint32_t dstNum, uint32_t srcNum);
    /*! Return the selection register from the GenIR one */
    GenRegister selReg(ir::Register, ir::Type type = ir::TYPE_FLOAT) const;
    /*! Compute the nth register part when using SIMD8 with Qn (n in 2,3,4) */
    GenRegister selRegQn(ir::Register, uint32_t quarter, ir::Type type = ir::TYPE_FLOAT) const;
    /*! Size of the stack (should be large enough) */
    enum { MAX_STATE_NUM = 16 };
    /*! Push the current instruction state */
    INLINE void push(void) {
      assert(stateNum < MAX_STATE_NUM);
      stack[stateNum++] = curr;
    }
    /*! Pop the latest pushed state */
    INLINE void pop(void) {
      assert(stateNum > 0);
      curr = stack[--stateNum];
    }
    /*! Create a new register in the register file and append it in the
     *  temporary list of the current block
     */
    INLINE ir::Register reg(ir::RegisterFamily family) {
      GBE_ASSERT(block != NULL);
      const ir::Register reg = file.append(family);
      block->append(reg);
      return reg;
    }
    /*! Append a block at the block stream tail. It becomes the current block */
    void appendBlock(const ir::BasicBlock &bb);
    /*! Append an instruction in the current block */
    SelectionInstruction *appendInsn(SelectionOpcode, uint32_t dstNum, uint32_t srcNum);
    /*! Append a new vector of registers in the current block */
    SelectionVector *appendVector(void);
    /*! Build a DAG for the basic block (return number of instructions) */
    uint32_t buildBasicBlockDAG(const ir::BasicBlock &bb);
    /*! Perform the selection on the basic block */
    void matchBasicBlock(uint32_t insnNum);
    /*! A root instruction needs to be generated */
    bool isRoot(const ir::Instruction &insn) const;

    /*! To handle selection block allocation */
    DECL_POOL(SelectionBlock, blockPool);
    /*! To handle selection instruction allocation */
    LinearAllocator insnAllocator;
    /*! To handle selection vector allocation */
    DECL_POOL(SelectionVector, vecPool);
    /*! Per register information used with top-down block sweeping */
    vector<SelectionDAG*> regDAG;
    /*! Store one DAG per instruction */
    vector<SelectionDAG*> insnDAG;
    /*! Owns this structure */
    GenContext &ctx;
    /*! Tail of the code fragment for backward code generation */
    intrusive_list<SelectionInstruction> bwdList;
    /*! List of emitted blocks */
    SelectionBlock *blockHead, *blockTail;
    /*! Currently processed block */
    SelectionBlock *block;
    /*! Current instruction state to use */
    GenInstructionState curr;
    /*! We append new registers so we duplicate the function register file */
    ir::RegisterFile file;
    /*! State used to encode the instructions */
    GenInstructionState stack[MAX_STATE_NUM];
    /*! Maximum number of instructions in the basic blocks */
    uint32_t maxInsnNum;
    /*! Speed up instruction dag allocation */
    DECL_POOL(SelectionDAG, dagPool);
    /*! Total number of registers in the function we encode */
    uint32_t regNum;
    /*! Number of states currently pushed */
    uint32_t stateNum;
    /*! Number of vector allocated */
    uint32_t vectorNum;
    /*! If true, generate code backward */
    bool bwdCodeGeneration;
    /*! To make function prototypes more readable */
    typedef const GenRegister &Reg;

#define ALU1(OP) \
  INLINE void OP(Reg dst, Reg src) { ALU1(SEL_OP_##OP, dst, src); }
#define ALU2(OP) \
  INLINE void OP(Reg dst, Reg src0, Reg src1) { ALU2(SEL_OP_##OP, dst, src0, src1); }
#define ALU3(OP) \
  INLINE void OP(Reg dst, Reg src0, Reg src1, Reg src2) { ALU3(SEL_OP_##OP, dst, src0, src1, src2); }
    ALU1(MOV)
    ALU1(RNDZ)
    ALU1(RNDE)
    ALU2(SEL)
    ALU1(NOT)
    ALU2(AND)
    ALU2(OR)
    ALU2(XOR)
    ALU2(SHR)
    ALU2(SHL)
    ALU2(RSR)
    ALU2(RSL)
    ALU2(ASR)
    ALU2(ADD)
    ALU2(MUL)
    ALU1(FRC)
    ALU1(RNDD)
    ALU2(MACH)
    ALU1(LZD)
    ALU3(MAD)
#undef ALU1
#undef ALU2
#undef ALU3

    /*! Encode a label instruction */
    void LABEL(ir::LabelIndex label);
    /*! Jump indexed instruction */
    void JMPI(Reg src, ir::LabelIndex target);
    /*! Compare instructions */
    void CMP(uint32_t conditional, Reg src0, Reg src1);
    /*! EOT is used to finish GPGPU threads */
    void EOT(void);
    /*! No-op */
    void NOP(void);
    /*! Wait instruction (used for the barrier) */
    void WAIT(void);
    /*! Untyped read (up to 4 elements) */
    void UNTYPED_READ(Reg addr, const GenRegister *dst, uint32_t elemNum, uint32_t bti);
    /*! Untyped write (up to 4 elements) */
    void UNTYPED_WRITE(Reg addr, const GenRegister *src, uint32_t elemNum, uint32_t bti);
    /*! Byte gather (for unaligned bytes, shorts and ints) */
    void BYTE_GATHER(Reg dst, Reg addr, uint32_t elemSize, uint32_t bti);
    /*! Byte scatter (for unaligned bytes, shorts and ints) */
    void BYTE_SCATTER(Reg addr, Reg src, uint32_t elemSize, uint32_t bti);
    /*! Oblock read */
    void OBREAD(Reg dst, Reg addr, Reg header, uint32_t bti, uint32_t size);
    /*! Oblock write */
    void OBWRITE(Reg addr, Reg value, Reg header, uint32_t bti, uint32_t size);
    /*! Extended math function */
    void MATH(Reg dst, uint32_t function, Reg src0, Reg src1);
    /*! Encode unary instructions */
    void ALU1(SelectionOpcode opcode, Reg dst, Reg src);
    /*! Encode binary instructions */
    void ALU2(SelectionOpcode opcode, Reg dst, Reg src0, Reg src1);
    /*! Encode ternary instructions */
    void ALU3(SelectionOpcode opcode, Reg dst, Reg src0, Reg src1, Reg src2);
    /*! Encode regioning */
    void REGION(Reg dst0, Reg dst1, const GenRegister *src, uint32_t offset, uint32_t vstride, uint32_t width, uint32_t hstride, uint32_t srcNum);
    /*! Encode regioning */
    void RGATHER(Reg dst, const GenRegister *src, uint32_t srcNum);
    /*! Use custom allocators */
    GBE_CLASS(Opaque);
    friend class SelectionBlock;
    friend class SelectionInstruction;
  };

  Selection::Opaque::Opaque(GenContext &ctx) :
    ctx(ctx), blockHead(NULL), blockTail(NULL), block(NULL),
    curr(ctx.getSimdWidth()), file(ctx.getFunction().getRegisterFile()),
    maxInsnNum(ctx.getFunction().getLargestBlockSize()), dagPool(maxInsnNum),
    stateNum(0), vectorNum(0), bwdCodeGeneration(false)
  {
    const ir::Function &fn = ctx.getFunction();
    this->regNum = fn.regNum();
    this->regDAG.resize(regNum);
    this->insnDAG.resize(maxInsnNum);
  }

  Selection::Opaque::~Opaque(void) {
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

  SelectionInstruction*
  Selection::Opaque::create(SelectionOpcode opcode, uint32_t dstNum, uint32_t srcNum)
  {
    const size_t regSize =  (dstNum+srcNum)*sizeof(GenRegister);
    const size_t size = sizeof(SelectionInstruction) + regSize;
    void *ptr = insnAllocator.allocate(size);
    return new (ptr) SelectionInstruction(opcode, dstNum, srcNum);
  }

  void Selection::Opaque::startBackwardGeneration(void) {
    this->bwdCodeGeneration = true;
  }

  void Selection::Opaque::endBackwardGeneration(void) {
    for (auto it = bwdList.rbegin(); it != bwdList.rend();) {
      SelectionInstruction &insn = *it;
      auto toRemoveIt = it--;
      bwdList.erase(toRemoveIt);
      this->block->prepend(&insn);
    }

    this->bwdCodeGeneration = false;
  }

  uint32_t Selection::Opaque::getLargestBlockSize(void) const {
    uint32_t maxInsnNum = 0;
    this->foreach([&](const SelectionBlock &bb) {
      uint32_t insnNum = 0;
      bb.foreach([&](const SelectionInstruction &insn) {insnNum++;});
      maxInsnNum = std::max(maxInsnNum, insnNum);
    });
    return maxInsnNum;
  }

  void Selection::Opaque::appendBlock(const ir::BasicBlock &bb) {
    this->block = this->newSelectionBlock(&bb);
    if (this->blockTail != NULL)
      this->blockTail->next = this->block;
    if (this->blockHead == NULL)
      this->blockHead = this->block;
    this->blockTail = this->block;
  }

  SelectionInstruction *Selection::Opaque::appendInsn(SelectionOpcode opcode,
                                                      uint32_t dstNum,
                                                      uint32_t srcNum)
  {
    GBE_ASSERT(this->block != NULL);
    SelectionInstruction *insn = this->create(opcode, dstNum, srcNum);
    if (this->bwdCodeGeneration)
      this->bwdList.push_back(insn);
    else
      this->block->append(insn);
    insn->state = this->curr;
    return insn;
  }

  SelectionVector *Selection::Opaque::appendVector(void) {
    GBE_ASSERT(this->block != NULL);
    SelectionVector *vector = this->newSelectionVector();

    if (this->bwdCodeGeneration)
      vector->insn = this->bwdList.back();
    else
      vector->insn = this->block->insnList.back();
    this->block->append(vector);
    this->vectorNum++;
    return vector;
  }

  ir::Register Selection::Opaque::replaceSrc(SelectionInstruction *insn, uint32_t regID) {
    SelectionBlock *block = insn->parent;
    const uint32_t simdWidth = ctx.getSimdWidth();
    ir::Register tmp;

    // This will append the temporary register in the instruction block
    this->block = block;
    tmp = this->reg(ir::FAMILY_DWORD);

    // Generate the MOV instruction and replace the register in the instruction
    SelectionInstruction *mov = this->create(SEL_OP_MOV, 1, 1);
    mov->src(0) = GenRegister::retype(insn->src(regID), GEN_TYPE_F);
    mov->state = GenInstructionState(simdWidth);
    insn->src(regID) = mov->dst(0) = GenRegister::fxgrf(simdWidth, tmp);
    insn->prepend(*mov);

    return tmp;
  }

  ir::Register Selection::Opaque::replaceDst(SelectionInstruction *insn, uint32_t regID) {
    SelectionBlock *block = insn->parent;
    const uint32_t simdWidth = ctx.getSimdWidth();
    ir::Register tmp;

    // This will append the temporary register in the instruction block
    this->block = block;
    tmp = this->reg(ir::FAMILY_DWORD);

    // Generate the MOV instruction and replace the register in the instruction
    SelectionInstruction *mov = this->create(SEL_OP_MOV, 1, 1);
    mov->dst(0) = GenRegister::retype(insn->dst(regID), GEN_TYPE_F);
    mov->state = GenInstructionState(simdWidth);
    insn->dst(regID) = mov->src(0) = GenRegister::fxgrf(simdWidth, tmp);
    insn->append(*mov);

    return tmp;
  }

  bool Selection::Opaque::isScalarOrBool(ir::Register reg) const {
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

  GenRegister Selection::Opaque::selReg(ir::Register reg, ir::Type type) const {
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

  GenRegister Selection::Opaque::selRegQn(ir::Register reg, uint32_t q, ir::Type type) const {
    GenRegister sreg = this->selReg(reg, type);
    sreg.quarter = q;
    return sreg;
  }

  /*! Syntactic sugar for method declaration */
  typedef const GenRegister &Reg;

  void Selection::Opaque::LABEL(ir::LabelIndex index) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_LABEL, 0, 0);
    insn->index = uint16_t(index);
  }

  void Selection::Opaque::JMPI(Reg src, ir::LabelIndex index) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_JMPI, 0, 1);
    insn->src(0) = src;
    insn->index = uint16_t(index);
  }

  void Selection::Opaque::CMP(uint32_t conditional, Reg src0, Reg src1) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_CMP, 0, 2);
    insn->src(0) = src0;
    insn->src(1) = src1;
    insn->extra.function = conditional;
  }

  void Selection::Opaque::EOT(void) { this->appendInsn(SEL_OP_EOT, 0, 0); }
  void Selection::Opaque::NOP(void) { this->appendInsn(SEL_OP_NOP, 0, 0); }
  void Selection::Opaque::WAIT(void) { this->appendInsn(SEL_OP_WAIT, 0, 0); }

  void Selection::Opaque::UNTYPED_READ(Reg addr,
                                       const GenRegister *dst,
                                       uint32_t elemNum,
                                       uint32_t bti)
  {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_UNTYPED_READ, elemNum, 1);
    SelectionVector *srcVector = this->appendVector();
    SelectionVector *dstVector = this->appendVector();

    // Regular instruction to encode
    for (uint32_t elemID = 0; elemID < elemNum; ++elemID)
      insn->dst(elemID) = dst[elemID];
    insn->src(0) = addr;
    insn->extra.function = bti;
    insn->extra.elem = elemNum;

    // Sends require contiguous allocation
    dstVector->regNum = elemNum;
    dstVector->isSrc = 0;
    dstVector->reg = &insn->dst(0);

    // Source cannot be scalar (yet)
    srcVector->regNum = 1;
    srcVector->isSrc = 1;
    srcVector->reg = &insn->src(0);
  }

  void Selection::Opaque::UNTYPED_WRITE(Reg addr,
                                        const GenRegister *src,
                                        uint32_t elemNum,
                                        uint32_t bti)
  {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_UNTYPED_WRITE, 0, elemNum+1);
    SelectionVector *vector = this->appendVector();

    // Regular instruction to encode
    insn->src(0) = addr;
    for (uint32_t elemID = 0; elemID < elemNum; ++elemID)
      insn->src(elemID+1) = src[elemID];
    insn->extra.function = bti;
    insn->extra.elem = elemNum;

    // Sends require contiguous allocation for the sources
    vector->regNum = elemNum+1;
    vector->reg = &insn->src(0);
    vector->isSrc = 1;
  }

  void Selection::Opaque::BYTE_GATHER(Reg dst, Reg addr, uint32_t elemSize, uint32_t bti) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_BYTE_GATHER, 1, 1);
    SelectionVector *srcVector = this->appendVector();
    SelectionVector *dstVector = this->appendVector();

    // Instruction to encode
    insn->src(0) = addr;
    insn->dst(0) = dst;
    insn->extra.function = bti;
    insn->extra.elem = elemSize;

    // byte gather requires vector in the sense that scalar are not allowed
    // (yet)
    dstVector->regNum = 1;
    dstVector->isSrc = 0;
    dstVector->reg = &insn->dst(0);
    srcVector->regNum = 1;
    srcVector->isSrc = 1;
    srcVector->reg = &insn->src(0);
  }

  void Selection::Opaque::BYTE_SCATTER(Reg addr, Reg src, uint32_t elemSize, uint32_t bti) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_BYTE_SCATTER, 0, 2);
    SelectionVector *vector = this->appendVector();

    // Instruction to encode
    insn->src(0) = addr;
    insn->src(1) = src;
    insn->extra.function = bti;
    insn->extra.elem = elemSize;

    // value and address are contiguous in the send
    vector->regNum = 2;
    vector->isSrc = 1;
    vector->reg = &insn->src(0);
  }

  void Selection::Opaque::MATH(Reg dst, uint32_t function, Reg src0, Reg src1) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_MATH, 1, 2);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
    insn->extra.function = function;
  }

  void Selection::Opaque::ALU1(SelectionOpcode opcode, Reg dst, Reg src) {
    SelectionInstruction *insn = this->appendInsn(opcode, 1, 1);
    insn->dst(0) = dst;
    insn->src(0) = src;
  }

  void Selection::Opaque::ALU2(SelectionOpcode opcode, Reg dst, Reg src0, Reg src1) {
    SelectionInstruction *insn = this->appendInsn(opcode, 1, 2);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
  }

  void Selection::Opaque::ALU3(SelectionOpcode opcode, Reg dst, Reg src0, Reg src1, Reg src2) {
    SelectionInstruction *insn = this->appendInsn(opcode, 1, 3);
    insn->dst(0) = dst;
    insn->src(0) = src0;
    insn->src(1) = src1;
    insn->src(2) = src2;
  }

  void Selection::Opaque::REGION(Reg dst0, Reg dst1, const GenRegister *src,
                                 uint32_t offset, uint32_t vstride,
                                 uint32_t width, uint32_t hstride,
                                 uint32_t srcNum)
  {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_REGION, 2, srcNum);
    SelectionVector *vector = this->appendVector();

    // Instruction to encode
    insn->dst(0) = dst0;
    insn->dst(1) = dst1;
    GBE_ASSERT(srcNum <= SelectionInstruction::MAX_SRC_NUM);
    for (uint32_t srcID = 0; srcID < srcNum; ++srcID)
      insn->src(srcID) = src[srcID];
    insn->extra.vstride = vstride;
    insn->extra.width = width;
    insn->extra.offset = offset;
    insn->extra.hstride = hstride;

    // Regioning requires contiguous allocation for the sources
    vector->regNum = srcNum;
    vector->reg = &insn->src(0);
    vector->isSrc = 1;
  }

  void Selection::Opaque::RGATHER(Reg dst, const GenRegister *src, uint32_t srcNum)
  {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_RGATHER, 1, srcNum);
    SelectionVector *vector = this->appendVector();

    // Instruction to encode
    insn->dst(0) = dst;
    GBE_ASSERT(srcNum <= SelectionInstruction::MAX_SRC_NUM);
    for (uint32_t srcID = 0; srcID < srcNum; ++srcID)
      insn->src(srcID) = src[srcID];

    // Regioning requires contiguous allocation for the sources
    vector->regNum = srcNum;
    vector->reg = &insn->src(0);
    vector->isSrc = 1;
  }

  void Selection::Opaque::OBREAD(Reg dst, Reg addr, Reg header, uint32_t bti, uint32_t size) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_OBREAD, 1, 2);
    insn->dst(0) = dst;
    insn->src(0) = addr;
    insn->src(1) = header;
    insn->extra.function = bti;
    insn->extra.elem = size / sizeof(int[4]); // number of owords
  }

  void Selection::Opaque::OBWRITE(Reg addr, Reg value, Reg header, uint32_t bti, uint32_t size) {
    SelectionInstruction *insn = this->appendInsn(SEL_OP_OBWRITE, 0, 3);
    SelectionVector *vector = this->appendVector();
    insn->opcode = SEL_OP_OBWRITE;
    insn->src(0) = header;
    insn->src(1) = value;
    insn->src(2) = addr;
    insn->state = this->curr;
    insn->extra.function = bti;
    insn->extra.elem = size / sizeof(int[4]); // number of owords

    // We need to put the header and the data together
    vector->regNum = 2;
    vector->reg = &insn->src(0);
    vector->isSrc = 1;
  }

  // Boiler plate to initialize the selection library at c++ pre-main
  static SelectionLibrary *selLib = NULL;
  static void destroySelectionLibrary(void) { GBE_DELETE(selLib); }
  static struct SelectionLibraryInitializer {
    SelectionLibraryInitializer(void) {
      selLib = GBE_NEW(SelectionLibrary);
      atexit(destroySelectionLibrary);
    }
  } selectionLibraryInitializer;

  bool Selection::Opaque::isRoot(const ir::Instruction &insn) const {
    if (insn.getDstNum() > 1 ||
        insn.hasSideEffect() ||
        insn.isMemberOf<ir::BranchInstruction>() ||
        insn.isMemberOf<ir::LabelInstruction>())
    return true;

    // No side effect, not a branch and no destination? Impossible
    GBE_ASSERT(insn.getDstNum() == 1);

    // Root if alive outside the block.
    // XXX we should use Value and not registers in liveness info
    const ir::BasicBlock *insnBlock = insn.getParent();
    const ir::Liveness &liveness = this->ctx.getLiveness();
    const ir::Liveness::LiveOut &liveOut = liveness.getLiveOut(insnBlock);
    const ir::Register reg = insn.getDst(0);
    if (liveOut.contains(reg))
      return true;

    // The instruction is only used in the current basic block
    return false;
  }

  uint32_t Selection::Opaque::buildBasicBlockDAG(const ir::BasicBlock &bb)
  {
    using namespace ir;

    // Clear all registers
    for (uint32_t regID = 0; regID < this->regNum; ++regID)
      this->regDAG[regID] = NULL;

    // Build the DAG on the fly
    uint32_t insnNum = 0;
    bb.foreach([&](const Instruction &insn) {

      // Build a selectionDAG node for instruction
      SelectionDAG *dag = this->newSelectionDAG(insn);

      // Point to non-root children
      const uint32_t srcNum = insn.getSrcNum();
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        const ir::Register reg = insn.getSrc(srcID);
        SelectionDAG *child = this->regDAG[reg];
        if (child) {
          const ir::Instruction &childInsn = child->insn;
          const uint32_t childSrcNum = childInsn.getSrcNum();

          // We can merge a child only if its sources are still valid
          bool mergeable = true;
          for (uint32_t otherID = 0; otherID < childSrcNum; ++otherID) {
            const SelectionDAG *srcDAG = child->child[otherID];
            const ir::Register srcReg = childInsn.getSrc(otherID);
            SelectionDAG *currDAG = this->regDAG[srcReg];
            if (srcDAG != currDAG) {
              mergeable = false;
              break;
            }
          }
          if (mergeable) dag->setAsMergeable(srcID);
          dag->child[srcID] = child;
        } else
          dag->child[srcID] = NULL;
      }

      // Make it a root if we must
      if (this->isRoot(insn)) dag->isRoot = 1;

      // Save the DAG <-> instruction mapping
      this->insnDAG[insnNum++] = dag;

      // Associate all output registers to this instruction
      const uint32_t dstNum = insn.getDstNum();
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
        const ir::Register reg = insn.getDst(dstID);
        this->regDAG[reg] = dag;
      }
    });

    return insnNum;
  }

  void Selection::Opaque::matchBasicBlock(uint32_t insnNum)
  {
    // Bottom up code generation
    for (int32_t insnID = insnNum-1; insnID >= 0; --insnID) {
      // Process all possible patterns for this instruction
      SelectionDAG &dag = *insnDAG[insnID];
      if (dag.isRoot) {
        const ir::Instruction &insn = dag.insn;
        const ir::Opcode opcode = insn.getOpcode();
        auto it = selLib->patterns[opcode].begin();
        const auto end = selLib->patterns[opcode].end();

        // Start a new code fragment
        this->startBackwardGeneration();

        // Try all the patterns from best to worst
        do {
          if ((*it)->emit(*this, dag))
            break;
          ++it;
        } while (it != end);
        GBE_ASSERT(it != end);

        // Output the code in the current basic block
        this->endBackwardGeneration();
      }
    }
  }

  void Selection::Opaque::select(void)
  {
    using namespace ir;
    const Function &fn = ctx.getFunction();

    // Perform the selection per basic block
    fn.foreachBlock([&](const BasicBlock &bb) {
       this->dagPool.rewind();
      this->appendBlock(bb);
      const uint32_t insnNum = this->buildBasicBlockDAG(bb);
      this->matchBasicBlock(insnNum);
    });
  }

  ///////////////////////////////////////////////////////////////////////////
  // Code selection public implementation
  ///////////////////////////////////////////////////////////////////////////

  Selection::Selection(GenContext &ctx) {
    this->opaque = GBE_NEW(Selection::Opaque, ctx);
  }

  Selection::~Selection(void) { GBE_DELETE(this->opaque); }

  void Selection::select(void) {
    this->opaque->select();
    this->blockHead = this->opaque->blockHead;
    this->blockTail = this->opaque->blockTail;
  }

  bool Selection::isScalarOrBool(ir::Register reg) const {
    return this->opaque->isScalarOrBool(reg);
  }

  uint32_t Selection::getLargestBlockSize(void) const {
    return this->opaque->getLargestBlockSize();
  }

  uint32_t Selection::getVectorNum(void) const {
    return this->opaque->getVectorNum();
  }

  uint32_t Selection::getRegNum(void) const {
    return this->opaque->getRegNum();
  }

  ir::RegisterFamily Selection::getRegisterFamily(ir::Register reg) const {
    return this->opaque->getRegisterFamily(reg);
  }

  ir::RegisterData Selection::getRegisterData(ir::Register reg) const {
    return this->opaque->getRegisterData(reg);
  }

  ir::Register Selection::replaceSrc(SelectionInstruction *insn, uint32_t regID) {
    return this->opaque->replaceSrc(insn, regID);
  }

  ir::Register Selection::replaceDst(SelectionInstruction *insn, uint32_t regID) {
    return this->opaque->replaceDst(insn, regID);
  }

  SelectionInstruction *Selection::create(SelectionOpcode opcode, uint32_t dstNum, uint32_t srcNum) {
    return this->opaque->create(opcode, dstNum, srcNum);
  }

  ///////////////////////////////////////////////////////////////////////////
  // Implementation of all patterns
  ///////////////////////////////////////////////////////////////////////////

  GenRegister getRegisterFromImmediate(ir::Immediate imm) 
  {
    using namespace ir;
    switch (imm.type) {
      case TYPE_U32:   return GenRegister::immud(imm.data.u32);
      case TYPE_S32:   return GenRegister::immd(imm.data.s32);
      case TYPE_FLOAT: return GenRegister::immf(imm.data.f32);
      case TYPE_U16: return GenRegister::immuw(imm.data.u16);
      case TYPE_S16: return  GenRegister::immw(imm.data.s16);
      case TYPE_U8:  return GenRegister::immuw(imm.data.u8);
      case TYPE_S8:  return GenRegister::immw(imm.data.s8);
      default: NOT_SUPPORTED; return GenRegister::immuw(0);
    }
  }

  /*! Template for the one-to-many instruction patterns */
  template <typename T, typename U>
  class OneToManyPattern : public SelectionPattern
  {
  public:
    /*! Register the pattern for all opcodes of the family */
    OneToManyPattern(uint32_t insnNum, uint32_t cost) :
      SelectionPattern(insnNum, cost)
    {
      for (uint32_t op = 0; op < ir::OP_INVALID; ++op)
        if (ir::isOpcodeFrom<U>(ir::Opcode(op)) == true)
          this->opcodes.push_back(ir::Opcode(op));
    }
    /*! Call the child method with the proper prototype */
    virtual bool emit(Selection::Opaque &sel, SelectionDAG &dag) const {
      if (static_cast<const T*>(this)->emitOne(sel, ir::cast<U>(dag.insn))) {
        this->markAllChildren(dag);
        return true;
      }
      return false;
    }
  };

/*! Declare a naive one-to-many pattern */
#define DECL_PATTERN(FAMILY) \
  struct FAMILY##Pattern : public OneToManyPattern<FAMILY##Pattern, ir::FAMILY>

#define DECL_CTOR(FAMILY, INSN_NUM, COST) \
  FAMILY##Pattern(void) : OneToManyPattern<FAMILY##Pattern, ir::FAMILY>(INSN_NUM, COST) {}

  /*! Unary instruction patterns */
  DECL_PATTERN(UnaryInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::UnaryInstruction &insn) const {
      GBE_ASSERT(insn.getOpcode() == ir::OP_MOV);
      sel.MOV(sel.selReg(insn.getDst(0)), sel.selReg(insn.getSrc(0)));
      return true;
    }
    DECL_CTOR(UnaryInstruction, 1, 1)
  };

  BVAR(OCL_OPTIMIZE_IMMEDIATE, true);

  DECL_PATTERN(TernaryInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::TernaryInstruction &insn) const {
      using namespace ir;
      const Type type = insn.getType();
      const GenRegister dst  = sel.selReg(insn.getDst(0), type);
      const GenRegister src0 = sel.selReg(insn.getSrc(0), type);
      const GenRegister src1 = sel.selReg(insn.getSrc(1), type);
      const GenRegister src2 = sel.selReg(insn.getSrc(2), type);
#if GBE_DEBUG
      const Opcode opcode = insn.getOpcode();
      GBE_ASSERT(opcode == OP_MAD && type == TYPE_FLOAT);
#endif /* GBE_DEBUG */
      sel.MAD(dst, src2, src0, src1); // order different on the HW!
      return true;
    }
    DECL_CTOR(TernaryInstruction, 1, 1)
  };

  /*! Binary regular instruction pattern */
  class BinaryInstructionPattern : public SelectionPattern
  {
  public:
    BinaryInstructionPattern(void) : SelectionPattern(1,1) {
      for (uint32_t op = 0; op < ir::OP_INVALID; ++op)
        if (ir::isOpcodeFrom<ir::BinaryInstruction>(ir::Opcode(op)) == true)
          this->opcodes.push_back(ir::Opcode(op));
    }

    INLINE bool emit(Selection::Opaque &sel, SelectionDAG &dag) const
    {
      using namespace ir;
      const ir::BinaryInstruction &insn = cast<BinaryInstruction>(dag.insn);
      const Opcode opcode = insn.getOpcode();
      const Type type = insn.getType();
      GenRegister dst  = sel.selReg(insn.getDst(0), type);

      // Immediates not supported
      if (opcode == OP_DIV) {
        GBE_ASSERT(type == TYPE_FLOAT);
        GenRegister src0 = sel.selReg(insn.getSrc(0), type);
        GenRegister src1 = sel.selReg(insn.getSrc(1), type);
        sel.MATH(dst, GEN_MATH_FUNCTION_FDIV, src0, src1);
        this->markAllChildren(dag);
        return true;
      }

      sel.push();

      // Boolean values use scalars
      if (sel.isScalarOrBool(insn.getDst(0)) == true) {
        sel.curr.execWidth = 1;
        sel.curr.predicate = GEN_PREDICATE_NONE;
        sel.curr.noMask = 1;
      }

      // Look for immediate values
      GenRegister src0, src1;
      SelectionDAG *dag0 = dag.child[0];
      SelectionDAG *dag1 = dag.child[1];

      // Right source can always be an immediate
      if (OCL_OPTIMIZE_IMMEDIATE && dag1 != NULL && dag1->insn.getOpcode() == OP_LOADI) {
        const auto &childInsn = cast<LoadImmInstruction>(dag1->insn);
        src0 = sel.selReg(insn.getSrc(0), type);
        src1 = getRegisterFromImmediate(childInsn.getImmediate());
        if (dag0) dag0->isRoot = 1;
      }
      // Left source cannot be immediate but it is OK if we can commute
      else if (OCL_OPTIMIZE_IMMEDIATE && dag0 != NULL && insn.commutes() && dag0->insn.getOpcode() == OP_LOADI) {
        const auto &childInsn = cast<LoadImmInstruction>(dag0->insn);
        src0 = sel.selReg(insn.getSrc(1), type);
        src1 = getRegisterFromImmediate(childInsn.getImmediate());
        if (dag1) dag1->isRoot = 1;
      }
      // Just grab the two sources
      else {
        src0 = sel.selReg(insn.getSrc(0), type);
        src1 = sel.selReg(insn.getSrc(1), type);
        this->markAllChildren(dag);
      }

      // Output the binary instruction
      switch (opcode) {
        case OP_ADD: sel.ADD(dst, src0, src1); break;
        case OP_XOR: sel.XOR(dst, src0, src1); break;
        case OP_OR:  sel.OR(dst, src0,  src1); break;
        case OP_AND: sel.AND(dst, src0, src1); break;
        case OP_SUB: sel.ADD(dst, src0, GenRegister::negate(src1)); break;
        case OP_SHL: sel.SHL(dst, src0, src1); break;
        case OP_SHR: sel.SHR(dst, src0, src1); break;
        case OP_MUL:
          if (type == TYPE_FLOAT)
            sel.MUL(dst, src0, src1);
          else if (type == TYPE_U32 || type == TYPE_S32)
            return false;
          else
            NOT_IMPLEMENTED;
        break;
        default: NOT_IMPLEMENTED;
      }
      sel.pop();
      return true;
    }
  };

  /*! Force MAD pattern */
  BVAR(OCL_FORCE_MAD_PATTERN, false);

  /*! MAD pattern */
  class MulAddInstructionPattern : public SelectionPattern
  {
  public:
    /*! Register the pattern for all opcodes of the family */
    MulAddInstructionPattern(void) : SelectionPattern(2, 1) {
       this->opcodes.push_back(ir::OP_ADD);
    }

    /*! Implements base class */
    virtual bool emit(Selection::Opaque  &sel, SelectionDAG &dag) const
    {
      using namespace ir;
      const ir::BinaryInstruction &insn = cast<ir::BinaryInstruction>(dag.insn);
      if (insn.getType() != TYPE_FLOAT)
        return false;
      SelectionDAG *child0 = dag.child[0];
      SelectionDAG *child1 = dag.child[1];
      const GenRegister dst = sel.selReg(insn.getDst(0), TYPE_FLOAT);
      if (child0 && child0->insn.getOpcode() == OP_MUL) {
        GBE_ASSERT(cast<ir::BinaryInstruction>(child0->insn).getType() == TYPE_FLOAT);
        const GenRegister src0 = sel.selReg(child0->insn.getSrc(0), TYPE_FLOAT);
        const GenRegister src1 = sel.selReg(child0->insn.getSrc(1), TYPE_FLOAT);
        const GenRegister src2 = sel.selReg(insn.getSrc(1), TYPE_FLOAT);
        sel.MAD(dst, src2, src0, src1); // order different on HW!
        if (child0->child[0]) child0->child[0]->isRoot = 1;
        if (child0->child[1]) child0->child[1]->isRoot = 1;
        if (child1) child1->isRoot = 1;
        return true;
      }
      if (child1 && child1->insn.getOpcode() == OP_MUL) {
        GBE_ASSERT(cast<ir::BinaryInstruction>(child1->insn).getType() == TYPE_FLOAT);
        const GenRegister src0 = sel.selReg(child1->insn.getSrc(0), TYPE_FLOAT);
        const GenRegister src1 = sel.selReg(child1->insn.getSrc(1), TYPE_FLOAT);
        const GenRegister src2 = sel.selReg(insn.getSrc(0), TYPE_FLOAT);
        sel.MAD(dst, src2, src0, src1); // order different on HW!
        if (child1->child[0]) child1->child[0]->isRoot = 1;
        if (child1->child[1]) child1->child[1]->isRoot = 1;
        if (child0) child0->isRoot = 1;
        return true;
      }
      return false;
    }
  };

  /*! 32 bits integer multiply needs more instructions */
  class Int32x32MulInstructionPattern : public SelectionPattern
  {
  public:
    /*! Register the pattern for all opcodes of the family */
    Int32x32MulInstructionPattern(void) : SelectionPattern(1, 4) {
       this->opcodes.push_back(ir::OP_MUL);
    }

    /*! Implements base class */
    virtual bool emit(Selection::Opaque  &sel, SelectionDAG &dag) const
    {
      using namespace ir;
      const ir::BinaryInstruction &insn = cast<ir::BinaryInstruction>(dag.insn);
      const uint32_t simdWidth = sel.curr.execWidth;
      const Type type = insn.getType();
      if (type == TYPE_U32 || type == TYPE_S32) {
        GenRegister dst  = sel.selReg(insn.getDst(0), type);
        GenRegister src0 = sel.selReg(insn.getSrc(0), type);
        GenRegister src1 = sel.selReg(insn.getSrc(1), type);

        sel.push();

        // Either left part of the 16-wide register or just a simd 8 register
        dst  = GenRegister::retype(dst,  GEN_TYPE_D);
        src0 = GenRegister::retype(src0, GEN_TYPE_D);
        src1 = GenRegister::retype(src1, GEN_TYPE_D);
        sel.curr.execWidth = 8;
        sel.curr.quarterControl = GEN_COMPRESSION_Q1;
        sel.MUL(GenRegister::retype(GenRegister::acc(), GEN_TYPE_D), src0, src1);
        sel.curr.accWrEnable = 1;
        sel.MACH(GenRegister::retype(GenRegister::null(), GEN_TYPE_D), src0, src1);
        sel.curr.accWrEnable = 0;
        sel.MOV(GenRegister::retype(dst, GEN_TYPE_F), GenRegister::acc());

        // Right part of the 16-wide register now
        if (simdWidth == 16) {
          sel.curr.noMask = 1;
          const GenRegister nextSrc0 = sel.selRegQn(insn.getSrc(0), 1, TYPE_S32);
          const GenRegister nextSrc1 = sel.selRegQn(insn.getSrc(1), 1, TYPE_S32);
          sel.MUL(GenRegister::retype(GenRegister::acc(), GEN_TYPE_D), nextSrc0, nextSrc1);
          sel.curr.accWrEnable = 1;
          sel.MACH(GenRegister::retype(GenRegister::null(), GEN_TYPE_D), nextSrc0, nextSrc1);
          sel.curr.accWrEnable = 0;
          sel.curr.quarterControl = GEN_COMPRESSION_Q2;
          const ir::Register reg = sel.reg(FAMILY_DWORD);
          sel.MOV(GenRegister::f8grf(reg), GenRegister::acc());
          sel.curr.noMask = 0;
          sel.MOV(GenRegister::retype(GenRegister::next(dst), GEN_TYPE_F),
                  GenRegister::f8grf(reg));
        }

        sel.pop();

        // All children are marked as root
        this->markAllChildren(dag);
        return true;
      } else
        return false;
    }
  };

  /*! 32x16 bits integer can be done in one instruction */
  class Int32x16MulInstructionPattern : public SelectionPattern
  {
  public:
    /*! Register the pattern for all opcodes of the family */
    Int32x16MulInstructionPattern(void) : SelectionPattern(1, 1) {
       this->opcodes.push_back(ir::OP_MUL);
    }

    bool is16BitSpecialReg(ir::Register reg) const {
      if (reg == ir::ocl::lid0 ||
          reg == ir::ocl::lid1 ||
          reg == ir::ocl::lid2 ||
          reg == ir::ocl::lsize0 ||
          reg == ir::ocl::lsize1||
          reg == ir::ocl::lsize2)
        return true;
      else
        return false;
    }

    /*! Try to emit a multiply where child childID is a 16 immediate */
    bool emitMulImmediate(Selection::Opaque  &sel, SelectionDAG &dag, uint32_t childID) const {
      using namespace ir;
      const ir::BinaryInstruction &insn = cast<ir::BinaryInstruction>(dag.insn);
      const Register dst  = insn.getDst(0);
      const Register src1 = insn.getSrc(childID ^ 1);
      const SelectionDAG *src0DAG = dag.child[childID];
      if (src0DAG != NULL) {
        if (src0DAG->insn.getOpcode() == OP_LOADI) {
          const auto &loadimm = cast<LoadImmInstruction>(src0DAG->insn);
          const Immediate imm = loadimm.getImmediate();
          const Type type = imm.type;
          GBE_ASSERT(type == TYPE_U32 || type == TYPE_S32);
          if (type == TYPE_U32 && imm.data.u32 <= 0xffff) {
            sel.MUL(sel.selReg(dst, type),
                    sel.selReg(src1, type),
                    GenRegister::immuw(imm.data.u32));
            if (dag.child[childID ^ 1] != NULL)
              dag.child[childID ^ 1]->isRoot = 1;
            return true;
          }
          if (type == TYPE_S32 && (imm.data.s32 >= -32768 && imm.data.s32 <= 32767)) {
            sel.MUL(sel.selReg(dst, type),
                    sel.selReg(src1, type),
                    GenRegister::immw(imm.data.s32));
            if (dag.child[childID ^ 1] != NULL)
              dag.child[childID ^ 1]->isRoot = 1;
            return true;
          }
        }
      }
      return false;
    }

    /*! Try to emit a multiply with a 16 bit special register */
    bool emitMulSpecialReg(Selection::Opaque &sel, SelectionDAG &dag, uint32_t childID) const {
      using namespace ir;
      const BinaryInstruction &insn = cast<ir::BinaryInstruction>(dag.insn);
      const Type type = insn.getType();
      const Register dst  = insn.getDst(0);
      const Register src0 = insn.getSrc(childID);
      const Register src1 = insn.getSrc(childID ^ 1);
      if (is16BitSpecialReg(src0)) {
        sel.MUL(sel.selReg(dst, type),
                sel.selReg(src1, type),
                sel.selReg(src0, TYPE_U16));
        this->markAllChildren(dag);
        return true;
      }
      return false;
    }

    virtual bool emit(Selection::Opaque &sel, SelectionDAG &dag) const
    {
      using namespace ir;
      const BinaryInstruction &insn = cast<ir::BinaryInstruction>(dag.insn);
      const Type type = insn.getType();
      if (type == TYPE_U32 || type == TYPE_S32) {
        if (this->emitMulSpecialReg(sel, dag, 0))
          return true;
        if (this->emitMulSpecialReg(sel, dag, 1))
          return true;
        if (this->emitMulImmediate(sel, dag, 0))
          return true;
        if (this->emitMulImmediate(sel, dag, 1))
          return true;
      }
      return false;
    }
  };

#define DECL_NOT_IMPLEMENTED_ONE_TO_MANY(FAMILY) \
  struct FAMILY##Pattern : public OneToManyPattern<FAMILY##Pattern, ir::FAMILY>\
  {\
    INLINE bool emitOne(Selection::Opaque &sel, const ir::FAMILY &insn) const {\
      NOT_IMPLEMENTED;\
      return false;\
    }\
    DECL_CTOR(FAMILY, 1, 1); \
  }
  DECL_NOT_IMPLEMENTED_ONE_TO_MANY(SampleInstruction);
  DECL_NOT_IMPLEMENTED_ONE_TO_MANY(TypedWriteInstruction);
  DECL_NOT_IMPLEMENTED_ONE_TO_MANY(FenceInstruction);
#undef DECL_NOT_IMPLEMENTED_ONE_TO_MANY

  /*! Load immediate pattern */
  DECL_PATTERN(LoadImmInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::LoadImmInstruction &insn) const
    {
      using namespace ir;
      const Type type = insn.getType();
      const Immediate imm = insn.getImmediate();
      const GenRegister dst = sel.selReg(insn.getDst(0), type);

      switch (type) {
        case TYPE_U32:
        case TYPE_S32:
        case TYPE_FLOAT:
          sel.MOV(GenRegister::retype(dst, GEN_TYPE_F),
                  GenRegister::immf(imm.data.f32));
        break;
        case TYPE_U16: sel.MOV(dst, GenRegister::immuw(imm.data.u16)); break;
        case TYPE_S16: sel.MOV(dst, GenRegister::immw(imm.data.s16)); break;
        case TYPE_U8:  sel.MOV(dst, GenRegister::immuw(imm.data.u8)); break;
        case TYPE_S8:  sel.MOV(dst, GenRegister::immw(imm.data.s8)); break;
        default: NOT_SUPPORTED;
      }
      return true;
    }

    DECL_CTOR(LoadImmInstruction, 1,1);
  };

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

  /*! Load instruction pattern */
  DECL_PATTERN(LoadInstruction)
  {
    void emitUntypedRead(Selection::Opaque &sel, const ir::LoadInstruction &insn, GenRegister addr) const
    {
      using namespace ir;
      const uint32_t valueNum = insn.getValueNum();
      GenRegister dst[valueNum];
      for (uint32_t dstID = 0; dstID < valueNum; ++dstID)
        dst[dstID] = GenRegister::retype(sel.selReg(insn.getValue(dstID)), GEN_TYPE_F);
      sel.UNTYPED_READ(addr, dst, valueNum, 0);
    }

    void emitByteGather(Selection::Opaque &sel,
                        const ir::LoadInstruction &insn,
                        GenRegister address,
                        GenRegister value) const
    {
      using namespace ir;
      GBE_ASSERT(insn.getValueNum() == 1);
      const Type type = insn.getValueType();
      const uint32_t elemSize = getByteScatterGatherSize(type);
      const uint32_t simdWidth = sel.ctx.getSimdWidth();

      // We need a temporary register if we read bytes or words
      Register dst = Register(value.value.reg);
      if (elemSize == GEN_BYTE_SCATTER_WORD ||
          elemSize == GEN_BYTE_SCATTER_BYTE) {
        dst = sel.reg(FAMILY_DWORD);
        sel.BYTE_GATHER(GenRegister::fxgrf(simdWidth, dst), address, elemSize, 0);
      }

      // Repack bytes or words using a converting mov instruction
      if (elemSize == GEN_BYTE_SCATTER_WORD)
        sel.MOV(GenRegister::retype(value, GEN_TYPE_UW), GenRegister::unpacked_uw(dst));
      else if (elemSize == GEN_BYTE_SCATTER_BYTE)
        sel.MOV(GenRegister::retype(value, GEN_TYPE_UB), GenRegister::unpacked_ub(dst));
    }

    INLINE bool emitOne(Selection::Opaque &sel, const ir::LoadInstruction &insn) const {
      using namespace ir;
      const GenRegister address = sel.selReg(insn.getAddress());
      GBE_ASSERT(insn.getAddressSpace() == MEM_GLOBAL ||
                 insn.getAddressSpace() == MEM_PRIVATE);
      GBE_ASSERT(sel.ctx.isScalarReg(insn.getValue(0)) == false);
      if (insn.isAligned() == true)
        this->emitUntypedRead(sel, insn, address);
      else {
        const GenRegister value = sel.selReg(insn.getValue(0));
        this->emitByteGather(sel, insn, address, value);
      }
      return true;
    }
    DECL_CTOR(LoadInstruction, 1, 1);
  };

  /*! Store instruction pattern */
  DECL_PATTERN(StoreInstruction)
  {
    void emitUntypedWrite(Selection::Opaque &sel, const ir::StoreInstruction &insn) const
    {
      using namespace ir;
      const uint32_t valueNum = insn.getValueNum();
      const uint32_t addrID = ir::StoreInstruction::addressIndex;
      GenRegister addr, value[valueNum];

      addr = GenRegister::retype(sel.selReg(insn.getSrc(addrID)), GEN_TYPE_F);;
      for (uint32_t valueID = 0; valueID < valueNum; ++valueID)
        value[valueID] = GenRegister::retype(sel.selReg(insn.getValue(valueID)), GEN_TYPE_F);
      sel.UNTYPED_WRITE(addr, value, valueNum, 0);
    }

    void emitByteScatter(Selection::Opaque &sel,
                         const ir::StoreInstruction &insn,
                         GenRegister addr,
                         GenRegister value) const
    {
      using namespace ir;
      const Type type = insn.getValueType();
      const uint32_t elemSize = getByteScatterGatherSize(type);
      const uint32_t simdWidth = sel.ctx.getSimdWidth();
      const GenRegister dst = value;

      GBE_ASSERT(insn.getValueNum() == 1);
      if (elemSize == GEN_BYTE_SCATTER_WORD) {
        value = GenRegister::udxgrf(simdWidth, sel.reg(FAMILY_DWORD));
        sel.MOV(value, GenRegister::retype(dst, GEN_TYPE_UW));
      } else if (elemSize == GEN_BYTE_SCATTER_BYTE) {
        value = GenRegister::udxgrf(simdWidth, sel.reg(FAMILY_DWORD));
        sel.MOV(value, GenRegister::retype(dst, GEN_TYPE_UB));
      }
      sel.BYTE_SCATTER(addr, value, elemSize, 1);
    }

    INLINE bool emitOne(Selection::Opaque &sel, const ir::StoreInstruction &insn) const {
      using namespace ir;
      if (insn.isAligned() == true)
        this->emitUntypedWrite(sel, insn);
      else {
        const GenRegister address = sel.selReg(insn.getAddress());
        const GenRegister value = sel.selReg(insn.getValue(0));
        this->emitByteScatter(sel, insn, address, value);
      }
      return true;
    }
    DECL_CTOR(StoreInstruction, 1, 1);
  };

  /*! Compare instruction pattern */
  DECL_PATTERN(CompareInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::CompareInstruction &insn) const
    {
      using namespace ir;
      const Opcode opcode = insn.getOpcode();
      const Type type = insn.getType();
      const uint32_t genCmp = getGenCompare(opcode);
      const Register dst  = insn.getDst(0);
      const GenRegister src0 = sel.selReg(insn.getSrc(0), type);
      const GenRegister src1 = sel.selReg(insn.getSrc(1), type);

      // Limit the compare to the active lanes. Use the same compare as for f0.0
      sel.push();
        const LabelIndex label = insn.getParent()->getLabelIndex();
        const GenRegister blockip = sel.selReg(ocl::blockip, TYPE_U16);
        const GenRegister labelReg = GenRegister::immuw(label);
        sel.curr.predicate = GEN_PREDICATE_NONE;
        sel.curr.physicalFlag = 0;
        sel.curr.flagIndex = uint16_t(dst);
        sel.CMP(GEN_CONDITIONAL_LE, blockip, labelReg);
      sel.pop();

      sel.push();
        sel.curr.physicalFlag = 0;
        sel.curr.flagIndex = uint16_t(dst);
        sel.CMP(genCmp, src0, src1);
      sel.pop();
      return true;
    }
    DECL_CTOR(CompareInstruction, 1, 1);
  };

  /*! Convert instruction pattern */
  DECL_PATTERN(ConvertInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::ConvertInstruction &insn) const
    {
      using namespace ir;
      const Type dstType = insn.getDstType();
      const Type srcType = insn.getSrcType();
      const RegisterFamily dstFamily = getFamily(dstType);
      const RegisterFamily srcFamily = getFamily(srcType);
      const GenRegister dst = sel.selReg(insn.getDst(0), dstType);
      const GenRegister src = sel.selReg(insn.getSrc(0), srcType);

      // We need two instructions to make the conversion
      if (dstFamily != FAMILY_DWORD && srcFamily == FAMILY_DWORD) {
        GenRegister unpacked;
        if (dstFamily == FAMILY_WORD) {
          const uint32_t type = TYPE_U16 ? GEN_TYPE_UW : GEN_TYPE_W;
          unpacked = GenRegister::unpacked_uw(sel.reg(FAMILY_DWORD));
          unpacked = GenRegister::retype(unpacked, type);
        } else {
          const uint32_t type = TYPE_U8 ? GEN_TYPE_UB : GEN_TYPE_B;
          unpacked = GenRegister::unpacked_ub(sel.reg(FAMILY_DWORD));
          unpacked = GenRegister::retype(unpacked, type);
        }
        sel.MOV(unpacked, src);
        sel.MOV(dst, unpacked);
      } else
        sel.MOV(dst, src);
      return true;
    }
    DECL_CTOR(ConvertInstruction, 1, 1);
  };

  /*! Select instruction pattern */
  DECL_PATTERN(SelectInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::SelectInstruction &insn) const
    {
      using namespace ir;

      // Get all registers for the instruction
      const Type type = insn.getType();
      const GenRegister dst  = sel.selReg(insn.getDst(0), type);
      const GenRegister src0 = sel.selReg(insn.getSrc(SelectInstruction::src0Index), type);
      const GenRegister src1 = sel.selReg(insn.getSrc(SelectInstruction::src1Index), type);

      // Since we cannot predicate the select instruction with our current mask,
      // we need to perform the selection in two steps (one to select, one to
      // update the destination register)
      const RegisterFamily family = getFamily(type);
      const GenRegister tmp = sel.selReg(sel.reg(family), type);
      const uint32_t simdWidth = sel.ctx.getSimdWidth();
      const Register pred = insn.getPredicate();
      sel.push();
        sel.curr.predicate = GEN_PREDICATE_NORMAL;
        sel.curr.execWidth = simdWidth;
        sel.curr.physicalFlag = 0;
        sel.curr.flagIndex = uint16_t(pred);
        sel.curr.noMask = 0;
        sel.SEL(tmp, src0, src1);
      sel.pop();

      // Update the destination register properly now
      sel.MOV(dst, tmp);
      return true;
    }
    DECL_CTOR(SelectInstruction, 1, 1);
  };

  /*! Label instruction pattern */
  DECL_PATTERN(LabelInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::LabelInstruction &insn) const
    {
      using namespace ir;
      const LabelIndex label = insn.getLabelIndex();
      const GenRegister src0 = sel.selReg(ocl::blockip);
      const GenRegister src1 = GenRegister::immuw(label);
      const uint32_t simdWidth = sel.ctx.getSimdWidth();
      sel.LABEL(label);

     // Do not emit any code for the "returning" block. There is no need for it
     if (insn.getParent() == &sel.ctx.getFunction().getBottomBlock())
        return true;

      // Emit the mask computation at the head of each basic block
      sel.push();
        sel.curr.predicate = GEN_PREDICATE_NONE;
        sel.curr.flag = 0;
        sel.curr.subFlag = 0;
        sel.CMP(GEN_CONDITIONAL_LE, GenRegister::retype(src0, GEN_TYPE_UW), src1);
      sel.pop();

      // If it is required, insert a JUMP to bypass the block
      if (sel.ctx.hasJIP(&insn)) {
        const LabelIndex jip = sel.ctx.getLabelIndex(&insn);
        sel.push();
          if (simdWidth == 8)
            sel.curr.predicate = GEN_PREDICATE_ALIGN1_ANY8H;
          else if (simdWidth == 16)
            sel.curr.predicate = GEN_PREDICATE_ALIGN1_ANY16H;
          else
            NOT_IMPLEMENTED;
          sel.curr.inversePredicate = 1;
          sel.curr.execWidth = 1;
          sel.curr.flag = 0;
          sel.curr.subFlag = 0;
          sel.curr.noMask = 1;
          sel.JMPI(GenRegister::immd(0), jip);
        sel.pop();
      }
      return true;
    }
    DECL_CTOR(LabelInstruction, 1, 1);
  };

  /*! Region instruction pattern */
  DECL_PATTERN(RegionInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::RegionInstruction &insn) const
    {
      using namespace ir;

      // Two destinations: one is the real destination, one is a temporary
      GenRegister dst0 = sel.selReg(insn.getDst(0)), dst1;
      if (sel.ctx.getSimdWidth() == 8)
        dst1 = GenRegister::ud8grf(sel.reg(FAMILY_DWORD));
      else
        dst1 = GenRegister::ud16grf(sel.reg(FAMILY_DWORD));

      // Get all the sources
      GenRegister src[SelectionInstruction::MAX_SRC_NUM];
      const uint32_t srcNum = insn.getSrcNum();
      GBE_ASSERT(srcNum <= SelectionInstruction::MAX_SRC_NUM);
      for (uint32_t srcID = 0; srcID < insn.getSrcNum(); ++srcID)
        src[srcID] = sel.selReg(insn.getSrc(srcID));

      // Get the region parameters
      const uint32_t offset = insn.getOffset();
      const uint32_t vstride = insn.getVStride();
      const uint32_t width = insn.getWidth();
      const uint32_t hstride = insn.getHStride();
      sel.REGION(dst0, dst1, src, offset, vstride, width, hstride, srcNum);
      return true;
    }
    DECL_CTOR(RegionInstruction, 1, 1);
  };

  /*! Vote instruction pattern */
  DECL_PATTERN(VoteInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::VoteInstruction &insn) const
    {
      using namespace ir;
      const uint32_t simdWidth = sel.ctx.getSimdWidth();
      const GenRegister dst = sel.selReg(insn.getDst(0), TYPE_U16);
      const GenRegister src = sel.selReg(insn.getSrc(0), TYPE_U16);

      // Limit the vote to the active lanes. Use the same compare as for f0.0
      sel.push();
        const LabelIndex label = insn.getParent()->getLabelIndex();
        const GenRegister blockip = sel.selReg(ocl::blockip, TYPE_U16);
        const GenRegister labelReg = GenRegister::immuw(label);
        sel.curr.predicate = GEN_PREDICATE_NONE;
        sel.curr.flag = 0;
        sel.curr.subFlag = 1;
        sel.CMP(GEN_CONDITIONAL_LE, blockip, labelReg);
      sel.pop();

      // Emit the compare instruction to get the flag register
      sel.push();
        const VotePredicate vote = insn.getVotePredicate();
        const uint32_t genCmp = vote == VOTE_ANY ? GEN_CONDITIONAL_NEQ : GEN_CONDITIONAL_EQ;
        sel.curr.flag = 0;
        sel.curr.subFlag = 1;
        sel.CMP(genCmp, src, GenRegister::immuw(0));
      sel.pop();

      // Broadcast the result to the destination
      if (vote == VOTE_ANY)
        sel.MOV(dst, GenRegister::flag(0,1));
      else {
        const GenRegister tmp = sel.selReg(sel.reg(FAMILY_WORD), TYPE_U16);
        sel.push();
          // Set all lanes of tmp to zero
          sel.curr.predicate = GEN_PREDICATE_NONE;
          sel.MOV(tmp, GenRegister::immuw(0));

          // Compute the short values with no mask
          sel.curr.flag = 0;
          sel.curr.subFlag = 1;
          sel.curr.inversePredicate = 1;
          sel.curr.predicate = simdWidth == 8 ?
            GEN_PREDICATE_ALIGN1_ANY8H :
            GEN_PREDICATE_ALIGN1_ANY16H;
          sel.MOV(tmp, GenRegister::immuw(1));
        sel.pop();

        // Update the destination with the proper mask
        sel.MOV(dst, tmp);
      }
      return true;
    }

    DECL_CTOR(VoteInstruction, 1, 1);
  };

  /*! Register file gather instruction pattern */
  DECL_PATTERN(RGatherInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::RGatherInstruction &insn) const
    {
      using namespace ir;
      // Two destinations: one is the real destination, one is a temporary
      const GenRegister dst = sel.selReg(insn.getDst(0)), dst1;

      // Get all the sources
      GenRegister src[SelectionInstruction::MAX_SRC_NUM];
      const uint32_t srcNum = insn.getSrcNum();
      GBE_ASSERT(srcNum <= SelectionInstruction::MAX_SRC_NUM);
      for (uint32_t srcID = 0; srcID < insn.getSrcNum(); ++srcID)
        src[srcID] = sel.selReg(insn.getSrc(srcID));

      // Get the region parameters
      sel.RGATHER(dst, src, srcNum);
      return true;
    }

    DECL_CTOR(RGatherInstruction, 1, 1);
  };

  /*! OBlock read instruction pattern */
  DECL_PATTERN(OBReadInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::OBReadInstruction &insn) const
    {
      using namespace ir;
      const GenRegister header = sel.selReg(sel.reg(FAMILY_DWORD), TYPE_U32);
      const GenRegister addr = sel.selReg(insn.getAddress(), TYPE_U32);
      const GenRegister value = sel.selReg(insn.getValue(), TYPE_U32);
      const uint32_t simdWidth = sel.ctx.getSimdWidth();
      sel.OBREAD(value, addr, header, 0xff, simdWidth * sizeof(int));
      return true;
    }

    DECL_CTOR(OBReadInstruction, 1, 1);
  };

  /*! OBlock write instruction pattern */
  DECL_PATTERN(OBWriteInstruction)
  {
    INLINE bool emitOne(Selection::Opaque &sel, const ir::OBWriteInstruction &insn) const
    {
      using namespace ir;
      const GenRegister header = sel.selReg(sel.reg(FAMILY_DWORD), TYPE_U32);
      const GenRegister addr = sel.selReg(insn.getAddress(), TYPE_U32);
      const GenRegister value = sel.selReg(insn.getValue(), TYPE_U32);
      const uint32_t simdWidth = sel.ctx.getSimdWidth();
      sel.OBWRITE(addr, value, header, 0xff, simdWidth * sizeof(int));
      return true;
    }

    DECL_CTOR(OBWriteInstruction, 1, 1);
  };

  /*! Branch instruction pattern */
  DECL_PATTERN(BranchInstruction)
  {
    void emitForwardBranch(Selection::Opaque &sel,
                           const ir::BranchInstruction &insn,
                           ir::LabelIndex dst,
                           ir::LabelIndex src) const
    {
      using namespace ir;
      const GenRegister ip = sel.selReg(ocl::blockip, TYPE_U16);
      const LabelIndex jip = sel.ctx.getLabelIndex(&insn);
      const uint32_t simdWidth = sel.ctx.getSimdWidth();

      // We will not emit any jump if we must go the next block anyway
      const BasicBlock *curr = insn.getParent();
      const BasicBlock *next = curr->getNextBlock();
      const LabelIndex nextLabel = next->getLabelIndex();

      if (insn.isPredicated() == true) {
        const Register pred = insn.getPredicateIndex();

        // Update the PcIPs
        sel.push();
          sel.curr.physicalFlag = 0;
          sel.curr.flagIndex = uint16_t(pred);
          sel.MOV(ip, GenRegister::immuw(uint16_t(dst)));
        sel.pop();

        if (nextLabel == jip) return;

        // It is slightly more complicated than for backward jump. We check that
        // all PcIPs are greater than the next block IP to be sure that we can
        // jump
        sel.push();
          sel.curr.physicalFlag = 0;
          sel.curr.flagIndex = uint16_t(pred);
          sel.curr.predicate = GEN_PREDICATE_NONE;
          sel.CMP(GEN_CONDITIONAL_G, ip, GenRegister::immuw(nextLabel));

          // Branch to the jump target
          if (simdWidth == 8)
            sel.curr.predicate = GEN_PREDICATE_ALIGN1_ALL8H;
          else if (simdWidth == 16)
            sel.curr.predicate = GEN_PREDICATE_ALIGN1_ALL16H;
          else
            NOT_SUPPORTED;
          sel.curr.execWidth = 1;
          sel.curr.noMask = 1;
          sel.JMPI(GenRegister::immd(0), jip);
        sel.pop();

      } else {
        // Update the PcIPs
        sel.MOV(ip, GenRegister::immuw(uint16_t(dst)));

        // Do not emit branch when we go to the next block anyway
        if (nextLabel == jip) return;
        sel.push();
          sel.curr.execWidth = 1;
          sel.curr.noMask = 1;
          sel.curr.predicate = GEN_PREDICATE_NONE;
          sel.JMPI(GenRegister::immd(0), jip);
        sel.pop();
      }
    }

    void emitBackwardBranch(Selection::Opaque &sel,
                            const ir::BranchInstruction &insn,
                            ir::LabelIndex dst,
                            ir::LabelIndex src) const
    {
      using namespace ir;
      const GenRegister ip = sel.selReg(ocl::blockip, TYPE_U16);
      const Function &fn = sel.ctx.getFunction();
      const BasicBlock &bb = fn.getBlock(src);
      const LabelIndex jip = sel.ctx.getLabelIndex(&insn);
      const uint32_t simdWidth = sel.ctx.getSimdWidth();
      GBE_ASSERT(bb.getNextBlock() != NULL);

      if (insn.isPredicated() == true) {
        const Register pred = insn.getPredicateIndex();

        // Update the PcIPs for all the branches. Just put the IPs of the next
        // block. Next instruction will properly reupdate the IPs of the lanes
        // that actually take the branch
        const LabelIndex next = bb.getNextBlock()->getLabelIndex();
        sel.MOV(ip, GenRegister::immuw(uint16_t(next)));

        sel.push();
          // Re-update the PcIPs for the branches that takes the backward jump
          sel.curr.physicalFlag = 0;
          sel.curr.flagIndex = uint16_t(pred);
          sel.MOV(ip, GenRegister::immuw(uint16_t(dst)));

          // Branch to the jump target
          if (simdWidth == 8)
            sel.curr.predicate = GEN_PREDICATE_ALIGN1_ANY8H;
          else if (simdWidth == 16)
            sel.curr.predicate = GEN_PREDICATE_ALIGN1_ANY16H;
          else
            NOT_SUPPORTED;
          sel.curr.execWidth = 1;
          sel.curr.noMask = 1;
          sel.JMPI(GenRegister::immd(0), jip);
        sel.pop();

      } else {

        // Update the PcIPs
        sel.MOV(ip, GenRegister::immuw(uint16_t(dst)));

        // Branch to the jump target
        sel.push();
          sel.curr.execWidth = 1;
          sel.curr.noMask = 1;
          sel.curr.predicate = GEN_PREDICATE_NONE;
          sel.JMPI(GenRegister::immd(0), jip);
        sel.pop();
      }
    }

    INLINE bool emitOne(Selection::Opaque &sel, const ir::BranchInstruction &insn) const {
      using namespace ir;
      const Opcode opcode = insn.getOpcode();
      if (opcode == OP_RET)
        sel.EOT();
      else if (opcode == OP_BRA) {
        const LabelIndex dst = insn.getLabelIndex();
        const LabelIndex src = insn.getParent()->getLabelIndex();

        // We handle foward and backward branches differently
        if (uint32_t(dst) <= uint32_t(src))
          this->emitBackwardBranch(sel, insn, dst, src);
        else
          this->emitForwardBranch(sel, insn, dst, src);
      } else
        NOT_IMPLEMENTED;
      return true;
    }

    DECL_CTOR(BranchInstruction, 1, 1);
  };

  /*! Sort patterns */
  INLINE bool cmp(const SelectionPattern *p0, const SelectionPattern *p1) {
    if (p0->insnNum != p1->insnNum)
      return p0->insnNum > p1->insnNum;
    return p0->cost < p1->cost;
  }

  SelectionLibrary::SelectionLibrary(void) {
    this->insert<UnaryInstructionPattern>();
    this->insert<BinaryInstructionPattern>();
    this->insert<TernaryInstructionPattern>();
    this->insert<SampleInstructionPattern>();
    this->insert<TypedWriteInstructionPattern>();
    this->insert<FenceInstructionPattern>();
    this->insert<LoadImmInstructionPattern>();
    this->insert<LoadInstructionPattern>();
    this->insert<StoreInstructionPattern>();
    this->insert<SelectInstructionPattern>();
    this->insert<CompareInstructionPattern>();
    this->insert<ConvertInstructionPattern>();
    this->insert<LabelInstructionPattern>();
    this->insert<RegionInstructionPattern>();
    this->insert<VoteInstructionPattern>();
    this->insert<RGatherInstructionPattern>();
    this->insert<OBReadInstructionPattern>();
    this->insert<OBWriteInstructionPattern>();
    this->insert<BranchInstructionPattern>();
    this->insert<Int32x32MulInstructionPattern>();
    this->insert<Int32x16MulInstructionPattern>();
    if (OCL_FORCE_MAD_PATTERN)
      this->insert<MulAddInstructionPattern>();

    // Sort all the patterns with the number of instructions they output
    for (uint32_t op = 0; op < ir::OP_INVALID; ++op)
      std::sort(this->patterns[op].begin(), this->patterns[op].end(), cmp);
  }

  SelectionLibrary::~SelectionLibrary(void) {
    for (auto pattern : this->toFree)
      GBE_DELETE(const_cast<SelectionPattern*>(pattern));
  }

  template <typename PatternType>
  void SelectionLibrary::insert(void) {
    const SelectionPattern *pattern = GBE_NEW(PatternType);
    this->toFree.push_back(pattern);
    for (auto opcode : pattern->opcodes)
      this->patterns[opcode].push_back(pattern);
  }

} /* namespace gbe */


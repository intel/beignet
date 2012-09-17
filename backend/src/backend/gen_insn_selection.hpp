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
 * \file gen_insn_selection.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __GEN_INSN_SELECTION_HPP__
#define __GEN_INSN_SELECTION_HPP__

#include "ir/register.hpp"
#include "ir/instruction.hpp"
#include "backend/gen_register.hpp"
#include "backend/gen_encoder.hpp"
#include "backend/gen_context.hpp"
#include "sys/vector.hpp"

namespace gbe
{
  /*! Translate IR type to Gen type */
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

  /*! Translate IR compare to Gen compare */
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

  /*! Selection opcodes properly encoded from 0 to n for fast jump tables
   *  generations
   */
  enum SelectionOpcode {
#define DECL_SELECTION_IR(OP, FN) SEL_OP_##OP,
#include "backend/gen_insn_selection.hxx"
#undef DECL_SELECTION_IR
  };

  // Owns the selection instructions
  class SelectionBlock;

  /*! A selection instruction is also almost a Gen instruction but *before* the
   *  register allocation
   */
  class SelectionInstruction
  {
  public:
    INLINE SelectionInstruction(void) : parent(NULL), prev(NULL), next(NULL) {}
    /*! Owns the instruction */
    SelectionBlock *parent;
    /*! Instruction are chained in the block */
    SelectionInstruction *prev, *next;
    /*! Append an instruction before this one */
    void appendBefore(const SelectionInstruction &insn);
    /*! Append an instruction after this one */
    void appendAfter(const SelectionInstruction &insn);
    /*! No more than 6 sources (used by typed writes) */
    enum { MAX_SRC_NUM = 8 };
    /*! No more than 4 destinations (used by samples and untyped reads) */
    enum { MAX_DST_NUM = 4 };
    /*! All destinations */
    GenRegister dst[MAX_DST_NUM];
    /*! All sources */
    GenRegister src[MAX_SRC_NUM];
    /*! State of the instruction (extra fields neeed for the encoding) */
    GenInstructionState state;
    /*! Gen opcode */
    uint8_t opcode;
    union {
      struct {
        /*! Store bti for loads/stores and function for math and compares */
        uint16_t function:8;
        /*! elemSize for byte scatters / gathers, elemNum for untyped msg */
        uint16_t elem:8;
      };
      struct {
        /*! Number of sources in the tuple */
        uint8_t width:4;
        /*! vertical stride (0,1,2,4,8 or 16) */
        uint16_t vstride:5;
        /*! horizontal stride (0,1,2,4,8 or 16) */
        uint16_t hstride:5;
        /*! offset (0 to 7) */
        uint16_t offset:5;
      };
    } extra;
    /*! Number of sources */
    uint8_t srcNum:4;
    /*! Number of destinations */
    uint8_t dstNum:4;
    /*! To store various indices */
    uint16_t index;
  };

  /*! Some instructions like sends require to make some registers contiguous in
   *  memory
   */
  class SelectionVector
  {
  public:
    INLINE SelectionVector(void) :
      insn(NULL), next(NULL), reg(NULL), regNum(0), isSrc(0) {}
    /*! The instruction that requires the vector of registers */
    SelectionInstruction *insn;
    /*! We chain the selection vectors together */
    SelectionVector *next;
    /*! Directly points to the selection instruction registers */
    GenRegister *reg;
    /*! Number of registers in the vector */
    uint16_t regNum;
    /*! Indicate if this a destination or a source vector */
    uint16_t isSrc;
  };

  // Owns the selection block
  class Selection;

  /*! A selection block is the counterpart of the IR Basic block. It contains
   *  the instructions generated from an IR basic block
   */
  class SelectionBlock
  {
  public:
    INLINE SelectionBlock(const ir::BasicBlock *bb) :
      insnHead(NULL), insnTail(NULL), vector(NULL), next(NULL), bb(bb) {}
    /*! Minimum of temporary registers per block */
    enum { MAX_TMP_REGISTER = 8 };
    /*! All the emitted instructions in the block */
    SelectionInstruction *insnHead, *insnTail;
    /*! The vectors that may be required by some instructions of the block */
    SelectionVector *vector;
    /*! Own the selection block */
    Selection *parent;
    /*! Extra registers needed by the block (only live in the block) */
    gbe::vector<ir::Register> tmp;
    /*! We chain the blocks together */
    SelectionBlock *next;
    /*! Associated IR basic block */
    const ir::BasicBlock *bb;
    /*! Apply the given functor on all the instructions */
    template <typename T>
    INLINE void foreach(const T &functor) const {
      SelectionInstruction *curr = insnHead;
      while (curr) {
        SelectionInstruction *succ = curr->next;
        functor(*curr);
        curr = succ;
      }
    }
    /*! Append a new temporary register */
    INLINE void append(ir::Register reg) { tmp.push_back(reg); }
    /*! Append a new selection instruction in the block */
    INLINE void append(SelectionInstruction *insn) {
      if (this->insnTail != NULL) {
        this->insnTail->next = insn;
        insn->prev = this->insnTail;
      }
      if (this->insnHead == NULL)
        this->insnHead = insn;
      this->insnTail = insn;
      insn->parent = this;
    }
    /*! Append a new selection vector in the block */
    INLINE void append(SelectionVector *vec) {
      SelectionVector *tmp = this->vector;
      this->vector = vec;
      this->vector->next = tmp;
    }
  };

  /*! Owns the selection engine */
  class GenContext;

  /*! Selection engine produces the pre-ISA instruction blocks */
  class Selection
  {
  public:
    /*! simdWidth is the default width for the instructions */
    Selection(GenContext &ctx);
    /*! Release everything */
    virtual ~Selection(void);
    /*! Implements the instruction selection itself */
    virtual void select(void) = 0;
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
    /*! Apply the given functor all the instructions */
    template <typename T>
    INLINE void foreachInstruction(const T &functor) const {
      SelectionBlock *curr = blockHead;
      while (curr) {
        SelectionBlock *succ = curr->next;
        curr->foreach(functor);
        curr = succ;
      }
    }
    /*! Number of register vectors in the selection */
    INLINE uint32_t getVectorNum(void) const { return this->vectorNum; }
    /*! Replace a source by the returned temporary register */
    ir::Register replaceSrc(SelectionInstruction *insn, uint32_t regID);
    /*! Replace a destination to the returned temporary register */
    ir::Register replaceDst(SelectionInstruction *insn, uint32_t regID);
    /*! Bool registers will use scalar words. So we will consider them as
     *  scalars in Gen backend
     */
    bool isScalarOrBool(ir::Register reg) const;
    /*! Get the data for the given register */
    INLINE ir::RegisterData getRegisterData(ir::Register reg) const {
      return file.get(reg);
    }
    /*! Get the family for the given register */
    INLINE ir::RegisterFamily getRegisterFamily(ir::Register reg) const {
      return file.get(reg).family;
    }
    /*! Registers in the register file */
    INLINE uint32_t regNum(void) const { return file.regNum(); }
  protected:
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
    SelectionInstruction *appendInsn(void);
    /*! Append a new vector of registers in the current block */
    SelectionVector *appendVector(void);
    /*! Return the selection register from the GenIR one */
    GenRegister selReg(ir::Register, ir::Type type = ir::TYPE_FLOAT);
    /*! Compute the nth register part when using SIMD8 with Qn (n in 2,3,4) */
    GenRegister selRegQn(ir::Register, uint32_t quarter, ir::Type type = ir::TYPE_FLOAT);
    /*! To handle selection block allocation */
    DECL_POOL(SelectionBlock, blockPool);
    /*! To handle selection instruction allocation */
    DECL_POOL(SelectionInstruction, insnPool);
    /*! To handle selection vector allocation */
    DECL_POOL(SelectionVector, vecPool);
    /*! Owns this structure */
    GenContext &ctx;
    /*! List of emitted blocks */
    SelectionBlock *blockHead, *blockTail;
    /*! Currently processed block */
    SelectionBlock *block;
    /*! Current instruction state to use */
    GenInstructionState curr;
    /*! State used to encode the instructions */
    GenInstructionState stack[MAX_STATE_NUM];
    /*! We append new registers so we duplicate the function register file */
    ir::RegisterFile file;
    /*! Number of states currently pushed */
    uint32_t stateNum;
    /*! Number of vector allocated */
    uint32_t vectorNum;
    /*! To make function prototypes more readable */
    typedef const GenRegister &Reg;

#define ALU1(OP) \
  INLINE void OP(Reg dst, Reg src) { ALU1(SEL_OP_##OP, dst, src); }
#define ALU2(OP) \
  INLINE void OP(Reg dst, Reg src0, Reg src1) { ALU2(SEL_OP_##OP, dst, src0, src1); }
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
#undef ALU1
#undef ALU2

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
    void ALU1(uint32_t opcode, Reg dst, Reg src);
    /*! Encode binary instructions */
    void ALU2(uint32_t opcode, Reg dst, Reg src0, Reg src1);
    /*! Encode regioning */
    void REGION(Reg dst0, Reg dst1, const GenRegister *src, uint32_t offset, uint32_t vstride, uint32_t width, uint32_t hstride, uint32_t srcNum);
    /*! Encode regioning */
    void RGATHER(Reg dst, const GenRegister *src, uint32_t srcNum);
    /*! Use custom allocators */
    GBE_CLASS(Selection);
    friend class SelectionBlock;
    friend class SelectionInstruction;
  };

  /*! This is a simple one-to-many instruction selection */
  Selection *newSimpleSelection(GenContext &ctx);

} /* namespace gbe */

#endif /*  __GEN_INSN_SELECTION_HPP__ */


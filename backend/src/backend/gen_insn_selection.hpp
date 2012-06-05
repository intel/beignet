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
 * \file gen_selector.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __GEN_INSN_SELECTION_HPP__
#define __GEN_INSN_SELECTION_HPP__

#include "ir/register.hpp"
#include "ir/instruction.hpp"
#include "backend/gen_defs.hpp"
#include "backend/gen_context.hpp"
#include "sys/vector.hpp"

namespace gbe
{
  /*! The state for each instruction */
  struct SelectionState
  {
    INLINE SelectionState(uint32_t simdWidth = 8) {
      this->execWidth = simdWidth;
      this->quarterControl = GEN_COMPRESSION_Q1;
      this->noMask = 0;
      this->flag = 0;
      this->subFlag = 0;
      this->predicate = GEN_PREDICATE_NORMAL;
      this->inversePredicate = 0;
    }
    uint32_t execWidth:6;
    uint32_t quarterControl:2;
    uint32_t noMask:1;
    uint32_t flag:1;
    uint32_t subFlag:1;
    uint32_t predicate:4;
    uint32_t inversePredicate:1;
  };

  /*! Type size in bytes for each Gen type */
  INLINE int typeSize2(uint32_t type) {
    switch(type) {
      case GEN_TYPE_UD:
      case GEN_TYPE_D:
      case GEN_TYPE_F:
        return 4;
      case GEN_TYPE_HF:
      case GEN_TYPE_UW:
      case GEN_TYPE_W:
        return 2;
      case GEN_TYPE_UB:
      case GEN_TYPE_B:
        return 1;
      default:
        assert(0);
        return 0;
    }
  }

  /*! This is a book-keeping structure that is not exactly a virtual register
   *  and not exactly a physical register. Basically, it is a Gen register
   *  *before* the register allocation i.e. it contains the info to get it
   *  properly encoded later but it is not associated to a GRF, a flag or
   *  anything Gen related yet.
   */
  struct SelectionReg
  {
    /*! Associated virtual register */
    ir::Register reg;

    /*! For immediates */
    union {
      float f;
      int32_t d;
      uint32_t ud;
    } immediate;

    uint32_t nr:8;        //!< Just for some physical registers (acc, null)
    uint32_t subnr:5;     //!< Idem
    uint32_t type:4;      //!< Gen type
    uint32_t file:2;      //!< Register file
    uint32_t negation:1;  //!< For source
    uint32_t absolute:1;  //!< For source
    uint32_t vstride:4;   //!< Vertical stride
    uint32_t width:3;     //!< Width
    uint32_t hstride:2;   //!< Horizontal stride
    uint32_t quarter:2;   //!< To choose which part we want

    /*! Empty constructor */
    INLINE SelectionReg(void) {}

    /*! General constructor */
    INLINE SelectionReg(uint32_t file,
                        ir::Register reg,
                        uint32_t type,
                        uint32_t vstride,
                        uint32_t width,
                        uint32_t hstride)
    {
      this->type = type;
      this->file = file;
      this->reg = reg;
      this->negation = 0;
      this->absolute = 0;
      this->vstride = vstride;
      this->width = width;
      this->hstride = hstride;
      this->quarter = 0;
      this->nr = this->subnr = 0;
    }

    /*! For specific physical registers only (acc, null) */
    INLINE SelectionReg(uint32_t file,
                        uint32_t nr,
                        uint32_t subnr,
                        uint32_t type,
                        uint32_t vstride,
                        uint32_t width,
                        uint32_t hstride)
    {
      this->type = type;
      this->file = file;
      this->nr = nr;
      this->subnr = subnr * typeSize2(type);
      this->negation = 0;
      this->absolute = 0;
      this->vstride = vstride;
      this->width = width;
      this->hstride = hstride;
      this->quarter = 0;
    }

    static INLINE SelectionReg Qn(SelectionReg reg, uint32_t quarter) {
      if (reg.hstride == GEN_HORIZONTAL_STRIDE_0) // scalar register
        return reg;
      else {
        reg.quarter = quarter;
        return reg;
      }
    }

    static INLINE SelectionReg vec16(uint32_t file, ir::Register reg) {
      return SelectionReg(file,
                          reg,
                          GEN_TYPE_F,
                          GEN_VERTICAL_STRIDE_8,
                          GEN_WIDTH_8,
                          GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE SelectionReg vec8(uint32_t file, ir::Register reg) {
      return SelectionReg(file,
                          reg,
                          GEN_TYPE_F,
                          GEN_VERTICAL_STRIDE_8,
                          GEN_WIDTH_8,
                          GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE SelectionReg vec4(uint32_t file, ir::Register reg) {
      return SelectionReg(file,
                          reg,
                          GEN_TYPE_F,
                          GEN_VERTICAL_STRIDE_4,
                          GEN_WIDTH_4,
                          GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE SelectionReg vec2(uint32_t file, ir::Register reg) {
      return SelectionReg(file,
                          reg,
                          GEN_TYPE_F,
                          GEN_VERTICAL_STRIDE_2,
                          GEN_WIDTH_2,
                          GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE SelectionReg vec1(uint32_t file, ir::Register reg) {
      return SelectionReg(file,
                          reg,
                          GEN_TYPE_F,
                          GEN_VERTICAL_STRIDE_0,
                          GEN_WIDTH_1,
                          GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE SelectionReg retype(SelectionReg reg, uint32_t type) {
      reg.type = type;
      return reg;
    }

    static INLINE SelectionReg ud16(uint32_t file, ir::Register reg) {
      return retype(vec16(file, reg), GEN_TYPE_UD);
    }

    static INLINE SelectionReg ud8(uint32_t file, ir::Register reg) {
      return retype(vec8(file, reg), GEN_TYPE_UD);
    }

    static INLINE SelectionReg ud1(uint32_t file, ir::Register reg) {
      return retype(vec1(file, reg), GEN_TYPE_UD);
    }

    static INLINE SelectionReg d8(uint32_t file, ir::Register reg) {
      return retype(vec8(file, reg), GEN_TYPE_D);
    }

    static INLINE SelectionReg uw16(uint32_t file, ir::Register reg) {
      return retype(vec16(file, reg), GEN_TYPE_UW);
    }

    static INLINE SelectionReg uw8(uint32_t file, ir::Register reg) {
      return retype(vec8(file, reg), GEN_TYPE_UW);
    }

    static INLINE SelectionReg uw1(uint32_t file, ir::Register reg) {
      return retype(vec1(file, reg), GEN_TYPE_UW);
    }

    static INLINE SelectionReg ub16(uint32_t file, ir::Register reg) {
      return SelectionReg(file,
                          reg,
                          GEN_TYPE_UB,
                          GEN_VERTICAL_STRIDE_16,
                          GEN_WIDTH_8,
                          GEN_HORIZONTAL_STRIDE_2);
    }

    static INLINE SelectionReg ub8(uint32_t file, ir::Register reg) {
      return SelectionReg(file,
                          reg,
                          GEN_TYPE_UB,
                          GEN_VERTICAL_STRIDE_16,
                          GEN_WIDTH_8,
                          GEN_HORIZONTAL_STRIDE_2);
    }

    static INLINE SelectionReg ub1(uint32_t file, ir::Register reg) {
      return retype(vec1(file, reg), GEN_TYPE_UB);
    }

    static INLINE SelectionReg unpacked_uw(ir::Register reg) {
        return SelectionReg(GEN_GENERAL_REGISTER_FILE,
                            reg,
                            GEN_TYPE_UW,
                            GEN_VERTICAL_STRIDE_16,
                            GEN_WIDTH_8,
                            GEN_HORIZONTAL_STRIDE_2);
    }

    static INLINE SelectionReg unpacked_ub(ir::Register reg) {
      return SelectionReg(GEN_GENERAL_REGISTER_FILE,
                          reg,
                          GEN_TYPE_UB,
                          GEN_VERTICAL_STRIDE_32,
                          GEN_WIDTH_8,
                          GEN_HORIZONTAL_STRIDE_4);
    }

    static INLINE SelectionReg imm(uint32_t type) {
      return SelectionReg(GEN_IMMEDIATE_VALUE,
                          ir::Register(),
                          type,
                          GEN_VERTICAL_STRIDE_0,
                          GEN_WIDTH_1,
                          GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE SelectionReg immf(float f) {
      SelectionReg immediate = imm(GEN_TYPE_F);
      immediate.immediate.f = f;
      return immediate;
    }

    static INLINE SelectionReg immd(int d) {
      SelectionReg immediate = imm(GEN_TYPE_D);
      immediate.immediate.d = d;
      return immediate;
    }

    static INLINE SelectionReg immud(uint32_t ud) {
      SelectionReg immediate = imm(GEN_TYPE_UD);
      immediate.immediate.ud = ud;
      return immediate;
    }

    static INLINE SelectionReg immuw(uint16_t uw) {
      SelectionReg immediate = imm(GEN_TYPE_UW);
      immediate.immediate.ud = uw | (uw << 16);
      return immediate;
    }

    static INLINE SelectionReg immw(int16_t w) {
      SelectionReg immediate = imm(GEN_TYPE_W);
      immediate.immediate.d = w | (w << 16);
      return immediate;
    }

    static INLINE SelectionReg immv(uint32_t v) {
      SelectionReg immediate = imm(GEN_TYPE_V);
      immediate.vstride = GEN_VERTICAL_STRIDE_0;
      immediate.width = GEN_WIDTH_8;
      immediate.hstride = GEN_HORIZONTAL_STRIDE_1;
      immediate.immediate.ud = v;
      return immediate;
    }

    static INLINE SelectionReg immvf(uint32_t v) {
      SelectionReg immediate = imm(GEN_TYPE_VF);
      immediate.vstride = GEN_VERTICAL_STRIDE_0;
      immediate.width = GEN_WIDTH_4;
      immediate.hstride = GEN_HORIZONTAL_STRIDE_1;
      immediate.immediate.ud = v;
      return immediate;
    }

    static INLINE SelectionReg immvf4(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3) {
      SelectionReg immediate = imm(GEN_TYPE_VF);
      immediate.vstride = GEN_VERTICAL_STRIDE_0;
      immediate.width = GEN_WIDTH_4;
      immediate.hstride = GEN_HORIZONTAL_STRIDE_1;
      immediate.immediate.ud = ((v0 << 0) | (v1 << 8) | (v2 << 16) | (v3 << 24));
      return immediate;
    }

    static INLINE SelectionReg f1grf(ir::Register reg) {
      return vec1(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE SelectionReg f2grf(ir::Register reg) {
      return vec2(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE SelectionReg f4grf(ir::Register reg) {
      return vec4(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE SelectionReg f8grf(ir::Register reg) {
      return vec8(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE SelectionReg f16grf(ir::Register reg) {
      return vec16(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE SelectionReg ud16grf(ir::Register reg) {
      return ud16(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE SelectionReg ud8grf(ir::Register reg) {
      return ud8(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE SelectionReg ud1grf(ir::Register reg) {
      return ud1(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE SelectionReg uw1grf(ir::Register reg) {
      return uw1(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE SelectionReg uw8grf(ir::Register reg) {
      return uw8(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE SelectionReg uw16grf(ir::Register reg) {
      return uw16(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE SelectionReg ub1grf(ir::Register reg) {
      return ub1(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE SelectionReg ub8grf(ir::Register reg) {
      return ub8(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE SelectionReg ub16grf(ir::Register reg) {
      return ub16(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE SelectionReg null(void) {
      return SelectionReg(GEN_ARCHITECTURE_REGISTER_FILE,
                          GEN_ARF_NULL,
                          0,
                          GEN_TYPE_F,
                          GEN_VERTICAL_STRIDE_8,
                          GEN_WIDTH_8,
                          GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE SelectionReg acc(void) {
      return SelectionReg(GEN_ARCHITECTURE_REGISTER_FILE,
                          GEN_ARF_ACCUMULATOR,
                          0,
                          GEN_TYPE_F,
                          GEN_VERTICAL_STRIDE_8,
                          GEN_WIDTH_8,
                          GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE SelectionReg ip(void) {
      return SelectionReg(GEN_ARCHITECTURE_REGISTER_FILE,
                          GEN_ARF_IP,
                          0,
                          GEN_TYPE_D,
                          GEN_VERTICAL_STRIDE_4,
                          GEN_WIDTH_1,
                          GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE SelectionReg notification1(void) {
      return SelectionReg(GEN_ARCHITECTURE_REGISTER_FILE,
                          GEN_ARF_NOTIFICATION_COUNT,
                          1,
                          GEN_TYPE_UD,
                          GEN_VERTICAL_STRIDE_0,
                          GEN_WIDTH_1,
                          GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE SelectionReg flag(uint32_t nr, uint32_t subnr) {
      return SelectionReg(GEN_ARCHITECTURE_REGISTER_FILE,
                          GEN_ARF_FLAG | nr,
                          subnr,
                          GEN_TYPE_UW,
                          GEN_VERTICAL_STRIDE_0,
                          GEN_WIDTH_1,
                          GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE SelectionReg next(SelectionReg reg) {
      reg.quarter++;
      return reg;
    }

    static INLINE SelectionReg negate(SelectionReg reg) {
      reg.negation ^= 1;
      return reg;
    }

    static INLINE SelectionReg abs(SelectionReg reg) {
      reg.absolute = 1;
      reg.negation = 0;
      return reg;
    }
  };

  /*! Selection opcodes properly encoded from 0 to n for fast jump tables
   *  generations
   */
  enum SelectionOpcode {
#define DECL_SELECTION_IR(OP, FN) SEL_OP_##OP,
#include "backend/gen_insn_selection.hxx"
#undef DECL_SELECTION_IR
  };

  // Owns the selection instructions
  class SelectionTile;

  /*! A selection instruction is also almost a Gen instruction but *before* the
   *  register allocation
   */
  class SelectionInstruction
  {
  public:
    INLINE SelectionInstruction(void) : parent(NULL), prev(NULL), next(NULL) {}
    /*! No more than 6 sources (used by typed writes) */
    enum { MAX_SRC_NUM = 6 };
    /*! No more than 4 destinations (used by samples and untyped reads) */
    enum { MAX_DST_NUM = 4 };
    /*! Owns the instruction */
    SelectionTile *parent;
    /*! Instruction are chained in the tile */
    SelectionInstruction *prev, *next;
    /*! All destinations */
    SelectionReg dst[MAX_DST_NUM];
    /*! All sources */
    SelectionReg src[MAX_SRC_NUM];
    /*! State of the instruction (extra fields neeed for the encoding) */
    SelectionState state;
    /*! Gen opcode */
    uint8_t opcode;
    /*! For math and cmp instructions. Store bti for loads/stores */
    uint8_t function:4;
    /*! elemSize for byte scatters / gathers, elemNum for untyped msg */
    uint8_t elem:4;
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
  struct SelectionVector
  {
    INLINE SelectionVector(void) :
      insn(NULL), next(NULL), reg(NULL), regNum(0), isSrc(0) {}
    /*! The instruction that requires the vector of registers */
    SelectionInstruction *insn;
    /*! We chain the selection vectors together */
    SelectionVector *next;
    /*! Directly points to the selection instruction registers */
    SelectionReg *reg;
    /*! Number of registers in the vector */
    uint16_t regNum;
    /*! Indicate if this a destination or a source vector */
    uint16_t isSrc;
  };

  // Owns the selection tile
  class Selection;

  /*! A selection tile is the result of a m-to-n IR instruction to selection
   *  instructions mapping (the role of the instruction selection pass).
   */
  class SelectionTile
  {
  public:
    INLINE SelectionTile(void) :
      insnHead(NULL), insnTail(NULL), vector(NULL), next(NULL),
      outputNum(0), inputNum(0), tmpNum(0), irNum(0) {}
    /*! Maximum of output registers per tile */
    enum { MAX_OUT_REGISTER = 8 };
    /*! Minimum of input registers per tile */
    enum { MAX_IN_REGISTER = 8 };
    /*! Minimum of temporary registers per tile */
    enum { MAX_TMP_REGISTER = 8 };
    /*! Maximum number of instructions in the tile */
    enum { MAX_IR_INSN = 8 };
    /*! All the emitted instructions in the tile */
    SelectionInstruction *insnHead, *insnTail;
    /*! The vectors that may be required by some instructions of the tile */
    SelectionVector *vector;
    /*! Own the selection tile */
    Selection *parent;
    /*! Registers output by the tile (i.e. produced values) */
    ir::Register out[MAX_OUT_REGISTER];
    /*! Registers required by the tile (i.e. input values) */
    ir::Register in[MAX_IN_REGISTER];
    /*! Extra registers needed by the tile (only live in the tile) */
    ir::Register tmp[MAX_TMP_REGISTER];
    /*! Instructions actually captured by the tile (used by RA) */
    ir::Instruction *ir[MAX_IR_INSN];
    /*! We chain the tiles together */
    SelectionTile *next;
    /*! Number of output registers */
    uint8_t outputNum;
    /*! Number of input registers */
    uint8_t inputNum;
    /*! Number of temporary registers */
    uint8_t tmpNum;
    /*! Number of ir instructions */
    uint8_t irNum;
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

#define DECL_APPEND_FN(TYPE, FN, WHICH, NUM, MAX) \
  INLINE void FN(TYPE reg) { \
    GBE_ASSERT(NUM < MAX); \
    WHICH[NUM++] = reg; \
  }
    DECL_APPEND_FN(ir::Register, appendInput, in, inputNum, MAX_IN_REGISTER)
    DECL_APPEND_FN(ir::Register, appendOutput, out, outputNum, MAX_OUT_REGISTER)
    DECL_APPEND_FN(ir::Register, appendTmp, tmp, tmpNum, MAX_TMP_REGISTER)
    DECL_APPEND_FN(ir::Instruction*, append, ir, irNum, MAX_IR_INSN)
#undef DECL_APPEND_FN

    /*! Append a new selection instruction in the tile */
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
    /*! Append a new selection vector in the tile */
    INLINE void append(SelectionVector *vec) {
      SelectionVector *tmp = this->vector;
      this->vector = vec;
      this->vector->next = tmp;
    }
  };

  /*! Owns the selection engine */
  class GenContext;

  /*! Selection engine produces the pre-ISA instruction tiles */
  class Selection
  {
  public:
    /*! simdWidth is the default width for the instructions */
    Selection(GenContext &ctx);
    /*! Release everything */
    virtual ~Selection(void);
    /*! Implements the instruction selection itself */
    virtual void select(void) = 0;
    /*! Apply the given functor on all selection tile */
    template <typename T>
    INLINE void foreach(const T &functor) const {
      SelectionTile *curr = tileHead;
      while (curr) {
        SelectionTile *succ = curr->next;
        functor(*curr);
        curr = succ;
      }
    }
    /*! Apply the given functor all the instructions */
    template <typename T>
    INLINE void foreachInstruction(const T &functor) const {
      SelectionTile *curr = tileHead;
      while (curr) {
        SelectionTile *succ = curr->next;
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
     *  temporary list of the current tile
     */
    INLINE ir::Register reg(ir::RegisterFamily family) {
      GBE_ASSERT(tile != NULL);
      const ir::Register reg = file.append(family);
      tile->appendTmp(reg);
      return reg;
    }
    /*! Append a tile at the tile stream tail. It becomes the current tile */
    void appendTile(void);
    /*! Append an instruction in the current tile */
    SelectionInstruction *appendInsn(void);
    /*! Append a new vector of registers in the current tile */
    SelectionVector *appendVector(void);
    /*! Return the selection register from the GenIR one */
    SelectionReg selReg(ir::Register, ir::Type type = ir::TYPE_FLOAT);
    /*! Compute the nth register part when using SIMD8 with Qn (n in 2,3,4) */
    SelectionReg selRegQn(ir::Register, uint32_t quarter, ir::Type type = ir::TYPE_FLOAT);
    /*! To handle selection tile allocation */
    DECL_POOL(SelectionTile, tilePool);
    /*! To handle selection instruction allocation */
    DECL_POOL(SelectionInstruction, insnPool);
    /*! To handle selection vector allocation */
    DECL_POOL(SelectionVector, vecPool);
    /*! Owns this structure */
    GenContext &ctx;
    /*! List of emitted tiles */
    SelectionTile *tileHead, *tileTail;
    /*! Currently processed tile */
    SelectionTile *tile;
    /*! Current instruction state to use */
    SelectionState curr;
    /*! State used to encode the instructions */
    SelectionState stack[MAX_STATE_NUM];
    /*! We append new registers so we duplicate the function register file */
    ir::RegisterFile file;
    /*! Number of states currently pushed */
    uint32_t stateNum;
    /*! Number of vector allocated */
    uint32_t vectorNum;
    /*! To make function prototypes more readable */
    typedef const SelectionReg &Reg;

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
    void UNTYPED_READ(Reg addr, const SelectionReg *dst, uint32_t elemNum, uint32_t bti);
    /*! Untyped write (up to 4 elements) */
    void UNTYPED_WRITE(Reg addr, const SelectionReg *src, uint32_t elemNum, uint32_t bti);
    /*! Byte gather (for unaligned bytes, shorts and ints) */
    void BYTE_GATHER(Reg dst, Reg addr, uint32_t elemSize, uint32_t bti);
    /*! Byte scatter (for unaligned bytes, shorts and ints) */
    void BYTE_SCATTER(Reg addr, Reg src, uint32_t elemSize, uint32_t bti);
    /*! Extended math function */
    void MATH(Reg dst, uint32_t function, Reg src0, Reg src1);
    /*! Encode unary instructions */
    void ALU1(uint32_t opcode, Reg dst, Reg src);
    /*! Encode binary instructions */
    void ALU2(uint32_t opcode, Reg dst, Reg src0, Reg src1);
  };

  /*! This is a simple one-to-many instruction selection */
  Selection *newSimpleSelection(GenContext &ctx);

} /* namespace gbe */

#endif /*  __GEN_INSN_SELECTION_HPP__ */


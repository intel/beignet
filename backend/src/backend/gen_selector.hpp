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
 * \file gen_instruction_selection.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __GEN_SELECTOR_HPP__
#define  __GEN_SELECTOR_HPP__

#include "ir/register.hpp"
#include "ir/instruction.hpp"
#include "backend/gen_defs.hpp"
#include "sys/vector.hpp"

namespace gbe
{
  /*! The state for each instruction */
  struct GenInstructionState
  {
    uint32_t execWidth:6;
    uint32_t quarterControl:2;
    uint32_t noMask:1;
    uint32_t flag:1;
    uint32_t subFlag:1;
    uint32_t predicate:4;
    uint32_t inversePredicate:1;
  };

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
    uint32_t subnr:6;     //!< Idem
    uint32_t type:4;      //!< Gen type
    uint32_t file:2;      //!< Register file
    uint32_t negation:1;  //!< For source
    uint32_t absolute:1;  //!< For source
    uint32_t vstride:4;   //!< Vertical stride
    uint32_t width:3;     //!< Width
    uint32_t hstride:2;   //!< Horizontal stride
    uint32_t quarter:1;   //!< To choose which part we want

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
      this->subnr = subnr;
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

    static INLINE SelectionReg flag(ir::Register reg) {
      return uw1(GEN_ARCHITECTURE_REGISTER_FILE, reg);
    }
#if 0
    static INLINE SelectionReg next(SelectionReg reg) {
      return reg;
    }
#endif

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

  /*! A selection instruction is also almost a Gen instruction but *before* the
   *  register allocation
   */
  struct SelectionInstruction
  {
    /*! No more than 6 sources (used by typed writes) */
    enum { MAX_SRC_NUM = 6 };
    /*! No more than 4 destinations (used by samples and untyped reads) */
    enum { MAX_DST_NUM = 4 };
    /*! All destinations */
    SelectionReg dst[MAX_DST_NUM];
    /*! All sources */
    SelectionReg src[MAX_SRC_NUM];
    /*! State of the instruction (extra fields neeed for the encoding) */
    GenInstructionState state;
    /*! Gen opcode */
    uint8_t opcode;
    /*! For math instructions only */
    uint8_t function:4;
    /*! For byte scattered reads / writes */
    uint16_t elemSize:4;
  };

  /*! A selection tile is the result of a m-to-n IR instruction to selection
   *  instructions mapping (the role of the instruction selection pass).
   */
  struct SelectionTile
  {
    INLINE SelectionTile(void) : next(NULL) {}
    /*! All the emitted instructions */
    vector<SelectionInstruction> insn;
    /*! Registers output by the tile (i.e. produced values) */
    vector<ir::Register> out;
    /*! Registers required by the tile (i.e. input values) */
    vector<ir::Register> in;
    /*! Extra registers needed by the tile (only live in the tile) */
    vector<ir::Register> tmp;
    /*! Instructions actually captured by the tile (used by RA) */
    vector<ir::Instruction*> ir;
    /*! We chain the tiles together */
    SelectionTile *next;
  };

  /*! Owns the selection engine */
  class GenContext;

  /*! Selection engine produces the pre-ISA instruction tiles */
  struct SelectionEngine
  {
    /*! simdWidth is the default width for the instructions */
    SelectionEngine(GenContext &ctx);
    /*! Release everything */
    ~SelectionEngine(void);
    /*! Implement the instruction selection itself */
    virtual void select(void) = 0;
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
    /*! Owns this structure */
    GenContext &ctx;
    /*! List of emitted tiles */
    SelectionTile *tileList;
    /*! Currently processed tile */
    SelectionTile *tile;
    /*! Current instruction state to use */
    GenInstructionState curr;
    /*! State used to encode the instructions */
    GenInstructionState stack[MAX_STATE_NUM];
    /*! Number of states currently pushed */
    uint32_t stateNum;
    /*! To make function prototypes more readable */
    typedef const SelectionReg &Reg;

#define ALU1(OP) \
  INLINE void OP(Reg dst, Reg src) { ALU1(GEN_OPCODE_##OP, dst, src); }
#define ALU2(OP) \
  INLINE void OP(Reg dst, Reg src0, Reg src1) { ALU2(GEN_OPCODE_##OP, dst, src0, src1); }
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
    ALU2(MAC)
    ALU2(MACH)
    ALU1(LZD)
#undef ALU1
#undef ALU2

    /*! Jump indexed instruction */
    void JMPI(Reg src);
    /*! Compare instructions */
    void CMP(uint32_t conditional, Reg src0, Reg src1);
    /*! EOT is used to finish GPGPU threads */
    void EOT(Reg src);
    /*! No-op */
    void NOP(void);
    /*! Wait instruction (used for the barrier) */
    void WAIT(void);
    /*! Untyped read (up to 4 elements) */
    void UNTYPED_READ(Reg dst0, Reg addr, uint32_t bti);
    void UNTYPED_READ(Reg dst0, Reg dst1, Reg addr, uint32_t bti);
    void UNTYPED_READ(Reg dst0, Reg dst1, Reg dst2, Reg addr, uint32_t bti);
    void UNTYPED_READ(Reg dst0, Reg dst1, Reg dst2, Reg dst3, Reg addr, uint32_t bti);
    /*! Untyped write (up to 4 elements) */
    void UNTYPED_WRITE(Reg addr, Reg src0, uint32_t bti);
    void UNTYPED_WRITE(Reg addr, Reg src0, Reg src1, uint32_t bti);
    void UNTYPED_WRITE(Reg addr, Reg src0, Reg src1, Reg src2, uint32_t bti);
    void UNTYPED_WRITE(Reg addr, Reg src0, Reg src1, Reg src2, Reg src3, uint32_t bti);
    /*! Byte gather (for unaligned bytes, shorts and ints) */
    void BYTE_GATHER(Reg dst, Reg addr, uint32_t bti, uint32_t elemSize);
    /*! Byte scatter (for unaligned bytes, shorts and ints) */
    void BYTE_SCATTER(Reg addr, Reg src, uint32_t bti, uint32_t elemSize);
    /*! Extended math function */
    void MATH(Reg dst, uint32_t function, Reg src0, Reg src1);
    /*! Encode unary instructions */
    void ALU1(uint32_t opcode, Reg dst, Reg src);
    /*! Encode binary instructions */
    void ALU2(uint32_t opcode, Reg dst, Reg src0, Reg src1);
  };

  /*! This is a stupid one-to-many instruction selection */
  SelectionEngine *newPoorManSelectionEngine(void);

} /* namespace gbe */

#endif /*  __GEN_SELECTOR_HPP__ */


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
 * \file gen_eu.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 * This is a revamped Gen ISA encoder from Mesa code base
 */

#ifndef GEN_EU_H
#define GEN_EU_H

#include "backend/gen_defs.hpp"
#include "sys/platform.hpp"
#include <cassert>

#define GEN_REG_SIZE (8*4)
#define GEN_EU_MAX_INSN_STACK 5

namespace gbe
{
  /*! Type size in bytes for each Gen type */
  INLINE int typeSize(uint32_t type) {
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

  /*! Convert a hstride to a number of element */
  INLINE uint32_t stride(uint32_t stride) {
    switch (stride) {
      case 0: return 0;
      case 1: return 1;
      case 2: return 2;
      case 3: return 4;
      case 4: return 8;
      case 5: return 16;
      default: assert(0); return 0;
    }
  }

  /*! These are not hardware structs, just something useful to pass around */
  struct GenReg
  {
    /*! Empty constructor */
    INLINE GenReg(void) {}

    /*! General constructor */
    INLINE GenReg(uint32_t file,
                  uint32_t nr,
                  uint32_t subnr,
                  uint32_t type,
                  uint32_t vstride,
                  uint32_t width,
                  uint32_t hstride)
    {
      if (file == GEN_GENERAL_REGISTER_FILE)
        assert(nr < GEN_MAX_GRF);
      else if (file == GEN_ARCHITECTURE_REGISTER_FILE)
        assert(nr <= GEN_ARF_IP);

      this->type = type;
      this->file = file;
      this->nr = nr;
      this->subnr = subnr * typeSize(type);
      this->negation = 0;
      this->absolute = 0;
      this->vstride = vstride;
      this->width = width;
      this->hstride = hstride;
      this->address_mode = GEN_ADDRESS_DIRECT;
    }

    static INLINE GenReg Qn(GenReg reg, uint32_t quarter) {
      if (reg.hstride == GEN_HORIZONTAL_STRIDE_0) // scalar register
        return reg;
      else {
        const uint32_t typeSz = typeSize(reg.type);
        const uint32_t horizontal = stride(reg.hstride);
        const uint32_t grfOffset = reg.nr*GEN_REG_SIZE + typeSz*reg.subnr;
        const uint32_t nextOffset = grfOffset + 8*quarter*typeSz*horizontal;
        reg.nr = nextOffset / GEN_REG_SIZE;
        reg.subnr = (nextOffset % GEN_REG_SIZE) / typeSz;
        return reg;
      }
    }

    static INLINE GenReg vec16(uint32_t file, uint32_t nr, uint32_t subnr) {
      return GenReg(file,
                    nr,
                    subnr,
                    GEN_TYPE_F,
                    GEN_VERTICAL_STRIDE_8,
                    GEN_WIDTH_8,
                    GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenReg vec8(uint32_t file, uint32_t nr, uint32_t subnr) {
      return GenReg(file,
                    nr,
                    subnr,
                    GEN_TYPE_F,
                    GEN_VERTICAL_STRIDE_8,
                    GEN_WIDTH_8,
                    GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenReg vec4(uint32_t file, uint32_t nr, uint32_t subnr) {
      return GenReg(file,
                    nr,
                    subnr,
                    GEN_TYPE_F,
                    GEN_VERTICAL_STRIDE_4,
                    GEN_WIDTH_4,
                    GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenReg vec2(uint32_t file, uint32_t nr, uint32_t subnr) {
      return GenReg(file,
                    nr,
                    subnr,
                    GEN_TYPE_F,
                    GEN_VERTICAL_STRIDE_2,
                    GEN_WIDTH_2,
                    GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenReg vec1(uint32_t file, uint32_t nr, uint32_t subnr) {
      return GenReg(file,
                    nr,
                    subnr,
                    GEN_TYPE_F,
                    GEN_VERTICAL_STRIDE_0,
                    GEN_WIDTH_1,
                    GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE GenReg retype(GenReg reg, uint32_t type) {
      reg.type = type;
      return reg;
    }

    static INLINE GenReg suboffset(GenReg reg, uint32_t delta) {
      reg.subnr += delta * typeSize(reg.type);
      return reg;
    }

    static INLINE GenReg ud16(uint32_t file, uint32_t nr, uint32_t subnr) {
      return retype(vec16(file, nr, subnr), GEN_TYPE_UD);
    }

    static INLINE GenReg ud8(uint32_t file, uint32_t nr, uint32_t subnr) {
      return retype(vec8(file, nr, subnr), GEN_TYPE_UD);
    }

    static INLINE GenReg ud1(uint32_t file, uint32_t nr, uint32_t subnr) {
      return retype(vec1(file, nr, subnr), GEN_TYPE_UD);
    }

    static INLINE GenReg d8(uint32_t file, uint32_t nr, uint32_t subnr) {
      return retype(vec8(file, nr, subnr), GEN_TYPE_D);
    }

    static INLINE GenReg uw16(uint32_t file, uint32_t nr, uint32_t subnr) {
      return suboffset(retype(vec16(file, nr, 0), GEN_TYPE_UW), subnr);
    }

    static INLINE GenReg uw8(uint32_t file, uint32_t nr, uint32_t subnr) {
      return suboffset(retype(vec8(file, nr, 0), GEN_TYPE_UW), subnr);
    }

    static INLINE GenReg uw1(uint32_t file, uint32_t nr, uint32_t subnr) {
      return suboffset(retype(vec1(file, nr, 0), GEN_TYPE_UW), subnr);
    }

    static INLINE GenReg ub16(uint32_t file, uint32_t nr, uint32_t subnr) {
      return GenReg(file,
                    nr,
                    subnr,
                    GEN_TYPE_UB,
                    GEN_VERTICAL_STRIDE_16,
                    GEN_WIDTH_8,
                    GEN_HORIZONTAL_STRIDE_2);
    }

    static INLINE GenReg ub8(uint32_t file, uint32_t nr, uint32_t subnr) {
      return GenReg(file,
                    nr,
                    subnr,
                    GEN_TYPE_UB,
                    GEN_VERTICAL_STRIDE_16,
                    GEN_WIDTH_8,
                    GEN_HORIZONTAL_STRIDE_2);
    }

    static INLINE GenReg ub1(uint32_t file, uint32_t nr, uint32_t subnr) {
      return suboffset(retype(vec1(file, nr, 0), GEN_TYPE_UB), subnr);
    }

    static INLINE GenReg imm(uint32_t type) {
      return GenReg(GEN_IMMEDIATE_VALUE,
                    0,
                    0,
                    type,
                    GEN_VERTICAL_STRIDE_0,
                    GEN_WIDTH_1,
                    GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE GenReg immf(float f) {
      GenReg immediate = imm(GEN_TYPE_F);
      immediate.dw1.f = f;
      return immediate;
    }

    static INLINE GenReg immd(int d) {
      GenReg immediate = imm(GEN_TYPE_D);
      immediate.dw1.d = d;
      return immediate;
    }

    static INLINE GenReg immud(uint32_t ud) {
      GenReg immediate = imm(GEN_TYPE_UD);
      immediate.dw1.ud = ud;
      return immediate;
    }

    static INLINE GenReg immuw(uint16_t uw) {
      GenReg immediate = imm(GEN_TYPE_UW);
      immediate.dw1.ud = uw | (uw << 16);
      return immediate;
    }

    static INLINE GenReg immw(int16_t w) {
      GenReg immediate = imm(GEN_TYPE_W);
      immediate.dw1.d = w | (w << 16);
      return immediate;
    }

    static INLINE GenReg immv(uint32_t v) {
      GenReg immediate = imm(GEN_TYPE_V);
      immediate.vstride = GEN_VERTICAL_STRIDE_0;
      immediate.width = GEN_WIDTH_8;
      immediate.hstride = GEN_HORIZONTAL_STRIDE_1;
      immediate.dw1.ud = v;
      return immediate;
    }

    static INLINE GenReg f1grf(uint32_t nr, uint32_t subnr) {
      return vec1(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenReg f2grf(uint32_t nr, uint32_t subnr) {
      return vec2(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenReg f4grf(uint32_t nr, uint32_t subnr) {
      return vec4(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenReg f8grf(uint32_t nr, uint32_t subnr) {
      return vec8(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenReg f16grf(uint32_t nr, uint32_t subnr) {
      return vec16(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenReg ud16grf(uint32_t nr, uint32_t subnr) {
      return ud16(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenReg ud8grf(uint32_t nr, uint32_t subnr) {
      return ud8(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenReg ud1grf(uint32_t nr, uint32_t subnr) {
      return ud1(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenReg uw1grf(uint32_t nr, uint32_t subnr) {
      return uw1(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenReg uw8grf(uint32_t nr, uint32_t subnr) {
      return uw8(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenReg uw16grf(uint32_t nr, uint32_t subnr) {
      return uw16(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenReg ub1grf(uint32_t nr, uint32_t subnr) {
      return ub1(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenReg ub8grf(uint32_t nr, uint32_t subnr) {
      return ub8(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenReg ub16grf(uint32_t nr, uint32_t subnr) {
      return ub16(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenReg null(void) {
      return vec8(GEN_ARCHITECTURE_REGISTER_FILE, GEN_ARF_NULL, 0);
    }

    static INLINE GenReg acc(void) {
      return vec8(GEN_ARCHITECTURE_REGISTER_FILE, GEN_ARF_ACCUMULATOR, 0);
    }

    static INLINE GenReg ip(void) {
      return GenReg(GEN_ARCHITECTURE_REGISTER_FILE, 
                    GEN_ARF_IP,
                    0,
                    GEN_TYPE_D,
                    GEN_VERTICAL_STRIDE_4,
                    GEN_WIDTH_1,
                    GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE GenReg notification1(void) {
      return GenReg(GEN_ARCHITECTURE_REGISTER_FILE,
                    GEN_ARF_NOTIFICATION_COUNT,
                    1,
                    GEN_TYPE_UD,
                    GEN_VERTICAL_STRIDE_0,
                    GEN_WIDTH_1,
                    GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE GenReg flag(uint32_t nr, uint32_t subnr) {
      return uw1(GEN_ARCHITECTURE_REGISTER_FILE, GEN_ARF_FLAG | nr, subnr);
    }

    static INLINE GenReg mask(uint32_t subnr) {
      return uw1(GEN_ARCHITECTURE_REGISTER_FILE, GEN_ARF_MASK, subnr);
    }

    static INLINE GenReg next(GenReg reg) {
      reg.nr++;
      return reg;
    }

    static INLINE GenReg negate(GenReg reg) {
      reg.negation ^= 1;
      return reg;
    }

    static INLINE GenReg abs(GenReg reg) {
      reg.absolute = 1;
      reg.negation = 0;
      return reg;
    }

    uint32_t type:4;
    uint32_t file:2;
    uint32_t nr:8;
    uint32_t subnr:5;        /* :1 in align16 */
    uint32_t negation:1;     /* source only */
    uint32_t absolute:1;     /* source only */
    uint32_t vstride:4;      /* source only */
    uint32_t width:3;        /* src only, align1 only */
    uint32_t hstride:2;      /* align1 only */
    uint32_t address_mode:1; /* relative addressing, hopefully! */

    union {
      float f;
      int32_t d;
      uint32_t ud;
    } dw1;
  };

  /*! The state for each instruction.  */
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

  /*! Helper structure to emit Gen instructions */
  struct GenEmitter
  {
    /*! simdWidth is the default width for the instructions */
    GenEmitter(uint32_t simdWidth, uint32_t gen);
    /*! TODO use a vector */
    enum { MAX_INSN_NUM = 8192 };
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
    /*! TODO Update that with a vector */
    GenInstruction store[MAX_INSN_NUM]; 
    /*! Number of instructions currently pushed */
    uint32_t insnNum;
    /*! Current instruction state to use */
    GenInstructionState curr;
    /*! State used to encode the instructions */
    GenInstructionState stack[MAX_STATE_NUM];
    /*! Number of states currently pushed */
    uint32_t stateNum;
    /*! Gen generation to encode */
    uint32_t gen;

    ////////////////////////////////////////////////////////////////////////
    // Encoding functions
    ////////////////////////////////////////////////////////////////////////

#define ALU1(OP) void OP(GenReg dest, GenReg src0);
#define ALU2(OP) void OP(GenReg dest, GenReg src0, GenReg src1);
#define ALU3(OP) void OP(GenReg dest, GenReg src0, GenReg src1, GenReg src2);
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
    ALU2(LINE)
    ALU2(PLN)
    // ALU3(MAD)
#undef ALU1
#undef ALU2
#undef ALU3

    /*! Jump indexed instruction */
    void JMPI(GenReg src);
    /*! Compare instructions */
    void CMP(uint32_t conditional, GenReg src0, GenReg src1);
    /*! EOT is used to finish GPGPU threads */
    void EOT(uint32_t msg_nr);
    /*! No-op */
    void NOP(void);
    /*! Wait instruction (used for the barrier) */
    void WAIT(void);
    /*! Untyped read (upto 4 channels) */
    void UNTYPED_READ(GenReg dst, GenReg src, uint32_t bti, uint32_t elemNum);
    /*! Untyped write (upto 4 channels) */
    void UNTYPED_WRITE(GenReg src, uint32_t bti, uint32_t elemNum);
    /*! Byte gather (for unaligned bytes, shorts and ints) */
    void BYTE_GATHER(GenReg dst, GenReg src, uint32_t bti, uint32_t elemSize);
    /*! Byte scatter (for unaligned bytes, shorts and ints) */
    void BYTE_SCATTER(GenReg src, uint32_t bti, uint32_t elemSize);
    /*! Send instruction for the sampler */
    void SAMPLE(GenReg dest,
                uint32_t msg_reg_nr,
                GenReg src0,
                uint32_t bti,
                uint32_t sampler,
                uint32_t writemask,
                uint32_t msg_type,
                uint32_t response_length,
                uint32_t msg_length,
                uint32_t header_present,
                uint32_t simd_mode,
                uint32_t return_format);
    /*! Extended math function */
    void MATH(GenReg dst, uint32_t function, GenReg src0, GenReg src1);

    /*! Patch JMPI (located at index insnID) with the given jump distance */
    void patchJMPI(uint32_t insnID, int32_t jumpDistance);

    ////////////////////////////////////////////////////////////////////////
    // Helper functions to encode
    ////////////////////////////////////////////////////////////////////////
    void setHeader(GenInstruction *insn);
    void setDst(GenInstruction *insn, GenReg dest);
    void setSrc0(GenInstruction *insn, GenReg reg);
    void setSrc1(GenInstruction *insn, GenReg reg);
    GenInstruction *next(uint32_t opcode);
  };

} /* namespace gbe */

#endif /* GEN_EU_H */


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

/*
 Copyright (C) Intel Corp.  2006.  All Rights Reserved.
 Intel funded Tungsten Graphics (http://www.tungstengraphics.com) to
 develop this 3D driver.
 
 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:
 
 The above copyright notice and this permission notice (including the
 next paragraph) shall be included in all copies or substantial
 portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 **********************************************************************/
 /*
  * Authors:
  *   Keith Whitwell <keith@tungstengraphics.com>
  */

#ifndef __GBE_GEN_ENCODER_HPP__
#define __GBE_GEN_ENCODER_HPP__

#include "backend/gen_defs.hpp"
#include "backend/gen_register.hpp"
#include "sys/platform.hpp"
#include "sys/vector.hpp"
#include <cassert>

namespace gbe
{
  /*! Helper structure to emit Gen instructions */
  class GenEncoder
  {
  public:
    /*! simdWidth is the default width for the instructions */
    GenEncoder(uint32_t simdWidth, uint32_t gen);
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
    /*! The instruction stream we are building */
    vector<GenInstruction> store;
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

#define ALU1(OP) void OP(GenRegister dest, GenRegister src0);
#define ALU2(OP) void OP(GenRegister dest, GenRegister src0, GenRegister src1);
#define ALU3(OP) void OP(GenRegister dest, GenRegister src0, GenRegister src1, GenRegister src2);
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
    ALU3(MAD)
#undef ALU1
#undef ALU2
#undef ALU3

    /*! Jump indexed instruction */
    void JMPI(GenRegister src);
    /*! Compare instructions */
    void CMP(uint32_t conditional, GenRegister src0, GenRegister src1);
    /*! EOT is used to finish GPGPU threads */
    void EOT(uint32_t msg_nr);
    /*! No-op */
    void NOP(void);
    /*! Wait instruction (used for the barrier) */
    void WAIT(void);
    /*! Untyped read (upto 4 channels) */
    void UNTYPED_READ(GenRegister dst, GenRegister src, uint32_t bti, uint32_t elemNum);
    /*! Untyped write (upto 4 channels) */
    void UNTYPED_WRITE(GenRegister src, uint32_t bti, uint32_t elemNum);
    /*! Byte gather (for unaligned bytes, shorts and ints) */
    void BYTE_GATHER(GenRegister dst, GenRegister src, uint32_t bti, uint32_t elemSize);
    /*! Byte scatter (for unaligned bytes, shorts and ints) */
    void BYTE_SCATTER(GenRegister src, uint32_t bti, uint32_t elemSize);
    /*! OBlock read */
    void OBREAD(GenRegister dst, GenRegister header, uint32_t bti, uint32_t elemSize);
    /*! OBlock read */
    void OBWRITE(GenRegister header, uint32_t bti, uint32_t elemSize);
    /*! Send instruction for the sampler */
    void SAMPLE(GenRegister dest,
                uint32_t msg_reg_nr,
                GenRegister src0,
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
    void MATH(GenRegister dst, uint32_t function, GenRegister src0, GenRegister src1);

    /*! Patch JMPI (located at index insnID) with the given jump distance */
    void patchJMPI(uint32_t insnID, int32_t jumpDistance);

    ////////////////////////////////////////////////////////////////////////
    // Helper functions to encode
    ////////////////////////////////////////////////////////////////////////
    void setHeader(GenInstruction *insn);
    void setDst(GenInstruction *insn, GenRegister dest);
    void setSrc0(GenInstruction *insn, GenRegister reg);
    void setSrc1(GenInstruction *insn, GenRegister reg);
    GenInstruction *next(uint32_t opcode);
    GBE_CLASS(GenEncoder); //!< Use custom allocators
  };

} /* namespace gbe */

#endif /* __GBE_GEN_ENCODER_HPP__ */



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
#include "src/cl_device_data.h"

namespace gbe
{
  /*! Helper structure to emit Gen instructions */
  class GenEncoder
  {
  public:
    /*! simdWidth is the default width for the instructions */
    GenEncoder(uint32_t simdWidth, uint32_t gen, uint32_t deviceID);

    virtual ~GenEncoder(void) { }
    /*! Size of the stack (should be large enough) */
    enum { MAX_STATE_NUM = 16 };
    /*! gen7 exec width of the double data type */
    #define GEN7_DOUBLE_EXEC_WIDTH  8
    /*! Push the current instruction state */
    void push(void);
    /*! Pop the latest pushed state */
    void pop(void);
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
    /*! Device ID */
    uint32_t deviceID;
    /*! simd width for this codegen */
    uint32_t simdWidth;
    ////////////////////////////////////////////////////////////////////////
    // Encoding functions
    ////////////////////////////////////////////////////////////////////////

#define ALU1(OP) void OP(GenRegister dest, GenRegister src0, uint32_t condition = 0);
#define ALU2(OP) void OP(GenRegister dest, GenRegister src0, GenRegister src1);
#define ALU2_MOD(OP) void OP(GenRegister dest, GenRegister src0, GenRegister src1, uint32_t condition = 0);
#define ALU3(OP) void OP(GenRegister dest, GenRegister src0, GenRegister src1, GenRegister src2);
    ALU1(MOV)
    ALU1(FBH)
    ALU1(FBL)
    ALU2(SUBB)
    ALU2(UPSAMPLE_SHORT)
    ALU2(UPSAMPLE_INT)
    ALU1(RNDZ)
    ALU1(RNDE)
    ALU1(RNDD)
    ALU1(RNDU)
    ALU1(F16TO32)
    ALU1(F32TO16)
    ALU2(SEL)
    ALU1(NOT)
    ALU2_MOD(AND)
    ALU2_MOD(OR)
    ALU2_MOD(XOR)
    ALU2(SHR)
    ALU2(SHL)
    ALU2(RSR)
    ALU2(RSL)
    ALU2(ASR)
    ALU2(ADD)
    ALU2(ADDC)
    ALU2(MUL)
    ALU1(FRC)
    ALU2(MAC)
    ALU2(MACH)
    ALU1(LZD)
    ALU2(LINE)
    ALU2(PLN)
    ALU3(MAD)
    //ALU2(MOV_DF);
    ALU2(BRC)
    ALU1(BRD)
#undef ALU1
#undef ALU2
#undef ALU2_MOD
#undef ALU3
    /*! Get double/long exec width */
    virtual int getDoubleExecWidth(void) { return GEN7_DOUBLE_EXEC_WIDTH; }
    virtual void MOV_DF(GenRegister dest, GenRegister src0, GenRegister tmp = GenRegister::null());
    virtual void LOAD_DF_IMM(GenRegister dest, GenRegister tmp, double value);
    void LOAD_INT64_IMM(GenRegister dest, int64_t value);
    /*! Barrier message (to synchronize threads of a workgroup) */
    void BARRIER(GenRegister src);
    /*! Memory fence message (to order loads and stores between threads) */
    void FENCE(GenRegister dst);
    /*! Jump indexed instruction */
    virtual void JMPI(GenRegister src, bool longjmp = false);
    /*! IF indexed instruction */
    void IF(GenRegister src);
    /*! ENDIF indexed instruction */
    void ENDIF(GenRegister src);
    /*! BRC indexed instruction */
    void BRC(GenRegister src);
    /*! BRD indexed instruction */
    void BRD(GenRegister src);
    /*! Compare instructions */
    void CMP(uint32_t conditional, GenRegister src0, GenRegister src1, GenRegister dst = GenRegister::null());
    /*! Select with embedded compare (like sel.le ...) */
    void SEL_CMP(uint32_t conditional, GenRegister dst, GenRegister src0, GenRegister src1);
    /*! EOT is used to finish GPGPU threads */
    void EOT(uint32_t msg_nr);
    /*! No-op */
    void NOP(void);
    /*! Wait instruction (used for the barrier) */
    void WAIT(void);
    /*! Atomic instructions */
    virtual void ATOMIC(GenRegister dst, uint32_t function, GenRegister src, uint32_t bti, uint32_t srcNum);
    /*! Untyped read (upto 4 channels) */
    virtual void UNTYPED_READ(GenRegister dst, GenRegister src, uint32_t bti, uint32_t elemNum);
    /*! Untyped write (upto 4 channels) */
    virtual void UNTYPED_WRITE(GenRegister src, uint32_t bti, uint32_t elemNum);
    /*! Byte gather (for unaligned bytes, shorts and ints) */
    void BYTE_GATHER(GenRegister dst, GenRegister src, uint32_t bti, uint32_t elemSize);
    /*! Byte scatter (for unaligned bytes, shorts and ints) */
    void BYTE_SCATTER(GenRegister src, uint32_t bti, uint32_t elemSize);
    /*! DWord gather (for constant cache read) */
    void DWORD_GATHER(GenRegister dst, GenRegister src, uint32_t bti);
    /*! for scratch memory read */
    void SCRATCH_READ(GenRegister msg, GenRegister dst, uint32_t offset, uint32_t size, uint32_t dst_num, uint32_t channel_mode);
    /*! for scratch memory write */
    void SCRATCH_WRITE(GenRegister msg, uint32_t offset, uint32_t size, uint32_t src_num, uint32_t channel_mode);
    /*! Send instruction for the sampler */
    void SAMPLE(GenRegister dest,
                GenRegister msg,
                unsigned int msg_len,
                bool header_present,
                unsigned char bti,
                unsigned char sampler,
                unsigned int simdWidth,
                uint32_t writemask,
                uint32_t return_format,
                bool isLD,
                bool isUniform);

    /*! TypedWrite instruction for texture */
    virtual void TYPED_WRITE(GenRegister header,
                             bool header_present,
                             unsigned char bti);
    /*! Extended math function (2 sources) */
    void MATH(GenRegister dst, uint32_t function, GenRegister src0, GenRegister src1);
    /*! Extended math function (1 source) */
    void MATH(GenRegister dst, uint32_t function, GenRegister src);

    /*! Patch JMPI/BRC/BRD (located at index insnID) with the given jump distance */
    virtual void patchJMPI(uint32_t insnID, int32_t jumpDistance);

    ////////////////////////////////////////////////////////////////////////
    // Helper functions to encode
    ////////////////////////////////////////////////////////////////////////
    virtual void setHeader(GenNativeInstruction *insn);
    virtual void setDPUntypedRW(GenNativeInstruction *insn, uint32_t bti, uint32_t rgba,
                                uint32_t msg_type, uint32_t msg_length,
                                uint32_t response_length);
    virtual void setTypedWriteMessage(GenNativeInstruction *insn, unsigned char bti,
                                      unsigned char msg_type, uint32_t msg_length,
                                      bool header_present);
    void setMessageDescriptor(GenNativeInstruction *inst, enum GenMessageTarget sfid,
                              unsigned msg_length, unsigned response_length,
                              bool header_present = false, bool end_of_thread = false);
    void setDst(GenNativeInstruction *insn, GenRegister dest);
    void setSrc0(GenNativeInstruction *insn, GenRegister reg);
    void setSrc1(GenNativeInstruction *insn, GenRegister reg);
    GenCompactInstruction *nextCompact(uint32_t opcode);
    GenNativeInstruction *next(uint32_t opcode);
    uint32_t n_instruction(void) const { return store.size(); }
    GBE_CLASS(GenEncoder); //!< Use custom allocators
  };

  void alu1(GenEncoder *p, uint32_t opcode, GenRegister dst,
            GenRegister src, uint32_t condition = 0);

  void alu2(GenEncoder *p, uint32_t opcode, GenRegister dst,
            GenRegister src0, GenRegister src1, uint32_t condition = 0);
} /* namespace gbe */

#endif /* __GBE_GEN_ENCODER_HPP__ */



/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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
    DebugInfo DBGInfo;
    vector<DebugInfo> storedbg;
    void setDBGInfo(DebugInfo in, bool hasHigh);
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
    ALU1(CBIT)
    ALU2(SUBB)
    ALU1(RNDZ)
    ALU1(RNDE)
    ALU1(RNDD)
    ALU1(RNDU)
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
    ALU3(LRP)
    ALU2(BRC)
    ALU1(BRD)
    ALU1(BFREV)
#undef ALU1
#undef ALU2
#undef ALU2_MOD
#undef ALU3

    virtual void F16TO32(GenRegister dest, GenRegister src0);
    virtual void F32TO16(GenRegister dest, GenRegister src0);
    virtual void LOAD_INT64_IMM(GenRegister dest, GenRegister value);
    /*! Barrier message (to synchronize threads of a workgroup) */
    void BARRIER(GenRegister src);
    /*! Forward the gateway message. */
    void FWD_GATEWAY_MSG(GenRegister src, uint32_t notifyN = 0);
    /*! Memory fence message (to order loads and stores between threads) */
    virtual void FENCE(GenRegister dst, bool flushRWCache);
    /*! Jump indexed instruction */
    virtual void JMPI(GenRegister src, bool longjmp = false);
    /*! IF indexed instruction */
    void IF(GenRegister src);
    /*! ELSE indexed instruction */
    void ELSE(GenRegister src);
    /*! ENDIF indexed instruction */
    void ENDIF(GenRegister src);
    /*! WHILE indexed instruction */
    void WHILE(GenRegister src);
    /*! BRC indexed instruction */
    void BRC(GenRegister src);
    /*! BRD indexed instruction */
    void BRD(GenRegister src);
    /*! Compare instructions */
    void CMP(uint32_t conditional, GenRegister src0, GenRegister src1, GenRegister dst = GenRegister::null());
    virtual bool needToSplitCmpBySrcType(GenEncoder *p, GenRegister src0, GenRegister src1);
    /*! Select with embedded compare (like sel.le ...) */
    void SEL_CMP(uint32_t conditional, GenRegister dst, GenRegister src0, GenRegister src1);
    /*! EOT is used to finish GPGPU threads */
    void EOT(uint32_t msg_nr);
    /*! No-op */
    void NOP(void);
    /*! Wait instruction (used for the barrier) */
    void WAIT(uint32_t n = 0);
    /*! Atomic instructions */
    virtual void ATOMIC(GenRegister dst, uint32_t function, GenRegister addr, GenRegister data, GenRegister bti, uint32_t srcNum, bool useSends);
    /*! AtomicA64 instructions */
    virtual void ATOMICA64(GenRegister dst, uint32_t function, GenRegister src, GenRegister bti, uint32_t srcNum);
    /*! Untyped read (upto 4 channels) */
    virtual void UNTYPED_READ(GenRegister dst, GenRegister src, GenRegister bti, uint32_t elemNum);
    /*! Untyped write (upto 4 channels) */
    virtual void UNTYPED_WRITE(GenRegister addr, GenRegister data, GenRegister bti, uint32_t elemNum, bool useSends);
    /*! Untyped read A64(upto 4 channels) */
    virtual void UNTYPED_READA64(GenRegister dst, GenRegister src, uint32_t elemNum);
    /*! Untyped write (upto 4 channels) */
    virtual void UNTYPED_WRITEA64(GenRegister src, uint32_t elemNum);
    /*! Byte gather (for unaligned bytes, shorts and ints) */
    void BYTE_GATHER(GenRegister dst, GenRegister src, GenRegister bti, uint32_t elemSize);
    /*! Byte scatter (for unaligned bytes, shorts and ints) */
    virtual void BYTE_SCATTER(GenRegister addr, GenRegister data, GenRegister bti, uint32_t elemSize, bool useSends);
    /*! Byte gather a64 (for unaligned bytes, shorts and ints) */
    virtual void BYTE_GATHERA64(GenRegister dst, GenRegister src, uint32_t elemSize);
    /*! Byte scatter a64 (for unaligned bytes, shorts and ints) */
    virtual void BYTE_SCATTERA64(GenRegister src, uint32_t elemSize);
    /*! DWord gather (for constant cache read) */
    void DWORD_GATHER(GenRegister dst, GenRegister src, uint32_t bti);
    /*! for scratch memory read */
    void SCRATCH_READ(GenRegister msg, GenRegister dst, uint32_t offset, uint32_t size, uint32_t dst_num, uint32_t channel_mode);
    /*! for scratch memory write */
    void SCRATCH_WRITE(GenRegister msg, uint32_t offset, uint32_t size, uint32_t src_num, uint32_t channel_mode);
    /*! Send instruction for the sampler */
    virtual void SAMPLE(GenRegister dest,
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
    void setSamplerMessage(GenNativeInstruction *insn,
                           unsigned char bti,
                           unsigned char sampler,
                           uint32_t msg_type,
                           uint32_t response_length,
                           uint32_t msg_length,
                           bool header_present,
                           uint32_t simd_mode,
                           uint32_t return_format);
    virtual void VME(unsigned char bti,
                         GenRegister dest,
                         GenRegister msg,
                         uint32_t msg_type,
                         uint32_t vme_search_path_lut,
                         uint32_t lut_sub);
    void setVmeMessage(GenNativeInstruction *insn,
                          unsigned char bti,
                          uint32_t response_length,
                          uint32_t msg_length,
                          uint32_t msg_type,
                          unsigned char vme_search_path_lut,
                          unsigned char lut_sub);
    virtual void IME(unsigned char bti,
                         GenRegister dest,
                         GenRegister msg,
                         uint32_t msg_type);
    virtual void FLUSH_SAMPLERCACHE(GenRegister dst);

    /*! TypedWrite instruction for texture */
    virtual void TYPED_WRITE(GenRegister header,
                             GenRegister data,
                             bool header_present,
                             unsigned char bti,
                             bool useSends);
    /*! Extended math function (2 sources) */
    void MATH(GenRegister dst, uint32_t function, GenRegister src0, GenRegister src1);
    /*! Extended math function (1 source) */
    void MATH(GenRegister dst, uint32_t function, GenRegister src);

    /*! Patch JMPI/BRC/BRD (located at index insnID) with the given jump distance */
    virtual void patchJMPI(uint32_t insnID, int32_t jip, int32_t uip);

    ////////////////////////////////////////////////////////////////////////
    // Helper functions to encode
    ////////////////////////////////////////////////////////////////////////
    void setDPByteScatterGather(GenNativeInstruction *insn, uint32_t bti, uint32_t elem_size,
                                     uint32_t msg_type, uint32_t msg_length, uint32_t response_length);
    virtual void setDPUntypedRW(GenNativeInstruction *insn, uint32_t bti, uint32_t rgba,
                                uint32_t msg_type, uint32_t msg_length,
                                uint32_t response_length);
    virtual void setTypedWriteMessage(GenNativeInstruction *insn, unsigned char bti,
                                      unsigned char msg_type, uint32_t msg_length,
                                      bool header_present);
    void setMessageDescriptor(GenNativeInstruction *inst, enum GenMessageTarget sfid,
                              unsigned msg_length, unsigned response_length,
                              bool header_present = false, bool end_of_thread = false);
    virtual unsigned setAtomicMessageDesc(GenNativeInstruction *insn, unsigned function, unsigned bti, unsigned srcNum);
    virtual unsigned setAtomicA64MessageDesc(GenNativeInstruction *insn, unsigned function, unsigned bti, unsigned srcNum, int type_long);
    virtual unsigned setUntypedReadMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemNum);
    virtual unsigned setUntypedWriteMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemNum);
    virtual unsigned setUntypedWriteSendsMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemNum);
    unsigned setByteGatherMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemSize);
    unsigned setByteScatterMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemSize);
    virtual unsigned setByteScatterSendsMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemSize);

    unsigned generateAtomicMessageDesc(unsigned function, unsigned bti, unsigned srcNum);
    unsigned generateUntypedReadMessageDesc(unsigned bti, unsigned elemNum);
    unsigned generateUntypedWriteMessageDesc(unsigned bti, unsigned elemNum);
    unsigned generateUntypedWriteSendsMessageDesc(unsigned bti, unsigned elemNum);
    unsigned generateByteGatherMessageDesc(unsigned bti, unsigned elemSize);
    unsigned generateByteScatterMessageDesc(unsigned bti, unsigned elemSize);
    unsigned generateByteScatterSendsMessageDesc(unsigned bti, unsigned elemSize);

    virtual void setHeader(GenNativeInstruction *insn) = 0;
    virtual void setDst(GenNativeInstruction *insn, GenRegister dest) = 0;
    virtual void setSrc0(GenNativeInstruction *insn, GenRegister reg) = 0;
    virtual void setSrc1(GenNativeInstruction *insn, GenRegister reg) = 0;
    GenCompactInstruction *nextCompact(uint32_t opcode);
    virtual uint32_t getCompactVersion() { return 7; }
    GenNativeInstruction *next(uint32_t opcode);
    uint32_t n_instruction(void) const { return store.size(); }
    virtual bool canHandleLong(uint32_t opcode, GenRegister dst, GenRegister src0,
                            GenRegister src1 = GenRegister::null());
    virtual void handleDouble(GenEncoder *p, uint32_t opcode, GenRegister dst, GenRegister src0, GenRegister src1 = GenRegister::null());

    /*! OBlock helper function */
    uint32_t getOBlockSize(uint32_t oword_size, bool low_half = true);
    void setMBlockRW(GenNativeInstruction *insn, uint32_t bti, uint32_t msg_type, uint32_t msg_length, uint32_t response_length);
    void setOBlockRW(GenNativeInstruction *insn, uint32_t bti, uint32_t block_size, uint32_t msg_type, uint32_t msg_length, uint32_t response_lengtha);

    /*! OBlock read */
    void OBREAD(GenRegister dst, GenRegister header, uint32_t bti, uint32_t ow_size);
    /*! OBlock write */
    virtual void OBWRITE(GenRegister header, GenRegister data, uint32_t bti, uint32_t ow_size, bool useSends);
    /*! MBlock read */
    virtual void MBREAD(GenRegister dst, GenRegister header, uint32_t bti, uint32_t response_size);
    /*! MBlock write */
    virtual void MBWRITE(GenRegister header, GenRegister data, uint32_t bti, uint32_t data_size, bool useSends);
    /*! A64 OBlock read */
    virtual void OBREADA64(GenRegister dst, GenRegister header, uint32_t bti, uint32_t ow_size);
    /*! A64 OBlock write */
    virtual void OBWRITEA64(GenRegister header, uint32_t bti, uint32_t ow_size);

    GBE_CLASS(GenEncoder); //!< Use custom allocators
    virtual void alu3(uint32_t opcode, GenRegister dst,
                       GenRegister src0, GenRegister src1, GenRegister src2) = 0;
  };

  void alu1(GenEncoder *p, uint32_t opcode, GenRegister dst,
            GenRegister src, uint32_t condition = 0);

  void alu2(GenEncoder *p, uint32_t opcode, GenRegister dst,
            GenRegister src0, GenRegister src1, uint32_t condition = 0);
} /* namespace gbe */

#endif /* __GBE_GEN_ENCODER_HPP__ */



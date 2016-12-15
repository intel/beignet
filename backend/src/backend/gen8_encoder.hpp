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
 */

/**
 * \file gen8_context.hpp
 */
#ifndef __GBE_GEN8_ENCODER_HPP__
#define __GBE_GEN8_ENCODER_HPP__

#include "backend/gen_encoder.hpp"

namespace gbe
{
  /* This class is used to implement the HSW
     specific logic for encoder. */
  class Gen8Encoder : public GenEncoder
  {
  public:
    virtual ~Gen8Encoder(void) { }

    Gen8Encoder(uint32_t simdWidth, uint32_t gen, uint32_t deviceID)
         : GenEncoder(simdWidth, gen, deviceID) { }

    /*! Jump indexed instruction */
    virtual void JMPI(GenRegister src, bool longjmp = false);
    virtual void FENCE(GenRegister dst, bool flushRWCache);
    /*! Patch JMPI/BRC/BRD (located at index insnID) with the given jump distance */
    virtual void patchJMPI(uint32_t insnID, int32_t jip, int32_t uip);
    virtual void F16TO32(GenRegister dest, GenRegister src0);
    virtual void F32TO16(GenRegister dest, GenRegister src0);
    virtual void LOAD_INT64_IMM(GenRegister dest, GenRegister value);
    virtual void ATOMIC(GenRegister dst, uint32_t function, GenRegister addr, GenRegister data, GenRegister bti, uint32_t srcNum, bool useSends);
    virtual void ATOMICA64(GenRegister dst, uint32_t function, GenRegister src, GenRegister bti, uint32_t srcNum);
    virtual void UNTYPED_READ(GenRegister dst, GenRegister src, GenRegister bti, uint32_t elemNum);
    virtual void UNTYPED_WRITE(GenRegister src, GenRegister data, GenRegister bti, uint32_t elemNum, bool useSends);
    virtual void UNTYPED_READA64(GenRegister dst, GenRegister src, uint32_t elemNum);
    virtual void UNTYPED_WRITEA64(GenRegister src, uint32_t elemNum);
    virtual void BYTE_GATHERA64(GenRegister dst, GenRegister src, uint32_t elemSize);
    virtual void BYTE_SCATTERA64(GenRegister src, uint32_t elemSize);
    virtual void setHeader(GenNativeInstruction *insn);
    virtual void setDPUntypedRW(GenNativeInstruction *insn, uint32_t bti, uint32_t rgba,
                   uint32_t msg_type, uint32_t msg_length, uint32_t response_length);
    virtual void setTypedWriteMessage(GenNativeInstruction *insn, unsigned char bti,
                                      unsigned char msg_type, uint32_t msg_length,
                                      bool header_present);
    virtual void FLUSH_SAMPLERCACHE(GenRegister dst);
    virtual void setDst(GenNativeInstruction *insn, GenRegister dest);
    virtual void setSrc0(GenNativeInstruction *insn, GenRegister reg);
    virtual void setSrc1(GenNativeInstruction *insn, GenRegister reg);
    virtual uint32_t getCompactVersion() { return 8; }
    virtual void alu3(uint32_t opcode, GenRegister dst,
                       GenRegister src0, GenRegister src1, GenRegister src2);
    virtual bool canHandleLong(uint32_t opcode, GenRegister dst, GenRegister src0,
                            GenRegister src1 = GenRegister::null());
    virtual void handleDouble(GenEncoder *p, uint32_t opcode, GenRegister dst, GenRegister src0, GenRegister src1 = GenRegister::null());
    virtual unsigned setAtomicMessageDesc(GenNativeInstruction *insn, unsigned function, unsigned bti, unsigned srcNum);
    virtual unsigned setAtomicA64MessageDesc(GenNativeInstruction *insn, unsigned function, unsigned bti, unsigned srcNum, int type_long);
    virtual unsigned setUntypedReadMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemNum);
    virtual unsigned setUntypedWriteMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemNum);
    void setSrc0WithAcc(GenNativeInstruction *insn, GenRegister reg, uint32_t accN);
    void setSrc1WithAcc(GenNativeInstruction *insn, GenRegister reg, uint32_t accN);

    void MATH_WITH_ACC(GenRegister dst, uint32_t function, GenRegister src0, GenRegister src1,
                       uint32_t dstAcc, uint32_t src0Acc, uint32_t src1Acc);
    void MADM(GenRegister dst, GenRegister src0, GenRegister src1, GenRegister src2,
              uint32_t dstAcc, uint32_t src0Acc, uint32_t src1Acc, uint32_t src2Acc);
    /*! A64 OBlock read */
    virtual void OBREADA64(GenRegister dst, GenRegister header, uint32_t bti, uint32_t elemSize);
    /*! A64 OBlock write */
    virtual void OBWRITEA64(GenRegister header, uint32_t bti, uint32_t elemSize);
  };
}
#endif /* __GBE_GEN8_ENCODER_HPP__ */

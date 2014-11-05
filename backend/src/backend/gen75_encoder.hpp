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
 * \file gen75_context.hpp
 */
#ifndef __GBE_GEN75_ENCODER_HPP__
#define __GBE_GEN75_ENCODER_HPP__

#include "backend/gen_encoder.hpp"
#include "backend/gen7_encoder.hpp"


namespace gbe
{
  /* This class is used to implement the HSW
     specific logic for encoder. */
  class Gen75Encoder : public Gen7Encoder
  {
  public:
    /*! exec width of the double data type */    
    #define GEN75_DOUBLE_EXEC_WIDTH  4
    virtual ~Gen75Encoder(void) { }

    Gen75Encoder(uint32_t simdWidth, uint32_t gen, uint32_t deviceID)
         : Gen7Encoder(simdWidth, gen, deviceID) { }

    /*! Jump indexed instruction */
    virtual void JMPI(GenRegister src, bool longjmp = false);
    /*! Patch JMPI/BRC/BRD (located at index insnID) with the given jump distance */
    virtual void patchJMPI(uint32_t insnID, int32_t jip, int32_t uip);
    /*! Get double/long exec width */
    virtual int getDoubleExecWidth(void) { return GEN75_DOUBLE_EXEC_WIDTH; }
    virtual void MOV_DF(GenRegister dest, GenRegister src0, GenRegister tmp = GenRegister::null());
    virtual void LOAD_DF_IMM(GenRegister dest, GenRegister tmp, double value);
    virtual void ATOMIC(GenRegister dst, uint32_t function, GenRegister src, uint32_t bti, uint32_t srcNum);
    virtual void UNTYPED_READ(GenRegister dst, GenRegister src, uint32_t bti, uint32_t elemNum);
    virtual void UNTYPED_WRITE(GenRegister src, uint32_t bti, uint32_t elemNum);
    virtual void setHeader(GenNativeInstruction *insn);
    virtual void setDPUntypedRW(GenNativeInstruction *insn, uint32_t bti, uint32_t rgba,
                   uint32_t msg_type, uint32_t msg_length, uint32_t response_length);
    virtual void setTypedWriteMessage(GenNativeInstruction *insn, unsigned char bti,
                                      unsigned char msg_type, uint32_t msg_length,
                                      bool header_present);
  };
}
#endif /* __GBE_GEN75_ENCODER_HPP__ */

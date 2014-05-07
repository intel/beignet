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
 */

/**
 * \file gen75_context.hpp
 */
#ifndef __GBE_GEN75_ENCODER_HPP__
#define __GBE_GEN75_ENCODER_HPP__

#include "backend/gen_encoder.hpp"

namespace gbe
{
  /* This class is used to implement the HSW
     specific logic for encoder. */
  class Gen75Encoder : public GenEncoder
  {
  public:
    Gen75Encoder(uint32_t simdWidth, uint32_t gen, uint32_t deviceID)
         : GenEncoder(simdWidth, gen, deviceID, 8) { };

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

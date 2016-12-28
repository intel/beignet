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
 * \file gen9_context.hpp
 */
#ifndef __GBE_GEN9_ENCODER_HPP__
#define __GBE_GEN9_ENCODER_HPP__

#include "backend/gen8_encoder.hpp"

namespace gbe
{
  /* This class is used to implement the SKL
     specific logic for encoder. */
  class Gen9Encoder : public Gen8Encoder
  {
  public:
    virtual ~Gen9Encoder(void) { }

    Gen9Encoder(uint32_t simdWidth, uint32_t gen, uint32_t deviceID)
         : Gen8Encoder(simdWidth, gen, deviceID) { }
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
    void setSendsOperands(Gen9NativeInstruction *gen9_insn, GenRegister dst, GenRegister src0, GenRegister src1);
    virtual void UNTYPED_WRITE(GenRegister addr, GenRegister data, GenRegister bti, uint32_t elemNum, bool useSends);
    virtual void TYPED_WRITE(GenRegister header, GenRegister data, bool header_present, unsigned char bti, bool useSends);
    virtual unsigned setUntypedWriteSendsMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemNum);
    virtual void BYTE_SCATTER(GenRegister addr, GenRegister data, GenRegister bti, uint32_t elemSize, bool useSends);
    virtual unsigned setByteScatterSendsMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemSize);
    virtual void ATOMIC(GenRegister dst, uint32_t function, GenRegister addr, GenRegister data, GenRegister bti, uint32_t srcNum, bool useSends);
    virtual void OBWRITE(GenRegister header, GenRegister data, uint32_t bti, uint32_t ow_size, bool useSends);
    virtual void MBWRITE(GenRegister header, GenRegister data, uint32_t bti, uint32_t data_size, bool useSends);
  };
}
#endif /* __GBE_GEN9_ENCODER_HPP__ */

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
 * \file gen7_context.hpp
 */
#ifndef __GBE_GEN7_ENCODER_HPP__
#define __GBE_GEN7_ENCODER_HPP__

#include "backend/gen_encoder.hpp"

namespace gbe
{
  /* This class is used to implement the HSW
     specific logic for encoder. */
  class Gen7Encoder : public GenEncoder
  {
  public:
    /*! gen7 exec width of the double data type */
    #define GEN7_DOUBLE_EXEC_WIDTH  8
    virtual ~Gen7Encoder(void) { }

    Gen7Encoder(uint32_t simdWidth, uint32_t gen, uint32_t deviceID)
         : GenEncoder(simdWidth, gen, deviceID) { }

    /*! Get double/long exec width */
    virtual int getDoubleExecWidth(void) { return GEN7_DOUBLE_EXEC_WIDTH; }
    virtual void setHeader(GenNativeInstruction *insn);
    virtual void setDst(GenNativeInstruction *insn, GenRegister dest);
    virtual void setSrc0(GenNativeInstruction *insn, GenRegister reg);
    virtual void setSrc1(GenNativeInstruction *insn, GenRegister reg);
    virtual void alu3(uint32_t opcode, GenRegister dst,
                       GenRegister src0, GenRegister src1, GenRegister src2);
  };
}
#endif /* __GBE_GEN7_ENCODER_HPP__ */

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

#include "backend/gen9_encoder.hpp"

namespace gbe
{
  void Gen9Encoder::SAMPLE(GenRegister dest,
                          GenRegister msg,
                          unsigned int msg_len,
                          bool header_present,
                          unsigned char bti,
                          unsigned char sampler,
                          uint32_t simdWidth,
                          uint32_t writemask,
                          uint32_t return_format,
                          bool isLD,
                          bool isUniform)
  {
     if (writemask == 0) return;
     uint32_t msg_type = isLD ? GEN_SAMPLER_MESSAGE_SIMD8_LD :
                                GEN_SAMPLER_MESSAGE_SIMD8_SAMPLE;
     uint32_t response_length = (4 * (simdWidth / 8));
     uint32_t msg_length = (msg_len * (simdWidth / 8));
     if (header_present)
       msg_length++;
     uint32_t simd_mode = (simdWidth == 16) ?
                            GEN_SAMPLER_SIMD_MODE_SIMD16 : GEN_SAMPLER_SIMD_MODE_SIMD8;
    if(isUniform) {
      response_length = 1;
      msg_type = GEN_SAMPLER_MESSAGE_SIMD4X2_LD;
      msg_length = 1;
      simd_mode = GEN_SAMPLER_SIMD_MODE_SIMD8;
    }
     GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
     this->setHeader(insn);
     this->setDst(insn, dest);
     this->setSrc0(insn, msg);
     this->setSrc1(insn, GenRegister::immud(0));
     setSamplerMessage(insn, bti, sampler, msg_type,
                       response_length, msg_length,
                       header_present,
                       simd_mode, return_format);
  }
} /* End of the name space. */

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

#include "backend/gen75_encoder.hpp"

namespace gbe
{
  void Gen75Encoder::setDPUntypedRW(GenNativeInstruction *insn,
                                    uint32_t bti,
                                    uint32_t rgba,
                                    uint32_t msg_type,
                                    uint32_t msg_length,
                                    uint32_t response_length)
  {
    const GenMessageTarget sfid = GEN_SFID_DATAPORT1_DATA_CACHE;
    setMessageDescriptor(insn, sfid, msg_length, response_length);
    insn->bits3.gen7_untyped_rw.msg_type = msg_type;
    insn->bits3.gen7_untyped_rw.bti = bti;
    insn->bits3.gen7_untyped_rw.rgba = rgba;
    if (curr.execWidth == 8)
      insn->bits3.gen7_untyped_rw.simd_mode = GEN_UNTYPED_SIMD8;
    else if (curr.execWidth == 16)
      insn->bits3.gen7_untyped_rw.simd_mode = GEN_UNTYPED_SIMD16;
    else
      NOT_SUPPORTED;
  }

  void Gen75Encoder::setTypedWriteMessage(GenNativeInstruction *insn, unsigned char bti,
                                          unsigned char msg_type, uint32_t msg_length, bool header_present)
  {
    const GenMessageTarget sfid = GEN_SFID_DATAPORT1_DATA_CACHE;
    setMessageDescriptor(insn, sfid, msg_length, 0, header_present);
    insn->bits3.gen7_typed_rw.bti = bti;
    insn->bits3.gen7_typed_rw.msg_type = msg_type;

    /* Always using the low 8 slots here. */
    insn->bits3.gen7_typed_rw.slot = 1;
  }
} /* End of the name space. */

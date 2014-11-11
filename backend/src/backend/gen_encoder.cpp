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

#include "backend/gen_encoder.hpp"
#include <cstring>


namespace gbe
{
  extern bool compactAlu2(GenEncoder *p, uint32_t opcode, GenRegister dst, GenRegister src0, GenRegister src1, uint32_t condition, bool split);
  extern bool compactAlu1(GenEncoder *p, uint32_t opcode, GenRegister dst, GenRegister src, uint32_t condition, bool split);
  //////////////////////////////////////////////////////////////////////////
  // Some helper functions to encode
  //////////////////////////////////////////////////////////////////////////
  INLINE bool isVectorOfBytes(GenRegister reg) {
    if (reg.hstride != GEN_HORIZONTAL_STRIDE_0 &&
        (reg.type == GEN_TYPE_UB || reg.type == GEN_TYPE_B))
      return true;
    else
      return false;
  }

  INLINE bool needToSplitAlu1(GenEncoder *p, GenRegister dst, GenRegister src) {
    if (p->curr.execWidth != 16 || src.hstride == GEN_HORIZONTAL_STRIDE_0) return false;
    if (isVectorOfBytes(dst) == true) return true;
    if (isVectorOfBytes(src) == true) return true;
    return false;
  }

  INLINE bool needToSplitAlu2(GenEncoder *p, GenRegister dst, GenRegister src0, GenRegister src1) {
    if (p->curr.execWidth != 16 ||
         (src0.hstride == GEN_HORIZONTAL_STRIDE_0 &&
          src1.hstride == GEN_HORIZONTAL_STRIDE_0))
      return false;
    if (isVectorOfBytes(dst) == true) return true;
    if (isVectorOfBytes(src0) == true) return true;
    if (isVectorOfBytes(src1) == true) return true;
    return false;
  }

  INLINE bool needToSplitCmp(GenEncoder *p, GenRegister src0, GenRegister src1) {
    if (p->curr.execWidth != 16 ||
         (src0.hstride == GEN_HORIZONTAL_STRIDE_0 &&
          src1.hstride == GEN_HORIZONTAL_STRIDE_0))
      return false;
    if (isVectorOfBytes(src0) == true) return true;
    if (isVectorOfBytes(src1) == true) return true;
    if (src0.type == GEN_TYPE_D || src0.type == GEN_TYPE_UD || src0.type == GEN_TYPE_F)
      return true;
    if (src1.type == GEN_TYPE_D || src1.type == GEN_TYPE_UD || src1.type == GEN_TYPE_F)
      return true;
    return false;
  }

  void GenEncoder::setMessageDescriptor(GenNativeInstruction *inst, enum GenMessageTarget sfid,
                                        unsigned msg_length, unsigned response_length,
                                        bool header_present, bool end_of_thread)
  {
     setSrc1(inst, GenRegister::immd(0));
     inst->bits3.generic_gen5.header_present = header_present;
     inst->bits3.generic_gen5.response_length = response_length;
     inst->bits3.generic_gen5.msg_length = msg_length;
     inst->bits3.generic_gen5.end_of_thread = end_of_thread;
     inst->header.destreg_or_condmod = sfid;
  }

  void GenEncoder::setTypedWriteMessage(GenNativeInstruction *insn, unsigned char bti,
                                        unsigned char msg_type, uint32_t msg_length,
                                        bool header_present)
  {
    const GenMessageTarget sfid = GEN_SFID_DATAPORT_RENDER;
    setMessageDescriptor(insn, sfid, msg_length, 0, header_present);
    insn->bits3.gen7_typed_rw.bti = bti;
    insn->bits3.gen7_typed_rw.msg_type = msg_type;
  }

  void GenEncoder::setDPUntypedRW(GenNativeInstruction *insn, uint32_t bti,
                                  uint32_t rgba, uint32_t msg_type,
                                  uint32_t msg_length, uint32_t response_length)
  {
    const GenMessageTarget sfid = GEN_SFID_DATAPORT_DATA;
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

  static void setDPByteScatterGather(GenEncoder *p,
                                     GenNativeInstruction *insn,
                                     uint32_t bti,
                                     uint32_t elem_size,
                                     uint32_t msg_type,
                                     uint32_t msg_length,
                                     uint32_t response_length)
  {
    const GenMessageTarget sfid = GEN_SFID_DATAPORT_DATA;
    p->setMessageDescriptor(insn, sfid, msg_length, response_length);
    insn->bits3.gen7_byte_rw.msg_type = msg_type;
    insn->bits3.gen7_byte_rw.bti = bti;
    insn->bits3.gen7_byte_rw.data_size = elem_size;
    if (p->curr.execWidth == 8)
      insn->bits3.gen7_byte_rw.simd_mode = GEN_BYTE_SCATTER_SIMD8;
    else if (p->curr.execWidth == 16)
      insn->bits3.gen7_byte_rw.simd_mode = GEN_BYTE_SCATTER_SIMD16;
    else
      NOT_SUPPORTED;
  }
#if 0
  static void setOBlockRW(GenEncoder *p,
                          GenNativeInstruction *insn,
                          uint32_t bti,
                          uint32_t size,
                          uint32_t msg_type,
                          uint32_t msg_length,
                          uint32_t response_length)
  {
    const GenMessageTarget sfid = GEN_SFID_DATAPORT_DATA;
    p->setMessageDescriptor(insn, sfid, msg_length, response_length);
    assert(size == 2 || size == 4);
    insn->bits3.gen7_oblock_rw.msg_type = msg_type;
    insn->bits3.gen7_oblock_rw.bti = bti;
    insn->bits3.gen7_oblock_rw.block_size = size == 2 ? 2 : 3;
    insn->bits3.gen7_oblock_rw.header_present = 1;
  }
#endif

  static void setSamplerMessage(GenEncoder *p,
                                GenNativeInstruction *insn,
                                unsigned char bti,
                                unsigned char sampler,
                                uint32_t msg_type,
                                uint32_t response_length,
                                uint32_t msg_length,
                                bool header_present,
                                uint32_t simd_mode,
                                uint32_t return_format)
  {
     const GenMessageTarget sfid = GEN_SFID_SAMPLER;
     p->setMessageDescriptor(insn, sfid, msg_length, response_length);
     insn->bits3.sampler_gen7.bti = bti;
     insn->bits3.sampler_gen7.sampler = sampler;
     insn->bits3.sampler_gen7.msg_type = msg_type;
     insn->bits3.sampler_gen7.simd_mode = simd_mode;
  }

  static void setDWordScatterMessgae(GenEncoder *p,
                                     GenNativeInstruction *insn,
                                     uint32_t bti,
                                     uint32_t block_size,
                                     uint32_t msg_type,
                                     uint32_t msg_length,
                                     uint32_t response_length)
  {
    // FIXME there is a unknown issue with baytrail-t platform, the DWORD scatter
    // message causes a hang at unit test case compiler_global_constant.
    // We workaround it to use DATA CACHE instead.
    const GenMessageTarget sfid = (p->deviceID == PCI_CHIP_BAYTRAIL_T) ?
                                 GEN_SFID_DATAPORT_DATA : GEN_SFID_DATAPORT_CONSTANT;
    p->setMessageDescriptor(insn, sfid, msg_length, response_length);
    insn->bits3.gen7_dword_rw.msg_type = msg_type;
    insn->bits3.gen7_dword_rw.bti = bti;
    insn->bits3.gen7_dword_rw.block_size = block_size;
    insn->bits3.gen7_dword_rw.invalidate_after_read = 0;
  }
  //////////////////////////////////////////////////////////////////////////
  // Gen Emitter encoding class
  //////////////////////////////////////////////////////////////////////////
  GenEncoder::GenEncoder(uint32_t simdWidth, uint32_t gen, uint32_t deviceID) :
    stateNum(0), gen(gen), deviceID(deviceID)
  {
    this->simdWidth = simdWidth;
    this->curr.execWidth = simdWidth;
    this->curr.quarterControl = GEN_COMPRESSION_Q1;
    this->curr.noMask = 0;
    this->curr.flag = 0;
    this->curr.subFlag = 0;
    this->curr.predicate = GEN_PREDICATE_NORMAL;
    this->curr.inversePredicate = 0;
  }

  void GenEncoder::push(void) {
    assert(stateNum < MAX_STATE_NUM);
    stack[stateNum++] = curr;
  }

  void GenEncoder::pop(void) {
    assert(stateNum > 0);
    curr = stack[--stateNum];
  }

  static const uint32_t untypedRWMask[] = {
    GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE|GEN_UNTYPED_GREEN|GEN_UNTYPED_RED,
    GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE|GEN_UNTYPED_GREEN,
    GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE,
    GEN_UNTYPED_ALPHA,
    0
  };

  void GenEncoder::UNTYPED_READ(GenRegister dst, GenRegister src, uint32_t bti, uint32_t elemNum) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    assert(elemNum >= 1 || elemNum <= 4);
    uint32_t msg_length = 0;
    uint32_t response_length = 0;
    if (this->curr.execWidth == 8) {
      msg_length = 1;
      response_length = elemNum;
    } else if (this->curr.execWidth == 16) {
      msg_length = 2;
      response_length = 2*elemNum;
    } else
      NOT_IMPLEMENTED;

    this->setHeader(insn);
    this->setDst(insn,  GenRegister::uw16grf(dst.nr, 0));
    this->setSrc0(insn, GenRegister::ud8grf(src.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));
    setDPUntypedRW(insn,
                   bti,
                   untypedRWMask[elemNum],
                   GEN7_UNTYPED_READ,
                   msg_length,
                   response_length);
  }

  void GenEncoder::UNTYPED_WRITE(GenRegister msg, uint32_t bti, uint32_t elemNum) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    assert(elemNum >= 1 || elemNum <= 4);
    uint32_t msg_length = 0;
    uint32_t response_length = 0;
    this->setHeader(insn);
    if (this->curr.execWidth == 8) {
      this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UD));
      msg_length = 1+elemNum;
    } else if (this->curr.execWidth == 16) {
      this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UW));
      msg_length = 2*(1+elemNum);
    }
    else
      NOT_IMPLEMENTED;
    this->setSrc0(insn, GenRegister::ud8grf(msg.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));
    setDPUntypedRW(insn,
                   bti,
                   untypedRWMask[elemNum],
                   GEN7_UNTYPED_WRITE,
                   msg_length,
                   response_length);
  }

  void GenEncoder::BYTE_GATHER(GenRegister dst, GenRegister src, uint32_t bti, uint32_t elemSize) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    uint32_t msg_length = 0;
    uint32_t response_length = 0;
    if (this->curr.execWidth == 8) {
      msg_length = 1;
      response_length = 1;
    } else if (this->curr.execWidth == 16) {
      msg_length = 2;
      response_length = 2;
    } else
      NOT_IMPLEMENTED;

    this->setHeader(insn);
    this->setDst(insn, GenRegister::uw16grf(dst.nr, 0));
    this->setSrc0(insn, GenRegister::ud8grf(src.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));
    setDPByteScatterGather(this,
                           insn,
                           bti,
                           elemSize,
                           GEN7_BYTE_GATHER,
                           msg_length,
                           response_length);
  }

  void GenEncoder::BYTE_SCATTER(GenRegister msg, uint32_t bti, uint32_t elemSize) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    uint32_t msg_length = 0;
    uint32_t response_length = 0;
    this->setHeader(insn);
    if (this->curr.execWidth == 8) {
      this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UD));
      msg_length = 2;
    } else if (this->curr.execWidth == 16) {
      this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UW));
      msg_length = 4;
    } else
      NOT_IMPLEMENTED;
    this->setSrc0(insn, GenRegister::ud8grf(msg.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));
    setDPByteScatterGather(this,
                           insn,
                           bti,
                           elemSize,
                           GEN7_BYTE_SCATTER,
                           msg_length,
                           response_length);
  }

  void GenEncoder::DWORD_GATHER(GenRegister dst, GenRegister src, uint32_t bti) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    uint32_t msg_length = 0;
    uint32_t response_length = 0;
    uint32_t block_size = 0;
    if (this->curr.execWidth == 8) {
      msg_length = 1;
      response_length = 1;
      block_size = GEN_DWORD_SCATTER_8_DWORDS;
    } else if (this->curr.execWidth == 16) {
      msg_length = 2;
      response_length = 2;
      block_size = GEN_DWORD_SCATTER_16_DWORDS;
    } else
      NOT_IMPLEMENTED;

    this->setHeader(insn);
    this->setDst(insn, dst);
    this->setSrc0(insn, src);
    this->setSrc1(insn, GenRegister::immud(0));
    setDWordScatterMessgae(this,
                           insn,
                           bti,
                           block_size,
                           GEN7_DWORD_GATHER,
                           msg_length,
                           response_length);

  }

  void GenEncoder::ATOMIC(GenRegister dst, uint32_t function, GenRegister src, uint32_t bti, uint32_t srcNum) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    uint32_t msg_length = 0;
    uint32_t response_length = 0;

    if (this->curr.execWidth == 8) {
      msg_length = srcNum;
      response_length = 1;
    } else if (this->curr.execWidth == 16) {
      msg_length = 2*srcNum;
      response_length = 2;
    } else
      NOT_IMPLEMENTED;

    this->setHeader(insn);
    this->setDst(insn, GenRegister::uw16grf(dst.nr, 0));
    this->setSrc0(insn, GenRegister::ud8grf(src.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));

    const GenMessageTarget sfid = GEN_SFID_DATAPORT_DATA;
    setMessageDescriptor(insn, sfid, msg_length, response_length);
    insn->bits3.gen7_atomic_op.msg_type = GEN7_UNTYPED_ATOMIC_READ;
    insn->bits3.gen7_atomic_op.bti = bti;
    insn->bits3.gen7_atomic_op.return_data = 1;
    insn->bits3.gen7_atomic_op.aop_type = function;

    if (this->curr.execWidth == 8)
      insn->bits3.gen7_atomic_op.simd_mode = GEN_ATOMIC_SIMD8;
    else if (this->curr.execWidth == 16)
      insn->bits3.gen7_atomic_op.simd_mode = GEN_ATOMIC_SIMD16;
    else
      NOT_SUPPORTED;

  }
  GenCompactInstruction *GenEncoder::nextCompact(uint32_t opcode) {
    GenCompactInstruction insn;
    std::memset(&insn, 0, sizeof(GenCompactInstruction));
    insn.bits1.opcode = opcode;
    this->store.push_back(insn.low);
    return (GenCompactInstruction *)&this->store.back();
  }

  GenNativeInstruction *GenEncoder::next(uint32_t opcode) {
     GenNativeInstruction insn;
     std::memset(&insn, 0, sizeof(GenNativeInstruction));
     insn.header.opcode = opcode;
     this->store.push_back(insn.low);
     this->store.push_back(insn.high);
     return (GenNativeInstruction *)(&this->store.back()-1);
  }

  INLINE void _handleDouble(GenEncoder *p, uint32_t opcode, GenRegister dst,
                            GenRegister src0, GenRegister src1 = GenRegister::null()) {
       int w = p->curr.execWidth;
       p->push();
       p->curr.execWidth = p->getDoubleExecWidth();
       p->curr.nibControl = 0;
       GenNativeInstruction *insn = p->next(opcode);
       p->setHeader(insn);
       p->setDst(insn, dst);
       p->setSrc0(insn, src0);
       if (!GenRegister::isNull(src1))
         p->setSrc1(insn, src1);
       if (w == 8)
         p->curr.nibControl = 1; // second 1/8 mask
       insn = p->next(opcode);
       p->setHeader(insn);
       p->setDst(insn, GenRegister::suboffset(dst, w / 2));
       p->setSrc0(insn, GenRegister::suboffset(src0, w / 2));
       if (!GenRegister::isNull(src1))
         p->setSrc1(insn, GenRegister::suboffset(src1, w / 2));
       p->pop();
  }

  // Double register accessing is a little special,
  // Per Gen spec, then only supported mode is SIMD8 and, it only
  // handles four doubles each time.
  // We need to lower down SIMD16 to two SIMD8 and lower down SIMD8
  // to two SIMD1x4.
  INLINE void handleDouble(GenEncoder *p, uint32_t opcode, GenRegister dst,
                           GenRegister src0, GenRegister src1 = GenRegister::null()) {
      if (p->curr.execWidth == 8)
        _handleDouble(p, opcode, dst, src0, src1);
      else if (p->curr.execWidth == 16) {
        p->push();
        p->curr.execWidth = 8;
        p->curr.quarterControl = GEN_COMPRESSION_Q1;
        _handleDouble(p, opcode, dst, src0, src1);
        p->curr.quarterControl = GEN_COMPRESSION_Q2;
        if (!GenRegister::isNull(src1))
          src1 = GenRegister::offset(src1, 2);
        _handleDouble(p, opcode, GenRegister::offset(dst, 2), GenRegister::offset(src0, 2), src1);
        p->pop();
      }
  }

  void alu1(GenEncoder *p, uint32_t opcode, GenRegister dst,
            GenRegister src, uint32_t condition) {
     if (dst.isdf() && src.isdf()) {
       handleDouble(p, opcode, dst, src);
     } else if (dst.isint64() && src.isint64()) { // handle int64
       p->MOV(dst.bottom_half(), src.bottom_half());
       p->MOV(dst.top_half(p->simdWidth), src.top_half(p->simdWidth));
     } else if (needToSplitAlu1(p, dst, src) == false) {
      if(compactAlu1(p, opcode, dst, src, condition, false))
        return;
       GenNativeInstruction *insn = p->next(opcode);
       if (condition != 0) {
         GBE_ASSERT(opcode == GEN_OPCODE_MOV ||
                    opcode == GEN_OPCODE_NOT);
         insn->header.destreg_or_condmod = condition;
       }
       p->setHeader(insn);
       p->setDst(insn, dst);
       p->setSrc0(insn, src);
     } else {
       GenNativeInstruction *insnQ1, *insnQ2;

       // Instruction for the first quarter
       insnQ1 = p->next(opcode);
       p->setHeader(insnQ1);
       insnQ1->header.quarter_control = GEN_COMPRESSION_Q1;
       insnQ1->header.execution_size = GEN_WIDTH_8;
       p->setDst(insnQ1, dst);
       p->setSrc0(insnQ1, src);

       // Instruction for the second quarter
       insnQ2 = p->next(opcode);
       p->setHeader(insnQ2);
       insnQ2->header.quarter_control = GEN_COMPRESSION_Q2;
       insnQ2->header.execution_size = GEN_WIDTH_8;
       p->setDst(insnQ2, GenRegister::Qn(dst, 1));
       p->setSrc0(insnQ2, GenRegister::Qn(src, 1));
     }
  }

  void alu2(GenEncoder *p,
            uint32_t opcode,
            GenRegister dst,
            GenRegister src0,
            GenRegister src1,
            uint32_t condition)
  {
    if (dst.isdf() && src0.isdf() && src1.isdf()) {
       handleDouble(p, opcode, dst, src0, src1);
    } else if (needToSplitAlu2(p, dst, src0, src1) == false) {
       if(compactAlu2(p, opcode, dst, src0, src1, condition, false))
         return;
       GenNativeInstruction *insn = p->next(opcode);
       if (condition != 0) {
         GBE_ASSERT(opcode == GEN_OPCODE_OR ||
                    opcode == GEN_OPCODE_XOR ||
                    opcode == GEN_OPCODE_AND);
         insn->header.destreg_or_condmod = condition;
       }
       p->setHeader(insn);
       p->setDst(insn, dst);
       p->setSrc0(insn, src0);
       p->setSrc1(insn, src1);
    } else {
       GenNativeInstruction *insnQ1, *insnQ2;

       // Instruction for the first quarter
       insnQ1 = p->next(opcode);
       p->setHeader(insnQ1);
       insnQ1->header.quarter_control = GEN_COMPRESSION_Q1;
       insnQ1->header.execution_size = GEN_WIDTH_8;
       p->setDst(insnQ1, dst);
       p->setSrc0(insnQ1, src0);
       p->setSrc1(insnQ1, src1);

       // Instruction for the second quarter
       insnQ2 = p->next(opcode);
       p->setHeader(insnQ2);
       insnQ2->header.quarter_control = GEN_COMPRESSION_Q2;
       insnQ2->header.execution_size = GEN_WIDTH_8;
       p->setDst(insnQ2, GenRegister::Qn(dst, 1));
       p->setSrc0(insnQ2, GenRegister::Qn(src0, 1));
       p->setSrc1(insnQ2, GenRegister::Qn(src1, 1));
    }
  }

#define ALU1(OP) \
  void GenEncoder::OP(GenRegister dest, GenRegister src0, uint32_t condition) { \
    alu1(this, GEN_OPCODE_##OP, dest, src0, condition); \
  }

#define ALU2(OP) \
  void GenEncoder::OP(GenRegister dest, GenRegister src0, GenRegister src1) { \
    alu2(this, GEN_OPCODE_##OP, dest, src0, src1, 0); \
  }

#define ALU2_MOD(OP) \
  void GenEncoder::OP(GenRegister dest, GenRegister src0, GenRegister src1, uint32_t condition) { \
    alu2(this, GEN_OPCODE_##OP, dest, src0, src1, condition); \
  }


#define ALU3(OP) \
  void GenEncoder::OP(GenRegister dest, GenRegister src0, GenRegister src1, GenRegister src2) { \
    this->alu3(GEN_OPCODE_##OP, dest, src0, src1, src2); \
  }

  void GenEncoder::LOAD_DF_IMM(GenRegister dest, GenRegister tmp, double value) {
    union { double d; unsigned u[2]; } u;
    u.d = value;
    GenRegister r = GenRegister::retype(tmp, GEN_TYPE_UD);
    push();
    curr.predicate = GEN_PREDICATE_NONE;
    curr.noMask = 1;
    curr.execWidth = 1;
    MOV(r, GenRegister::immud(u.u[1]));
    MOV(GenRegister::suboffset(r, 1), GenRegister::immud(u.u[0]));
    pop();
    r.type = GEN_TYPE_DF;
    r.vstride = GEN_VERTICAL_STRIDE_0;
    r.width = GEN_WIDTH_1;
    r.hstride = GEN_HORIZONTAL_STRIDE_0;
    push();
    uint32_t width = curr.execWidth;
    curr.execWidth = 8;
    curr.predicate = GEN_PREDICATE_NONE;
    curr.noMask = 1;
    curr.quarterControl = GEN_COMPRESSION_Q1;
    MOV(dest, r);
    if (width == 16) {
      curr.quarterControl = GEN_COMPRESSION_Q2;
      MOV(GenRegister::offset(dest, 2), r);
    }
    pop();
  }

  void GenEncoder::LOAD_INT64_IMM(GenRegister dest, int64_t value) {
    GenRegister u0 = GenRegister::immd((int)value), u1 = GenRegister::immd(value >> 32);
    MOV(dest.bottom_half(), u0);
    MOV(dest.top_half(this->simdWidth), u1);
  }

  void GenEncoder::MOV_DF(GenRegister dest, GenRegister src0, GenRegister tmp) {
    GBE_ASSERT((src0.type == GEN_TYPE_F && dest.isdf()) || (src0.isdf() && dest.type == GEN_TYPE_F));
    GenRegister r = GenRegister::retype(tmp, GEN_TYPE_F);
    int w = curr.execWidth;
    GenRegister r0;
    int factor = 1;
    if (dest.type == GEN_TYPE_F) {
      r0 = r;
      r = GenRegister::h2(r);
      factor = 2;
    } else {
      r0 = GenRegister::h2(r);
    }
    push();
    curr.execWidth = 8;
    curr.predicate = GEN_PREDICATE_NONE;
    curr.noMask = 1;
    MOV(r0, src0);
    MOV(GenRegister::suboffset(r0, 4 * factor), GenRegister::suboffset(src0, 4));
    curr.noMask = 0;
    curr.quarterControl = 0;
    curr.nibControl = 0;
    MOV(dest, r);
    curr.nibControl = 1;
    MOV(GenRegister::suboffset(dest, 4), GenRegister::suboffset(r, 8 / factor));
    pop();
    if (w == 16) {
      push();
      curr.execWidth = 8;
      curr.predicate = GEN_PREDICATE_NONE;
      curr.noMask = 1;
      MOV(r0, GenRegister::suboffset(src0, 8));
      MOV(GenRegister::suboffset(r0, 4 * factor), GenRegister::suboffset(src0, 12));
      curr.noMask = 0;
      curr.quarterControl = 1;
      curr.nibControl = 0;
      MOV(GenRegister::suboffset(dest, 8), r);
      curr.nibControl = 1;
      MOV(GenRegister::suboffset(dest, 12), GenRegister::suboffset(r, 8 / factor));
      pop();
    }
  }

  void GenEncoder::F16TO32(GenRegister dest, GenRegister src0) {
    alu1(this, GEN_OPCODE_F16TO32, dest, src0);
  }

  void GenEncoder::F32TO16(GenRegister dest, GenRegister src0) {
    alu1(this, GEN_OPCODE_F32TO16, dest, src0);
  }

  ALU1(MOV)
  ALU1(RNDZ)
  ALU1(RNDE)
  ALU1(RNDD)
  ALU1(RNDU)
  ALU1(FBH)
  ALU1(FBL)
  ALU1(CBIT)
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
  ALU1(FRC)
  ALU2(MAC)
  ALU1(LZD)
  ALU2(LINE)
  ALU2(PLN)
  ALU2(MACH)
  ALU3(MAD)
 // ALU2(BRC)
 // ALU1(ENDIF)
 //  ALU1(IF)

  void GenEncoder::SUBB(GenRegister dest, GenRegister src0, GenRegister src1) {
    push();
    curr.accWrEnable = 1;
    alu2(this, GEN_OPCODE_SUBB, dest, src0, src1);
    pop();
  }

  void GenEncoder::ADDC(GenRegister dest, GenRegister src0, GenRegister src1) {
    push();
    curr.accWrEnable = 1;
    alu2(this, GEN_OPCODE_ADDC, dest, src0, src1);
    pop();
  }

  void GenEncoder::ADD(GenRegister dest, GenRegister src0, GenRegister src1) {
     if (src0.type == GEN_TYPE_F ||
         (src0.file == GEN_IMMEDIATE_VALUE &&
          src0.type == GEN_TYPE_VF)) {
        assert(src1.type != GEN_TYPE_UD);
        assert(src1.type != GEN_TYPE_D);
     }

     if (src1.type == GEN_TYPE_F ||
         (src1.file == GEN_IMMEDIATE_VALUE &&
          src1.type == GEN_TYPE_VF)) {
        assert(src0.type != GEN_TYPE_UD);
        assert(src0.type != GEN_TYPE_D);
     }

     alu2(this, GEN_OPCODE_ADD, dest, src0, src1);
  }

  void GenEncoder::MUL(GenRegister dest, GenRegister src0, GenRegister src1) {
     if (src0.type == GEN_TYPE_D ||
         src0.type == GEN_TYPE_UD ||
         src1.type == GEN_TYPE_D ||
         src1.type == GEN_TYPE_UD)
        assert(dest.type != GEN_TYPE_F);

     if (src0.type == GEN_TYPE_F ||
         (src0.file == GEN_IMMEDIATE_VALUE &&
          src0.type == GEN_TYPE_VF)) {
        assert(src1.type != GEN_TYPE_UD);
        assert(src1.type != GEN_TYPE_D);
     }

     if (src1.type == GEN_TYPE_F ||
         (src1.file == GEN_IMMEDIATE_VALUE &&
          src1.type == GEN_TYPE_VF)) {
        assert(src0.type != GEN_TYPE_UD);
        assert(src0.type != GEN_TYPE_D);
     }

     assert(src0.file != GEN_ARCHITECTURE_REGISTER_FILE ||
            src0.nr != GEN_ARF_ACCUMULATOR);
     assert(src1.file != GEN_ARCHITECTURE_REGISTER_FILE ||
            src1.nr != GEN_ARF_ACCUMULATOR);

     alu2(this, GEN_OPCODE_MUL, dest, src0, src1);
  }


  void GenEncoder::NOP(void) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_NOP);
    this->setDst(insn, GenRegister::retype(GenRegister::f4grf(0,0), GEN_TYPE_UD));
    this->setSrc0(insn, GenRegister::retype(GenRegister::f4grf(0,0), GEN_TYPE_UD));
    this->setSrc1(insn, GenRegister::immud(0x0));
  }

  void GenEncoder::BARRIER(GenRegister src) {
     GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
     this->setHeader(insn);
     this->setDst(insn, GenRegister::null());
     this->setSrc0(insn, src);
     setMessageDescriptor(insn, GEN_SFID_MESSAGE_GATEWAY, 1, 0);
     insn->bits3.msg_gateway.sub_function_id = GEN_BARRIER_MSG;
     insn->bits3.msg_gateway.notify = 0x1;
  }
  void GenEncoder::FENCE(GenRegister dst) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    this->setHeader(insn);
    this->setDst(insn, dst);
    this->setSrc0(insn, dst);
    setMessageDescriptor(insn, GEN_SFID_DATAPORT_DATA, 1, 1, 1);
    insn->bits3.gen7_memory_fence.msg_type = GEN_MEM_FENCE;
    insn->bits3.gen7_memory_fence.commit_enable = 0x1;
  }

  void GenEncoder::JMPI(GenRegister src, bool longjmp) {
    alu2(this, GEN_OPCODE_JMPI, GenRegister::ip(), GenRegister::ip(), src);
    if (longjmp)
      NOP();
  }

#define ALU2_BRA(OP) \
  void GenEncoder::OP(GenRegister src) { \
    alu2(this, GEN_OPCODE_##OP, GenRegister::nullud(), GenRegister::nullud(), src); \
  }

  ALU2_BRA(IF)
  ALU2_BRA(ELSE)
  ALU2_BRA(ENDIF)
  ALU2_BRA(WHILE)
  ALU2_BRA(BRD)
  ALU2_BRA(BRC)

  void GenEncoder::patchJMPI(uint32_t insnID, int32_t jip, int32_t uip) {
    GenNativeInstruction &insn = *(GenNativeInstruction *)&this->store[insnID];
    GBE_ASSERT(insnID < this->store.size());
    GBE_ASSERT(insn.header.opcode == GEN_OPCODE_JMPI ||
               insn.header.opcode == GEN_OPCODE_BRD  ||
               insn.header.opcode == GEN_OPCODE_ENDIF ||
               insn.header.opcode == GEN_OPCODE_IF ||
               insn.header.opcode == GEN_OPCODE_BRC ||
               insn.header.opcode == GEN_OPCODE_WHILE ||
               insn.header.opcode == GEN_OPCODE_ELSE);

    if( insn.header.opcode == GEN_OPCODE_WHILE ){
      // if this WHILE instruction jump back to an ELSE instruction,
      // need add distance to go to the next instruction.
      GenNativeInstruction & insn_else = *(GenNativeInstruction *)&this->store[insnID+jip];
      if(insn_else.header.opcode == GEN_OPCODE_ELSE){
        jip += 2;
      }
    }

    if (insn.header.opcode != GEN_OPCODE_JMPI || (jip > -32769 && jip < 32768))  {
      if (insn.header.opcode == GEN_OPCODE_IF) {
        this->setSrc1(&insn, GenRegister::immd((jip & 0xffff) | uip<<16));
        return;
      } else if (insn.header.opcode == GEN_OPCODE_JMPI) {
        jip = jip - 2;
      } else if(insn.header.opcode == GEN_OPCODE_ENDIF)
        jip += 2;
       this->setSrc1(&insn, GenRegister::immd((jip & 0xffff) | uip<<16));
    } else if ( insn.header.predicate_control == GEN_PREDICATE_NONE ) {
      // For the conditional jump distance out of S15 range, we need to use an
      // inverted jmp followed by a add ip, ip, distance to implement.
      // A little hacky as we need to change the nop instruction to add
      // instruction manually.
      // If this is a unconditional jump, we just need to add the IP directly.
      // FIXME there is an optimization method which we can insert a
      // ADD instruction on demand. But that will need some extra analysis
      // for all the branching instruction. And need to adjust the distance
      // for those branch instruction's start point and end point contains
      // this instruction.
      GBE_ASSERT(((GenNativeInstruction *)&this->store[insnID+2])->header.opcode == GEN_OPCODE_NOP);
      insn.header.opcode = GEN_OPCODE_ADD;
      this->setDst(&insn, GenRegister::ip());
      this->setSrc0(&insn, GenRegister::ip());
      this->setSrc1(&insn, GenRegister::immd(jip * 8));
    } else {
      GenNativeInstruction &insn2 = *(GenNativeInstruction *)&this->store[insnID+2];
      insn.header.predicate_inverse ^= 1;
      this->setSrc1(&insn, GenRegister::immd(2));
      GBE_ASSERT(insn2.header.opcode == GEN_OPCODE_NOP);
      GBE_ASSERT(insnID < this->store.size());
      insn2.header.predicate_control = GEN_PREDICATE_NONE;
      insn2.header.opcode = GEN_OPCODE_ADD;
      this->setDst(&insn2, GenRegister::ip());
      this->setSrc0(&insn2, GenRegister::ip());
      this->setSrc1(&insn2, GenRegister::immd((jip - 2) * 8));
    }
  }

  void GenEncoder::CMP(uint32_t conditional, GenRegister src0, GenRegister src1, GenRegister dst) {
    if (needToSplitCmp(this, src0, src1) == false) {
      if(!GenRegister::isNull(dst) && compactAlu2(this, GEN_OPCODE_CMP, dst, src0, src1, conditional, false)) {
        return;
      }
      GenNativeInstruction *insn = this->next(GEN_OPCODE_CMP);
      this->setHeader(insn);
      insn->header.destreg_or_condmod = conditional;
      if (GenRegister::isNull(dst))
        insn->header.thread_control = GEN_THREAD_SWITCH;
      this->setDst(insn, dst);
      this->setSrc0(insn, src0);
      this->setSrc1(insn, src1);
    } else {
      GenNativeInstruction *insnQ1, *insnQ2;

      // Instruction for the first quarter
      insnQ1 = this->next(GEN_OPCODE_CMP);
      this->setHeader(insnQ1);
      if (GenRegister::isNull(dst))
        insnQ1->header.thread_control = GEN_THREAD_SWITCH;
      insnQ1->header.quarter_control = GEN_COMPRESSION_Q1;
      insnQ1->header.execution_size = GEN_WIDTH_8;
      insnQ1->header.destreg_or_condmod = conditional;
      this->setDst(insnQ1, dst);
      this->setSrc0(insnQ1, src0);
      this->setSrc1(insnQ1, src1);

      // Instruction for the second quarter
      insnQ2 = this->next(GEN_OPCODE_CMP);
      this->setHeader(insnQ2);
      if (GenRegister::isNull(dst))
        insnQ2->header.thread_control = GEN_THREAD_SWITCH;
      insnQ2->header.quarter_control = GEN_COMPRESSION_Q2;
      insnQ2->header.execution_size = GEN_WIDTH_8;
      insnQ2->header.destreg_or_condmod = conditional;
      this->setDst(insnQ2, GenRegister::Qn(dst, 1));
      this->setSrc0(insnQ2, GenRegister::Qn(src0, 1));
      this->setSrc1(insnQ2, GenRegister::Qn(src1, 1));
    }
  }

  void GenEncoder::SEL_CMP(uint32_t conditional,
                           GenRegister dst,
                           GenRegister src0,
                           GenRegister src1)
  {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEL);
    GBE_ASSERT(curr.predicate == GEN_PREDICATE_NONE);
    this->setHeader(insn);
    insn->header.destreg_or_condmod = conditional;
    this->setDst(insn, dst);
    this->setSrc0(insn, src0);
    this->setSrc1(insn, src1);
  }

  void GenEncoder::WAIT(void) {
     GenNativeInstruction *insn = this->next(GEN_OPCODE_WAIT);
     GenRegister src = GenRegister::notification1();
     this->setDst(insn, GenRegister::null());
     this->setSrc0(insn, src);
     this->setSrc1(insn, GenRegister::null());
     insn->header.execution_size = 0; /* must */
     insn->header.predicate_control = 0;
     insn->header.quarter_control = 0;
  }

  void GenEncoder::MATH(GenRegister dst, uint32_t function, GenRegister src0, GenRegister src1) {
     GenNativeInstruction *insn = this->next(GEN_OPCODE_MATH);
     assert(dst.file == GEN_GENERAL_REGISTER_FILE);
     assert(src0.file == GEN_GENERAL_REGISTER_FILE);
     assert(src1.file == GEN_GENERAL_REGISTER_FILE);
     assert(dst.hstride == GEN_HORIZONTAL_STRIDE_1 || dst.hstride == GEN_HORIZONTAL_STRIDE_0);

     if (function == GEN_MATH_FUNCTION_INT_DIV_QUOTIENT ||
         function == GEN_MATH_FUNCTION_INT_DIV_REMAINDER ||
         function == GEN_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER) {
        assert(src0.type != GEN_TYPE_F);
        assert(src1.type != GEN_TYPE_F);
     } else {
        assert(src0.type == GEN_TYPE_F);
        assert(src1.type == GEN_TYPE_F);
     }

     insn->header.destreg_or_condmod = function;
     this->setHeader(insn);
     this->setDst(insn, dst);
     this->setSrc0(insn, src0);
     this->setSrc1(insn, src1);

     if (function == GEN_MATH_FUNCTION_INT_DIV_QUOTIENT ||
         function == GEN_MATH_FUNCTION_INT_DIV_REMAINDER) {
        insn->header.execution_size = this->curr.execWidth == 1 ? GEN_WIDTH_1 : GEN_WIDTH_8;
        insn->header.quarter_control = GEN_COMPRESSION_Q1;

        if(this->curr.execWidth == 16) {
          GenNativeInstruction *insn2 = this->next(GEN_OPCODE_MATH);
          GenRegister new_dest, new_src0, new_src1;
          new_dest = GenRegister::QnPhysical(dst, 1);
          new_src0 = GenRegister::QnPhysical(src0, 1);
          new_src1 = GenRegister::QnPhysical(src1, 1);
          insn2->header.destreg_or_condmod = function;
          this->setHeader(insn2);
          insn2->header.execution_size = GEN_WIDTH_8;
          insn2->header.quarter_control = GEN_COMPRESSION_Q2;
          this->setDst(insn2, new_dest);
          this->setSrc0(insn2, new_src0);
          this->setSrc1(insn2, new_src1);
        }

     }
  }

  void GenEncoder::MATH(GenRegister dst, uint32_t function, GenRegister src) {
     GenNativeInstruction *insn = this->next(GEN_OPCODE_MATH);
     assert(dst.file == GEN_GENERAL_REGISTER_FILE);
     assert(src.file == GEN_GENERAL_REGISTER_FILE);
     assert(dst.hstride == GEN_HORIZONTAL_STRIDE_1 || dst.hstride == GEN_HORIZONTAL_STRIDE_0);
     assert(src.type == GEN_TYPE_F);

     insn->header.destreg_or_condmod = function;
     this->setHeader(insn);
     this->setDst(insn, dst);
     this->setSrc0(insn, src);
  }

  void GenEncoder::SAMPLE(GenRegister dest,
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
      simd_mode = GEN_SAMPLER_SIMD_MODE_SIMD4X2;
    }
     GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
     this->setHeader(insn);
     this->setDst(insn, dest);
     this->setSrc0(insn, msg);
     setSamplerMessage(this, insn, bti, sampler, msg_type,
                       response_length, msg_length,
                       header_present,
                       simd_mode, return_format);
  }

  void GenEncoder::TYPED_WRITE(GenRegister msg, bool header_present, unsigned char bti)
  {
     GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
     uint32_t msg_type = GEN_TYPED_WRITE;
     uint32_t msg_length = header_present ? 9 : 8;
     this->setHeader(insn);
     this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UD));
     this->setSrc0(insn, msg);
     setTypedWriteMessage(insn, bti, msg_type, msg_length, header_present);
  }
  static void setScratchMessage(GenEncoder *p,
                                   GenNativeInstruction *insn,
                                   uint32_t offset,
                                   uint32_t block_size,
                                   uint32_t channel_mode,
                                   uint32_t msg_type,
                                   uint32_t msg_length,
                                   uint32_t response_length)
  {
     const GenMessageTarget sfid = GEN_SFID_DATAPORT_DATA;
     p->setMessageDescriptor(insn, sfid, msg_length, response_length, true);
     insn->bits3.gen7_scratch_rw.block_size = block_size;
     insn->bits3.gen7_scratch_rw.msg_type = msg_type;
     insn->bits3.gen7_scratch_rw.channel_mode = channel_mode;
     insn->bits3.gen7_scratch_rw.offset = offset;
     insn->bits3.gen7_scratch_rw.category = 1;
  }

  void GenEncoder::SCRATCH_WRITE(GenRegister msg, uint32_t offset, uint32_t size, uint32_t src_num, uint32_t channel_mode)
  {
     assert(src_num == 1 || src_num ==2);
     uint32_t block_size = src_num == 1 ? GEN_SCRATCH_BLOCK_SIZE_1 : GEN_SCRATCH_BLOCK_SIZE_2;
     GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
     this->setHeader(insn);
     this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UD));
     this->setSrc0(insn, msg);
     this->setSrc1(insn, GenRegister::immud(0));
     // here src_num means register that will be write out: in terms of 32byte register number
     setScratchMessage(this, insn, offset, block_size, channel_mode, GEN_SCRATCH_WRITE, src_num+1, 0);
  }

  void GenEncoder::SCRATCH_READ(GenRegister dst, GenRegister src, uint32_t offset, uint32_t size, uint32_t dst_num, uint32_t channel_mode)
  {
     assert(dst_num == 1 || dst_num ==2);
     uint32_t block_size = dst_num == 1 ? GEN_SCRATCH_BLOCK_SIZE_1 : GEN_SCRATCH_BLOCK_SIZE_2;
     GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
     this->setHeader(insn);
     this->setDst(insn, dst);
     this->setSrc0(insn, src);
     this->setSrc1(insn, GenRegister::immud(0));
      // here dst_num is the register that will be write-back: in terms of 32byte register
     setScratchMessage(this, insn, offset, block_size, channel_mode, GEN_SCRATCH_READ, 1, dst_num);
  }

  void GenEncoder::EOT(uint32_t msg) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UD));
    this->setSrc0(insn, GenRegister::ud8grf(msg,0));
    this->setSrc1(insn, GenRegister::immud(0));
    insn->header.execution_size = GEN_WIDTH_8;
    insn->bits3.spawner_gen5.resource = GEN_DO_NOT_DEREFERENCE_URB;
    insn->bits3.spawner_gen5.msg_length = 1;
    insn->bits3.spawner_gen5.end_of_thread = 1;
    insn->header.destreg_or_condmod = GEN_SFID_THREAD_SPAWNER;
  }
} /* namespace gbe */


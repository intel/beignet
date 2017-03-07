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

  INLINE bool isVectorOfLongs(GenRegister reg) {
    if (reg.hstride != GEN_HORIZONTAL_STRIDE_0 &&
        (reg.type == GEN_TYPE_UL || reg.type == GEN_TYPE_L))
      return true;
    else
      return false;
  }

  INLINE bool isCrossMoreThan2(GenRegister reg) {
    if (reg.hstride == GEN_HORIZONTAL_STRIDE_0)
      return false;

    const uint32_t typeSz = typeSize(reg.type);
    const uint32_t horizontal = stride(reg.hstride);
    if (horizontal * typeSz * 16 > GEN_REG_SIZE * 2) {
      return true;
    }
    return false;
  }

  INLINE bool isSrcDstDiffSpan(GenRegister dst, GenRegister src) {
    if (src.hstride == GEN_HORIZONTAL_STRIDE_0) return false;

    GBE_ASSERT(dst.hstride != GEN_HORIZONTAL_STRIDE_0 && "dst register is uniform but src is not.");

    uint32_t typeSz = typeSize(dst.type);
    uint32_t horizontal = stride(dst.hstride);
    uint32_t spans = (dst.subnr / (horizontal * typeSz)) * (horizontal * typeSz)  + horizontal * typeSz * 16;
    uint32_t dstSpan = spans / GEN_REG_SIZE;
    dstSpan = dstSpan + (spans % GEN_REG_SIZE == 0 ? 0 : 1);
    if (dstSpan < 2) return false;

    typeSz = typeSize(src.type);
    horizontal = stride(src.hstride);
    spans = (src.subnr / (horizontal * typeSz)) * (horizontal * typeSz)  + horizontal * typeSz * 16;
    uint32_t srcSpan = (horizontal * typeSz * 16) / GEN_REG_SIZE;
    srcSpan = srcSpan + (spans % GEN_REG_SIZE == 0 ? 0 : 1);

    GBE_ASSERT(srcSpan <= 2);
    GBE_ASSERT(dstSpan == 2);

    if (srcSpan == dstSpan) return false;

    /* Special case, dst is DW and src is w.
       the case:
       mov (16) r10.0<1>:d r12<8;8,1>:w
       is allowed. */
    if ((dst.type == GEN_TYPE_UD || dst.type == GEN_TYPE_D)
          && (src.type == GEN_TYPE_UW || src.type == GEN_TYPE_W)
          && dstSpan == 2 && srcSpan == 1
          && dst.subnr == 0 && src.subnr == 0) return false;

    return true;
  }

  INLINE bool needToSplitAlu1(GenEncoder *p, GenRegister dst, GenRegister src) {
    if (p->curr.execWidth != 16) return false;
    if (isVectorOfLongs(dst) == true) return true;
    if (isCrossMoreThan2(dst) == true) return true;

    if (src.hstride == GEN_HORIZONTAL_STRIDE_0) return false;

    if (isCrossMoreThan2(src) == true) return true;
    if (isVectorOfLongs(src) == true) return true;

    if (isSrcDstDiffSpan(dst, src) == true) return true;

    if (isVectorOfBytes(dst) == true &&
        ((isVectorOfBytes(src) == true && src.hstride == dst.hstride)
          || src.hstride == GEN_HORIZONTAL_STRIDE_0))
      return false;
    if (isVectorOfBytes(dst) == true) return true;
    if (isVectorOfBytes(src) == true) return true;
    return false;
  }

  INLINE bool needToSplitAlu2(GenEncoder *p, GenRegister dst, GenRegister src0, GenRegister src1) {
    if (p->curr.execWidth != 16) return false;
    if (isVectorOfLongs(dst) == true) return true;
    if (isCrossMoreThan2(dst) == true) return true;

    if (src0.hstride == GEN_HORIZONTAL_STRIDE_0 &&
		src1.hstride == GEN_HORIZONTAL_STRIDE_0)
      return false;

    if (isVectorOfLongs(src0) == true) return true;
    if (isVectorOfLongs(src1) == true) return true;
    if (isCrossMoreThan2(src0) == true) return true;
    if (isCrossMoreThan2(src1) == true) return true;

    if (isSrcDstDiffSpan(dst, src0) == true) return true;
    if (isSrcDstDiffSpan(dst, src1) == true) return true;

    if (isVectorOfBytes(dst) == true &&
        ((isVectorOfBytes(src0) == true && src0.hstride == dst.hstride) ||
         src0.hstride == GEN_HORIZONTAL_STRIDE_0) &&
        ((isVectorOfBytes(src1) == true && src1.hstride == dst.hstride) ||
         src1.hstride == GEN_HORIZONTAL_STRIDE_0))
      return false;
    if (isVectorOfBytes(dst) == true ) return true;
    if (isVectorOfBytes(src0) == true) return true;
    if (isVectorOfBytes(src1) == true) return true;
    return false;
  }

  INLINE bool needToSplitCmp(GenEncoder *p, GenRegister src0, GenRegister src1, GenRegister dst) {
    if (p->curr.execWidth != 16) return false;
    if (isVectorOfLongs(dst) == true) return true;
    if (isCrossMoreThan2(dst) == true) return true;

    if (src0.hstride == GEN_HORIZONTAL_STRIDE_0 &&
            src1.hstride == GEN_HORIZONTAL_STRIDE_0)
      return false;

    if (isVectorOfBytes(src0) == true) return true;
    if (isVectorOfBytes(src1) == true) return true;

    if (isVectorOfLongs(src0) == true) return true;
    if (isVectorOfLongs(src1) == true) return true;
    if (isCrossMoreThan2(src0) == true) return true;
    if (isCrossMoreThan2(src1) == true) return true;

    if (isSrcDstDiffSpan(dst, src0) == true) return true;
    if (isSrcDstDiffSpan(dst, src1) == true) return true;

    return p->needToSplitCmpBySrcType(p, src0, src1);
  }

  bool GenEncoder::needToSplitCmpBySrcType(GenEncoder *p, GenRegister src0, GenRegister src1) {
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

  void GenEncoder::setDPByteScatterGather(GenNativeInstruction *insn,
                                     uint32_t bti,
                                     uint32_t elem_size,
                                     uint32_t msg_type,
                                     uint32_t msg_length,
                                     uint32_t response_length)
  {
    const GenMessageTarget sfid = GEN_SFID_DATAPORT_DATA;
    setMessageDescriptor(insn, sfid, msg_length, response_length);
    insn->bits3.gen7_byte_rw.msg_type = msg_type;
    insn->bits3.gen7_byte_rw.bti = bti;
    insn->bits3.gen7_byte_rw.data_size = elem_size;
    if (curr.execWidth == 8)
      insn->bits3.gen7_byte_rw.simd_mode = GEN_BYTE_SCATTER_SIMD8;
    else if (curr.execWidth == 16)
      insn->bits3.gen7_byte_rw.simd_mode = GEN_BYTE_SCATTER_SIMD16;
    else
      NOT_SUPPORTED;
  }

  void GenEncoder::setOBlockRW(GenNativeInstruction *insn,
                               uint32_t bti,
                               uint32_t block_size,
                               uint32_t msg_type,
                               uint32_t msg_length,
                               uint32_t response_length)
  {
    const GenMessageTarget sfid = GEN_SFID_DATAPORT_DATA;
    setMessageDescriptor(insn, sfid, msg_length, response_length);
    insn->bits3.gen7_oblock_rw.msg_type = msg_type;
    insn->bits3.gen7_oblock_rw.bti = bti;
    insn->bits3.gen7_oblock_rw.block_size = block_size;
    insn->bits3.gen7_oblock_rw.header_present = 1;
  }

  uint32_t GenEncoder::getOBlockSize(uint32_t oword_size, bool low_half)
  {
    /* 000: 1 OWord, read into or written from the low 128 bits of the destination register.
     * 001: 1 OWord, read into or written from the high 128 bits of the destination register.
     * 010: 2 OWords
     * 011: 4 OWords
     * 100: 8 OWords */
    switch(oword_size)
    {
      case 1: return low_half ? 0 : 1;
      case 2: return 2;
      case 4: return 3;
      case 8: return 4;
      default: NOT_SUPPORTED;
    }
    return 0;
  }

  void GenEncoder::setMBlockRW(GenNativeInstruction *insn,
                               uint32_t bti,
                               uint32_t msg_type,
                               uint32_t msg_length,
                               uint32_t response_length)
  {
    const GenMessageTarget sfid = GEN_SFID_DATAPORT1_DATA;
    setMessageDescriptor(insn, sfid, msg_length, response_length);
    insn->bits3.gen7_mblock_rw.msg_type = msg_type;
    insn->bits3.gen7_mblock_rw.bti = bti;
    insn->bits3.gen7_mblock_rw.header_present = 1;
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
  unsigned GenEncoder::generateUntypedReadMessageDesc(unsigned bti, unsigned elemNum) {
    GenNativeInstruction insn;
    memset(&insn, 0, sizeof(GenNativeInstruction));
    return setUntypedReadMessageDesc(&insn, bti, elemNum);
  }

  unsigned GenEncoder::setUntypedReadMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemNum) {
    uint32_t msg_length = 0;
    uint32_t response_length = 0;
    if (this->curr.execWidth == 8) {
      msg_length = 1;
      response_length = elemNum;
    } else if (this->curr.execWidth == 16) {
      msg_length = 2;
      response_length = 2 * elemNum;
    } else
      NOT_IMPLEMENTED;
    setDPUntypedRW(insn,
                   bti,
                   untypedRWMask[elemNum],
                   GEN7_UNTYPED_READ,
                   msg_length,
                   response_length);
    return insn->bits3.ud;
  }

  void GenEncoder::UNTYPED_READ(GenRegister dst, GenRegister src, GenRegister bti, uint32_t elemNum) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    assert(elemNum >= 1 || elemNum <= 4);

    this->setHeader(insn);
    this->setDst(insn,  GenRegister::uw16grf(dst.nr, 0));
    this->setSrc0(insn, GenRegister::ud8grf(src.nr, 0));
    insn->header.destreg_or_condmod = GEN_SFID_DATAPORT_DATA;

    if (bti.file == GEN_IMMEDIATE_VALUE) {
      this->setSrc1(insn, GenRegister::immud(0));
      setUntypedReadMessageDesc(insn, bti.value.ud, elemNum);
    } else {
      this->setSrc1(insn, bti);
    }
  }

  unsigned GenEncoder::generateUntypedWriteMessageDesc(unsigned bti, unsigned elemNum) {
    GenNativeInstruction insn;
    memset(&insn, 0, sizeof(GenNativeInstruction));
    return setUntypedWriteMessageDesc(&insn, bti, elemNum);
  }

  unsigned GenEncoder::setUntypedWriteMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemNum) {
    uint32_t msg_length = 0;
    uint32_t response_length = 0;
    if (this->curr.execWidth == 8) {
      msg_length = 1 + elemNum;
    } else if (this->curr.execWidth == 16) {
      msg_length = 2 * (1 + elemNum);
    }
    else
      NOT_IMPLEMENTED;
    setDPUntypedRW(insn,
                   bti,
                   untypedRWMask[elemNum],
                   GEN7_UNTYPED_WRITE,
                   msg_length,
                   response_length);
    return insn->bits3.ud;
  }

  unsigned GenEncoder::generateUntypedWriteSendsMessageDesc(unsigned bti, unsigned elemNum) {
    GenNativeInstruction insn;
    memset(&insn, 0, sizeof(GenNativeInstruction));
    return setUntypedWriteSendsMessageDesc(&insn, bti, elemNum);
  }

  unsigned GenEncoder::setUntypedWriteSendsMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemNum)
  {
    assert(0);
    return 0;
  }

  void GenEncoder::UNTYPED_READA64(GenRegister dst, GenRegister src, uint32_t elemNum) {
    assert(0);
  }

  void GenEncoder::UNTYPED_WRITEA64(GenRegister src, uint32_t elemNum){
    assert(0);
  }

  void GenEncoder::ATOMICA64(GenRegister dst, uint32_t function, GenRegister src, GenRegister bti, uint32_t srcNum) {
    assert(0);
  }

  void GenEncoder::UNTYPED_WRITE(GenRegister msg, GenRegister data, GenRegister bti, uint32_t elemNum, bool useSends) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    assert(elemNum >= 1 || elemNum <= 4);
    this->setHeader(insn);
    if (this->curr.execWidth == 8) {
      this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UD));
    } else if (this->curr.execWidth == 16) {
      this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UW));
    }
    else
      NOT_IMPLEMENTED;
    this->setSrc0(insn, GenRegister::ud8grf(msg.nr, 0));
    insn->header.destreg_or_condmod = GEN_SFID_DATAPORT_DATA;
    if (bti.file == GEN_IMMEDIATE_VALUE) {
      this->setSrc1(insn, GenRegister::immud(0));
      setUntypedWriteMessageDesc(insn, bti.value.ud, elemNum);
    } else {
      this->setSrc1(insn, bti);
    }
  }

  unsigned GenEncoder::generateByteGatherMessageDesc(unsigned bti, unsigned elemSize) {
    GenNativeInstruction insn;
    memset(&insn, 0, sizeof(GenNativeInstruction));
    return setByteGatherMessageDesc(&insn, bti, elemSize);
  }

  unsigned GenEncoder::setByteGatherMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemSize) {
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
    setDPByteScatterGather(insn,
                           bti,
                           elemSize,
                           GEN7_BYTE_GATHER,
                           msg_length,
                           response_length);
    return insn->bits3.ud;

  }

  void GenEncoder::BYTE_GATHER(GenRegister dst, GenRegister src, GenRegister bti, uint32_t elemSize) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    this->setHeader(insn);
    insn->header.destreg_or_condmod = GEN_SFID_DATAPORT_DATA;

    this->setDst(insn, GenRegister::ud8grf(dst.nr, 0));
    this->setSrc0(insn, GenRegister::ud8grf(src.nr, 0));

    if (bti.file == GEN_IMMEDIATE_VALUE) {
      this->setSrc1(insn, GenRegister::immud(0));
      setByteGatherMessageDesc(insn, bti.value.ud, elemSize);
    } else {
      this->setSrc1(insn, bti);
    }
  }

  unsigned GenEncoder::generateByteScatterMessageDesc(unsigned bti, unsigned elemSize) {
    GenNativeInstruction insn;
    memset(&insn, 0, sizeof(GenNativeInstruction));
    return setByteScatterMessageDesc(&insn, bti, elemSize);
  }

  unsigned GenEncoder::generateByteScatterSendsMessageDesc(unsigned bti, unsigned elemSize) {
    GenNativeInstruction insn;
    memset(&insn, 0, sizeof(GenNativeInstruction));
    return setByteScatterSendsMessageDesc(&insn, bti, elemSize);
  }

  unsigned GenEncoder::setByteScatterSendsMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemSize)
  {
    assert(0);
    return 0;
  }

  unsigned GenEncoder::setByteScatterMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemSize) {
    uint32_t msg_length = 0;
    uint32_t response_length = 0;
    if (this->curr.execWidth == 8) {
      msg_length = 2;
    } else if (this->curr.execWidth == 16) {
      msg_length = 4;
    } else
      NOT_IMPLEMENTED;

    setDPByteScatterGather(insn,
                           bti,
                           elemSize,
                           GEN7_BYTE_SCATTER,
                           msg_length,
                           response_length);
    return insn->bits3.ud;
  }

  void GenEncoder::BYTE_SCATTER(GenRegister msg, GenRegister data, GenRegister bti, uint32_t elemSize, bool useSends) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);

    this->setHeader(insn);
    insn->header.destreg_or_condmod = GEN_SFID_DATAPORT_DATA;

    if (this->curr.execWidth == 8) {
      this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UD));
    } else if (this->curr.execWidth == 16) {
      this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UW));
    } else
      NOT_IMPLEMENTED;

    this->setSrc0(insn, GenRegister::ud8grf(msg.nr, 0));

    if (bti.file == GEN_IMMEDIATE_VALUE) {
      this->setSrc1(insn, GenRegister::immud(0));
      setByteScatterMessageDesc(insn, bti.value.ud, elemSize);
    } else {
      this->setSrc1(insn, bti);
    }
  }
  void GenEncoder::BYTE_GATHERA64(GenRegister dst, GenRegister src, uint32_t elemSize) {
    assert(0);
  }

  void GenEncoder::BYTE_SCATTERA64(GenRegister src, uint32_t elemSize){
    assert(0);
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

  unsigned GenEncoder::generateAtomicMessageDesc(unsigned function, unsigned bti, unsigned srcNum) {
    GenNativeInstruction insn;
    memset(&insn, 0, sizeof(GenNativeInstruction));
    return setAtomicMessageDesc(&insn, function, bti, srcNum);
  }

  unsigned GenEncoder::setAtomicMessageDesc(GenNativeInstruction *insn, unsigned function, unsigned bti, unsigned srcNum) {
    uint32_t msg_length = 0;
    uint32_t response_length = 0;

    if (this->curr.execWidth == 8) {
      msg_length = srcNum;
      response_length = 1;
    } else if (this->curr.execWidth == 16) {
      msg_length = 2 * srcNum;
      response_length = 2;
    } else
      NOT_IMPLEMENTED;

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
    return insn->bits3.ud;
  }
  unsigned GenEncoder::setAtomicA64MessageDesc(GenNativeInstruction *insn, unsigned function, unsigned bti, unsigned srcNum, int type_long) {
    GBE_ASSERT(0);
    return 0;
  }

  void GenEncoder::ATOMIC(GenRegister dst, uint32_t function, GenRegister addr, GenRegister data, GenRegister bti, uint32_t srcNum, bool useSends) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);

    this->setHeader(insn);
    insn->header.destreg_or_condmod = GEN_SFID_DATAPORT_DATA;

    this->setDst(insn, GenRegister::uw16grf(dst.nr, 0));
    this->setSrc0(insn, GenRegister::ud8grf(addr.nr, 0));
    if (bti.file == GEN_IMMEDIATE_VALUE) {
      this->setSrc1(insn, GenRegister::immud(0));
      setAtomicMessageDesc(insn, function, bti.value.ud, srcNum);
    } else {
      this->setSrc1(insn, bti);
    }
  }

  extern bool OCL_DEBUGINFO; // first defined by calling BVAR in program.cpp
  void GenEncoder::setDBGInfo(DebugInfo in, bool hasHigh)
  {
    if(OCL_DEBUGINFO)
    {
      storedbg.push_back(in);
      if(hasHigh) storedbg.push_back(in);
    }
  }
  
  GenCompactInstruction *GenEncoder::nextCompact(uint32_t opcode) {
    GenCompactInstruction insn;
    std::memset(&insn, 0, sizeof(GenCompactInstruction));
    insn.bits1.opcode = opcode;
    this->store.push_back(insn.low);
    setDBGInfo(DBGInfo, false);
    return (GenCompactInstruction *)&this->store.back();
  }

  GenNativeInstruction *GenEncoder::next(uint32_t opcode) {
     GenNativeInstruction insn;
     std::memset(&insn, 0, sizeof(GenNativeInstruction));
     insn.header.opcode = opcode;
     this->store.push_back(insn.low);
     this->store.push_back(insn.high);
     setDBGInfo(DBGInfo, true);
     return (GenNativeInstruction *)(&this->store.back()-1);
  }

  bool GenEncoder::canHandleLong(uint32_t opcode, GenRegister dst, GenRegister src0, GenRegister src1)
  {
    /* By now, just alu1 insn will come to here. So just MOV */
    this->MOV(dst.bottom_half(), src0.bottom_half());
    this->MOV(dst.top_half(this->simdWidth), src0.top_half(this->simdWidth));
    return true;
  }

  void GenEncoder::handleDouble(GenEncoder *p, uint32_t opcode, GenRegister dst, GenRegister src0, GenRegister src1) {
    /* For platform before gen8, we do not support double and can not get here. */
    GBE_ASSERT(0);
  }

  void alu1(GenEncoder *p, uint32_t opcode, GenRegister dst,
            GenRegister src, uint32_t condition) {
     if (dst.isdf() && src.isdf()) {
       p->handleDouble(p, opcode, dst, src);
     } else if (dst.isint64() && src.isint64()
                && p->canHandleLong(opcode, dst, src)) { // handle int64
       return;
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
       p->handleDouble(p, opcode, dst, src0, src1);
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

  void GenEncoder::LOAD_INT64_IMM(GenRegister dest, GenRegister value) {
    GenRegister u0 = GenRegister::immd((int)value.value.i64), u1 = GenRegister::immd(value.value.i64 >> 32);
    MOV(dest.bottom_half(), u0);
    MOV(dest.top_half(this->simdWidth), u1);
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
  ALU3(LRP)
  ALU1(BFREV)
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
     this->setSrc1(insn, GenRegister::immud(0));
     setMessageDescriptor(insn, GEN_SFID_MESSAGE_GATEWAY, 1, 0);
     insn->bits3.msg_gateway.sub_function_id = GEN_BARRIER_MSG;
     insn->bits3.msg_gateway.notify = 0x1;
  }

  void GenEncoder::FWD_GATEWAY_MSG(GenRegister src, uint32_t notifyN) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    this->setHeader(insn);
    this->setDst(insn, GenRegister::null());
    this->setSrc0(insn, src);
    this->setSrc1(insn, GenRegister::immud(0));
    setMessageDescriptor(insn, GEN_SFID_MESSAGE_GATEWAY, 1, 0);
    insn->bits3.msg_gateway.sub_function_id = GEN_FORWARD_MSG;
    GBE_ASSERT(notifyN <= 2);
    insn->bits3.msg_gateway.notify = notifyN;
  }

  void GenEncoder::FENCE(GenRegister dst, bool flushRWCache) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    this->setHeader(insn);
    this->setDst(insn, dst);
    this->setSrc0(insn, dst);
    this->setSrc1(insn, GenRegister::immud(0));
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

  // jip is the distance between jump instruction and jump-target. we have handled
  // pre/post-increment in patchJMPI() function body
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
    if (needToSplitCmp(this, src0, src1, dst) == false) {
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

  void GenEncoder::WAIT(uint32_t n) {
     GenNativeInstruction *insn = this->next(GEN_OPCODE_WAIT);
     GBE_ASSERT(curr.predicate == GEN_PREDICATE_NONE);
     GenRegister src = GenRegister::notification0(n);
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

  void GenEncoder::setSamplerMessage(GenNativeInstruction *insn,
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
     setMessageDescriptor(insn, sfid, msg_length, response_length);
     insn->bits3.sampler_gen7.bti = bti;
     insn->bits3.sampler_gen7.sampler = sampler;
     insn->bits3.sampler_gen7.msg_type = msg_type;
     insn->bits3.sampler_gen7.simd_mode = simd_mode;
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
     this->setSrc1(insn, GenRegister::immud(0));
     setSamplerMessage(insn, bti, sampler, msg_type,
                       response_length, msg_length,
                       header_present,
                       simd_mode, return_format);
  }
  void GenEncoder::FLUSH_SAMPLERCACHE(GenRegister dst) {
    // only Gen8+ support flushing sampler cache
    assert(0);
  }

  void GenEncoder::setVmeMessage(GenNativeInstruction *insn,
                                unsigned char bti,
                                uint32_t response_length,
                                uint32_t msg_length,
                                uint32_t msg_type,
                                unsigned char vme_search_path_lut,
                                unsigned char lut_sub)
  {
     const GenMessageTarget sfid = GEN_SFID_VIDEO_MOTION_EST;
     setMessageDescriptor(insn, sfid, msg_length, response_length, true);
     insn->bits3.vme_gen7.bti = bti;
     insn->bits3.vme_gen7.vme_search_path_lut = vme_search_path_lut;
     insn->bits3.vme_gen7.lut_sub = lut_sub;
     insn->bits3.vme_gen7.msg_type = msg_type;
     insn->bits3.vme_gen7.stream_in = 0;
     insn->bits3.vme_gen7.stream_out = 0;
     insn->bits3.vme_gen7.reserved_mbz = 0;

  }

  void GenEncoder::VME(unsigned char bti,
                       GenRegister dest,
                       GenRegister msg,
                       uint32_t msg_type,
                       uint32_t vme_search_path_lut,
                       uint32_t lut_sub)
  {
    /* Currectly we just support inter search only, we will support other
     * modes in future.
     */
    GBE_ASSERT(msg_type == 1);
    uint32_t msg_length, response_length;
    if(msg_type == 1){
      msg_length = 5;
      response_length = 6;
    }
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    this->setHeader(insn);
    this->setDst(insn, dest);
    this->setSrc0(insn, msg);
    this->setSrc1(insn, GenRegister::immud(0));
    setVmeMessage(insn, bti, response_length, msg_length,
                  msg_type, vme_search_path_lut, lut_sub);
  }

  void GenEncoder::TYPED_WRITE(GenRegister msg, GenRegister data, bool header_present, unsigned char bti, bool useSends)
  {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    uint32_t msg_type = GEN_TYPED_WRITE;
    uint32_t msg_length = header_present ? 9 : 8;
    this->setHeader(insn);
    this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UD));
    this->setSrc0(insn, msg);
    this->setSrc1(insn, GenRegister::immud(0));
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

  void GenEncoder::OBREAD(GenRegister dst, GenRegister header, uint32_t bti, uint32_t ow_size) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    const uint32_t msg_length = 1;
    uint32_t sizeinreg = ow_size / 2;
    // half reg should also have size 1
    sizeinreg = sizeinreg == 0 ? 1 : sizeinreg;
    const uint32_t block_size = getOBlockSize(ow_size, dst.subnr == 0);
    const uint32_t response_length = sizeinreg; // Size is in reg

    this->setHeader(insn);
    this->setDst(insn, GenRegister::uw16grf(dst.nr, 0));
    this->setSrc0(insn, GenRegister::ud8grf(header.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));
    setOBlockRW(insn,
                bti,
                block_size,
                GEN7_UNALIGNED_OBLOCK_READ,
                msg_length,
                response_length);
  }

  void GenEncoder::OBWRITE(GenRegister header, GenRegister data, uint32_t bti, uint32_t ow_size, bool useSends) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    uint32_t sizeinreg = ow_size / 2;
    // half reg should also have size 1
    sizeinreg = sizeinreg == 0 ? 1 : sizeinreg;
    const uint32_t msg_length = 1 + sizeinreg; // Size is in reg and header
    const uint32_t response_length = 0;
    const uint32_t block_size = getOBlockSize(ow_size);

    this->setHeader(insn);
    this->setSrc0(insn, GenRegister::ud8grf(header.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));
    this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UW));
    setOBlockRW(insn,
                bti,
                block_size,
                GEN7_OBLOCK_WRITE,
                msg_length,
                response_length);
  }

  void GenEncoder::MBREAD(GenRegister dst, GenRegister header, uint32_t bti, uint32_t response_size) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    const uint32_t msg_length = 1;
    const uint32_t response_length = response_size; // Size of registers
    this->setHeader(insn);
    this->setDst(insn, GenRegister::ud8grf(dst.nr, 0));
    this->setSrc0(insn, GenRegister::ud8grf(header.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));
    setMBlockRW(insn,
                bti,
                GEN75_P1_MEDIA_BREAD,
                msg_length,
                response_length);
  }

  void GenEncoder::MBWRITE(GenRegister header, GenRegister data, uint32_t bti, uint32_t data_size, bool useSends) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    const uint32_t msg_length = 1 + data_size;
    const uint32_t response_length = 0; // Size of registers
    this->setHeader(insn);
    this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UW));
    this->setSrc0(insn, GenRegister::ud8grf(header.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));
    setMBlockRW(insn,
                bti,
                GEN75_P1_MEDIA_TYPED_BWRITE,
                msg_length,
                response_length);
  }

  void GenEncoder::OBREADA64(GenRegister dst, GenRegister header, uint32_t bti, uint32_t elemSize) {
    NOT_SUPPORTED;
  }

  void GenEncoder::OBWRITEA64(GenRegister header, uint32_t bti, uint32_t elemSize) {
    NOT_SUPPORTED;
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


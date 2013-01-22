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
    if (p->curr.execWidth != 16) return false;
    if (isVectorOfBytes(dst) == true) return true;
    if (isVectorOfBytes(src) == true) return true;
    return false;
  }

  INLINE bool needToSplitAlu2(GenEncoder *p, GenRegister dst, GenRegister src0, GenRegister src1) {
    if (p->curr.execWidth != 16) return false;
    if (isVectorOfBytes(dst) == true) return true;
    if (isVectorOfBytes(src0) == true) return true;
    if (isVectorOfBytes(src1) == true) return true;
    return false;
  }

  INLINE bool needToSplitCmp(GenEncoder *p, GenRegister src0, GenRegister src1) {
    if (p->curr.execWidth != 16) return false;
    if (isVectorOfBytes(src0) == true) return true;
    if (isVectorOfBytes(src1) == true) return true;
    if (src0.type == GEN_TYPE_D || src0.type == GEN_TYPE_UD || src0.type == GEN_TYPE_F)
      return true;
    if (src1.type == GEN_TYPE_D || src1.type == GEN_TYPE_UD || src1.type == GEN_TYPE_F)
      return true;
    return false;
  }

  static void setMessageDescriptor(GenEncoder *p,
                                   GenInstruction *inst,
                                   enum GenMessageTarget sfid,
                                   unsigned msg_length,
                                   unsigned response_length,
                                   bool header_present = false,
                                   bool end_of_thread = false)
  {
     p->setSrc1(inst, GenRegister::immd(0));
     inst->bits3.generic_gen5.header_present = header_present;
     inst->bits3.generic_gen5.response_length = response_length;
     inst->bits3.generic_gen5.msg_length = msg_length;
     inst->bits3.generic_gen5.end_of_thread = end_of_thread;
     inst->header.destreg_or_condmod = sfid;
  }

  static void setDPUntypedRW(GenEncoder *p,
                             GenInstruction *insn,
                             uint32_t bti,
                             uint32_t rgba,
                             uint32_t msg_type,
                             uint32_t msg_length,
                             uint32_t response_length)
  {
    const GenMessageTarget sfid = GEN_SFID_DATAPORT_DATA_CACHE;
    setMessageDescriptor(p, insn, sfid, msg_length, response_length);
    insn->bits3.gen7_untyped_rw.msg_type = msg_type;
    insn->bits3.gen7_untyped_rw.bti = bti;
    insn->bits3.gen7_untyped_rw.rgba = rgba;
    if (p->curr.execWidth == 8)
      insn->bits3.gen7_untyped_rw.simd_mode = GEN_UNTYPED_SIMD8;
    else if (p->curr.execWidth == 16)
      insn->bits3.gen7_untyped_rw.simd_mode = GEN_UNTYPED_SIMD16;
    else
      NOT_SUPPORTED;
  }

  static void setDPByteScatterGather(GenEncoder *p,
                                     GenInstruction *insn,
                                     uint32_t bti,
                                     uint32_t elem_size,
                                     uint32_t msg_type,
                                     uint32_t msg_length,
                                     uint32_t response_length)
  {
    const GenMessageTarget sfid = GEN_SFID_DATAPORT_DATA_CACHE;
    setMessageDescriptor(p, insn, sfid, msg_length, response_length);
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
                          GenInstruction *insn,
                          uint32_t bti,
                          uint32_t size,
                          uint32_t msg_type,
                          uint32_t msg_length,
                          uint32_t response_length)
  {
    const GenMessageTarget sfid = GEN_SFID_DATAPORT_DATA_CACHE;
    setMessageDescriptor(p, insn, sfid, msg_length, response_length);
    assert(size == 2 || size == 4);
    insn->bits3.gen7_oblock_rw.msg_type = msg_type;
    insn->bits3.gen7_oblock_rw.bti = bti;
    insn->bits3.gen7_oblock_rw.block_size = size == 2 ? 2 : 3;
    insn->bits3.gen7_oblock_rw.header_present = 1;
  }
#endif

  //////////////////////////////////////////////////////////////////////////
  // Gen Emitter encoding class
  //////////////////////////////////////////////////////////////////////////
  GenEncoder::GenEncoder(uint32_t simdWidth, uint32_t gen) :
    stateNum(0), gen(gen)
  {
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

  void GenEncoder::setHeader(GenInstruction *insn) {
    if (this->curr.execWidth == 8)
      insn->header.execution_size = GEN_WIDTH_8;
    else if (this->curr.execWidth == 16)
      insn->header.execution_size = GEN_WIDTH_16;
    else if (this->curr.execWidth == 1)
      insn->header.execution_size = GEN_WIDTH_1;
    else
      NOT_IMPLEMENTED;
    insn->header.acc_wr_control = this->curr.accWrEnable;
    insn->header.quarter_control = this->curr.quarterControl;
    insn->header.mask_control = this->curr.noMask;
    insn->bits2.ia1.flag_reg_nr = this->curr.flag;
    insn->bits2.ia1.flag_sub_reg_nr = this->curr.subFlag;
    if (this->curr.predicate != GEN_PREDICATE_NONE) {
      insn->header.predicate_control = this->curr.predicate;
      insn->header.predicate_inverse = this->curr.inversePredicate;
    }
    insn->header.saturate = this->curr.saturate;
  }

  void GenEncoder::setDst(GenInstruction *insn, GenRegister dest) {
     if (dest.file != GEN_ARCHITECTURE_REGISTER_FILE)
        assert(dest.nr < 128);

     insn->bits1.da1.dest_reg_file = dest.file;
     insn->bits1.da1.dest_reg_type = dest.type;
     insn->bits1.da1.dest_address_mode = dest.address_mode;
     insn->bits1.da1.dest_reg_nr = dest.nr;
     insn->bits1.da1.dest_subreg_nr = dest.subnr;
     if (dest.hstride == GEN_HORIZONTAL_STRIDE_0)
       dest.hstride = GEN_HORIZONTAL_STRIDE_1;
     insn->bits1.da1.dest_horiz_stride = dest.hstride;
  }

  void GenEncoder::setSrc0(GenInstruction *insn, GenRegister reg) {
     if (reg.file != GEN_ARCHITECTURE_REGISTER_FILE)
        assert(reg.nr < 128);

     if (reg.address_mode == GEN_ADDRESS_DIRECT) {
       insn->bits1.da1.src0_reg_file = reg.file;
       insn->bits1.da1.src0_reg_type = reg.type;
       insn->bits2.da1.src0_abs = reg.absolute;
       insn->bits2.da1.src0_negate = reg.negation;
       insn->bits2.da1.src0_address_mode = reg.address_mode;

       if (reg.file == GEN_IMMEDIATE_VALUE) {
          insn->bits3.ud = reg.value.ud;

          /* Required to set some fields in src1 as well: */
          insn->bits1.da1.src1_reg_file = 0; /* arf */
          insn->bits1.da1.src1_reg_type = reg.type;
       }
       else {
         if (insn->header.access_mode == GEN_ALIGN_1) {
           insn->bits2.da1.src0_subreg_nr = reg.subnr;
           insn->bits2.da1.src0_reg_nr = reg.nr;
         } else {
           insn->bits2.da16.src0_subreg_nr = reg.subnr / 16;
           insn->bits2.da16.src0_reg_nr = reg.nr;
         }

         if (reg.width == GEN_WIDTH_1 &&
             insn->header.execution_size == GEN_WIDTH_1) {
           insn->bits2.da1.src0_horiz_stride = GEN_HORIZONTAL_STRIDE_0;
           insn->bits2.da1.src0_width = GEN_WIDTH_1;
           insn->bits2.da1.src0_vert_stride = GEN_VERTICAL_STRIDE_0;
         }
         else {
           insn->bits2.da1.src0_horiz_stride = reg.hstride;
           insn->bits2.da1.src0_width = reg.width;
           insn->bits2.da1.src0_vert_stride = reg.vstride;
         }
       }
    } else {
       insn->bits1.ia1.src0_reg_file = GEN_GENERAL_REGISTER_FILE;
       insn->bits1.ia1.src0_reg_type = reg.type;
       insn->bits2.ia1.src0_subreg_nr = 0;
       insn->bits2.ia1.src0_indirect_offset = 0;
       insn->bits2.ia1.src0_abs = 0;
       insn->bits2.ia1.src0_negate = 0;
       insn->bits2.ia1.src0_address_mode = reg.address_mode;
       insn->bits2.ia1.src0_horiz_stride = GEN_HORIZONTAL_STRIDE_0;
       insn->bits2.ia1.src0_width = GEN_WIDTH_1;
       insn->bits2.ia1.src0_vert_stride = GEN_VERTICAL_STRIDE_ONE_DIMENSIONAL;
    }
  }

  void GenEncoder::setSrc1(GenInstruction *insn, GenRegister reg) {
     assert(reg.nr < 128);

     insn->bits1.da1.src1_reg_file = reg.file;
     insn->bits1.da1.src1_reg_type = reg.type;
     insn->bits3.da1.src1_abs = reg.absolute;
     insn->bits3.da1.src1_negate = reg.negation;

     assert(insn->bits1.da1.src0_reg_file != GEN_IMMEDIATE_VALUE);

     if (reg.file == GEN_IMMEDIATE_VALUE)
       insn->bits3.ud = reg.value.ud;
     else {
       assert (reg.address_mode == GEN_ADDRESS_DIRECT);
       if (insn->header.access_mode == GEN_ALIGN_1) {
         insn->bits3.da1.src1_subreg_nr = reg.subnr;
         insn->bits3.da1.src1_reg_nr = reg.nr;
       } else {
         insn->bits3.da16.src1_subreg_nr = reg.subnr / 16;
         insn->bits3.da16.src1_reg_nr = reg.nr;
       }

       if (reg.width == GEN_WIDTH_1 && 
           insn->header.execution_size == GEN_WIDTH_1) {
         insn->bits3.da1.src1_horiz_stride = GEN_HORIZONTAL_STRIDE_0;
         insn->bits3.da1.src1_width = GEN_WIDTH_1;
         insn->bits3.da1.src1_vert_stride = GEN_VERTICAL_STRIDE_0;
       } else {
         insn->bits3.da1.src1_horiz_stride = reg.hstride;
         insn->bits3.da1.src1_width = reg.width;
         insn->bits3.da1.src1_vert_stride = reg.vstride;
       }
     }
  }

  static const uint32_t untypedRWMask[] = {
    GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE|GEN_UNTYPED_GREEN|GEN_UNTYPED_RED,
    GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE|GEN_UNTYPED_GREEN,
    GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE,
    GEN_UNTYPED_ALPHA,
    0
  };

  void GenEncoder::UNTYPED_READ(GenRegister dst, GenRegister src, uint32_t bti, uint32_t elemNum) {
    GenInstruction *insn = this->next(GEN_OPCODE_SEND);
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
    this->setDst(insn, GenRegister::uw16grf(dst.nr, 0));
    this->setSrc0(insn, GenRegister::ud8grf(src.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));
    setDPUntypedRW(this,
                   insn,
                   bti,
                   untypedRWMask[elemNum],
                   GEN_UNTYPED_READ,
                   msg_length,
                   response_length);
  }

  void GenEncoder::UNTYPED_WRITE(GenRegister msg, uint32_t bti, uint32_t elemNum) {
    GenInstruction *insn = this->next(GEN_OPCODE_SEND);
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
    setDPUntypedRW(this,
                   insn,
                   bti,
                   untypedRWMask[elemNum],
                   GEN_UNTYPED_WRITE,
                   msg_length,
                   response_length);
  }

  void GenEncoder::BYTE_GATHER(GenRegister dst, GenRegister src, uint32_t bti, uint32_t elemSize) {
    GenInstruction *insn = this->next(GEN_OPCODE_SEND);
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
                           GEN_BYTE_GATHER,
                           msg_length,
                           response_length);
  }

  void GenEncoder::BYTE_SCATTER(GenRegister msg, uint32_t bti, uint32_t elemSize) {
    GenInstruction *insn = this->next(GEN_OPCODE_SEND);
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
                           GEN_BYTE_SCATTER,
                           msg_length,
                           response_length);
  }

  GenInstruction *GenEncoder::next(uint32_t opcode) {
     GenInstruction insn;
     std::memset(&insn, 0, sizeof(GenInstruction));
     insn.header.opcode = opcode;
     this->store.push_back(insn);
     return &this->store.back();
  }

  INLINE void alu1(GenEncoder *p, uint32_t opcode, GenRegister dst, GenRegister src) {
     if (needToSplitAlu1(p, dst, src) == false) {
       GenInstruction *insn = p->next(opcode);
       p->setHeader(insn);
       p->setDst(insn, dst);
       p->setSrc0(insn, src);
     } else {
       GenInstruction *insnQ1, *insnQ2;

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

  INLINE void alu2(GenEncoder *p,
                   uint32_t opcode,
                   GenRegister dst,
                   GenRegister src0,
                   GenRegister src1)
  {
    if (needToSplitAlu2(p, dst, src0, src1) == false) {
       GenInstruction *insn = p->next(opcode);
       p->setHeader(insn);
       p->setDst(insn, dst);
       p->setSrc0(insn, src0);
       p->setSrc1(insn, src1);
    } else {
       GenInstruction *insnQ1, *insnQ2;

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

#define NO_SWIZZLE ((0<<0) | (1<<2) | (2<<4) | (3<<6))

  static GenInstruction *alu3(GenEncoder *p,
                              uint32_t opcode,
                              GenRegister dest,
                              GenRegister src0,
                              GenRegister src1,
                              GenRegister src2)
  {
     GenInstruction *insn = p->next(opcode);

     assert(dest.file == GEN_GENERAL_REGISTER_FILE);
     assert(dest.nr < 128);
     assert(dest.address_mode == GEN_ADDRESS_DIRECT);
     assert(dest.type = GEN_TYPE_F);
     insn->bits1.da3src.dest_reg_file = 0;
     insn->bits1.da3src.dest_reg_nr = dest.nr;
     insn->bits1.da3src.dest_subreg_nr = dest.subnr / 16;
     insn->bits1.da3src.dest_writemask = 0xf;
     p->setHeader(insn);
     insn->header.access_mode = GEN_ALIGN_16;
     insn->header.execution_size = GEN_WIDTH_8;

     assert(src0.file == GEN_GENERAL_REGISTER_FILE);
     assert(src0.address_mode == GEN_ADDRESS_DIRECT);
     assert(src0.nr < 128);
     assert(src0.type == GEN_TYPE_F);
     insn->bits2.da3src.src0_swizzle = NO_SWIZZLE;
     insn->bits2.da3src.src0_subreg_nr = src0.subnr / 4 ;
     insn->bits2.da3src.src0_reg_nr = src0.nr;
     insn->bits1.da3src.src0_abs = src0.absolute;
     insn->bits1.da3src.src0_negate = src0.negation;
     insn->bits2.da3src.src0_rep_ctrl = src0.vstride == GEN_VERTICAL_STRIDE_0;

     assert(src1.file == GEN_GENERAL_REGISTER_FILE);
     assert(src1.address_mode == GEN_ADDRESS_DIRECT);
     assert(src1.nr < 128);
     assert(src1.type == GEN_TYPE_F);
     insn->bits2.da3src.src1_swizzle = NO_SWIZZLE;
     insn->bits2.da3src.src1_subreg_nr_low = (src1.subnr / 4) & 0x3;
     insn->bits3.da3src.src1_subreg_nr_high = (src1.subnr / 4) >> 2;
     insn->bits2.da3src.src1_rep_ctrl = src1.vstride == GEN_VERTICAL_STRIDE_0;
     insn->bits3.da3src.src1_reg_nr = src1.nr;
     insn->bits1.da3src.src1_abs = src1.absolute;
     insn->bits1.da3src.src1_negate = src1.negation;

     assert(src2.file == GEN_GENERAL_REGISTER_FILE);
     assert(src2.address_mode == GEN_ADDRESS_DIRECT);
     assert(src2.nr < 128);
     assert(src2.type == GEN_TYPE_F);
     insn->bits3.da3src.src2_swizzle = NO_SWIZZLE;
     insn->bits3.da3src.src2_subreg_nr = src2.subnr / 4;
     insn->bits3.da3src.src2_rep_ctrl = src2.vstride == GEN_VERTICAL_STRIDE_0;
     insn->bits3.da3src.src2_reg_nr = src2.nr;
     insn->bits1.da3src.src2_abs = src2.absolute;
     insn->bits1.da3src.src2_negate = src2.negation;

     // Emit second half of the instruction
     if (p->curr.execWidth == 16) {
      GenInstruction q1Insn = *insn;
      insn = p->next(opcode);
      *insn = q1Insn;
      insn->header.quarter_control = GEN_COMPRESSION_Q2;
      insn->bits1.da3src.dest_reg_nr++;
      if (insn->bits2.da3src.src0_rep_ctrl == 0)
        insn->bits2.da3src.src0_reg_nr++;
      if (insn->bits2.da3src.src1_rep_ctrl == 0)
        insn->bits3.da3src.src1_reg_nr++;
      if (insn->bits3.da3src.src2_rep_ctrl == 0)
        insn->bits3.da3src.src2_reg_nr++;
     }

     return insn;
  }

#undef NO_SWIZZLE

#define ALU1(OP) \
  void GenEncoder::OP(GenRegister dest, GenRegister src0) { \
    alu1(this, GEN_OPCODE_##OP, dest, src0); \
  }

#define ALU2(OP) \
  void GenEncoder::OP(GenRegister dest, GenRegister src0, GenRegister src1) { \
    alu2(this, GEN_OPCODE_##OP, dest, src0, src1); \
  }

#define ALU3(OP) \
  void GenEncoder::OP(GenRegister dest, GenRegister src0, GenRegister src1, GenRegister src2) { \
    alu3(this, GEN_OPCODE_##OP, dest, src0, src1, src2); \
  }

  ALU1(MOV)
  ALU1(RNDZ)
  ALU1(RNDE)
  ALU1(RNDD)
  ALU1(RNDU)
  ALU2(SEL)
  ALU1(NOT)
  ALU2(AND)
  ALU2(OR)
  ALU2(XOR)
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
    GenInstruction *insn = this->next(GEN_OPCODE_NOP);
    this->setDst(insn, GenRegister::retype(GenRegister::f4grf(0,0), GEN_TYPE_UD));
    this->setSrc0(insn, GenRegister::retype(GenRegister::f4grf(0,0), GEN_TYPE_UD));
    this->setSrc1(insn, GenRegister::immud(0x0));
  }

  void GenEncoder::BARRIER(GenRegister src) {
     GenInstruction *insn = this->next(GEN_OPCODE_SEND);
     this->setHeader(insn);
     this->setDst(insn, GenRegister::null());
     this->setSrc0(insn, src);
     setMessageDescriptor(this, insn, GEN_SFID_MESSAGE_GATEWAY, 1, 0);
     insn->bits3.msg_gateway.sub_function_id = GEN_BARRIER_MSG;
     insn->bits3.msg_gateway.notify = 0x1;
  }

  void GenEncoder::JMPI(GenRegister src) {
    alu2(this, GEN_OPCODE_JMPI, GenRegister::ip(), GenRegister::ip(), src);
  }

  void GenEncoder::patchJMPI(uint32_t insnID, int32_t jumpDistance) {
    GenInstruction &insn = this->store[insnID];
    assert(insnID < this->store.size());
    assert(insn.header.opcode == GEN_OPCODE_JMPI);
    this->setSrc1(&insn, GenRegister::immd(jumpDistance));
  }

  void GenEncoder::CMP(uint32_t conditional, GenRegister src0, GenRegister src1) {
    if (needToSplitCmp(this, src0, src1) == false) {
      GenInstruction *insn = this->next(GEN_OPCODE_CMP);
      this->setHeader(insn);
      insn->header.destreg_or_condmod = conditional;
      this->setDst(insn, GenRegister::null());
      this->setSrc0(insn, src0);
      this->setSrc1(insn, src1);
    } else {
      GenInstruction *insnQ1, *insnQ2;

      // Instruction for the first quarter
      insnQ1 = this->next(GEN_OPCODE_CMP);
      this->setHeader(insnQ1);
      insnQ1->header.quarter_control = GEN_COMPRESSION_Q1;
      insnQ1->header.execution_size = GEN_WIDTH_8;
      insnQ1->header.destreg_or_condmod = conditional;
      this->setDst(insnQ1, GenRegister::null());
      this->setSrc0(insnQ1, src0);
      this->setSrc1(insnQ1, src1);

      // Instruction for the second quarter
      insnQ2 = this->next(GEN_OPCODE_CMP);
      this->setHeader(insnQ2);
      insnQ2->header.quarter_control = GEN_COMPRESSION_Q2;
      insnQ2->header.execution_size = GEN_WIDTH_8;
      insnQ2->header.destreg_or_condmod = conditional;
      this->setDst(insnQ2, GenRegister::null());
      this->setSrc0(insnQ2, GenRegister::Qn(src0, 1));
      this->setSrc1(insnQ2, GenRegister::Qn(src1, 1));
    }
  }

  void GenEncoder::SEL_CMP(uint32_t conditional,
                           GenRegister dst,
                           GenRegister src0,
                           GenRegister src1)
  {
    GenInstruction *insn = this->next(GEN_OPCODE_SEL);
    GBE_ASSERT(curr.predicate == GEN_PREDICATE_NONE);
    this->setHeader(insn);
    insn->header.destreg_or_condmod = conditional;
    this->setDst(insn, dst);
    this->setSrc0(insn, src0);
    this->setSrc1(insn, src1);
  }

  void GenEncoder::WAIT(void) {
     GenInstruction *insn = this->next(GEN_OPCODE_WAIT);
     GenRegister src = GenRegister::notification1();
     this->setDst(insn, GenRegister::null());
     this->setSrc0(insn, src);
     this->setSrc1(insn, GenRegister::null());
     insn->header.execution_size = 0; /* must */
     insn->header.predicate_control = 0;
     insn->header.quarter_control = 0;
  }

  void GenEncoder::MATH(GenRegister dst, uint32_t function, GenRegister src0, GenRegister src1) {
     GenInstruction *insn = this->next(GEN_OPCODE_MATH);
     assert(dst.file == GEN_GENERAL_REGISTER_FILE);
     assert(src0.file == GEN_GENERAL_REGISTER_FILE);
     assert(src1.file == GEN_GENERAL_REGISTER_FILE);
     assert(dst.hstride == GEN_HORIZONTAL_STRIDE_1);

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
        assert(insn->header.execution_size == GEN_WIDTH_16);
        insn->header.execution_size = GEN_WIDTH_8;

        GenInstruction *insn2 = this->next(GEN_OPCODE_MATH);
        GenRegister new_dest, new_src0, new_src1;
        new_dest = GenRegister::QnPhysical(dst, 1);
        new_src0 = GenRegister::QnPhysical(src0, 1);
        new_src1 = GenRegister::QnPhysical(src1, 1);
        insn2->header.destreg_or_condmod = function;
        this->setHeader(insn2);
        insn2->header.execution_size = GEN_WIDTH_8;
        this->setDst(insn2, new_dest);
        this->setSrc0(insn2, new_src0);
        this->setSrc1(insn2, new_src1);
     }
  }

  void GenEncoder::MATH(GenRegister dst, uint32_t function, GenRegister src) {
     GenInstruction *insn = this->next(GEN_OPCODE_MATH);
     assert(dst.file == GEN_GENERAL_REGISTER_FILE);
     assert(src.file == GEN_GENERAL_REGISTER_FILE);
     assert(dst.hstride == GEN_HORIZONTAL_STRIDE_1);
     assert(src.type == GEN_TYPE_F);

     insn->header.destreg_or_condmod = function;
     this->setHeader(insn);
     this->setDst(insn, dst);
     this->setSrc0(insn, src);
  }

  void GenEncoder::SAMPLE(GenRegister dest,
                          GenRegister src0,
                          GenRegister src1,
                          uint32_t writemask,
                          uint32_t return_format)
  {
     if (writemask == 0) return;

     GenInstruction *insn = this->next(GEN_OPCODE_SEND);
     insn->header.predicate_control = 0; /* XXX */
     this->setHeader(insn);
     this->setDst(insn, dest);
     this->setSrc0(insn, src0);
     this->setSrc1(insn, src1);
     insn->header.destreg_or_condmod = GEN_SFID_SAMPLER;
  }

  void GenEncoder::TYPED_WRITE(GenRegister header, GenRegister desc)
  {
     GenInstruction *insn = this->next(GEN_OPCODE_SEND);
     insn->header.predicate_control = 0; /* XXX */
     this->setHeader(insn);
     this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UD));
     this->setSrc0(insn, header);
     this->setSrc1(insn, desc);
     insn->header.destreg_or_condmod = GEN6_SFID_DATAPORT_RENDER_CACHE;
  }

  void GenEncoder::EOT(uint32_t msg) {
    GenInstruction *insn = this->next(GEN_OPCODE_SEND);
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


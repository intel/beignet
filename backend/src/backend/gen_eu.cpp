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
  * Authors:
  *   Keith Whitwell <keith@tungstengraphics.com>
  */

#include "backend/gen_eu.hpp"
#include <cstring>

namespace gbe
{
  GenEmitter::GenEmitter(uint32_t simdWidth, uint32_t gen) :
    insnNum(0), stateNum(0), gen(gen)
  {
    this->curr.execWidth = simdWidth;
    this->curr.quarterControl = GEN_COMPRESSION_Q1;
    this->curr.noMask = 0;
  }

  void GenEmitter::setExecutionWidth(GenInstruction *insn) {
    if (this->curr.execWidth == 8)
      insn->header.execution_size = GEN_WIDTH_8;
    else if (this->curr.execWidth == 16)
      insn->header.execution_size = GEN_WIDTH_16;
    else
      GBE_ASSERT(0);
  }
  void GenEmitter::setQuarterControl(GenInstruction *insn) {
    insn->header.quarter_control = this->curr.quarterControl;
  }
  void GenEmitter::setNoMask(GenInstruction *insn) {
    insn->header.mask_control = this->curr.noMask;
  }
  void GenEmitter::setHeader(GenInstruction *insn) {
    this->setExecutionWidth(insn);
    this->setQuarterControl(insn);
    this->setNoMask(insn);
  }

  /* Returns the corresponding conditional mod for swapping src0 and
   * src1 in e.g. CMP.
   */
  uint32_t brw_swap_cmod(uint32_t cmod)
  {
     switch (cmod) {
     case GEN_CONDITIONAL_Z:
     case GEN_CONDITIONAL_NZ:
        return cmod;
     case GEN_CONDITIONAL_G:
        return GEN_CONDITIONAL_LE;
     case GEN_CONDITIONAL_GE:
        return GEN_CONDITIONAL_L;
     case GEN_CONDITIONAL_L:
        return GEN_CONDITIONAL_GE;
     case GEN_CONDITIONAL_LE:
        return GEN_CONDITIONAL_G;
     default:
        return ~0;
     }
  }

  void GenEmitter::setDst(GenInstruction *insn, GenReg dest)
  {
     if (dest.file != GEN_ARCHITECTURE_REGISTER_FILE)
        assert(dest.nr < 128);

     insn->bits1.da1.dest_reg_file = dest.file;
     insn->bits1.da1.dest_reg_type = dest.type;
     insn->bits1.da1.dest_address_mode = dest.address_mode;

     if (dest.address_mode == GEN_ADDRESS_DIRECT) {   
        insn->bits1.da1.dest_reg_nr = dest.nr;

        if (insn->header.access_mode == GEN_ALIGN_1) {
           insn->bits1.da1.dest_subreg_nr = dest.subnr;
           if (dest.hstride == GEN_HORIZONTAL_STRIDE_0)
              dest.hstride = GEN_HORIZONTAL_STRIDE_1;
           insn->bits1.da1.dest_horiz_stride = dest.hstride;
        }
        else {
           insn->bits1.da16.dest_subreg_nr = dest.subnr / 16;
           insn->bits1.da16.dest_writemask = dest.dw1.bits.writemask;
           /* even ignored in da16, still need to set as '01' */
           insn->bits1.da16.dest_horiz_stride = 1;
        }
     }
     else {
        insn->bits1.ia1.dest_subreg_nr = dest.subnr;

        /* These are different sizes in align1 vs align16:
         */
        if (insn->header.access_mode == GEN_ALIGN_1) {
           insn->bits1.ia1.dest_indirect_offset = dest.dw1.bits.indirect_offset;
           if (dest.hstride == GEN_HORIZONTAL_STRIDE_0)
              dest.hstride = GEN_HORIZONTAL_STRIDE_1;
           insn->bits1.ia1.dest_horiz_stride = dest.hstride;
        }
        else {
           insn->bits1.ia16.dest_indirect_offset = dest.dw1.bits.indirect_offset;
           /* even ignored in da16, still need to set as '01' */
           insn->bits1.ia16.dest_horiz_stride = 1;
        }
     }
  }

  static const int reg_type_size[8] = { 4, 4, 2, 2, 1, 1, 4 };

  void GenEmitter::setSrc0(GenInstruction *insn, GenReg reg)
  {
     if (reg.type != GEN_ARCHITECTURE_REGISTER_FILE)
        assert(reg.nr < 128);

     insn->bits1.da1.src0_reg_file = reg.file;
     insn->bits1.da1.src0_reg_type = reg.type;
     insn->bits2.da1.src0_abs = reg.absolute;
     insn->bits2.da1.src0_negate = reg.negation;
     insn->bits2.da1.src0_address_mode = reg.address_mode;

     if (reg.file == GEN_IMMEDIATE_VALUE) {
        insn->bits3.ud = reg.dw1.ud;

        /* Required to set some fields in src1 as well: */
        insn->bits1.da1.src1_reg_file = 0; /* arf */
        insn->bits1.da1.src1_reg_type = reg.type;
     }
     else {
        if (reg.address_mode == GEN_ADDRESS_DIRECT) {
           if (insn->header.access_mode == GEN_ALIGN_1) {
              insn->bits2.da1.src0_subreg_nr = reg.subnr;
              insn->bits2.da1.src0_reg_nr = reg.nr;
           }
           else {
              insn->bits2.da16.src0_subreg_nr = reg.subnr / 16;
              insn->bits2.da16.src0_reg_nr = reg.nr;
           }
        }
        else {
           insn->bits2.ia1.src0_subreg_nr = reg.subnr;

           if (insn->header.access_mode == GEN_ALIGN_1) {
              insn->bits2.ia1.src0_indirect_offset = reg.dw1.bits.indirect_offset; 
           }
           else {
              insn->bits2.ia16.src0_subreg_nr = reg.dw1.bits.indirect_offset;
           }
        }

        if (insn->header.access_mode == GEN_ALIGN_1) {
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
        else {
           insn->bits2.da16.src0_swz_x = GEN_GET_SWZ(reg.dw1.bits.swizzle, GEN_CHANNEL_X);
           insn->bits2.da16.src0_swz_y = GEN_GET_SWZ(reg.dw1.bits.swizzle, GEN_CHANNEL_Y);
           insn->bits2.da16.src0_swz_z = GEN_GET_SWZ(reg.dw1.bits.swizzle, GEN_CHANNEL_Z);
           insn->bits2.da16.src0_swz_w = GEN_GET_SWZ(reg.dw1.bits.swizzle, GEN_CHANNEL_W);

           /* This is an oddity of the fact we're using the same
            * descriptions for registers in align_16 as align_1:
            */
           if (reg.vstride == GEN_VERTICAL_STRIDE_8)
              insn->bits2.da16.src0_vert_stride = GEN_VERTICAL_STRIDE_4;
           else
              insn->bits2.da16.src0_vert_stride = reg.vstride;
        }
     }
  }


  void GenEmitter::setSrc1(GenInstruction *insn, GenReg reg)
  {
     assert(reg.nr < 128);

     insn->bits1.da1.src1_reg_file = reg.file;
     insn->bits1.da1.src1_reg_type = reg.type;
     insn->bits3.da1.src1_abs = reg.absolute;
     insn->bits3.da1.src1_negate = reg.negation;

     /* Only src1 can be immediate in two-argument instructions.
      */
     assert(insn->bits1.da1.src0_reg_file != GEN_IMMEDIATE_VALUE);

     if (reg.file == GEN_IMMEDIATE_VALUE) {
        insn->bits3.ud = reg.dw1.ud;
     }
     else {
        /* This is a hardware restriction, which may or may not be lifted
         * in the future:
         */
        assert (reg.address_mode == GEN_ADDRESS_DIRECT);
        /* assert (reg.file == GEN_GENERAL_REGISTER_FILE); */

        if (insn->header.access_mode == GEN_ALIGN_1) {
           insn->bits3.da1.src1_subreg_nr = reg.subnr;
           insn->bits3.da1.src1_reg_nr = reg.nr;
        }
        else {
           insn->bits3.da16.src1_subreg_nr = reg.subnr / 16;
           insn->bits3.da16.src1_reg_nr = reg.nr;
        }

        if (insn->header.access_mode == GEN_ALIGN_1) {
           if (reg.width == GEN_WIDTH_1 && 
               insn->header.execution_size == GEN_WIDTH_1) {
              insn->bits3.da1.src1_horiz_stride = GEN_HORIZONTAL_STRIDE_0;
              insn->bits3.da1.src1_width = GEN_WIDTH_1;
              insn->bits3.da1.src1_vert_stride = GEN_VERTICAL_STRIDE_0;
           }
           else {
              insn->bits3.da1.src1_horiz_stride = reg.hstride;
              insn->bits3.da1.src1_width = reg.width;
              insn->bits3.da1.src1_vert_stride = reg.vstride;
           }
        }
        else {
           insn->bits3.da16.src1_swz_x = GEN_GET_SWZ(reg.dw1.bits.swizzle, GEN_CHANNEL_X);
           insn->bits3.da16.src1_swz_y = GEN_GET_SWZ(reg.dw1.bits.swizzle, GEN_CHANNEL_Y);
           insn->bits3.da16.src1_swz_z = GEN_GET_SWZ(reg.dw1.bits.swizzle, GEN_CHANNEL_Z);
           insn->bits3.da16.src1_swz_w = GEN_GET_SWZ(reg.dw1.bits.swizzle, GEN_CHANNEL_W);

           /* This is an oddity of the fact we're using the same
            * descriptions for registers in align_16 as align_1:
            */
           if (reg.vstride == GEN_VERTICAL_STRIDE_8)
              insn->bits3.da16.src1_vert_stride = GEN_VERTICAL_STRIDE_4;
           else
              insn->bits3.da16.src1_vert_stride = reg.vstride;
        }
     }
  }

  /**
   * Set the Message Descriptor and Extended Message Descriptor fields
   * for SEND messages.
   *
   * \note This zeroes out the Function Control bits, so it must be called
   *       \b before filling out any message-specific data.  Callers can
   *       choose not to fill in irrelevant bits; they will be zero.
   */
  static void
  brw_set_message_descriptor(GenEmitter *p,
                             GenInstruction *inst,
                             enum GenMessageTarget sfid,
                             unsigned msg_length,
                             unsigned response_length,
                             bool header_present = false,
                             bool end_of_thread = false)
  {
     p->setSrc1(inst, GenReg::immd(0));
     inst->bits3.generic_gen5.header_present = header_present;
     inst->bits3.generic_gen5.response_length = response_length;
     inst->bits3.generic_gen5.msg_length = msg_length;
     inst->bits3.generic_gen5.end_of_thread = end_of_thread;
     inst->header.destreg__conditionalmod = sfid;
  }

  void
  set_dp_untyped_rw(GenEmitter *p,
                    GenInstruction *insn,
                    uint32_t bti,
                    uint32_t rgba,
                    uint32_t msg_type,
                    uint32_t msg_length,
                    uint32_t response_length)
  {
     GenMessageTarget sfid = GEN_SFID_DATAPORT_DATA_CACHE;
     brw_set_message_descriptor(p, insn, sfid, msg_length, response_length);
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

  static const uint32_t untypedRWMask[] = {
    GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE|GEN_UNTYPED_GREEN|GEN_UNTYPED_RED,
    GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE|GEN_UNTYPED_GREEN,
    GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE,
    GEN_UNTYPED_ALPHA,
    0
  };

  void
  GenEmitter::UNTYPED_READ(GenReg dst, GenReg src, uint32_t bti, uint32_t elemNum)
  {
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
    this->setDst(insn, GenReg::uw16grf(dst.nr, 0));
    this->setSrc0(insn, GenReg::ud8grf(src.nr, 0));
    this->setSrc1(insn, GenReg::immud(0));
    set_dp_untyped_rw(this,
                      insn,
                      bti,
                      untypedRWMask[elemNum],
                      GEN_UNTYPED_READ,
                      msg_length,
                      response_length);
  }

  void
  GenEmitter::UNTYPED_WRITE(GenReg msg, uint32_t bti, uint32_t elemNum)
  {
    GenInstruction *insn = this->next(GEN_OPCODE_SEND);
    assert(elemNum >= 1 || elemNum <= 4);
    uint32_t msg_length = 0;
    uint32_t response_length = 0;
    if (this->curr.execWidth == 8)
      msg_length = 1+elemNum;
    else if (this->curr.execWidth == 16)
      msg_length = 2*(1+elemNum);
    else
      NOT_IMPLEMENTED;
    this->setHeader(insn);
    this->setDst(insn, GenReg::retype(GenReg::null(), GEN_TYPE_UW));
    this->setSrc0(insn, GenReg::ud8grf(msg.nr, 0));
    this->setSrc1(insn, GenReg::immud(0));
    set_dp_untyped_rw(this,
                      insn,
                      bti,
                      untypedRWMask[elemNum],
                      GEN_UNTYPED_WRITE,
                      msg_length,
                      response_length);
  }

  void set_sampler_message(GenEmitter *p,
                           GenInstruction *insn,
                           uint32_t bti,
                           uint32_t sampler,
                           uint32_t msg_type,
                           uint32_t response_length,
                           uint32_t msg_length,
                           uint32_t header_present,
                           uint32_t simd_mode,
                           uint32_t return_format)
  {
     brw_set_message_descriptor(p, insn, GEN_SFID_SAMPLER, msg_length,
                                response_length, header_present);
     insn->bits3.sampler_gen7.bti = bti;
     insn->bits3.sampler_gen7.sampler = sampler;
     insn->bits3.sampler_gen7.msg_type = msg_type;
     insn->bits3.sampler_gen7.simd_mode = simd_mode;
  }

  GenInstruction *GenEmitter::next(uint32_t opcode)
  {
     GenInstruction *insn;
     insn = &this->store[this->insnNum++];
     std::memset(insn, 0, sizeof(GenInstruction));
     insn->header.opcode = opcode;
     return insn;
  }

  static GenInstruction *brw_alu1(GenEmitter *p,
                                   uint32_t opcode,
                                   GenReg dest,
                                   GenReg src)
  {
     GenInstruction *insn = p->next(opcode);
     p->setHeader(insn);
     p->setDst(insn, dest);
     p->setSrc0(insn, src);
     return insn;
  }

  static GenInstruction *brw_alu2(GenEmitter *p,
                                   uint32_t opcode,
                                   GenReg dest,
                                   GenReg src0,
                                   GenReg src1)
  {
     GenInstruction *insn = p->next(opcode);
     p->setHeader(insn);
     p->setDst(insn, dest);
     p->setSrc0(insn, src0);
     p->setSrc1(insn, src1);
     return insn;
  }

  static int
  get_3src_subreg_nr(GenReg reg)
  {
     if (reg.vstride == GEN_VERTICAL_STRIDE_0) {
        assert(brw_is_single_value_swizzle(reg.dw1.bits.swizzle));
        return reg.subnr / 4 + GEN_GET_SWZ(reg.dw1.bits.swizzle, 0);
     } else
        return reg.subnr / 4;
  }

  static GenInstruction *brw_alu3(GenEmitter *p,
                                   uint32_t opcode,
                                   GenReg dest,
                                   GenReg src0,
                                   GenReg src1,
                                   GenReg src2)
  {
     GenInstruction *insn = p->next(opcode);

     assert(insn->header.access_mode == GEN_ALIGN_16);

     assert(dest.file == GEN_GENERAL_REGISTER_FILE);
     assert(dest.nr < 128);
     assert(dest.address_mode == GEN_ADDRESS_DIRECT);
     assert(dest.type = GEN_TYPE_F);
     insn->bits1.da3src.dest_reg_file = 0;
     insn->bits1.da3src.dest_reg_nr = dest.nr;
     insn->bits1.da3src.dest_subreg_nr = dest.subnr / 16;
     insn->bits1.da3src.dest_writemask = dest.dw1.bits.writemask;
     p->setHeader(insn);

     assert(src0.file == GEN_GENERAL_REGISTER_FILE);
     assert(src0.address_mode == GEN_ADDRESS_DIRECT);
     assert(src0.nr < 128);
     assert(src0.type == GEN_TYPE_F);
     insn->bits2.da3src.src0_swizzle = src0.dw1.bits.swizzle;
     insn->bits2.da3src.src0_subreg_nr = get_3src_subreg_nr(src0);
     insn->bits2.da3src.src0_reg_nr = src0.nr;
     insn->bits1.da3src.src0_abs = src0.absolute;
     insn->bits1.da3src.src0_negate = src0.negation;
     insn->bits2.da3src.src0_rep_ctrl = src0.vstride == GEN_VERTICAL_STRIDE_0;

     assert(src1.file == GEN_GENERAL_REGISTER_FILE);
     assert(src1.address_mode == GEN_ADDRESS_DIRECT);
     assert(src1.nr < 128);
     assert(src1.type == GEN_TYPE_F);
     insn->bits2.da3src.src1_swizzle = src1.dw1.bits.swizzle;
     insn->bits2.da3src.src1_subreg_nr_low = get_3src_subreg_nr(src1) & 0x3;
     insn->bits3.da3src.src1_subreg_nr_high = get_3src_subreg_nr(src1) >> 2;
     insn->bits2.da3src.src1_rep_ctrl = src1.vstride == GEN_VERTICAL_STRIDE_0;
     insn->bits3.da3src.src1_reg_nr = src1.nr;
     insn->bits1.da3src.src1_abs = src1.absolute;
     insn->bits1.da3src.src1_negate = src1.negation;

     assert(src2.file == GEN_GENERAL_REGISTER_FILE);
     assert(src2.address_mode == GEN_ADDRESS_DIRECT);
     assert(src2.nr < 128);
     assert(src2.type == GEN_TYPE_F);
     insn->bits3.da3src.src2_swizzle = src2.dw1.bits.swizzle;
     insn->bits3.da3src.src2_subreg_nr = get_3src_subreg_nr(src2);
     insn->bits3.da3src.src2_rep_ctrl = src2.vstride == GEN_VERTICAL_STRIDE_0;
     insn->bits3.da3src.src2_reg_nr = src2.nr;
     insn->bits1.da3src.src2_abs = src2.absolute;
     insn->bits1.da3src.src2_negate = src2.negation;

     return insn;
  }


#define ALU1(OP) \
  GenInstruction *GenEmitter::OP(GenReg dest, GenReg src0) \
  { \
     return brw_alu1(this, GEN_OPCODE_##OP, dest, src0); \
  }

#define ALU2(OP) \
  GenInstruction *GenEmitter::OP(GenReg dest, GenReg src0, GenReg src1) \
  { \
     return brw_alu2(this, GEN_OPCODE_##OP, dest, src0, src1); \
  }

#define ALU3(OP) \
  GenInstruction *GenEmitter::OP(GenReg dest, GenReg src0, GenReg src1, GenReg src2) \
  { \
     return brw_alu3(this, GEN_OPCODE_##OP, dest, src0, src1, src2); \
  }

  ALU1(MOV)
  ALU1(RNDZ)
  ALU1(RNDE)
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
  ALU1(RNDD)
  ALU2(MAC)
  ALU1(LZD)
  ALU2(LINE)
  ALU2(PLN)
  ALU3(MAD)

  GenInstruction *GenEmitter::MACH(GenReg dest, GenReg src0, GenReg src1)
  {
     GenInstruction *insn = brw_alu2(this, GEN_OPCODE_MACH, dest, src0, src1);
     insn->header.acc_wr_control = 1;
     return insn;
  }

  GenInstruction *GenEmitter::ADD(GenReg dest, GenReg src0, GenReg src1)
  {
     /* 6.2.2: add */
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

     return brw_alu2(this, GEN_OPCODE_ADD, dest, src0, src1);
  }

  GenInstruction *GenEmitter::MUL(GenReg dest, GenReg src0, GenReg src1)
  {
     /* 6.32.38: mul */
     if (src0.type == GEN_TYPE_D ||
         src0.type == GEN_TYPE_UD ||
         src1.type == GEN_TYPE_D ||
         src1.type == GEN_TYPE_UD) {
        assert(dest.type != GEN_TYPE_F);
     }

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

     return brw_alu2(this, GEN_OPCODE_MUL, dest, src0, src1);
  }


  void GenEmitter::NOP(void)
  {
    GenInstruction *insn = this->next(GEN_OPCODE_NOP);
    this->setDst(insn, GenReg::retype(GenReg::vec4grf(0,0), GEN_TYPE_UD));
    this->setSrc0(insn, GenReg::retype(GenReg::vec4grf(0,0), GEN_TYPE_UD));
    this->setSrc1(insn, GenReg::immud(0x0));
  }

  GenInstruction *GenEmitter::JMPI(GenReg dest, GenReg src0, GenReg src1)
  {
    GenInstruction *insn = brw_alu2(this, GEN_OPCODE_JMPI, dest, src0, src1);
    insn->header.execution_size = 1;
    insn->header.mask_control = GEN_MASK_DISABLE;
    return insn;
  }

  /* To integrate with the above, it makes sense that the comparison
   * instruction should populate the flag register.  It might be simpler
   * just to use the flag reg for most WM tasks?
   */
  void GenEmitter::CMP(GenReg dest, uint32_t conditional, GenReg src0, GenReg src1)
  {
    GenInstruction *insn = this->next(GEN_OPCODE_CMP);

    insn->header.destreg__conditionalmod = conditional;
    this->setHeader(insn);
    this->setDst(insn, dest);
    this->setSrc0(insn, src0);
    this->setSrc1(insn, src1);
#if 0

     /* Make it so that future instructions will use the computed flag
      * value until brw_set_predicate_control_flag_value() is called
      * again.  
      */
     if (dest.file == GEN_ARCHITECTURE_REGISTER_FILE &&
         dest.nr == 0) {
        this->current->header.predicate_control = GEN_PREDICATE_NORMAL;
        this->flag_value = 0xff;
     }
#endif
  }

  /* Issue 'wait' instruction for n1, host could program MMIO
     to wake up thread. */
  void GenEmitter::WAIT(void)
  {
     GenInstruction *insn = this->next(GEN_OPCODE_WAIT);
     GenReg src = GenReg::notification1();

     this->setDst(insn, src);
     this->setSrc0(insn, src);
     this->setSrc1(insn, GenReg::null());
     insn->header.execution_size = 0; /* must */
     insn->header.predicate_control = 0;
     insn->header.quarter_control = 0;
  }

  void GenEmitter::MATH(GenReg dest,
                        uint32_t function,
                        uint32_t saturate,
                        uint32_t msg_reg_nr,
                        GenReg src,
                        uint32_t data_type,
                        uint32_t precision)
  {
    GenInstruction *insn = this->next(GEN_OPCODE_MATH);

    assert(dest.file == GEN_GENERAL_REGISTER_FILE);
    assert(src.file == GEN_GENERAL_REGISTER_FILE);
    assert(dest.hstride == GEN_HORIZONTAL_STRIDE_1);

    if (function == GEN_MATH_FUNCTION_INT_DIV_QUOTIENT ||
        function == GEN_MATH_FUNCTION_INT_DIV_REMAINDER ||
        function == GEN_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER) {
      assert(src.type != GEN_TYPE_F);
    } else
      assert(src.type == GEN_TYPE_F);

    insn->header.destreg__conditionalmod = function;
    insn->header.saturate = saturate;
    this->setDst(insn, dest);
    this->setSrc0(insn, src);
    this->setSrc1(insn, GenReg::null());
  }


  void GenEmitter::MATH2(GenReg dest, uint32_t function, GenReg src0, GenReg src1)
  {
     GenInstruction *insn = this->next(GEN_OPCODE_MATH);

     assert(dest.file == GEN_GENERAL_REGISTER_FILE);
     assert(src0.file == GEN_GENERAL_REGISTER_FILE);
     assert(src1.file == GEN_GENERAL_REGISTER_FILE);
     assert(dest.hstride == GEN_HORIZONTAL_STRIDE_1);

     if (function == GEN_MATH_FUNCTION_INT_DIV_QUOTIENT ||
         function == GEN_MATH_FUNCTION_INT_DIV_REMAINDER ||
         function == GEN_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER) {
        assert(src0.type != GEN_TYPE_F);
        assert(src1.type != GEN_TYPE_F);
     } else {
        assert(src0.type == GEN_TYPE_F);
        assert(src1.type == GEN_TYPE_F);
     }

     insn->header.destreg__conditionalmod = function;
     this->setHeader(insn);
     this->setDst(insn, dest);
     this->setSrc0(insn, src0);
     this->setSrc1(insn, src1);
  }

  void GenEmitter::MATH16(GenReg dest,
                          uint32_t function,
                          uint32_t saturate,
                          uint32_t msg_reg_nr,
                          GenReg src,
                          uint32_t precision)
  {
     GenInstruction *insn;

     insn = this->next(GEN_OPCODE_MATH);

     /* Math is the same ISA format as other opcodes, except that CondModifier
      * becomes FC[3:0] and ThreadCtrl becomes FC[5:4].
      */
     insn->header.destreg__conditionalmod = function;
     insn->header.saturate = saturate;

     /* Source modifiers are ignored for extended math instructions. */
     assert(!src.negate);
     assert(!src.abs);

     this->setHeader(insn);
     this->setDst(insn, dest);
     this->setSrc0(insn, src);
     this->setSrc1(insn, GenReg::null());
  }

  /**
   * Texture sample instruction.
   * Note: the msg_type plus msg_length values determine exactly what kind
   * of sampling operation is performed.  See volume 4, page 161 of docs.
   */
  void GenEmitter::SAMPLE(GenReg dest,
                          uint32_t msg_reg_nr,
                          GenReg src0,
                          uint32_t bti,
                          uint32_t sampler,
                          uint32_t writemask,
                          uint32_t msg_type,
                          uint32_t response_length,
                          uint32_t msg_length,
                          uint32_t header_present,
                          uint32_t simd_mode,
                          uint32_t return_format)
  {
     if (writemask == 0) return;

     GenInstruction *insn;

     insn = this->next(GEN_OPCODE_SEND);
     insn->header.predicate_control = 0; /* XXX */
     this->setHeader(insn);
     this->setDst(insn, dest);
     this->setSrc0(insn, src0);
     set_sampler_message(this,
                         insn,
                         bti,
                         sampler,
                         msg_type,
                         response_length, 
                         msg_length,
                         header_present,
                         simd_mode,
                         return_format);
  }

  void GenEmitter::EOT(uint32_t msg_nr)
  {
    GenInstruction *insn = NULL;

    this->pushState();
    this->curr.execWidth = 8;
    insn = this->MOV(GenReg::vec8grf(msg_nr,0), GenReg::vec8grf(0,0));
    this->popState();
    insn->header.mask_control = GEN_MASK_DISABLE;
    insn = this->next(GEN_OPCODE_SEND);
    this->setDst(insn, GenReg::retype(GenReg::null(), GEN_TYPE_UD));
    this->setSrc0(insn, GenReg::ud8grf(msg_nr,0));
    this->setSrc1(insn, GenReg::immud(0));
    insn->header.execution_size = GEN_WIDTH_8;
    insn->bits3.spawner_gen5.resource = GEN_DO_NOT_DEREFERENCE_URB;
    insn->bits3.spawner_gen5.msg_length = 1;
    insn->bits3.spawner_gen5.end_of_thread = 1;
    insn->header.destreg__conditionalmod = GEN_SFID_THREAD_SPAWNER;
  }
} /* namespace gbe */


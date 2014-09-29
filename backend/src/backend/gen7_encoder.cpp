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

#include "backend/gen7_encoder.hpp"


namespace gbe
{
  void Gen7Encoder::setHeader(GenNativeInstruction *insn) {
    Gen7NativeInstruction *gen7_insn = &insn->gen7_insn;
    if (this->curr.execWidth == 8)
      gen7_insn->header.execution_size = GEN_WIDTH_8;
    else if (this->curr.execWidth == 16)
      gen7_insn->header.execution_size = GEN_WIDTH_16;
    else if (this->curr.execWidth == 4)
      gen7_insn->header.execution_size = GEN_WIDTH_4;
    else if (this->curr.execWidth == 1)
      gen7_insn->header.execution_size = GEN_WIDTH_1;
    else
      NOT_IMPLEMENTED;
    gen7_insn->header.acc_wr_control = this->curr.accWrEnable;
    gen7_insn->header.quarter_control = this->curr.quarterControl;
    gen7_insn->bits1.ia1.nib_ctrl = this->curr.nibControl;
    gen7_insn->header.mask_control = this->curr.noMask;
    gen7_insn->bits2.ia1.flag_reg_nr = this->curr.flag;
    gen7_insn->bits2.ia1.flag_sub_reg_nr = this->curr.subFlag;
    if (this->curr.predicate != GEN_PREDICATE_NONE) {
      gen7_insn->header.predicate_control = this->curr.predicate;
      gen7_insn->header.predicate_inverse = this->curr.inversePredicate;
    }
    gen7_insn->header.saturate = this->curr.saturate;
  }

  void Gen7Encoder::setDst(GenNativeInstruction *insn, GenRegister dest) {
    Gen7NativeInstruction *gen7_insn = &insn->gen7_insn;
    if (dest.file != GEN_ARCHITECTURE_REGISTER_FILE)
      assert(dest.nr < 128);

    gen7_insn->bits1.da1.dest_reg_file = dest.file;
    gen7_insn->bits1.da1.dest_reg_type = dest.type;
    gen7_insn->bits1.da1.dest_address_mode = dest.address_mode;
    gen7_insn->bits1.da1.dest_reg_nr = dest.nr;
    gen7_insn->bits1.da1.dest_subreg_nr = dest.subnr;
    if (dest.hstride == GEN_HORIZONTAL_STRIDE_0) {
      if (dest.type == GEN_TYPE_UB || dest.type == GEN_TYPE_B)
        dest.hstride = GEN_HORIZONTAL_STRIDE_4;
      else if (dest.type == GEN_TYPE_UW || dest.type == GEN_TYPE_W)
        dest.hstride = GEN_HORIZONTAL_STRIDE_2;
      else
        dest.hstride = GEN_HORIZONTAL_STRIDE_1;
    }
    gen7_insn->bits1.da1.dest_horiz_stride = dest.hstride;
  }

  void Gen7Encoder::setSrc0(GenNativeInstruction *insn, GenRegister reg) {
    Gen7NativeInstruction *gen7_insn = &insn->gen7_insn;
    if (reg.file != GEN_ARCHITECTURE_REGISTER_FILE)
      assert(reg.nr < 128);

    if (reg.address_mode == GEN_ADDRESS_DIRECT) {
      gen7_insn->bits1.da1.src0_reg_file = reg.file;
      gen7_insn->bits1.da1.src0_reg_type = reg.type;
      gen7_insn->bits2.da1.src0_abs = reg.absolute;
      gen7_insn->bits2.da1.src0_negate = reg.negation;
      gen7_insn->bits2.da1.src0_address_mode = reg.address_mode;
      if (reg.file == GEN_IMMEDIATE_VALUE) {
        gen7_insn->bits3.ud = reg.value.ud;

        /* Required to set some fields in src1 as well: */
        gen7_insn->bits1.da1.src1_reg_file = 0; /* arf */
        gen7_insn->bits1.da1.src1_reg_type = reg.type;
      }
      else {
        if (gen7_insn->header.access_mode == GEN_ALIGN_1) {
          gen7_insn->bits2.da1.src0_subreg_nr = reg.subnr;
          gen7_insn->bits2.da1.src0_reg_nr = reg.nr;
        } else {
          gen7_insn->bits2.da16.src0_subreg_nr = reg.subnr / 16;
          gen7_insn->bits2.da16.src0_reg_nr = reg.nr;
        }

        if (reg.width == GEN_WIDTH_1 &&
            gen7_insn->header.execution_size == GEN_WIDTH_1) {
          gen7_insn->bits2.da1.src0_horiz_stride = GEN_HORIZONTAL_STRIDE_0;
          gen7_insn->bits2.da1.src0_width = GEN_WIDTH_1;
          gen7_insn->bits2.da1.src0_vert_stride = GEN_VERTICAL_STRIDE_0;
        }
        else {
          gen7_insn->bits2.da1.src0_horiz_stride = reg.hstride;
          gen7_insn->bits2.da1.src0_width = reg.width;
          gen7_insn->bits2.da1.src0_vert_stride = reg.vstride;
        }
      }
    } else {
      gen7_insn->bits1.ia1.src0_reg_file = GEN_GENERAL_REGISTER_FILE;
      gen7_insn->bits1.ia1.src0_reg_type = reg.type;
      gen7_insn->bits2.ia1.src0_subreg_nr = 0;
      gen7_insn->bits2.ia1.src0_indirect_offset = 0;
      gen7_insn->bits2.ia1.src0_abs = 0;
      gen7_insn->bits2.ia1.src0_negate = 0;
      gen7_insn->bits2.ia1.src0_address_mode = reg.address_mode;
      gen7_insn->bits2.ia1.src0_horiz_stride = GEN_HORIZONTAL_STRIDE_0;
      gen7_insn->bits2.ia1.src0_width = GEN_WIDTH_1;
      gen7_insn->bits2.ia1.src0_vert_stride = GEN_VERTICAL_STRIDE_ONE_DIMENSIONAL;
    }
  }

  void Gen7Encoder::setSrc1(GenNativeInstruction *insn, GenRegister reg) {
    Gen7NativeInstruction *gen7_insn = &insn->gen7_insn;
    assert(reg.nr < 128);
    assert(reg.file != GEN_ARCHITECTURE_REGISTER_FILE || reg.nr == 0);

    gen7_insn->bits1.da1.src1_reg_file = reg.file;
    gen7_insn->bits1.da1.src1_reg_type = reg.type;
    gen7_insn->bits3.da1.src1_abs = reg.absolute;
    gen7_insn->bits3.da1.src1_negate = reg.negation;

    assert(gen7_insn->bits1.da1.src0_reg_file != GEN_IMMEDIATE_VALUE);

    if (reg.file == GEN_IMMEDIATE_VALUE)
      gen7_insn->bits3.ud = reg.value.ud;
    else {
      assert (reg.address_mode == GEN_ADDRESS_DIRECT);
      if (gen7_insn->header.access_mode == GEN_ALIGN_1) {
        gen7_insn->bits3.da1.src1_subreg_nr = reg.subnr;
        gen7_insn->bits3.da1.src1_reg_nr = reg.nr;
      } else {
        gen7_insn->bits3.da16.src1_subreg_nr = reg.subnr / 16;
        gen7_insn->bits3.da16.src1_reg_nr = reg.nr;
      }

      if (reg.width == GEN_WIDTH_1 &&
          gen7_insn->header.execution_size == GEN_WIDTH_1) {
        gen7_insn->bits3.da1.src1_horiz_stride = GEN_HORIZONTAL_STRIDE_0;
        gen7_insn->bits3.da1.src1_width = GEN_WIDTH_1;
        gen7_insn->bits3.da1.src1_vert_stride = GEN_VERTICAL_STRIDE_0;
      } else {
        gen7_insn->bits3.da1.src1_horiz_stride = reg.hstride;
        gen7_insn->bits3.da1.src1_width = reg.width;
        gen7_insn->bits3.da1.src1_vert_stride = reg.vstride;
      }
    }
  }

#define NO_SWIZZLE ((0<<0) | (1<<2) | (2<<4) | (3<<6))

  void Gen7Encoder::alu3(uint32_t opcode,
                              GenRegister dest,
                              GenRegister src0,
                              GenRegister src1,
                              GenRegister src2)
  {
     GenNativeInstruction *insn = this->next(opcode);
     Gen7NativeInstruction *gen7_insn = &insn->gen7_insn;

     assert(dest.file == GEN_GENERAL_REGISTER_FILE);
     assert(dest.nr < 128);
     assert(dest.address_mode == GEN_ADDRESS_DIRECT);
     assert(dest.type = GEN_TYPE_F);
     gen7_insn->bits1.da3src.dest_reg_file = 0;
     gen7_insn->bits1.da3src.dest_reg_nr = dest.nr;
     gen7_insn->bits1.da3src.dest_subreg_nr = dest.subnr / 16;
     gen7_insn->bits1.da3src.dest_writemask = 0xf;
     this->setHeader(insn);
     gen7_insn->header.access_mode = GEN_ALIGN_16;
     gen7_insn->header.execution_size = GEN_WIDTH_8;

     assert(src0.file == GEN_GENERAL_REGISTER_FILE);
     assert(src0.address_mode == GEN_ADDRESS_DIRECT);
     assert(src0.nr < 128);
     assert(src0.type == GEN_TYPE_F);
     gen7_insn->bits2.da3src.src0_swizzle = NO_SWIZZLE;
     gen7_insn->bits2.da3src.src0_subreg_nr = src0.subnr / 4 ;
     gen7_insn->bits2.da3src.src0_reg_nr = src0.nr;
     gen7_insn->bits1.da3src.src0_abs = src0.absolute;
     gen7_insn->bits1.da3src.src0_negate = src0.negation;
     gen7_insn->bits2.da3src.src0_rep_ctrl = src0.vstride == GEN_VERTICAL_STRIDE_0;

     assert(src1.file == GEN_GENERAL_REGISTER_FILE);
     assert(src1.address_mode == GEN_ADDRESS_DIRECT);
     assert(src1.nr < 128);
     assert(src1.type == GEN_TYPE_F);
     gen7_insn->bits2.da3src.src1_swizzle = NO_SWIZZLE;
     gen7_insn->bits2.da3src.src1_subreg_nr_low = (src1.subnr / 4) & 0x3;
     gen7_insn->bits3.da3src.src1_subreg_nr_high = (src1.subnr / 4) >> 2;
     gen7_insn->bits2.da3src.src1_rep_ctrl = src1.vstride == GEN_VERTICAL_STRIDE_0;
     gen7_insn->bits3.da3src.src1_reg_nr = src1.nr;
     gen7_insn->bits1.da3src.src1_abs = src1.absolute;
     gen7_insn->bits1.da3src.src1_negate = src1.negation;

     assert(src2.file == GEN_GENERAL_REGISTER_FILE);
     assert(src2.address_mode == GEN_ADDRESS_DIRECT);
     assert(src2.nr < 128);
     assert(src2.type == GEN_TYPE_F);
     gen7_insn->bits3.da3src.src2_swizzle = NO_SWIZZLE;
     gen7_insn->bits3.da3src.src2_subreg_nr = src2.subnr / 4;
     gen7_insn->bits3.da3src.src2_rep_ctrl = src2.vstride == GEN_VERTICAL_STRIDE_0;
     gen7_insn->bits3.da3src.src2_reg_nr = src2.nr;
     gen7_insn->bits1.da3src.src2_abs = src2.absolute;
     gen7_insn->bits1.da3src.src2_negate = src2.negation;

     // Emit second half of the instruction
     if (this->curr.execWidth == 16) {
      GenNativeInstruction q1Insn = *insn;
      insn = this->next(opcode);
      *insn = q1Insn;
      gen7_insn = &insn->gen7_insn;
      gen7_insn->header.quarter_control = GEN_COMPRESSION_Q2;
      gen7_insn->bits1.da3src.dest_reg_nr++;
      if (gen7_insn->bits2.da3src.src0_rep_ctrl == 0)
        gen7_insn->bits2.da3src.src0_reg_nr++;
      if (gen7_insn->bits2.da3src.src1_rep_ctrl == 0)
        gen7_insn->bits3.da3src.src1_reg_nr++;
      if (gen7_insn->bits3.da3src.src2_rep_ctrl == 0)
        gen7_insn->bits3.da3src.src2_reg_nr++;
     }
  }

#undef NO_SWIZZLE
}

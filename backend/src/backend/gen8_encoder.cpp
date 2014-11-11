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

#include "backend/gen8_encoder.hpp"

static const uint32_t untypedRWMask[] = {
  GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE|GEN_UNTYPED_GREEN|GEN_UNTYPED_RED,
  GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE|GEN_UNTYPED_GREEN,
  GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE,
  GEN_UNTYPED_ALPHA,
  0
};

namespace gbe
{
  void Gen8Encoder::setHeader(GenNativeInstruction *insn) {
    Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;
    if (this->curr.execWidth == 8)
      gen8_insn->header.execution_size = GEN_WIDTH_8;
    else if (this->curr.execWidth == 16)
      gen8_insn->header.execution_size = GEN_WIDTH_16;
    else if (this->curr.execWidth == 1)
      gen8_insn->header.execution_size = GEN_WIDTH_1;
    else if (this->curr.execWidth == 4)
      gen8_insn->header.execution_size = GEN_WIDTH_4;
    else
      NOT_IMPLEMENTED;
    gen8_insn->header.acc_wr_control = this->curr.accWrEnable;
    gen8_insn->header.quarter_control = this->curr.quarterControl;
    gen8_insn->header.nib_ctrl = this->curr.nibControl;
    gen8_insn->bits1.ia1.mask_control = this->curr.noMask;
    gen8_insn->bits1.ia1.flag_reg_nr = this->curr.flag;
    gen8_insn->bits1.ia1.flag_sub_reg_nr = this->curr.subFlag;
    if (this->curr.predicate != GEN_PREDICATE_NONE) {
      gen8_insn->header.predicate_control = this->curr.predicate;
      gen8_insn->header.predicate_inverse = this->curr.inversePredicate;
    }
    gen8_insn->header.saturate = this->curr.saturate;
  }

  void Gen8Encoder::setDPUntypedRW(GenNativeInstruction *insn,
                                    uint32_t bti,
                                    uint32_t rgba,
                                    uint32_t msg_type,
                                    uint32_t msg_length,
                                    uint32_t response_length)
  {
    Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;
    const GenMessageTarget sfid = GEN_SFID_DATAPORT1_DATA;
    setMessageDescriptor(insn, sfid, msg_length, response_length);
    gen8_insn->bits3.gen7_untyped_rw.msg_type = msg_type;
    gen8_insn->bits3.gen7_untyped_rw.bti = bti;
    gen8_insn->bits3.gen7_untyped_rw.rgba = rgba;
    if (curr.execWidth == 8)
      gen8_insn->bits3.gen7_untyped_rw.simd_mode = GEN_UNTYPED_SIMD8;
    else if (curr.execWidth == 16)
      gen8_insn->bits3.gen7_untyped_rw.simd_mode = GEN_UNTYPED_SIMD16;
    else
      NOT_SUPPORTED;
  }

  void Gen8Encoder::setTypedWriteMessage(GenNativeInstruction *insn, unsigned char bti,
                                          unsigned char msg_type, uint32_t msg_length, bool header_present)
  {
    Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;
    const GenMessageTarget sfid = GEN_SFID_DATAPORT1_DATA;
    setMessageDescriptor(insn, sfid, msg_length, 0, header_present);
    gen8_insn->bits3.gen7_typed_rw.bti = bti;
    gen8_insn->bits3.gen7_typed_rw.msg_type = msg_type;

    /* Always using the low 8 slots here. */
    gen8_insn->bits3.gen7_typed_rw.slot = 1;
  }

  void Gen8Encoder::F16TO32(GenRegister dest, GenRegister src0) {
    MOV(GenRegister::retype(dest, GEN_TYPE_F), GenRegister::retype(src0, GEN_TYPE_HF));
  }

  void Gen8Encoder::F32TO16(GenRegister dest, GenRegister src0) {
    MOV(GenRegister::retype(dest, GEN_TYPE_HF), GenRegister::retype(src0, GEN_TYPE_F));
  }

  void Gen8Encoder::ATOMIC(GenRegister dst, uint32_t function, GenRegister src, uint32_t bti, uint32_t srcNum) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;
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

    this->setHeader(insn);
    this->setDst(insn, GenRegister::uw16grf(dst.nr, 0));
    this->setSrc0(insn, GenRegister::ud8grf(src.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));

    const GenMessageTarget sfid = GEN_SFID_DATAPORT1_DATA;
    setMessageDescriptor(insn, sfid, msg_length, response_length);
    gen8_insn->bits3.gen7_atomic_op.msg_type = GEN75_P1_UNTYPED_ATOMIC_OP;
    gen8_insn->bits3.gen7_atomic_op.bti = bti;
    gen8_insn->bits3.gen7_atomic_op.return_data = 1;
    gen8_insn->bits3.gen7_atomic_op.aop_type = function;

    if (this->curr.execWidth == 8)
      gen8_insn->bits3.gen7_atomic_op.simd_mode = GEN_ATOMIC_SIMD8;
    else if (this->curr.execWidth == 16)
      gen8_insn->bits3.gen7_atomic_op.simd_mode = GEN_ATOMIC_SIMD16;
    else
      NOT_SUPPORTED;
  }

  void Gen8Encoder::UNTYPED_READ(GenRegister dst, GenRegister src, uint32_t bti, uint32_t elemNum) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    assert(elemNum >= 1 || elemNum <= 4);
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

    this->setHeader(insn);
    this->setDst(insn,  GenRegister::uw16grf(dst.nr, 0));
    this->setSrc0(insn, GenRegister::ud8grf(src.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));
    setDPUntypedRW(insn,
                   bti,
                   untypedRWMask[elemNum],
                   GEN75_P1_UNTYPED_READ,
                   msg_length,
                   response_length);
  }

  void Gen8Encoder::UNTYPED_WRITE(GenRegister msg, uint32_t bti, uint32_t elemNum) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    assert(elemNum >= 1 || elemNum <= 4);
    uint32_t msg_length = 0;
    uint32_t response_length = 0;
    this->setHeader(insn);
    if (this->curr.execWidth == 8) {
      this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UD));
      msg_length = 1 + elemNum;
    } else if (this->curr.execWidth == 16) {
      this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UW));
      msg_length = 2 * (1 + elemNum);
    }
    else
      NOT_IMPLEMENTED;
    this->setSrc0(insn, GenRegister::ud8grf(msg.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));
    setDPUntypedRW(insn,
                   bti,
                   untypedRWMask[elemNum],
                   GEN75_P1_UNTYPED_SURFACE_WRITE,
                   msg_length,
                   response_length);
  }

  void Gen8Encoder::LOAD_DF_IMM(GenRegister dest, GenRegister tmp, double value) {
    union { double d; unsigned u[2]; } u;
    u.d = value;
    GenRegister r = GenRegister::retype(tmp, GEN_TYPE_UD);
    push();
    curr.predicate = GEN_PREDICATE_NONE;
    curr.noMask = 1;
    curr.execWidth = 1;
    MOV(r, GenRegister::immud(u.u[0]));
    MOV(GenRegister::suboffset(r, 1), GenRegister::immud(u.u[1]));
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

  void Gen8Encoder::MOV_DF(GenRegister dest, GenRegister src0, GenRegister tmp) {
    GBE_ASSERT((src0.type == GEN_TYPE_F && dest.isdf()) || (src0.isdf() && dest.type == GEN_TYPE_F));
    GenRegister r = GenRegister::retype(tmp, GEN_TYPE_F);
    int w = curr.execWidth;
    GenRegister r0;
    r0 = GenRegister::h2(r);
    push();
    curr.execWidth = 4;
    curr.predicate = GEN_PREDICATE_NONE;
    curr.noMask = 1;
    MOV(r0, src0);
    MOV(GenRegister::suboffset(r0, 4), GenRegister::suboffset(src0, 4));
    curr.noMask = 0;
    curr.quarterControl = 0;
    curr.nibControl = 0;
    MOV(dest, r0);
    curr.nibControl = 1;
    MOV(GenRegister::suboffset(dest, 4), GenRegister::suboffset(r0, 4));
    pop();
    if (w == 16) {
      push();
      curr.execWidth = 4;
      curr.predicate = GEN_PREDICATE_NONE;
      curr.noMask = 1;
      MOV(r0, GenRegister::suboffset(src0, 8));
      MOV(GenRegister::suboffset(r0, 4), GenRegister::suboffset(src0, 12));
      curr.noMask = 0;
      curr.quarterControl = 1;
      curr.nibControl = 0;
      MOV(GenRegister::suboffset(dest, 8), r0);
      curr.nibControl = 1;
      MOV(GenRegister::suboffset(dest, 12), GenRegister::suboffset(r0, 4));
      pop();
    }
  }

  void Gen8Encoder::JMPI(GenRegister src, bool longjmp) {
    alu2(this, GEN_OPCODE_JMPI, GenRegister::ip(), GenRegister::ip(), src);
  }

  void Gen8Encoder::patchJMPI(uint32_t insnID, int32_t jip, int32_t uip) {
    GenNativeInstruction &insn = *(GenNativeInstruction *)&this->store[insnID];
    GBE_ASSERT(insnID < this->store.size());
    GBE_ASSERT(insn.header.opcode == GEN_OPCODE_JMPI ||
               insn.header.opcode == GEN_OPCODE_BRD  ||
               insn.header.opcode == GEN_OPCODE_ENDIF ||
               insn.header.opcode == GEN_OPCODE_IF ||
               insn.header.opcode == GEN_OPCODE_BRC ||
               insn.header.opcode == GEN_OPCODE_WHILE ||
               insn.header.opcode == GEN_OPCODE_ELSE);

    if( insn.header.opcode == GEN_OPCODE_WHILE ) {
      // if this WHILE instruction jump back to an ELSE instruction,
      // need add distance to go to the next instruction.
      GenNativeInstruction & insn_else = *(GenNativeInstruction *)&this->store[insnID+jip];
      if(insn_else.header.opcode == GEN_OPCODE_ELSE) {
        jip += 2;
      }
    }

    if(insn.header.opcode == GEN_OPCODE_ELSE)
      uip = jip;

    if (insn.header.opcode == GEN_OPCODE_IF ||
        insn.header.opcode == GEN_OPCODE_ELSE) {
      Gen8NativeInstruction *gen8_insn = &insn.gen8_insn;
      this->setSrc0(&insn, GenRegister::immud(0));
      gen8_insn->bits2.gen8_branch.uip = uip*8;
      gen8_insn->bits3.gen8_branch.jip = jip*8;
      return;
    }
    else if (insn.header.opcode == GEN_OPCODE_JMPI) {
      //jumpDistance'unit is Qword, and the HSW's offset of jmpi is in byte, so multi 8
      jip = (jip - 2);
    }

    this->setSrc1(&insn, GenRegister::immd(jip*8));
  }

  void Gen8Encoder::setDst(GenNativeInstruction *insn, GenRegister dest) {
    Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;
    if (dest.file != GEN_ARCHITECTURE_REGISTER_FILE)
      assert(dest.nr < 128);

    gen8_insn->bits1.da1.dest_reg_file = dest.file;
    gen8_insn->bits1.da1.dest_reg_type = dest.type;
    gen8_insn->bits1.da1.dest_address_mode = dest.address_mode;
    gen8_insn->bits1.da1.dest_reg_nr = dest.nr;
    gen8_insn->bits1.da1.dest_subreg_nr = dest.subnr;
    if (dest.hstride == GEN_HORIZONTAL_STRIDE_0) {
      if (dest.type == GEN_TYPE_UB || dest.type == GEN_TYPE_B)
        dest.hstride = GEN_HORIZONTAL_STRIDE_4;
      else if (dest.type == GEN_TYPE_UW || dest.type == GEN_TYPE_W)
        dest.hstride = GEN_HORIZONTAL_STRIDE_2;
      else
        dest.hstride = GEN_HORIZONTAL_STRIDE_1;
    }
    gen8_insn->bits1.da1.dest_horiz_stride = dest.hstride;
  }

  void Gen8Encoder::setSrc0(GenNativeInstruction *insn, GenRegister reg) {
    Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;
    if (reg.file != GEN_ARCHITECTURE_REGISTER_FILE)
      assert(reg.nr < 128);

    if (reg.address_mode == GEN_ADDRESS_DIRECT) {
      gen8_insn->bits1.da1.src0_reg_file = reg.file;
      gen8_insn->bits1.da1.src0_reg_type = reg.type;
      gen8_insn->bits2.da1.src0_abs = reg.absolute;
      gen8_insn->bits2.da1.src0_negate = reg.negation;
      gen8_insn->bits2.da1.src0_address_mode = reg.address_mode;
      if (reg.file == GEN_IMMEDIATE_VALUE) {
        gen8_insn->bits3.ud = reg.value.ud;

        /* Required to set some fields in src1 as well: */
        gen8_insn->bits2.da1.src1_reg_file = 0; /* arf */
        gen8_insn->bits2.da1.src1_reg_type = reg.type;
      }
      else {
        if (gen8_insn->header.access_mode == GEN_ALIGN_1) {
          gen8_insn->bits2.da1.src0_subreg_nr = reg.subnr;
          gen8_insn->bits2.da1.src0_reg_nr = reg.nr;
        } else {
          gen8_insn->bits2.da16.src0_subreg_nr = reg.subnr / 16;
          gen8_insn->bits2.da16.src0_reg_nr = reg.nr;
        }

        if (reg.width == GEN_WIDTH_1 &&
            gen8_insn->header.execution_size == GEN_WIDTH_1) {
          gen8_insn->bits2.da1.src0_horiz_stride = GEN_HORIZONTAL_STRIDE_0;
          gen8_insn->bits2.da1.src0_width = GEN_WIDTH_1;
          gen8_insn->bits2.da1.src0_vert_stride = GEN_VERTICAL_STRIDE_0;
        }
        else {
          gen8_insn->bits2.da1.src0_horiz_stride = reg.hstride;
          gen8_insn->bits2.da1.src0_width = reg.width;
          gen8_insn->bits2.da1.src0_vert_stride = reg.vstride;
        }
      }
    } else {
      gen8_insn->bits1.ia1.src0_reg_file = GEN_GENERAL_REGISTER_FILE;
      gen8_insn->bits1.ia1.src0_reg_type = reg.type;
      gen8_insn->bits2.ia1.src0_subreg_nr = 0;
      gen8_insn->bits2.ia1.src0_indirect_offset = 0;
      gen8_insn->bits2.ia1.src0_abs = 0;
      gen8_insn->bits2.ia1.src0_negate = 0;
      gen8_insn->bits2.ia1.src0_address_mode = reg.address_mode;
      gen8_insn->bits2.ia1.src0_horiz_stride = GEN_HORIZONTAL_STRIDE_0;
      gen8_insn->bits2.ia1.src0_width = GEN_WIDTH_1;
      gen8_insn->bits2.ia1.src0_vert_stride = GEN_VERTICAL_STRIDE_ONE_DIMENSIONAL;
    }
  }

  void Gen8Encoder::setSrc1(GenNativeInstruction *insn, GenRegister reg) {
    Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;
    assert(reg.nr < 128);
    assert(reg.file != GEN_ARCHITECTURE_REGISTER_FILE || reg.nr == 0);

    gen8_insn->bits2.da1.src1_reg_file = reg.file;
    gen8_insn->bits2.da1.src1_reg_type = reg.type;
    gen8_insn->bits3.da1.src1_abs = reg.absolute;
    gen8_insn->bits3.da1.src1_negate = reg.negation;

    assert(gen8_insn->bits1.da1.src0_reg_file != GEN_IMMEDIATE_VALUE);

    if (reg.file == GEN_IMMEDIATE_VALUE)
      gen8_insn->bits3.ud = reg.value.ud;
    else {
      assert (reg.address_mode == GEN_ADDRESS_DIRECT);
      if (gen8_insn->header.access_mode == GEN_ALIGN_1) {
        gen8_insn->bits3.da1.src1_subreg_nr = reg.subnr;
        gen8_insn->bits3.da1.src1_reg_nr = reg.nr;
      } else {
        gen8_insn->bits3.da16.src1_subreg_nr = reg.subnr / 16;
        gen8_insn->bits3.da16.src1_reg_nr = reg.nr;
      }

      if (reg.width == GEN_WIDTH_1 &&
          gen8_insn->header.execution_size == GEN_WIDTH_1) {
        gen8_insn->bits3.da1.src1_horiz_stride = GEN_HORIZONTAL_STRIDE_0;
        gen8_insn->bits3.da1.src1_width = GEN_WIDTH_1;
        gen8_insn->bits3.da1.src1_vert_stride = GEN_VERTICAL_STRIDE_0;
      } else {
        gen8_insn->bits3.da1.src1_horiz_stride = reg.hstride;
        gen8_insn->bits3.da1.src1_width = reg.width;
        gen8_insn->bits3.da1.src1_vert_stride = reg.vstride;
      }
    }
  }

#define NO_SWIZZLE ((0<<0) | (1<<2) | (2<<4) | (3<<6))

  void Gen8Encoder::alu3(uint32_t opcode,
                              GenRegister dest,
                              GenRegister src0,
                              GenRegister src1,
                              GenRegister src2)
  {
     GenNativeInstruction *insn = this->next(opcode);
     Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;

     assert(dest.file == GEN_GENERAL_REGISTER_FILE);
     assert(dest.nr < 128);
     assert(dest.address_mode == GEN_ADDRESS_DIRECT);
     assert(dest.type = GEN_TYPE_F);
     //gen8_insn->bits1.da3src.dest_reg_file = 0;
     gen8_insn->bits1.da3src.dest_reg_nr = dest.nr;
     gen8_insn->bits1.da3src.dest_subreg_nr = dest.subnr / 16;
     gen8_insn->bits1.da3src.dest_writemask = 0xf;
     this->setHeader(insn);
     gen8_insn->header.access_mode = GEN_ALIGN_16;
     gen8_insn->header.execution_size = GEN_WIDTH_8;

     assert(src0.file == GEN_GENERAL_REGISTER_FILE);
     assert(src0.address_mode == GEN_ADDRESS_DIRECT);
     assert(src0.nr < 128);
     assert(src0.type == GEN_TYPE_F);
     gen8_insn->bits2.da3src.src0_swizzle = NO_SWIZZLE;
     gen8_insn->bits2.da3src.src0_subreg_nr = src0.subnr / 4 ;
     gen8_insn->bits2.da3src.src0_reg_nr = src0.nr;
     gen8_insn->bits1.da3src.src0_abs = src0.absolute;
     gen8_insn->bits1.da3src.src0_negate = src0.negation;
     gen8_insn->bits2.da3src.src0_rep_ctrl = src0.vstride == GEN_VERTICAL_STRIDE_0;

     assert(src1.file == GEN_GENERAL_REGISTER_FILE);
     assert(src1.address_mode == GEN_ADDRESS_DIRECT);
     assert(src1.nr < 128);
     assert(src1.type == GEN_TYPE_F);
     gen8_insn->bits2.da3src.src1_swizzle = NO_SWIZZLE;
     gen8_insn->bits2.da3src.src1_subreg_nr_low = (src1.subnr / 4) & 0x3;
     gen8_insn->bits3.da3src.src1_subreg_nr_high = (src1.subnr / 4) >> 2;
     gen8_insn->bits2.da3src.src1_rep_ctrl = src1.vstride == GEN_VERTICAL_STRIDE_0;
     gen8_insn->bits3.da3src.src1_reg_nr = src1.nr;
     gen8_insn->bits1.da3src.src1_abs = src1.absolute;
     gen8_insn->bits1.da3src.src1_negate = src1.negation;

     assert(src2.file == GEN_GENERAL_REGISTER_FILE);
     assert(src2.address_mode == GEN_ADDRESS_DIRECT);
     assert(src2.nr < 128);
     assert(src2.type == GEN_TYPE_F);
     gen8_insn->bits3.da3src.src2_swizzle = NO_SWIZZLE;
     gen8_insn->bits3.da3src.src2_subreg_nr = src2.subnr / 4;
     gen8_insn->bits3.da3src.src2_rep_ctrl = src2.vstride == GEN_VERTICAL_STRIDE_0;
     gen8_insn->bits3.da3src.src2_reg_nr = src2.nr;
     gen8_insn->bits1.da3src.src2_abs = src2.absolute;
     gen8_insn->bits1.da3src.src2_negate = src2.negation;

     // Emit second half of the instruction
     if (this->curr.execWidth == 16) {
      GenNativeInstruction q1Insn = *insn;
      insn = this->next(opcode);
      *insn = q1Insn;
      gen8_insn = &insn->gen8_insn;
      gen8_insn->header.quarter_control = GEN_COMPRESSION_Q2;
      gen8_insn->bits1.da3src.dest_reg_nr++;
      if (gen8_insn->bits2.da3src.src0_rep_ctrl == 0)
        gen8_insn->bits2.da3src.src0_reg_nr++;
      if (gen8_insn->bits2.da3src.src1_rep_ctrl == 0)
        gen8_insn->bits3.da3src.src1_reg_nr++;
      if (gen8_insn->bits3.da3src.src2_rep_ctrl == 0)
        gen8_insn->bits3.da3src.src2_reg_nr++;
     }
  }
} /* End of the name space. */

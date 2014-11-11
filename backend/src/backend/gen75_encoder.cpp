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

static const uint32_t untypedRWMask[] = {
  GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE|GEN_UNTYPED_GREEN|GEN_UNTYPED_RED,
  GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE|GEN_UNTYPED_GREEN,
  GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE,
  GEN_UNTYPED_ALPHA,
  0
};

namespace gbe
{
  void Gen75Encoder::setHeader(GenNativeInstruction *insn) {
    Gen7NativeInstruction *gen7_insn = &insn->gen7_insn;
    if (this->curr.execWidth == 8)
      gen7_insn->header.execution_size = GEN_WIDTH_8;
    else if (this->curr.execWidth == 16)
      gen7_insn->header.execution_size = GEN_WIDTH_16;
    else if (this->curr.execWidth == 1)
      gen7_insn->header.execution_size = GEN_WIDTH_1;
    else if (this->curr.execWidth == 4)
      gen7_insn->header.execution_size = GEN_WIDTH_4;
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

  void Gen75Encoder::setDPUntypedRW(GenNativeInstruction *insn,
                                    uint32_t bti,
                                    uint32_t rgba,
                                    uint32_t msg_type,
                                    uint32_t msg_length,
                                    uint32_t response_length)
  {
    Gen7NativeInstruction *gen7_insn = &insn->gen7_insn;
    const GenMessageTarget sfid = GEN_SFID_DATAPORT1_DATA;
    setMessageDescriptor(insn, sfid, msg_length, response_length);
    gen7_insn->bits3.gen7_untyped_rw.msg_type = msg_type;
    gen7_insn->bits3.gen7_untyped_rw.bti = bti;
    gen7_insn->bits3.gen7_untyped_rw.rgba = rgba;
    if (curr.execWidth == 8)
      gen7_insn->bits3.gen7_untyped_rw.simd_mode = GEN_UNTYPED_SIMD8;
    else if (curr.execWidth == 16)
      gen7_insn->bits3.gen7_untyped_rw.simd_mode = GEN_UNTYPED_SIMD16;
    else
      NOT_SUPPORTED;
  }

  void Gen75Encoder::setTypedWriteMessage(GenNativeInstruction *insn, unsigned char bti,
                                          unsigned char msg_type, uint32_t msg_length, bool header_present)
  {
    Gen7NativeInstruction *gen7_insn = &insn->gen7_insn;
    const GenMessageTarget sfid = GEN_SFID_DATAPORT1_DATA;
    setMessageDescriptor(insn, sfid, msg_length, 0, header_present);
    gen7_insn->bits3.gen7_typed_rw.bti = bti;
    gen7_insn->bits3.gen7_typed_rw.msg_type = msg_type;

    /* Always using the low 8 slots here. */
    gen7_insn->bits3.gen7_typed_rw.slot = 1;
  }

  void Gen75Encoder::ATOMIC(GenRegister dst, uint32_t function, GenRegister src, uint32_t bti, uint32_t srcNum) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    Gen7NativeInstruction *gen7_insn = &insn->gen7_insn;
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
    gen7_insn->bits3.gen7_atomic_op.msg_type = GEN75_P1_UNTYPED_ATOMIC_OP;
    gen7_insn->bits3.gen7_atomic_op.bti = bti;
    gen7_insn->bits3.gen7_atomic_op.return_data = 1;
    gen7_insn->bits3.gen7_atomic_op.aop_type = function;

    if (this->curr.execWidth == 8)
      gen7_insn->bits3.gen7_atomic_op.simd_mode = GEN_ATOMIC_SIMD8;
    else if (this->curr.execWidth == 16)
      gen7_insn->bits3.gen7_atomic_op.simd_mode = GEN_ATOMIC_SIMD16;
    else
      NOT_SUPPORTED;
  }

  void Gen75Encoder::UNTYPED_READ(GenRegister dst, GenRegister src, uint32_t bti, uint32_t elemNum) {
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

  void Gen75Encoder::UNTYPED_WRITE(GenRegister msg, uint32_t bti, uint32_t elemNum) {
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

  void Gen75Encoder::LOAD_DF_IMM(GenRegister dest, GenRegister tmp, double value) {
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

  void Gen75Encoder::MOV_DF(GenRegister dest, GenRegister src0, GenRegister tmp) {
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

  void Gen75Encoder::JMPI(GenRegister src, bool longjmp) {
    alu2(this, GEN_OPCODE_JMPI, GenRegister::ip(), GenRegister::ip(), src);
  }

  void Gen75Encoder::patchJMPI(uint32_t insnID, int32_t jip, int32_t uip) {
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

    if (insn.header.opcode != GEN_OPCODE_JMPI)
      this->setSrc1(&insn, GenRegister::immd((jip & 0xffff) | (uip<<16)));
    else if (insn.header.opcode == GEN_OPCODE_JMPI) {
      //jumpDistance'unit is Qword, and the HSW's JMPI offset of jmpi is in byte, so multi 8
      jip = (jip - 2) * 8;
      this->setSrc1(&insn, GenRegister::immd(jip));
    }
    return;
  }
} /* End of the name space. */

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
  extern bool compactAlu3(GenEncoder *p, uint32_t opcode, GenRegister dst, GenRegister src0, GenRegister src1, GenRegister src2);
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
    gen8_insn->bits3.gen8_untyped_rw_a64.msg_type = msg_type;
    gen8_insn->bits3.gen8_untyped_rw_a64.bti = bti;
    gen8_insn->bits3.gen8_untyped_rw_a64.rgba = rgba;
    if (curr.execWidth == 8)
      gen8_insn->bits3.gen8_untyped_rw_a64.simd_mode = GEN_UNTYPED_SIMD8;
    else if (curr.execWidth == 16)
      gen8_insn->bits3.gen8_untyped_rw_a64.simd_mode = GEN_UNTYPED_SIMD16;
    else
      NOT_SUPPORTED;
  }

  static void setDPByteScatterGatherA64(GenEncoder *p,
                                     GenNativeInstruction *insn,
                                     uint32_t bti,
                                     uint32_t block_size,
                                     uint32_t data_size,
                                     uint32_t msg_type,
                                     uint32_t msg_length,
                                     uint32_t response_length)
  {
    const GenMessageTarget sfid = GEN_SFID_DATAPORT1_DATA;
    Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;
    p->setMessageDescriptor(insn, sfid, msg_length, response_length);
    gen8_insn->bits3.gen8_scatter_rw_a64.msg_type = msg_type;
    gen8_insn->bits3.gen8_scatter_rw_a64.bti = bti;
    gen8_insn->bits3.gen8_scatter_rw_a64.data_sz = data_size;
    gen8_insn->bits3.gen8_scatter_rw_a64.block_sz = block_size;
    GBE_ASSERT(p->curr.execWidth == 8);
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
  unsigned Gen8Encoder::setAtomicMessageDesc(GenNativeInstruction *insn, unsigned function, unsigned bti, unsigned srcNum) {
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
    return gen8_insn->bits3.ud;
  }

  void Gen8Encoder::ATOMIC(GenRegister dst, uint32_t function, GenRegister src, GenRegister data, GenRegister bti, uint32_t srcNum, bool useSends) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);

    this->setHeader(insn);
    insn->header.destreg_or_condmod = GEN_SFID_DATAPORT1_DATA;

    this->setDst(insn, GenRegister::uw16grf(dst.nr, 0));
    this->setSrc0(insn, GenRegister::ud8grf(src.nr, 0));

    if (bti.file == GEN_IMMEDIATE_VALUE) {
      this->setSrc1(insn, GenRegister::immud(0));
      setAtomicMessageDesc(insn, function, bti.value.ud, srcNum);
    } else {
      this->setSrc1(insn, bti);
    }
  }

  unsigned Gen8Encoder::setAtomicA64MessageDesc(GenNativeInstruction *insn, unsigned function, unsigned bti, unsigned srcNum, int type_long) {
    Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;
    uint32_t msg_length = 0;
    uint32_t response_length = 0;
    assert(srcNum <= 3);

    if (this->curr.execWidth == 8) {
      msg_length = srcNum + 1 + type_long;
      if(srcNum == 3 && type_long)
        msg_length++;
      response_length = 1 + type_long;
    } else if (this->curr.execWidth == 16) {
      msg_length = 2 * (srcNum + 1);
      response_length = 2;
    } else
      NOT_IMPLEMENTED;

    const GenMessageTarget sfid = GEN_SFID_DATAPORT1_DATA;
    setMessageDescriptor(insn, sfid, msg_length, response_length);
    gen8_insn->bits3.gen8_atomic_a64.msg_type = GEN8_P1_UNTYPED_ATOMIC_A64;
    gen8_insn->bits3.gen8_atomic_a64.bti = bti;
    gen8_insn->bits3.gen8_atomic_a64.return_data = 1;
    gen8_insn->bits3.gen8_atomic_a64.aop_type = function;
    gen8_insn->bits3.gen8_atomic_a64.data_size = type_long;

    return gen8_insn->bits3.ud;
  }

  void Gen8Encoder::ATOMICA64(GenRegister dst, uint32_t function, GenRegister src, GenRegister bti, uint32_t srcNum) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);

    this->setHeader(insn);
    insn->header.destreg_or_condmod = GEN_SFID_DATAPORT_DATA;

    this->setDst(insn, GenRegister::uw16grf(dst.nr, 0));
    this->setSrc0(insn, GenRegister::ud8grf(src.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));
    int type_long = (dst.type == GEN_TYPE_UL || dst.type == GEN_TYPE_L) ? 1: 0;
    setAtomicA64MessageDesc(insn, function, bti.value.ud, srcNum, type_long);
  }

  unsigned Gen8Encoder::setUntypedReadMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemNum) {
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
                   GEN75_P1_UNTYPED_READ,
                   msg_length,
                   response_length);
    return insn->bits3.ud;
  }

  void Gen8Encoder::UNTYPED_READ(GenRegister dst, GenRegister src, GenRegister bti, uint32_t elemNum) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    assert(elemNum >= 1 || elemNum <= 4);

    this->setHeader(insn);
    this->setDst(insn,  GenRegister::uw16grf(dst.nr, 0));
    this->setSrc0(insn, GenRegister::ud8grf(src.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));
    insn->header.destreg_or_condmod = GEN_SFID_DATAPORT1_DATA;

    if (bti.file == GEN_IMMEDIATE_VALUE) {
      this->setSrc1(insn, GenRegister::immud(0));
      setUntypedReadMessageDesc(insn, bti.value.ud, elemNum);
    } else {
      this->setSrc1(insn, bti);
    }
  }

  unsigned Gen8Encoder::setUntypedWriteMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemNum) {
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
                   GEN75_P1_UNTYPED_SURFACE_WRITE,
                   msg_length,
                   response_length);
    return insn->bits3.ud;
  }

  void Gen8Encoder::UNTYPED_WRITE(GenRegister msg, GenRegister data, GenRegister bti, uint32_t elemNum, bool useSends) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    assert(elemNum >= 1 || elemNum <= 4);
    this->setHeader(insn);
    insn->header.destreg_or_condmod = GEN_SFID_DATAPORT1_DATA;
    if (this->curr.execWidth == 8) {
      this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UD));
    } else if (this->curr.execWidth == 16) {
      this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UW));
    }
    else
      NOT_IMPLEMENTED;
    this->setSrc0(insn, GenRegister::ud8grf(msg.nr, 0));

    if (bti.file == GEN_IMMEDIATE_VALUE) {
      this->setSrc1(insn, GenRegister::immud(0));
      setUntypedWriteMessageDesc(insn, bti.value.ud, elemNum);
    } else {
      this->setSrc1(insn, bti);
    }
  }

  void Gen8Encoder::UNTYPED_READA64(GenRegister dst, GenRegister src, uint32_t elemNum) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    assert(elemNum >= 1 || elemNum <= 4);
    uint32_t msg_length = 0;
    uint32_t response_length = 0;
    assert(this->curr.execWidth == 8);

    if (this->curr.execWidth == 8) {
      msg_length = 2;
      response_length = elemNum;
    } else
      NOT_IMPLEMENTED;

    this->setHeader(insn);
    this->setDst(insn,  GenRegister::uw16grf(dst.nr, 0));
    this->setSrc0(insn, GenRegister::ud8grf(src.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));
    setDPUntypedRW(insn,
                   255, // stateless bti
                   untypedRWMask[elemNum],
                   GEN8_P1_UNTYPED_READ_A64,
                   msg_length,
                   response_length);
  }

  void Gen8Encoder::UNTYPED_WRITEA64(GenRegister msg, uint32_t elemNum) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    assert(elemNum >= 1 || elemNum <= 4);
    uint32_t msg_length = 0;
    uint32_t response_length = 0;
    this->setHeader(insn);
    if (this->curr.execWidth == 8) {
      this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UD));
      msg_length = 2 + elemNum;
    } else
      NOT_IMPLEMENTED;

    this->setSrc0(insn, GenRegister::ud8grf(msg.nr, 0));
    this->setSrc1(insn, GenRegister::immud(0));
    setDPUntypedRW(insn,
                   255, //stateless bti
                   untypedRWMask[elemNum],
                   GEN8_P1_UNTYPED_WRITE_A64,
                   msg_length,
                   response_length);
  }

  void Gen8Encoder::BYTE_GATHERA64(GenRegister dst, GenRegister src, uint32_t elemSize) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    this->setHeader(insn);
    insn->header.destreg_or_condmod = GEN_SFID_DATAPORT1_DATA;

    this->setDst(insn, GenRegister::uw16grf(dst.nr, 0));
    this->setSrc0(insn, GenRegister::ud8grf(src.nr, 0));

    this->setSrc1(insn, GenRegister::immud(0));
    //setByteGatherMessageDesc(insn, bti.value.ud, elemSize);
    GBE_ASSERT(this->curr.execWidth == 8);
    const uint32_t msg_length = 2;
    const uint32_t response_length = 1;
    setDPByteScatterGatherA64(this,
                           insn,
                           0xff,
                           0x0,
                           elemSize,
                           GEN8_P1_BYTE_GATHER_A64,
                           msg_length,
                           response_length);
  }

  void Gen8Encoder::BYTE_SCATTERA64(GenRegister msg, uint32_t elemSize) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);

    this->setHeader(insn);
    insn->header.destreg_or_condmod = GEN_SFID_DATAPORT1_DATA;

    // only support simd8
    GBE_ASSERT(this->curr.execWidth == 8);
    this->setDst(insn, GenRegister::retype(GenRegister::null(), GEN_TYPE_UD));

    this->setSrc0(insn, GenRegister::ud8grf(msg.nr, 0));

    this->setSrc1(insn, GenRegister::immud(0));
    const uint32_t msg_length = 3;
    const uint32_t response_length = 0;
    setDPByteScatterGatherA64(this,
                           insn,
                           0xff,
                           0x0,
                           elemSize,
                           GEN8_P1_BYTE_SCATTER_A64,
                           msg_length,
                           response_length);
  }

  void Gen8Encoder::LOAD_INT64_IMM(GenRegister dest, GenRegister value) {
    MOV(dest, value);
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
  void Gen8Encoder::FENCE(GenRegister dst, bool flushRWCache) {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
    Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;
    this->setHeader(insn);
    this->setDst(insn, dst);
    this->setSrc0(insn, dst);
    setMessageDescriptor(insn, GEN_SFID_DATAPORT_DATA, 1, 1, 1);
    gen8_insn->bits3.gen7_memory_fence.msg_type = GEN_MEM_FENCE;
    gen8_insn->bits3.gen7_memory_fence.commit_enable = 0x1;
    gen8_insn->bits3.gen7_memory_fence.flush_rw = flushRWCache ? 1 : 0;
  }

  void Gen8Encoder::FLUSH_SAMPLERCACHE(GenRegister dst) {
     GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
     this->setHeader(insn);
     this->setDst(insn, dst);
     this->setSrc0(insn, GenRegister::ud8grf(0,0));
     unsigned msg_type = GEN_SAMPLER_MESSAGE_CACHE_FLUSH;
     unsigned simd_mode = GEN_SAMPLER_SIMD_MODE_SIMD32_64;
     setSamplerMessage(insn, 0, 0, msg_type,
                       1, 1,
                       true,
                       simd_mode, 0);
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

  void Gen8Encoder::setSrc0WithAcc(GenNativeInstruction *insn, GenRegister reg, uint32_t accN) {
    Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;
    assert(reg.file == GEN_GENERAL_REGISTER_FILE);
    assert(reg.nr < 128);
    assert(gen8_insn->header.access_mode == GEN_ALIGN_16);
    assert(reg.subnr == 0);
    assert(gen8_insn->header.execution_size >= GEN_WIDTH_4);

    gen8_insn->bits1.da16acc.src0_reg_file = reg.file;
    gen8_insn->bits1.da16acc.src0_reg_type = reg.type;
    gen8_insn->bits2.da16acc.src0_abs = reg.absolute;
    gen8_insn->bits2.da16acc.src0_negate = reg.negation;
    gen8_insn->bits2.da16acc.src0_address_mode = reg.address_mode;
    gen8_insn->bits2.da16acc.src0_subreg_nr = reg.subnr / 16;
    gen8_insn->bits2.da16acc.src0_reg_nr = reg.nr;
    gen8_insn->bits2.da16acc.src0_special_acc_lo = accN;
    gen8_insn->bits2.da16acc.src0_special_acc_hi = 0;
    gen8_insn->bits2.da16acc.src0_vert_stride = reg.vstride;
  }

  void Gen8Encoder::setSrc1WithAcc(GenNativeInstruction *insn, GenRegister reg, uint32_t accN) {
    Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;
    assert(reg.file == GEN_GENERAL_REGISTER_FILE);
    assert(reg.nr < 128);
    assert(gen8_insn->header.access_mode == GEN_ALIGN_16);
    assert(reg.subnr == 0);
    assert(gen8_insn->header.execution_size >= GEN_WIDTH_4);

    gen8_insn->bits2.da16acc.src1_reg_file = reg.file;
    gen8_insn->bits2.da16acc.src1_reg_type = reg.type;
    gen8_insn->bits3.da16acc.src1_abs = reg.absolute;
    gen8_insn->bits3.da16acc.src1_negate = reg.negation;
    gen8_insn->bits3.da16acc.src1_address_mode = reg.address_mode;
    gen8_insn->bits3.da16acc.src1_subreg_nr = reg.subnr / 16;
    gen8_insn->bits3.da16acc.src1_reg_nr = reg.nr;
    gen8_insn->bits3.da16acc.src1_special_acc_lo = accN;
    gen8_insn->bits3.da16acc.src1_special_acc_hi = 0;
    gen8_insn->bits3.da16acc.src1_vert_stride = reg.vstride;
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
        if (reg.type == GEN_TYPE_L || reg.type == GEN_TYPE_UL || reg.type == GEN_TYPE_DF_IMM) {
          gen8_insn->bits3.ud = (uint32_t)(reg.value.i64 >> 32);
          gen8_insn->bits2.ud = (uint32_t)(reg.value.i64);
        } else {
          gen8_insn->bits3.ud = reg.value.ud;
          /* Required to set some fields in src1 as well: */
          gen8_insn->bits2.da1.src1_reg_file = 0; /* arf */
          gen8_insn->bits2.da1.src1_reg_type = reg.type;
        }
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
      gen8_insn->bits2.ia1.src0_subreg_nr = reg.a0_subnr;
      gen8_insn->bits2.ia1.src0_indirect_offset = (reg.addr_imm & 0x1ff);
      gen8_insn->bits2.ia1.src0_abs = reg.absolute;
      gen8_insn->bits2.ia1.src0_negate = reg.negation;
      gen8_insn->bits2.ia1.src0_address_mode = reg.address_mode;
      gen8_insn->bits2.ia1.src0_horiz_stride = reg.hstride;
      gen8_insn->bits2.ia1.src0_width = reg.width;
      gen8_insn->bits2.ia1.src0_vert_stride = reg.vstride;
      gen8_insn->bits2.ia1.src0_indirect_offset_9 = (reg.addr_imm & 0x02) >> 9;
    }
  }

  void Gen8Encoder::setSrc1(GenNativeInstruction *insn, GenRegister reg) {
    Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;
    assert(reg.nr < 128);

    gen8_insn->bits2.da1.src1_reg_file = reg.file;
    gen8_insn->bits2.da1.src1_reg_type = reg.type;
    gen8_insn->bits3.da1.src1_abs = reg.absolute;
    gen8_insn->bits3.da1.src1_negate = reg.negation;

    assert(gen8_insn->bits1.da1.src0_reg_file != GEN_IMMEDIATE_VALUE);

    if (reg.file == GEN_IMMEDIATE_VALUE) {
      assert(!((reg.type == GEN_TYPE_L || reg.type == GEN_TYPE_UL || reg.type == GEN_TYPE_DF_IMM) && reg.value.u64 > 0xFFFFFFFFl));
      gen8_insn->bits3.ud = reg.value.ud;
    } else {
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

  bool Gen8Encoder::canHandleLong(uint32_t opcode, GenRegister dst, GenRegister src0, GenRegister src1)
  {
    return false;
  }

  void Gen8Encoder::handleDouble(GenEncoder *p, uint32_t opcode, GenRegister dst, GenRegister src0, GenRegister src1)
  {
    uint32_t w = p->curr.execWidth;
    GenNativeInstruction *insn = NULL;

    if (w <= 8) {
      insn = p->next(opcode);
      p->setHeader(insn);
      p->setDst(insn, dst);
      p->setSrc0(insn, src0);
      if (!GenRegister::isNull(src1))
        p->setSrc1(insn, src1);
      return;
    } else {
      GBE_ASSERT(w == 16);
      GBE_ASSERT(dst.hstride != GEN_HORIZONTAL_STRIDE_0); //Should not be a uniform.
      p->push(); {
        p->curr.execWidth = 8;
        p->curr.quarterControl = GEN_COMPRESSION_Q1;
        insn = p->next(opcode);
        p->setHeader(insn);
        p->setDst(insn, dst);
        p->setSrc0(insn, src0);
        if (!GenRegister::isNull(src1))
          p->setSrc1(insn, src1);

        // second half
        p->curr.quarterControl = GEN_COMPRESSION_Q2;
        insn = p->next(opcode);
        p->setHeader(insn);
        p->setDst(insn, GenRegister::offset(dst, 2));

        if (src0.hstride != GEN_HORIZONTAL_STRIDE_0)
          p->setSrc0(insn, GenRegister::offset(src0, 2));
        else
          p->setSrc0(insn, src0);

        if (!GenRegister::isNull(src1)) {
          if (src1.hstride != GEN_HORIZONTAL_STRIDE_0)
            p->setSrc1(insn, GenRegister::offset(src1, 2));
          else
            p->setSrc1(insn, src1);
        }
      } p->pop();
    }
  }

#define NO_SWIZZLE ((0<<0) | (1<<2) | (2<<4) | (3<<6))

  void Gen8Encoder::alu3(uint32_t opcode,
                              GenRegister dest,
                              GenRegister src0,
                              GenRegister src1,
                              GenRegister src2)
  {
     if(compactAlu3(this, opcode, dest, src0, src1, src2))
       return;
     GenNativeInstruction *insn = this->next(opcode);
     Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;

     int execution_size = 0;
     if (this->curr.execWidth == 1) {
       execution_size = GEN_WIDTH_1;
     }else if(this->curr.execWidth == 8) {
       execution_size = GEN_WIDTH_8;
     } else if(this->curr.execWidth == 16) {
       execution_size = GEN_WIDTH_16;
     }else
       NOT_IMPLEMENTED;

     assert(dest.file == GEN_GENERAL_REGISTER_FILE);
     assert(dest.nr < 128);
     assert(dest.address_mode == GEN_ADDRESS_DIRECT);
     assert(src0.type == GEN_TYPE_HF || src0.type == GEN_TYPE_F || src0.type == GEN_TYPE_DF);
     assert(src0.type == dest.type);
     assert(src0.type == src1.type);
     assert(src0.type == src2.type);
     int32_t dataType = src0.type == GEN_TYPE_DF ? 3 : (src0.type == GEN_TYPE_HF ? 4 : 0);
     //gen8_insn->bits1.da3src.dest_reg_file = 0;
     gen8_insn->bits1.da3src.dest_reg_nr = dest.nr;
     gen8_insn->bits1.da3src.dest_subreg_nr = dest.subnr / 4;
     gen8_insn->bits1.da3src.dest_writemask = 0xf;
     gen8_insn->bits1.da3src.dest_type = dataType;
     gen8_insn->bits1.da3src.src_type = dataType;
     gen8_insn->bits1.da3src.src1_type = src1.type == GEN_TYPE_HF;
     gen8_insn->bits1.da3src.src2_type = src2.type == GEN_TYPE_HF;
     this->setHeader(insn);
     gen8_insn->header.access_mode = GEN_ALIGN_16;
     gen8_insn->header.execution_size = execution_size;

     assert(src0.file == GEN_GENERAL_REGISTER_FILE);
     assert(src0.address_mode == GEN_ADDRESS_DIRECT);
     assert(src0.nr < 128);
     gen8_insn->bits2.da3src.src0_swizzle = NO_SWIZZLE;
     gen8_insn->bits2.da3src.src0_subreg_nr = src0.subnr / 4 ;
     gen8_insn->bits2.da3src.src0_reg_nr = src0.nr;
     gen8_insn->bits1.da3src.src0_abs = src0.absolute;
     gen8_insn->bits1.da3src.src0_negate = src0.negation;
     gen8_insn->bits2.da3src.src0_rep_ctrl = src0.vstride == GEN_VERTICAL_STRIDE_0;

     assert(src1.file == GEN_GENERAL_REGISTER_FILE);
     assert(src1.address_mode == GEN_ADDRESS_DIRECT);
     assert(src1.nr < 128);
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
     gen8_insn->bits3.da3src.src2_swizzle = NO_SWIZZLE;
     gen8_insn->bits3.da3src.src2_subreg_nr = src2.subnr / 4;
     gen8_insn->bits3.da3src.src2_rep_ctrl = src2.vstride == GEN_VERTICAL_STRIDE_0;
     gen8_insn->bits3.da3src.src2_reg_nr = src2.nr;
     gen8_insn->bits1.da3src.src2_abs = src2.absolute;
     gen8_insn->bits1.da3src.src2_negate = src2.negation;
  }

  void Gen8Encoder::MATH_WITH_ACC(GenRegister dst, uint32_t function, GenRegister src0, GenRegister src1,
                             uint32_t dstAcc, uint32_t src0Acc, uint32_t src1Acc)
  {
     GenNativeInstruction *insn = this->next(GEN_OPCODE_MATH);
     Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;
     assert(dst.file == GEN_GENERAL_REGISTER_FILE);
     assert(src0.file == GEN_GENERAL_REGISTER_FILE);
     assert(src1.file == GEN_GENERAL_REGISTER_FILE);
     assert(dst.hstride == GEN_HORIZONTAL_STRIDE_1 || dst.hstride == GEN_HORIZONTAL_STRIDE_0);

     gen8_insn->header.access_mode = GEN_ALIGN_16;
     insn->header.destreg_or_condmod = function;
     this->setHeader(insn);
     this->setDst(insn, dst);
     gen8_insn->bits1.da16acc.dst_special_acc = dstAcc;
     this->setSrc0WithAcc(insn, src0, src0Acc);
     this->setSrc1WithAcc(insn, src1, src1Acc);
  }

  void Gen8Encoder::MADM(GenRegister dst, GenRegister src0, GenRegister src1, GenRegister src2,
      uint32_t dstAcc, uint32_t src0Acc, uint32_t src1Acc, uint32_t src2Acc)
  {
    GenNativeInstruction *insn = this->next(GEN_OPCODE_MADM);
    Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;
    assert(dst.file == GEN_GENERAL_REGISTER_FILE);
    assert(src0.file == GEN_GENERAL_REGISTER_FILE);
    assert(src1.file == GEN_GENERAL_REGISTER_FILE);
    assert(src2.file == GEN_GENERAL_REGISTER_FILE);
    assert(dst.hstride == GEN_HORIZONTAL_STRIDE_1 || dst.hstride == GEN_HORIZONTAL_STRIDE_0);
    assert(src0.type == GEN_TYPE_DF || src0.type == GEN_TYPE_F);
    assert(src0.type == dst.type);
    assert(src0.type == src1.type);
    assert(src0.type == src2.type);
      // If in double, width should be less than 4
    assert((src0.type == GEN_TYPE_DF && this->curr.execWidth <= 4)
      // If in float, width should be less than 8
        || (src0.type == GEN_TYPE_F && this->curr.execWidth <= 8));

    int32_t dataType = src0.type == GEN_TYPE_DF ? 3 : 0;

    this->setHeader(insn);
    gen8_insn->bits1.da3srcacc.dest_reg_nr = dst.nr;
    gen8_insn->bits1.da3srcacc.dest_subreg_nr = dst.subnr / 16;
    gen8_insn->bits1.da3srcacc.dst_special_acc = dstAcc;
    gen8_insn->bits1.da3srcacc.src_type = dataType;
    gen8_insn->bits1.da3srcacc.dest_type = dataType;
    gen8_insn->header.access_mode = GEN_ALIGN_16;

    assert(src0.file == GEN_GENERAL_REGISTER_FILE);
    assert(src0.address_mode == GEN_ADDRESS_DIRECT);
    assert(src0.nr < 128);
    gen8_insn->bits2.da3srcacc.src0_special_acc = src0Acc;
    gen8_insn->bits2.da3srcacc.src0_subreg_nr = src0.subnr / 4 ;
    gen8_insn->bits2.da3srcacc.src0_reg_nr = src0.nr;
    gen8_insn->bits1.da3srcacc.src0_abs = src0.absolute;
    gen8_insn->bits1.da3srcacc.src0_negate = src0.negation;
    gen8_insn->bits2.da3srcacc.src0_rep_ctrl = src0.vstride == GEN_VERTICAL_STRIDE_0;

    assert(src1.file == GEN_GENERAL_REGISTER_FILE);
    assert(src1.address_mode == GEN_ADDRESS_DIRECT);
    assert(src1.nr < 128);
    gen8_insn->bits2.da3srcacc.src1_special_acc = src1Acc;
    gen8_insn->bits2.da3srcacc.src1_subreg_nr_low = (src1.subnr / 4) & 0x3;
    gen8_insn->bits3.da3srcacc.src1_subreg_nr_high = (src1.subnr / 4) >> 2;
    gen8_insn->bits2.da3srcacc.src1_rep_ctrl = src1.vstride == GEN_VERTICAL_STRIDE_0;
    gen8_insn->bits3.da3srcacc.src1_reg_nr = src1.nr;
    gen8_insn->bits1.da3srcacc.src1_abs = src1.absolute;
    gen8_insn->bits1.da3srcacc.src1_negate = src1.negation;

    assert(src2.file == GEN_GENERAL_REGISTER_FILE);
    assert(src2.address_mode == GEN_ADDRESS_DIRECT);
    assert(src2.nr < 128);
    gen8_insn->bits3.da3srcacc.src2_special_acc = src2Acc;
    gen8_insn->bits3.da3srcacc.src2_subreg_nr = src2.subnr / 4;
    gen8_insn->bits3.da3srcacc.src2_rep_ctrl = src2.vstride == GEN_VERTICAL_STRIDE_0;
    gen8_insn->bits3.da3srcacc.src2_reg_nr = src2.nr;
    gen8_insn->bits1.da3srcacc.src2_abs = src2.absolute;
    gen8_insn->bits1.da3srcacc.src2_negate = src2.negation;
  }

  static void setOBlockRWA64(GenEncoder *p,
                             GenNativeInstruction *insn,
                             uint32_t bti,
                             uint32_t size,
                             uint32_t msg_type,
                             uint32_t msg_length,
                             uint32_t response_length)
  {
    const GenMessageTarget sfid = GEN_SFID_DATAPORT1_DATA;
    p->setMessageDescriptor(insn, sfid, msg_length, response_length);
    Gen8NativeInstruction *gen8_insn = &insn->gen8_insn;

    gen8_insn->bits3.gen8_block_rw_a64.msg_type = msg_type;
    gen8_insn->bits3.gen8_block_rw_a64.bti = bti;
    // For OWord Block read, we use unaligned read
    gen8_insn->bits3.gen8_block_rw_a64.msg_sub_type = msg_type == GEN8_P1_BLOCK_READ_A64 ? 1 : 0;
    gen8_insn->bits3.gen8_block_rw_a64.block_size = size;
    gen8_insn->bits3.gen8_block_rw_a64.header_present = 1;
  }

  void Gen8Encoder::OBREADA64(GenRegister dst, GenRegister header, uint32_t bti, uint32_t ow_size) {
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
    setOBlockRWA64(this,
                   insn,
                   bti,
                   block_size,
                   GEN8_P1_BLOCK_READ_A64,
                   msg_length,
                   response_length);

  }

  void Gen8Encoder::OBWRITEA64(GenRegister header, uint32_t bti, uint32_t ow_size) {
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
    setOBlockRWA64(this,
                   insn,
                   bti,
                   block_size,
                   GEN8_P1_BLOCK_WRITE_A64,
                   msg_length,
                   response_length);
   }
} /* End of the name space. */

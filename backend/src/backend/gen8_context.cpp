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
 */

/**
 * \file gen8_context.cpp
 */

#include "backend/gen8_context.hpp"
#include "backend/gen8_encoder.hpp"
#include "backend/gen_program.hpp"
#include "backend/gen_defs.hpp"
#include "backend/gen_encoder.hpp"
#include "backend/gen_insn_selection.hpp"
#include "backend/gen_insn_scheduling.hpp"
#include "backend/gen_reg_allocation.hpp"
#include "sys/cvar.hpp"
#include "ir/function.hpp"
#include "ir/value.hpp"
#include <cstring>

namespace gbe
{
  void Gen8Context::emitSLMOffset(void) {
    return;
  }

  uint32_t Gen8Context::alignScratchSize(uint32_t size){
    if(size == 0)
      return 0;
    uint32_t i = 1024;
    while(i < size) i *= 2;
    return i;
  }

  void Gen8Context::newSelection(void) {
    this->sel = GBE_NEW(Selection8, *this);
  }

  void Gen8Context::emitUnaryInstruction(const SelectionInstruction &insn)
  {
    switch (insn.opcode) {
      case SEL_OP_CONVI64_TO_I:
        /* Should never come to here, just use the common OPCODE. */
        GBE_ASSERT(0);
        break;
      default:
        GenContext::emitUnaryInstruction(insn);
    }
  }

  void Gen8Context::emitUnaryWithTempInstruction(const SelectionInstruction &insn)
  {
    switch (insn.opcode) {
      case SEL_OP_CONVI_TO_I64:
        /* Should never come to here, just use the common OPCODE. */
        GBE_ASSERT(0);
        break;
      default:
        GenContext::emitUnaryWithTempInstruction(insn);
    }
  }

  void Gen8Context::emitBinaryInstruction(const SelectionInstruction &insn) {
    switch (insn.opcode) {
      case SEL_OP_SEL_INT64:
      case SEL_OP_I64AND:
      case SEL_OP_I64OR:
      case SEL_OP_I64XOR:
        /* Should never come to here, just use the common OPCODE. */
        GBE_ASSERT(0);
        break;
      default:
        GenContext::emitBinaryInstruction(insn);
    }
  }

  void Gen8Context::emitBinaryWithTempInstruction(const SelectionInstruction &insn)
  {
    switch (insn.opcode) {
      case SEL_OP_I64ADD:
      case SEL_OP_I64SUB:
        /* Should never come to here, just use the common OPCODE. */
        GBE_ASSERT(0);
        break;
      default:
        GenContext::emitBinaryWithTempInstruction(insn);
    }
  }

  static void calculateFullU64MUL(GenEncoder* p, GenRegister src0, GenRegister src1, GenRegister dst_h,
                                  GenRegister dst_l, GenRegister s0l_s1h, GenRegister s0h_s1l)
  {
    src0.type = src1.type = GEN_TYPE_UD;
    dst_h.type = dst_l.type = GEN_TYPE_UL;
    s0l_s1h.type = s0h_s1l.type = GEN_TYPE_UL;

    GenRegister s0l = src0.hstride == GEN_HORIZONTAL_STRIDE_0 ?
      GenRegister::retype(src0, GEN_TYPE_UD) : GenRegister::unpacked_ud(src0.nr, src0.subnr);
    GenRegister s1l = src1.hstride == GEN_HORIZONTAL_STRIDE_0 ?
      GenRegister::retype(src1, GEN_TYPE_UD)  : GenRegister::unpacked_ud(src1.nr, src1.subnr);
    GenRegister s0h = GenRegister::offset(s0l, 0, 4);
    GenRegister s1h = GenRegister::offset(s1l, 0, 4);

    /* Low 32 bits X low 32 bits. */
    p->MUL(dst_l, s0l, s1l);
    /* High 32 bits X High 32 bits. */
    p->MUL(dst_h, s0h, s1h);
    /* Low 32 bits X high 32 bits. */
    p->MUL(s0l_s1h, s0l, s1h);
    /* High 32 bits X low 32 bits. */
    p->MUL(s0h_s1l, s0h, s1l);

    /*  Because the max product of s0l*s1h is (2^N - 1) * (2^N - 1) = 2^2N + 1 - 2^(N+1), here N = 32
        The max of addding 2 32bits integer to it is
        2^2N + 1 - 2^(N+1) + 2*(2^N - 1) = 2^2N - 1
        which means the product s0h_s1l adds dst_l's high 32 bits and then adds s0l_s1h's low 32 bits will not
        overflow and have no carry.
        By this manner, we can avoid using acc register, which has a lot of restrictions. */

    GenRegister dst_l_h = dst_l.hstride == GEN_HORIZONTAL_STRIDE_0 ? GenRegister::retype(dst_l, GEN_TYPE_UD) :
      GenRegister::unpacked_ud(dst_l.nr, dst_l.subnr + 1);
    p->ADD(s0h_s1l, s0h_s1l, dst_l_h);
    GenRegister s0l_s1h_l = s0l_s1h.hstride == GEN_HORIZONTAL_STRIDE_0 ? GenRegister::retype(s0l_s1h, GEN_TYPE_UD) :
      GenRegister::unpacked_ud(s0l_s1h.nr, s0l_s1h.subnr);
    p->ADD(s0h_s1l, s0h_s1l, s0l_s1h_l);
    GenRegister s0l_s1h_h = s0l_s1h.hstride == GEN_HORIZONTAL_STRIDE_0 ? GenRegister::retype(s0l_s1h, GEN_TYPE_UD) :
      GenRegister::unpacked_ud(s0l_s1h.nr, s0l_s1h.subnr + 1);
    p->ADD(dst_h, dst_h, s0l_s1h_h);

    // No longer need s0l_s1h
    GenRegister tmp = s0l_s1h;

    p->SHL(tmp, s0h_s1l, GenRegister::immud(32));
    GenRegister tmp_unpacked = tmp.hstride == GEN_HORIZONTAL_STRIDE_0 ? GenRegister::retype(tmp, GEN_TYPE_UD) :
      GenRegister::unpacked_ud(tmp.nr, tmp.subnr + 1);
    p->MOV(dst_l_h, tmp_unpacked);

    p->SHR(tmp, s0h_s1l, GenRegister::immud(32));
    p->ADD(dst_h, dst_h, tmp);
  }

  static void calculateFullS64MUL(GenEncoder* p, GenRegister src0, GenRegister src1, GenRegister dst_h,
                                  GenRegister dst_l, GenRegister s0_abs, GenRegister s1_abs, 
                                  GenRegister tmp0, GenRegister tmp1, GenRegister sign, GenRegister flagReg)
  {
    tmp0.type = tmp1.type = GEN_TYPE_UL;
    sign.type = GEN_TYPE_UL;
    src0.type = src1.type = GEN_TYPE_UL;
    /* First, need to get the sign. */
    p->SHR(tmp0, src0, GenRegister::immud(63));
    p->SHR(tmp1, src1, GenRegister::immud(63));
    p->XOR(sign, tmp0, tmp1);

    src0.type = src1.type = GEN_TYPE_L;

    tmp0.type = tmp1.type = GEN_TYPE_UL;
    s0_abs.type = s1_abs.type = GEN_TYPE_L;
    p->MOV(s0_abs, GenRegister::abs(src0));
    p->MOV(s1_abs, GenRegister::abs(src1));
    calculateFullU64MUL(p, s0_abs, s1_abs, dst_h, dst_l, tmp0, tmp1);

    p->push();
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.noMask = 1;
    p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
    p->CMP(GEN_CONDITIONAL_NZ, sign, GenRegister::immud(0), tmp0);
    p->curr.noMask = 0;
    p->curr.predicate = GEN_PREDICATE_NORMAL;

    /* Calculate the neg for the whole 128 bits. */
    dst_l.type = GEN_TYPE_UL;
    dst_h.type = GEN_TYPE_L;
    p->NOT(dst_l, dst_l);
    p->NOT(dst_h, dst_h);
    p->ADD(dst_l, dst_l, GenRegister::immud(0x01));
    p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
    p->CMP(GEN_CONDITIONAL_Z, dst_l, GenRegister::immud(0), tmp0);
    p->ADD(dst_h, dst_h, GenRegister::immud(0x01));
    p->pop();
  }

  void Gen8Context::emitI64MULHIInstruction(const SelectionInstruction &insn)
  {
    GenRegister src0 = ra->genReg(insn.src(0));
    GenRegister src1 = ra->genReg(insn.src(1));
    GenRegister dst_h = ra->genReg(insn.dst(0));
    GenRegister dst_l = ra->genReg(insn.dst(1));
    GenRegister s0_abs = ra->genReg(insn.dst(2));
    GenRegister s1_abs = ra->genReg(insn.dst(3));
    GenRegister tmp0 = ra->genReg(insn.dst(4));
    GenRegister tmp1 = ra->genReg(insn.dst(5));
    GenRegister sign = ra->genReg(insn.dst(6));
    GenRegister flagReg = GenRegister::flag(insn.state.flag, insn.state.subFlag);

    if(src0.type == GEN_TYPE_UL) {
      GBE_ASSERT(src1.type == GEN_TYPE_UL);
      calculateFullU64MUL(p, src0, src1, dst_h, dst_l, tmp0, tmp1);
    } else {
      GBE_ASSERT(src0.type == GEN_TYPE_L);
      GBE_ASSERT(src1.type == GEN_TYPE_L);
      calculateFullS64MUL(p, src0, src1, dst_h, dst_l, s0_abs, s1_abs, tmp0,
                          tmp1, sign, flagReg);
    }
  }

  void Gen8Context::emitI64MADSATInstruction(const SelectionInstruction &insn)
  {
    GenRegister src0 = ra->genReg(insn.src(0));
    GenRegister src1 = ra->genReg(insn.src(1));
    GenRegister src2 = ra->genReg(insn.src(2));
    GenRegister dst_l = ra->genReg(insn.dst(0));
    GenRegister dst_h = ra->genReg(insn.dst(1));
    GenRegister s0_abs = ra->genReg(insn.dst(2));
    GenRegister s1_abs = ra->genReg(insn.dst(3));
    GenRegister tmp0 = ra->genReg(insn.dst(4));
    GenRegister tmp1 = ra->genReg(insn.dst(5));
    GenRegister sign = ra->genReg(insn.dst(6));
    GenRegister flagReg = GenRegister::flag(insn.state.flag, insn.state.subFlag);

    if (src0.type == GEN_TYPE_UL) {
      /* Always should be the same long type. */
      GBE_ASSERT(src1.type == GEN_TYPE_UL);
      GBE_ASSERT(src2.type == GEN_TYPE_UL);
      dst_l.type = dst_h.type = GEN_TYPE_UL;
      tmp0.type = tmp1.type = GEN_TYPE_UL;
      calculateFullU64MUL(p, src0, src1, dst_h, dst_l, tmp0, tmp1);

      /* Inplement the logic:
      dst_l += src2;
      if (dst_h)
        dst_l = 0xFFFFFFFFFFFFFFFFULL;
      if (dst_l < src2)  // carry if overflow
        dst_l = 0xFFFFFFFFFFFFFFFFULL;
      */
      p->ADD(dst_l, dst_l, src2);

      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_NZ, dst_h, GenRegister::immud(0), tmp0);
      p->curr.noMask = 0;
      p->MOV(dst_l, GenRegister::immuint64(0xFFFFFFFFFFFFFFFF));
      p->pop();

      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_L, dst_l, src2, tmp0);
      p->curr.noMask = 0;
      p->MOV(dst_l, GenRegister::immuint64(0xFFFFFFFFFFFFFFFF));
      p->pop();
    } else {
      GBE_ASSERT(src0.type == GEN_TYPE_L);
      GBE_ASSERT(src1.type == GEN_TYPE_L);
      GBE_ASSERT(src2.type == GEN_TYPE_L);

      calculateFullS64MUL(p, src0, src1, dst_h, dst_l, s0_abs, s1_abs, tmp0,
                          tmp1, sign, flagReg);

      GenRegister sum = sign;
      sum.type = GEN_TYPE_UL;
      src2.type = GEN_TYPE_L;
      dst_l.type = GEN_TYPE_UL;
      p->NOP();
      p->ADD(sum, src2, dst_l);

      /* Implement this logic:
      if(src2 >= 0) {
        if(dst_l > sum) {
          dst_h++;
          if(CL_LONG_MIN == dst_h) {
            dst_h = CL_LONG_MAX;
            sum = CL_ULONG_MAX;
          }
        }
      } */
      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_GE, src2, GenRegister::immud(0), tmp1);
      p->curr.noMask = 0;
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->CMP(GEN_CONDITIONAL_G, dst_l, sum, tmp1);
      p->ADD(dst_h, dst_h, GenRegister::immud(1));
      p->MOV(tmp0, GenRegister::immint64(-0x7FFFFFFFFFFFFFFFLL - 1LL));
      p->CMP(GEN_CONDITIONAL_EQ, dst_h, tmp0, tmp1);
      p->MOV(dst_h, GenRegister::immint64(0x7FFFFFFFFFFFFFFFLL));
      p->MOV(sum, GenRegister::immuint64(0xFFFFFFFFFFFFFFFFULL));
      p->pop();
      p->NOP();

      /* Implement this logic:
      else {
        if(dst_l < sum) {
          dst_h--;
          if(CL_LONG_MAX == dst_h) {
            dst_h = CL_LONG_MIN;
            sum = 0;
          }
        }
      } */
      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_L, src2, GenRegister::immud(0), tmp1);
      p->curr.noMask = 0;
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->CMP(GEN_CONDITIONAL_L, dst_l, sum, tmp1);
      p->ADD(dst_h, dst_h, GenRegister::immd(-1));
      p->MOV(tmp0, GenRegister::immint64(0x7FFFFFFFFFFFFFFFLL));
      p->CMP(GEN_CONDITIONAL_EQ, dst_h, tmp0, tmp1);
      p->MOV(dst_h, GenRegister::immint64(-0x7FFFFFFFFFFFFFFFLL - 1LL));
      p->MOV(sum, GenRegister::immud(0));
      p->pop();
      p->NOP();

      /* saturate logic:
      if(dst_h > 0)
        sum = CL_LONG_MAX;
      else if(dst_h < -1)
        sum = CL_LONG_MIN;
      cl_long result = (cl_long) sum; */
      p->MOV(dst_l, sum);

      dst_h.type = GEN_TYPE_L;
      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_G, dst_h, GenRegister::immud(0), tmp1);
      p->curr.noMask = 0;
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(dst_l, GenRegister::immint64(0x7FFFFFFFFFFFFFFFLL));
      p->pop();
      p->NOP();

      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_L, dst_h, GenRegister::immd(-1), tmp1);
      p->curr.noMask = 0;
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(dst_l, GenRegister::immint64(-0x7FFFFFFFFFFFFFFFLL - 1LL));
      p->pop();
      p->NOP();
    }
  }

  void Gen8Context::emitI64MULInstruction(const SelectionInstruction &insn)
  {
    GenRegister src0 = ra->genReg(insn.src(0));
    GenRegister src1 = ra->genReg(insn.src(1));
    GenRegister dst = ra->genReg(insn.dst(0));
    GenRegister res = ra->genReg(insn.dst(1));

    src0.type = src1.type = GEN_TYPE_UD;
    dst.type = GEN_TYPE_UL;
    res.type = GEN_TYPE_UL;

    /* Low 32 bits X low 32 bits. */
    GenRegister s0l = src0.hstride == GEN_HORIZONTAL_STRIDE_0 ?
      GenRegister::retype(src0, GEN_TYPE_UD) : GenRegister::unpacked_ud(src0.nr, src0.subnr);
    GenRegister s1l = src1.hstride == GEN_HORIZONTAL_STRIDE_0 ?
      GenRegister::retype(src1, GEN_TYPE_UD)  : GenRegister::unpacked_ud(src1.nr, src1.subnr);
    p->MUL(dst, s0l, s1l);

    /* Low 32 bits X high 32 bits. */
    GenRegister s1h = GenRegister::offset(s1l, 0, 4);
    p->MUL(res, s0l, s1h);
    p->SHL(res, res, GenRegister::immud(32));
    p->ADD(dst, dst, res);

    /* High 32 bits X low 32 bits. */
    GenRegister s0h = GenRegister::offset(s0l, 0, 4);
    p->MUL(res, s0h, s1l);
    p->SHL(res, res, GenRegister::immud(32));
    p->ADD(dst, dst, res);
  }

  void Gen8Context::emitI64HADDInstruction(const SelectionInstruction &insn)
  {
    GenRegister src0 = ra->genReg(insn.src(0));
    GenRegister src1 = ra->genReg(insn.src(1));
    GenRegister dst = ra->genReg(insn.dst(0));
    GenRegister tmp0 = ra->genReg(insn.dst(1));
    GenRegister tmp1 = ra->genReg(insn.dst(2));
    GenRegister tmp_dst = ra->genReg(insn.dst(3));
    int execWidth = p->curr.execWidth;

    /* Src0 and Src1 are always unsigned long type.*/
    GBE_ASSERT(src0.type == GEN_TYPE_UL && src1.type == GEN_TYPE_UL);
    dst.type = src0.type;
    tmp0.type = tmp1.type = GEN_TYPE_UD;
    tmp_dst.type = GEN_TYPE_UL;

    GBE_ASSERT(tmp_dst.subnr == 0);
    GenRegister dl = tmp_dst.hstride == GEN_HORIZONTAL_STRIDE_0 ? GenRegister::retype(tmp_dst, GEN_TYPE_UD) :
      GenRegister::retype(GenRegister::ud16grf(tmp_dst.nr, tmp_dst.subnr), GEN_TYPE_UD);
    GenRegister dh = tmp_dst.hstride == GEN_HORIZONTAL_STRIDE_0 ?
      GenRegister::retype(GenRegister::offset(tmp_dst, 0, 4), GEN_TYPE_UD) :
      GenRegister::retype(GenRegister::ud16grf(tmp_dst.nr + execWidth / 8, tmp_dst.subnr), GEN_TYPE_UD);
    GenRegister s0l = src0.hstride == GEN_HORIZONTAL_STRIDE_0 ?
      GenRegister::retype(src0, GEN_TYPE_UD) : GenRegister::unpacked_ud(src0.nr, src0.subnr);
    GenRegister s0h = src0.hstride == GEN_HORIZONTAL_STRIDE_0 ?
      GenRegister::retype(GenRegister::offset(src0, 0, 4), GEN_TYPE_UD) :
      GenRegister::unpacked_ud(src0.nr, src0.subnr + 1);
    GenRegister s1l = src1.hstride == GEN_HORIZONTAL_STRIDE_0 ?
      GenRegister::retype(src1, GEN_TYPE_UD) : GenRegister::unpacked_ud(src1.nr, src1.subnr);
    GenRegister s1h = src1.hstride == GEN_HORIZONTAL_STRIDE_0 ?
      GenRegister::retype(GenRegister::offset(src1, 0, 4), GEN_TYPE_UD) :
      GenRegister::unpacked_ud(src1.nr, src1.subnr + 1);

    GenRegister acc0 = GenRegister::retype(GenRegister::acc(), GEN_TYPE_D);
    p->push();
    p->curr.execWidth = 8;
    p->ADDC(dl, s0l, s1l);
    p->MOV(tmp0, acc0);
    p->ADDC(dh, s0h, s1h);
    p->MOV(tmp1, acc0);
    p->ADDC(dh, dh, tmp0);
    p->MOV(tmp0, acc0);
    p->ADD(tmp1, tmp0, tmp1);

    if (execWidth == 16) {
      p->curr.quarterControl = 1;
      p->ADDC(GenRegister::Qn(dl, 1), GenRegister::Qn(s0l, 1), GenRegister::Qn(s1l, 1));
      p->MOV(GenRegister::Qn(tmp0, 1), acc0);
      p->ADDC(GenRegister::Qn(dh, 1), GenRegister::Qn(s0h, 1), GenRegister::Qn(s1h, 1));
      p->MOV(GenRegister::Qn(tmp1, 1), acc0);
      p->ADDC(GenRegister::Qn(dh, 1), GenRegister::Qn(dh, 1), GenRegister::Qn(tmp0, 1));
      p->MOV(GenRegister::Qn(tmp0, 1), acc0);
      p->ADD(GenRegister::Qn(tmp1, 1), GenRegister::Qn(tmp0, 1), GenRegister::Qn(tmp1, 1));
    }
    p->pop();

    packLongVec(GenRegister::retype(tmp_dst, GEN_TYPE_UD), GenRegister::retype(dst, GEN_TYPE_UD), execWidth);

    p->SHR(dst, dst, GenRegister::immud(1));
    p->SHL(tmp_dst, tmp1, GenRegister::immud(63));
    p->ADD(dst, dst, tmp_dst);
  }

  void Gen8Context::emitI64RHADDInstruction(const SelectionInstruction &insn)
  {
    GenRegister src0 = ra->genReg(insn.src(0));
    GenRegister src1 = ra->genReg(insn.src(1));
    GenRegister dst = ra->genReg(insn.dst(0));
    GenRegister tmp0 = ra->genReg(insn.dst(1));
    GenRegister tmp1 = ra->genReg(insn.dst(2));
    GenRegister tmp_dst = ra->genReg(insn.dst(3));
    int execWidth = p->curr.execWidth;

    /* Src0 and Src1 are always unsigned long type.*/
    GBE_ASSERT(src0.type == GEN_TYPE_UL && src1.type == GEN_TYPE_UL);
    dst.type = src0.type;
    tmp0.type = tmp1.type = GEN_TYPE_UD;
    tmp_dst.type = GEN_TYPE_UL;

    GBE_ASSERT(tmp_dst.subnr == 0);
    GenRegister dl = tmp_dst.hstride == GEN_HORIZONTAL_STRIDE_0 ? GenRegister::retype(tmp_dst, GEN_TYPE_UD) :
      GenRegister::retype(GenRegister::ud16grf(tmp_dst.nr, tmp_dst.subnr), GEN_TYPE_UD);
    GenRegister dh = tmp_dst.hstride == GEN_HORIZONTAL_STRIDE_0 ?
      GenRegister::retype(GenRegister::offset(tmp_dst, 0, 4), GEN_TYPE_UD) :
      GenRegister::retype(GenRegister::ud16grf(tmp_dst.nr + execWidth / 8, tmp_dst.subnr), GEN_TYPE_UD);
    GenRegister s0l = src0.hstride == GEN_HORIZONTAL_STRIDE_0 ?
      GenRegister::retype(src0, GEN_TYPE_UD) : GenRegister::unpacked_ud(src0.nr, src0.subnr);
    GenRegister s0h = src0.hstride == GEN_HORIZONTAL_STRIDE_0 ?
      GenRegister::retype(GenRegister::offset(src0, 0, 4), GEN_TYPE_UD) :
      GenRegister::unpacked_ud(src0.nr, src0.subnr + 1);
    GenRegister s1l = src1.hstride == GEN_HORIZONTAL_STRIDE_0 ?
      GenRegister::retype(src1, GEN_TYPE_UD) : GenRegister::unpacked_ud(src1.nr, src1.subnr);
    GenRegister s1h = src1.hstride == GEN_HORIZONTAL_STRIDE_0 ?
      GenRegister::retype(GenRegister::offset(src1, 0, 4), GEN_TYPE_UD) :
      GenRegister::unpacked_ud(src1.nr, src1.subnr + 1);

    GenRegister acc0 = GenRegister::retype(GenRegister::acc(), GEN_TYPE_D);
    p->push();
    p->curr.execWidth = 8;
    p->ADDC(dl, s0l, s1l);
    p->MOV(tmp0, acc0);
    p->ADDC(dl, dl, GenRegister::immud(1));
    p->MOV(tmp1, acc0);
    p->ADD(tmp0, tmp0, tmp1);

    p->ADDC(dh, s0h, s1h);
    p->MOV(tmp1, acc0);
    p->ADDC(dh, dh, tmp0);
    p->MOV(tmp0, acc0);
    p->ADD(tmp1, tmp0, tmp1);

    if (execWidth == 16) {
      p->curr.quarterControl = 1;
      p->ADDC(GenRegister::Qn(dl, 1), GenRegister::Qn(s0l, 1), GenRegister::Qn(s1l, 1));
      p->MOV(GenRegister::Qn(tmp0, 1), acc0);
      p->ADDC(GenRegister::Qn(dl, 1), GenRegister::Qn(dl, 1), GenRegister::immud(1));
      p->MOV(GenRegister::Qn(tmp1, 1), acc0);
      p->ADD(GenRegister::Qn(tmp0, 1), GenRegister::Qn(tmp0, 1), GenRegister::Qn(tmp1, 1));

      p->ADDC(GenRegister::Qn(dh, 1), GenRegister::Qn(s0h, 1), GenRegister::Qn(s1h, 1));
      p->MOV(GenRegister::Qn(tmp1, 1), acc0);
      p->ADDC(GenRegister::Qn(dh, 1), GenRegister::Qn(dh, 1), GenRegister::Qn(tmp0, 1));
      p->MOV(GenRegister::Qn(tmp0, 1), acc0);
      p->ADD(GenRegister::Qn(tmp1, 1), GenRegister::Qn(tmp0, 1), GenRegister::Qn(tmp1, 1));
    }
    p->pop();

    packLongVec(GenRegister::retype(tmp_dst, GEN_TYPE_UD), GenRegister::retype(dst, GEN_TYPE_UD), execWidth);

    p->SHR(dst, dst, GenRegister::immud(1));
    p->SHL(tmp_dst, tmp1, GenRegister::immud(63));
    p->ADD(dst, dst, tmp_dst);
  }

  void Gen8Context::emitI64DIVREMInstruction(const SelectionInstruction &cnst_insn)
  {
    SelectionInstruction* insn = const_cast<SelectionInstruction*>(&cnst_insn);
    GenRegister packed_src0 = ra->genReg(insn->src(0));
    GenRegister packed_src1 = ra->genReg(insn->src(1));
    GenRegister dst = ra->genReg(insn->dst(0));
    int tmp_reg_n = 14;

    if (packed_src0.hstride != GEN_HORIZONTAL_STRIDE_0) {
      GenRegister unpacked_src0 = ra->genReg(insn->dst(tmp_reg_n));
      unpackLongVec(packed_src0, unpacked_src0, p->curr.execWidth);
      tmp_reg_n++;
      insn->src(0) = unpacked_src0;
    }
    if (packed_src1.hstride != GEN_HORIZONTAL_STRIDE_0) {
      GenRegister unpacked_src1 = ra->genReg(insn->dst(tmp_reg_n));
      unpackLongVec(packed_src1, unpacked_src1, p->curr.execWidth);
      tmp_reg_n++;
      insn->src(1) = unpacked_src1;
    }
    GBE_ASSERT(tmp_reg_n <= insn->dstNum);

    GenContext::emitI64DIVREMInstruction(*insn);

    if (dst.hstride != GEN_HORIZONTAL_STRIDE_0) {
      GenRegister dst_packed = ra->genReg(insn->dst(14));
      packLongVec(dst, dst_packed, p->curr.execWidth);
      p->MOV(dst, dst_packed);
    }
  }

  void Gen8Context::packLongVec(GenRegister unpacked, GenRegister packed, uint32_t simd)
  {
    GBE_ASSERT(packed.subnr == 0);
    GBE_ASSERT(unpacked.subnr == 0);

    unpacked = GenRegister::retype(unpacked, GEN_TYPE_UD);
    packed = GenRegister::retype(packed, GEN_TYPE_UD);

    if (simd == 16) {
      p->push();
      p->curr.execWidth = 8;
      p->MOV(GenRegister::h2(packed), unpacked);
      p->MOV(GenRegister::h2(GenRegister::offset(packed, 0, typeSize(GEN_TYPE_UD))),
             GenRegister::offset(unpacked, 2));
      p->curr.quarterControl = 1;
      p->MOV(GenRegister::h2(GenRegister::offset(packed, 2, 0)), GenRegister::offset(unpacked, 1));
      p->MOV(GenRegister::h2(GenRegister::offset(packed, 2, typeSize(GEN_TYPE_UD))),
             GenRegister::offset(unpacked, 3));
      p->pop();
    } else {
      GBE_ASSERT(simd == 8);
      p->MOV(GenRegister::h2(packed), unpacked);
      p->MOV(GenRegister::h2(GenRegister::offset(packed, 0, typeSize(GEN_TYPE_UD))),
             GenRegister::offset(unpacked, 1));
    }
  }

  void Gen8Context::unpackLongVec(GenRegister packed, GenRegister unpacked, uint32_t simd)
  {
    GBE_ASSERT(packed.subnr == 0);
    GBE_ASSERT(unpacked.subnr == 0);

    unpacked = GenRegister::retype(unpacked, GEN_TYPE_UD);
    packed = GenRegister::retype(packed, GEN_TYPE_UD);

    packed.vstride = GEN_VERTICAL_STRIDE_8;
    packed.width = GEN_WIDTH_4;

    p->push();
    p->curr.execWidth = 8;
    if (simd == 16) {
      p->MOV(unpacked, GenRegister::h2(packed));
      p->MOV(GenRegister::offset(unpacked, 2),
             GenRegister::h2(GenRegister::offset(packed, 0, typeSize(GEN_TYPE_UD))));

      p->curr.quarterControl = 1;
      p->MOV(GenRegister::offset(unpacked, 1), GenRegister::h2(GenRegister::offset(packed, 2)));
      p->MOV(GenRegister::offset(unpacked, 3),
             GenRegister::h2(GenRegister::offset(packed, 2, typeSize(GEN_TYPE_UD))));
    } else {
      GBE_ASSERT(simd == 8);
      p->MOV(unpacked, GenRegister::h2(packed));
      p->MOV(GenRegister::offset(unpacked, 1),
             GenRegister::h2(GenRegister::offset(packed, 0, typeSize(GEN_TYPE_UD))));
    }
    p->pop();
  }

  void Gen8Context::emitRead64Instruction(const SelectionInstruction &insn)
  {
    const uint32_t bti = insn.getbti();
    const uint32_t elemNum = insn.extra.elem;
    GBE_ASSERT(elemNum == 1);

    const GenRegister addr = ra->genReg(insn.src(0));
    const GenRegister tmp_dst = ra->genReg(insn.dst(0));

    /* Because BDW's store and load send instructions for 64 bits require the bti to be surfaceless,
       which we can not accept. We just fallback to 2 DW untyperead here. */
    p->UNTYPED_READ(tmp_dst, addr, bti, elemNum*2);

    for (uint32_t elemID = 0; elemID < elemNum; elemID++) {
      GenRegister long_tmp = ra->genReg(insn.dst(elemID));
      GenRegister the_long = ra->genReg(insn.dst(elemID + elemNum));
      this->packLongVec(long_tmp, the_long, p->curr.execWidth);
    }
  }

  void Gen8Context::emitWrite64Instruction(const SelectionInstruction &insn)
  {
    const uint32_t bti = insn.getbti();
    const uint32_t elemNum = insn.extra.elem;
    GBE_ASSERT(elemNum == 1);

    const GenRegister addr = ra->genReg(insn.src(elemNum));

    /* Because BDW's store and load send instructions for 64 bits require the bti to be surfaceless,
       which we can not accept. We just fallback to 2 DW untypewrite here. */
    for (uint32_t elemID = 0; elemID < elemNum; elemID++) {
      GenRegister the_long = ra->genReg(insn.src(elemID));
      GenRegister long_tmp = ra->genReg(insn.src(elemNum + 1 + elemID));
      this->unpackLongVec(the_long, long_tmp, p->curr.execWidth);
    }

    p->UNTYPED_WRITE(addr, bti, elemNum*2);
  }
}

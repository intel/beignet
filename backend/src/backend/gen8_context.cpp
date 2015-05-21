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
    GenRegister dst = ra->genReg(insn.dst(0));
    GenRegister src = ra->genReg(insn.src(0));
    GenRegister tmp = ra->genReg(insn.dst(1));
    switch (insn.opcode) {
      case SEL_OP_CONVI_TO_I64:
        /* Should never come to here, just use the common OPCODE. */
        GBE_ASSERT(0);
        break;
      case SEL_OP_BSWAP:
        {
          uint32_t simd = p->curr.execWidth;
          GBE_ASSERT(simd == 8 || simd == 16 || simd == 1);
          uint16_t new_a0[16];
          memset(new_a0, 0, sizeof(new_a0));

          GBE_ASSERT(src.type == dst.type);
          uint32_t start_addr = src.nr*32 + src.subnr;

          if (simd == 1) {
            GBE_ASSERT(src.hstride == GEN_HORIZONTAL_STRIDE_0
                && dst.hstride == GEN_HORIZONTAL_STRIDE_0);
            if (src.type == GEN_TYPE_UD || src.type == GEN_TYPE_D) {
              GBE_ASSERT(start_addr >= 0);
              new_a0[0] = start_addr + 3;
              new_a0[1] = start_addr + 2;
              new_a0[2] = start_addr + 1;
              new_a0[3] = start_addr;
              this->setA0Content(new_a0, 0, 4);

              p->push();
              p->curr.execWidth = 4;
              p->curr.predicate = GEN_PREDICATE_NONE;
              p->curr.noMask = 1;
              GenRegister ind_src = GenRegister::to_indirect1xN(GenRegister::retype(src, GEN_TYPE_UB), new_a0[0], 0);
              GenRegister dst_ = dst;
              dst_.type = GEN_TYPE_UB;
              dst_.hstride = GEN_HORIZONTAL_STRIDE_1;
              dst_.width = GEN_WIDTH_4;
              dst_.vstride = GEN_VERTICAL_STRIDE_4;
              p->MOV(dst_, ind_src);
              p->pop();
            } else if (src.type == GEN_TYPE_UW || src.type == GEN_TYPE_W) {
              p->MOV(GenRegister::retype(dst, GEN_TYPE_UB),
                  GenRegister::retype(GenRegister::offset(src, 0, 1), GEN_TYPE_UB));
              p->MOV(GenRegister::retype(GenRegister::offset(dst, 0, 1), GEN_TYPE_UB),
                  GenRegister::retype(src, GEN_TYPE_UB));
            } else {
              GBE_ASSERT(0);
            }
          } else {
            if (src.type == GEN_TYPE_UD || src.type == GEN_TYPE_D) {
              bool uniform_src = (src.hstride == GEN_HORIZONTAL_STRIDE_0);
              GBE_ASSERT(uniform_src || src.subnr == 0);
              GBE_ASSERT(dst.subnr == 0);
              GBE_ASSERT(tmp.subnr == 0);
              GBE_ASSERT(start_addr >= 0);
              new_a0[0] = start_addr + 3;
              new_a0[1] = start_addr + 2;
              new_a0[2] = start_addr + 1;
              new_a0[3] = start_addr;
              if (!uniform_src) {
                new_a0[4] = start_addr + 7;
                new_a0[5] = start_addr + 6;
                new_a0[6] = start_addr + 5;
                new_a0[7] = start_addr + 4;
                new_a0[8] = start_addr + 11;
                new_a0[9] = start_addr + 10;
                new_a0[10] = start_addr + 9;
                new_a0[11] = start_addr + 8;
                new_a0[12] = start_addr + 15;
                new_a0[13] = start_addr + 14;
                new_a0[14] = start_addr + 13;
                new_a0[15] = start_addr + 12;
              } else {
                new_a0[4] = start_addr + 3;
                new_a0[5] = start_addr + 2;
                new_a0[6] = start_addr + 1;
                new_a0[7] = start_addr;
                new_a0[8] = start_addr + 3;
                new_a0[9] = start_addr + 2;
                new_a0[10] = start_addr + 1;
                new_a0[11] = start_addr;
                new_a0[12] = start_addr + 3;
                new_a0[13] = start_addr + 2;
                new_a0[14] = start_addr + 1;
                new_a0[15] = start_addr;
              }
              this->setA0Content(new_a0, 48);

              p->push();
              p->curr.execWidth = 16;
              p->curr.predicate = GEN_PREDICATE_NONE;
              p->curr.noMask = 1;
              GenRegister ind_src = GenRegister::to_indirect1xN(GenRegister::retype(src, GEN_TYPE_UB), new_a0[0], 0);
              p->MOV(GenRegister::retype(tmp, GEN_TYPE_UB), ind_src);
              ind_src.addr_imm += 16;
              p->MOV(GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_UB), 0, 16), ind_src);
              if (simd == 16) {
                for (int i = 0; i < 2; i++) {
                  ind_src.addr_imm += 16;
                  p->MOV(GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_UB), 1, 16*i), ind_src);
                }
              }
              p->pop();

              p->MOV(dst, tmp);
            } else if (src.type == GEN_TYPE_UW || src.type == GEN_TYPE_W) {
              bool uniform_src = (src.hstride == GEN_HORIZONTAL_STRIDE_0);
              GBE_ASSERT(uniform_src || src.subnr == 0 || src.subnr == 16);
              GBE_ASSERT(dst.subnr == 0 || dst.subnr == 16);
              GBE_ASSERT(tmp.subnr == 0 || tmp.subnr == 16);
              GBE_ASSERT(start_addr >= 0);
              new_a0[0] = start_addr + 1;
              new_a0[1] = start_addr;
              if (!uniform_src) {
                new_a0[2] = start_addr + 3;
                new_a0[3] = start_addr + 2;
                new_a0[4] = start_addr + 5;
                new_a0[5] = start_addr + 4;
                new_a0[6] = start_addr + 7;
                new_a0[7] = start_addr + 6;
                new_a0[8] = start_addr + 9;
                new_a0[9] = start_addr + 8;
                new_a0[10] = start_addr + 11;
                new_a0[11] = start_addr + 10;
                new_a0[12] = start_addr + 13;
                new_a0[13] = start_addr + 12;
                new_a0[14] = start_addr + 15;
                new_a0[15] = start_addr + 14;
              } else {
                new_a0[2] = start_addr + 1;
                new_a0[3] = start_addr;
                new_a0[4] = start_addr + 1;
                new_a0[5] = start_addr;
                new_a0[6] = start_addr + 1;
                new_a0[7] = start_addr;
                new_a0[8] = start_addr + 1;
                new_a0[9] = start_addr;
                new_a0[10] = start_addr + 1;
                new_a0[11] = start_addr;
                new_a0[12] = start_addr + 1;
                new_a0[13] = start_addr;
                new_a0[14] = start_addr + 1;
                new_a0[15] = start_addr;
              }
              this->setA0Content(new_a0, 48);

              p->push();
              p->curr.execWidth = 16;
              p->curr.predicate = GEN_PREDICATE_NONE;
              p->curr.noMask = 1;
              GenRegister ind_src = GenRegister::to_indirect1xN(GenRegister::retype(src, GEN_TYPE_UB), new_a0[0], 0);
              p->MOV(GenRegister::retype(tmp, GEN_TYPE_UB), ind_src);
              if (simd == 16) {
                ind_src.addr_imm += 16;
                p->MOV(GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_UB), 0, 16), ind_src);
              }
              p->pop();

              p->MOV(dst, tmp);
            } else {
              GBE_ASSERT(0);
            }
          }
        }
        break;
      default:
        GenContext::emitUnaryWithTempInstruction(insn);
    }
  }

  void Gen8Context::emitBinaryInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src0 = ra->genReg(insn.src(0));
    const GenRegister src1 = ra->genReg(insn.src(1));
    switch (insn.opcode) {
      case SEL_OP_SEL_INT64:
      case SEL_OP_I64AND:
      case SEL_OP_I64OR:
      case SEL_OP_I64XOR:
        /* Should never come to here, just use the common OPCODE. */
        GBE_ASSERT(0);
        break;
      case SEL_OP_UPSAMPLE_LONG:
      {
        p->MOV(dst, src0);
        p->SHL(dst, dst, GenRegister::immud(32));
        p->ADD(dst, dst, src1);
        break;
      }
      case SEL_OP_SIMD_SHUFFLE:
      {
        uint32_t simd = p->curr.execWidth;
        if (src1.file == GEN_IMMEDIATE_VALUE) {
          uint32_t offset = src1.value.ud % simd;
          GenRegister reg = GenRegister::suboffset(src0, offset);
          p->MOV(dst, GenRegister::retype(GenRegister::ud1grf(reg.nr, reg.subnr / typeSize(reg.type)), reg.type));
        } else {
          uint32_t base = src0.nr * 32 + src0.subnr * 4;
          GenRegister baseReg = GenRegister::immuw(base);
          const GenRegister a0 = GenRegister::addr8(0);
          p->ADD(a0, GenRegister::unpacked_uw(src1.nr, src1.subnr / typeSize(GEN_TYPE_UW)), baseReg);
          GenRegister indirect = GenRegister::to_indirect1xN(src0, 0, 0);
          p->MOV(dst, indirect);
        }
        break;
      }
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

  void Gen8Context::emitI64ShiftInstruction(const SelectionInstruction &insn)
  {
    switch (insn.opcode) {
      case SEL_OP_I64SHL:
      case SEL_OP_I64SHR:
      case SEL_OP_I64ASR:
        /* Should never come to here, just use the common OPCODE. */
        GBE_ASSERT(0);
        break;
      default:
        GenContext::emitI64ShiftInstruction(insn);
    }
  }

  void Gen8Context::emitI64CompareInstruction(const SelectionInstruction &insn)
  {
    /* Should never come to here, just use the common OPCODE. */
    GBE_ASSERT(0);
  }

  void Gen8Context::emitI64SATADDInstruction(const SelectionInstruction &insn)
  {
    /* Should never come to here, just use the common OPCODE. */
    GBE_ASSERT(0);
  }

  void Gen8Context::emitI64SATSUBInstruction(const SelectionInstruction &insn)
  {
    /* Should never come to here, just use the common OPCODE. */
    GBE_ASSERT(0);
  }

  void Gen8Context::emitI64ToFloatInstruction(const SelectionInstruction &insn)
  {
    /* Should never come to here, just use the common OPCODE. */
    GBE_ASSERT(0);
  }

  void Gen8Context::emitFloatToI64Instruction(const SelectionInstruction &insn)
  {
    /* Should never come to here, just use the common OPCODE. */
    GBE_ASSERT(0);
  }

  static GenRegister unpacked_ud(GenRegister reg, uint32_t offset = 0)
  {
    if(reg.hstride == GEN_HORIZONTAL_STRIDE_0) {
      if(offset == 0)
        return GenRegister::retype(reg, GEN_TYPE_UD);
      else
        return GenRegister::retype(GenRegister::offset(reg, 0, typeSize(GEN_TYPE_UD)*offset), GEN_TYPE_UD);
    } else
      return GenRegister::unpacked_ud(reg.nr, reg.subnr + offset);
  }

  void Gen8Context::calculateFullU64MUL(GenRegister src0, GenRegister src1, GenRegister dst_h,
                                  GenRegister dst_l, GenRegister s0l_s1h, GenRegister s0h_s1l)
  {
    src0.type = src1.type = GEN_TYPE_UD;
    dst_h.type = dst_l.type = GEN_TYPE_UL;
    s0l_s1h.type = s0h_s1l.type = GEN_TYPE_UL;

    GenRegister s0l = unpacked_ud(src0);
    GenRegister s1l = unpacked_ud(src1);
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

    GenRegister dst_l_h = unpacked_ud(dst_l, 1);
    p->ADD(s0h_s1l, s0h_s1l, dst_l_h);
    GenRegister s0l_s1h_l = unpacked_ud(s0l_s1h);
    p->ADD(s0h_s1l, s0h_s1l, s0l_s1h_l);
    GenRegister s0l_s1h_h = unpacked_ud(s0l_s1h, 1);
    p->ADD(dst_h, dst_h, s0l_s1h_h);

    // No longer need s0l_s1h
    GenRegister tmp = s0l_s1h;

    p->SHL(tmp, s0h_s1l, GenRegister::immud(32));
    GenRegister tmp_unpacked = unpacked_ud(tmp, 1);
    p->MOV(dst_l_h, tmp_unpacked);

    p->SHR(tmp, s0h_s1l, GenRegister::immud(32));
    p->ADD(dst_h, dst_h, tmp);
  }

  void Gen8Context::calculateFullS64MUL(GenRegister src0, GenRegister src1, GenRegister dst_h,
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
    calculateFullU64MUL(s0_abs, s1_abs, dst_h, dst_l, tmp0, tmp1);

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
      calculateFullU64MUL(src0, src1, dst_h, dst_l, tmp0, tmp1);
    } else {
      GBE_ASSERT(src0.type == GEN_TYPE_L);
      GBE_ASSERT(src1.type == GEN_TYPE_L);
      calculateFullS64MUL(src0, src1, dst_h, dst_l, s0_abs, s1_abs, tmp0,
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
      calculateFullU64MUL(src0, src1, dst_h, dst_l, tmp0, tmp1);

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
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->curr.noMask = 0;
      p->MOV(dst_l, GenRegister::immuint64(0xFFFFFFFFFFFFFFFF));
      p->pop();

      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_L, dst_l, src2, tmp0);
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->curr.noMask = 0;
      p->MOV(dst_l, GenRegister::immuint64(0xFFFFFFFFFFFFFFFF));
      p->pop();
    } else {
      GBE_ASSERT(src0.type == GEN_TYPE_L);
      GBE_ASSERT(src1.type == GEN_TYPE_L);
      GBE_ASSERT(src2.type == GEN_TYPE_L);

      calculateFullS64MUL(src0, src1, dst_h, dst_l, s0_abs, s1_abs, tmp0,
                          tmp1, sign, flagReg);

      GenRegister sum = sign;
      sum.type = GEN_TYPE_UL;
      src2.type = GEN_TYPE_L;
      dst_l.type = GEN_TYPE_UL;
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

      /* saturate logic:
      if(dst_h > 0)
        sum = CL_LONG_MAX;
      else if (dst_h == 0 && sum > 0x7FFFFFFFFFFFFFFFLL) {
        sum = CL_LONG_MAX;
      else if (dst_h == -1 && sum < 0x8000000000000000)
        sum = CL_LONG_MIN;
      else (dst_h < -1)
        sum = CL_LONG_MIN;
      cl_long result = (cl_long) sum; */
      p->MOV(dst_l, sum);
      tmp0.type = GEN_TYPE_UL;

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

      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_EQ, dst_h, GenRegister::immd(0x0L), tmp1);
      p->curr.noMask = 0;
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(tmp0, GenRegister::immuint64(0x7FFFFFFFFFFFFFFFUL));
      p->CMP(GEN_CONDITIONAL_G, dst_l, tmp0, tmp1);
      p->MOV(dst_l, GenRegister::immint64(0x7FFFFFFFFFFFFFFFLL));
      p->pop();

      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      /* Fixme: HW bug ? 0xFFFFFFFFFFFFFFFF != 0xFFFFFFFFFFFFFFFF */
      p->ADD(tmp0, dst_h, GenRegister::immud(1));
      p->CMP(GEN_CONDITIONAL_EQ, tmp0, GenRegister::immud(0), tmp1);
      p->curr.noMask = 0;
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(tmp0, GenRegister::immuint64(0x8000000000000000UL));
      p->CMP(GEN_CONDITIONAL_L, dst_l, tmp0, tmp1);
      p->MOV(dst_l, GenRegister::immint64(-0x7FFFFFFFFFFFFFFFLL - 1LL));
      p->pop();

      p->push();
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->curr.useFlag(flagReg.flag_nr(), flagReg.flag_subnr());
      p->CMP(GEN_CONDITIONAL_L, dst_h, GenRegister::immd(-1), tmp1);
      p->curr.noMask = 0;
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      p->MOV(dst_l, GenRegister::immint64(-0x7FFFFFFFFFFFFFFFLL - 1LL));
      p->pop();
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
    GenRegister s0l = unpacked_ud(src0);
    GenRegister s1l = unpacked_ud(src1);
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

    /* Src0 and Src1 are always unsigned long type.*/
    GBE_ASSERT(src0.type == GEN_TYPE_UL && src1.type == GEN_TYPE_UL);
    dst.type = src0.type;
    tmp0.type = tmp1.type = GEN_TYPE_UL;

    //hadd = (src0>>1) + (src1>>1) + ((src0&0x1) & (src1&0x1))
    p->AND(tmp0, src0, GenRegister::immud(1));
    p->AND(dst, src1, tmp0);
    p->SHR(tmp0, src0, GenRegister::immud(1));
    p->SHR(tmp1, src1, GenRegister::immud(1));
    p->ADD(dst, dst, tmp0);
    p->ADD(dst, dst, tmp1);
  }

  void Gen8Context::emitI64RHADDInstruction(const SelectionInstruction &insn)
  {
    GenRegister src0 = ra->genReg(insn.src(0));
    GenRegister src1 = ra->genReg(insn.src(1));
    GenRegister dst = ra->genReg(insn.dst(0));
    GenRegister tmp0 = ra->genReg(insn.dst(1));
    GenRegister tmp1 = ra->genReg(insn.dst(2));

    /* Src0 and Src1 are always unsigned long type.*/
    GBE_ASSERT(src0.type == GEN_TYPE_UL && src1.type == GEN_TYPE_UL);
    dst.type = src0.type;
    tmp0.type = tmp1.type = GEN_TYPE_UL;

    //rhadd = (src0>>1) + (src1>>1) + ((src0&0x1) | (src1&0x1))
    p->AND(tmp0, src0, GenRegister::immud(1));
    p->AND(tmp1, src1, GenRegister::immud(1));
    p->OR(dst, tmp0, tmp1);
    p->SHR(tmp0, src0, GenRegister::immud(1));
    p->SHR(tmp1, src1, GenRegister::immud(1));
    p->ADD(dst, dst, tmp0);
    p->ADD(dst, dst, tmp1);
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
    bool isScalar = false;
    if (unpacked.hstride == GEN_HORIZONTAL_STRIDE_0)
      isScalar = true;

    GBE_ASSERT(packed.subnr == 0);
    GBE_ASSERT(packed.hstride != GEN_HORIZONTAL_STRIDE_0);
    GBE_ASSERT(unpacked.subnr == 0 || isScalar);

    unpacked = GenRegister::retype(unpacked, GEN_TYPE_UD);
    packed = GenRegister::retype(packed, GEN_TYPE_UD);

    if (isScalar) {
      p->MOV(packed, unpacked);
    } else {
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
  }

  void Gen8Context::unpackLongVec(GenRegister packed, GenRegister unpacked, uint32_t simd)
  {
    bool isScalar = false;
    if (packed.hstride == GEN_HORIZONTAL_STRIDE_0)
      isScalar = true;

    GBE_ASSERT(packed.subnr == 0 || isScalar);
    GBE_ASSERT(unpacked.hstride != GEN_HORIZONTAL_STRIDE_0);
    GBE_ASSERT(unpacked.subnr == 0);

    unpacked = GenRegister::retype(unpacked, GEN_TYPE_UD);
    packed = GenRegister::retype(packed, GEN_TYPE_UD);

    if (isScalar) {
      p->MOV(unpacked, packed);

      if (simd == 16) {
        p->MOV(GenRegister::offset(unpacked, 2), GenRegister::offset(packed, 0, typeSize(GEN_TYPE_UD)));
      } else {
        p->MOV(GenRegister::offset(unpacked, 1), GenRegister::offset(packed, 0, typeSize(GEN_TYPE_UD)));
      }
    } else {
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
  }
  void Gen8Context::emitRead64Instruction(const SelectionInstruction &insn)
  {
    const uint32_t elemNum = insn.extra.elem;
    GBE_ASSERT(elemNum == 1);

    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    const GenRegister bti = ra->genReg(insn.src(1));

    /* Because BDW's store and load send instructions for 64 bits require the bti to be surfaceless,
       which we can not accept. We just fallback to 2 DW untyperead here. */
    if (bti.file == GEN_IMMEDIATE_VALUE) {
      p->UNTYPED_READ(dst, src, bti, 2*elemNum);
    } else {
      const GenRegister tmp = ra->genReg(insn.dst(2*elemNum));
      unsigned desc = p->generateUntypedReadMessageDesc(0, 2*elemNum);

      unsigned jip0 = beforeMessage(insn, bti, tmp, desc);

      //predicated load
      p->push();
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(insn.state.flag, insn.state.subFlag);
        p->UNTYPED_READ(dst, src, GenRegister::retype(GenRegister::addr1(0), GEN_TYPE_UD), 2*elemNum);
      p->pop();
      afterMessage(insn, bti, tmp, jip0);
    }

    for (uint32_t elemID = 0; elemID < elemNum; elemID++) {
      GenRegister long_tmp = ra->genReg(insn.dst(elemID));
      GenRegister the_long = ra->genReg(insn.dst(elemID + elemNum));
      this->packLongVec(long_tmp, the_long, p->curr.execWidth);
    }
  }

  void Gen8Context::emitWrite64Instruction(const SelectionInstruction &insn)
  {
    const uint32_t elemNum = insn.extra.elem;
    GBE_ASSERT(elemNum == 1);
    const GenRegister addr = ra->genReg(insn.src(elemNum));
    const GenRegister bti = ra->genReg(insn.src(elemNum*2+1));

    /* Because BDW's store and load send instructions for 64 bits require the bti to be surfaceless,
       which we can not accept. We just fallback to 2 DW untypewrite here. */
    for (uint32_t elemID = 0; elemID < elemNum; elemID++) {
      GenRegister the_long = ra->genReg(insn.src(elemID));
      GenRegister long_tmp = ra->genReg(insn.src(elemNum + 1 + elemID));
      this->unpackLongVec(the_long, long_tmp, p->curr.execWidth);
    }

    if (bti.file == GEN_IMMEDIATE_VALUE) {
      p->UNTYPED_WRITE(addr, bti, elemNum*2);
    } else {
      const GenRegister tmp = ra->genReg(insn.dst(elemNum));
      unsigned desc = p->generateUntypedWriteMessageDesc(0, elemNum*2);

      unsigned jip0 = beforeMessage(insn, bti, tmp, desc);

      //predicated load
      p->push();
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(insn.state.flag, insn.state.subFlag);
        p->UNTYPED_WRITE(addr, GenRegister::addr1(0), elemNum*2);
      p->pop();
      afterMessage(insn, bti, tmp, jip0);
    }
  }
  void Gen8Context::emitPackLongInstruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.src(0));
    const GenRegister dst = ra->genReg(insn.dst(0));

    /* Scalar register need not to convert. */
    GBE_ASSERT(dst.hstride != GEN_HORIZONTAL_STRIDE_0 && src.hstride != GEN_HORIZONTAL_STRIDE_0);
    this->packLongVec(src, dst, p->curr.execWidth);
  }

  void Gen8Context::emitUnpackLongInstruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.src(0));
    const GenRegister dst = ra->genReg(insn.dst(0));

    /* Scalar register need not to convert. */
    GBE_ASSERT(dst.hstride != GEN_HORIZONTAL_STRIDE_0 && src.hstride != GEN_HORIZONTAL_STRIDE_0);
    this->unpackLongVec(src, dst, p->curr.execWidth);
  }

  void Gen8Context::setA0Content(uint16_t new_a0[16], uint16_t max_offset, int sz) {
    if (sz == 0)
      sz = 16;
    GBE_ASSERT(sz%4 == 0);
    GBE_ASSERT(new_a0[0] >= 0 && new_a0[0] < 4096);

    p->push();
    p->curr.execWidth = 1;
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.noMask = 1;
    for (int i = 0; i < sz/4; i++) {
      uint64_t addr = (new_a0[i*4 + 3] << 16) | (new_a0[i*4 + 2]);
      addr = addr << 32;
      addr = addr | (new_a0[i*4 + 1] << 16) | (new_a0[i*4]);
      p->MOV(GenRegister::retype(GenRegister::addr1(i*4), GEN_TYPE_UL), GenRegister::immuint64(addr));
    }
    p->pop();
  }

  void ChvContext::newSelection(void) {
    this->sel = GBE_NEW(SelectionChv, *this);
  }

  void ChvContext::calculateFullU64MUL(GenRegister src0, GenRegister src1, GenRegister dst_h,
                                             GenRegister dst_l, GenRegister s0l_s1h, GenRegister s0h_s1l)
  {
    src0.type = src1.type = GEN_TYPE_UD;
    dst_h.type = dst_l.type = GEN_TYPE_UL;
    s0l_s1h.type = s0h_s1l.type = GEN_TYPE_UL;

    //GenRegister tmp;

    GenRegister s0l = unpacked_ud(src0);
    GenRegister s1l = unpacked_ud(src1);
    GenRegister s0h = unpacked_ud(s0l_s1h); //s0h only used before s0l_s1h, reuse s0l_s1h
    GenRegister s1h = unpacked_ud(dst_l); //s1h only used before dst_l, reuse dst_l

    p->MOV(s0h, GenRegister::offset(s0l, 0, 4));
    p->MOV(s1h, GenRegister::offset(s1l, 0, 4));

    /* High 32 bits X High 32 bits. */
    p->MUL(dst_h, s0h, s1h);
    /* High 32 bits X low 32 bits. */
    p->MUL(s0h_s1l, s0h, s1l);
    /* Low 32 bits X high 32 bits. */
    p->MUL(s0l_s1h, s0l, s1h);
    /* Low 32 bits X low 32 bits. */
    p->MUL(dst_l, s0l, s1l);

    /*  Because the max product of s0l*s1h is (2^N - 1) * (2^N - 1) = 2^2N + 1 - 2^(N+1), here N = 32
        The max of addding 2 32bits integer to it is
        2^2N + 1 - 2^(N+1) + 2*(2^N - 1) = 2^2N - 1
        which means the product s0h_s1l adds dst_l's high 32 bits and then adds s0l_s1h's low 32 bits will not
        overflow and have no carry.
        By this manner, we can avoid using acc register, which has a lot of restrictions. */

    GenRegister s0l_s1h_l = unpacked_ud(s0l_s1h);
    p->ADD(s0h_s1l, s0h_s1l, s0l_s1h_l);

    p->SHR(s0l_s1h, s0l_s1h, GenRegister::immud(32));
    GenRegister s0l_s1h_h = unpacked_ud(s0l_s1h);
    p->ADD(dst_h, dst_h, s0l_s1h_h);

    GenRegister dst_l_h = unpacked_ud(s0l_s1h);
    p->MOV(dst_l_h, unpacked_ud(dst_l, 1));
    p->ADD(s0h_s1l, s0h_s1l, dst_l_h);

    // No longer need s0l_s1h
    GenRegister tmp = s0l_s1h;

    p->SHL(tmp, s0h_s1l, GenRegister::immud(32));
    GenRegister tmp_unpacked = unpacked_ud(tmp, 1);
    p->MOV(unpacked_ud(dst_l, 1), tmp_unpacked);

    p->SHR(tmp, s0h_s1l, GenRegister::immud(32));
    p->ADD(dst_h, dst_h, tmp);
  }

  void ChvContext::emitI64MULInstruction(const SelectionInstruction &insn)
  {
    GenRegister src0 = ra->genReg(insn.src(0));
    GenRegister src1 = ra->genReg(insn.src(1));
    GenRegister dst = ra->genReg(insn.dst(0));
    GenRegister res = ra->genReg(insn.dst(1));

    src0.type = src1.type = GEN_TYPE_UD;
    dst.type = GEN_TYPE_UL;
    res.type = GEN_TYPE_UL;

    /* Low 32 bits X low 32 bits. */
    GenRegister s0l = unpacked_ud(src0);
    GenRegister s1l = unpacked_ud(src1);
    p->MUL(dst, s0l, s1l);

    /* Low 32 bits X high 32 bits. */
    GenRegister s1h = unpacked_ud(res);
    p->MOV(s1h, unpacked_ud(src1, 1));

    p->MUL(res, s0l, s1h);
    p->SHL(res, res, GenRegister::immud(32));
    p->ADD(dst, dst, res);

    /* High 32 bits X low 32 bits. */
    GenRegister s0h = unpacked_ud(res);
    p->MOV(s0h, unpacked_ud(src0, 1));

    p->MUL(res, s0h, s1l);
    p->SHL(res, res, GenRegister::immud(32));
    p->ADD(dst, dst, res);
  }

  void ChvContext::setA0Content(uint16_t new_a0[16], uint16_t max_offset, int sz) {
    if (sz == 0)
      sz = 16;
    GBE_ASSERT(sz%4 == 0);
    GBE_ASSERT(new_a0[0] >= 0 && new_a0[0] < 4096);

    p->push();
    p->curr.execWidth = 1;
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.noMask = 1;
    for (int i = 0; i < sz/2; i++) {
      p->MOV(GenRegister::retype(GenRegister::addr1(i*2), GEN_TYPE_UD),
             GenRegister::immud(new_a0[i*2 + 1] << 16 | new_a0[i*2]));
    }
    p->pop();
  }

}

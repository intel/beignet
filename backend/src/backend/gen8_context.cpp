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

  bool Gen8Context::patchBranches(void) {
    using namespace ir;
    for (auto pair : branchPos2) {
      const LabelIndex label = pair.first;
      const int32_t insnID = pair.second;
      const int32_t targetID = labelPos.find(label)->second;
      p->patchJMPI(insnID, (targetID - insnID), 0);
    }
    for (auto pair : branchPos3) {
      const LabelPair labelPair = pair.first;
      const int32_t insnID = pair.second;
      const int32_t jip = labelPos.find(labelPair.l0)->second;
      const int32_t uip = labelPos.find(labelPair.l1)->second;
      p->patchJMPI(insnID, jip - insnID, uip - insnID);
    }
    return true;
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
              if(!uniform_src)
                ind_src.addr_imm += 16;
              p->MOV(GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_UB), 0, 16), ind_src);
              if (simd == 16) {
                for (int i = 0; i < 2; i++) {
                  if(!uniform_src)
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
                if(!uniform_src)
                  ind_src.addr_imm += 16;
                p->MOV(GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_UB), 0, 16), ind_src);
              }
              p->pop();

              p->MOV(dst, tmp);
          }else if (src.type == GEN_TYPE_UL || src.type == GEN_TYPE_L) {
              bool uniform_src = (src.hstride == GEN_HORIZONTAL_STRIDE_0);
              GBE_ASSERT(uniform_src || src.subnr == 0);
              GBE_ASSERT(dst.subnr == 0);
              GBE_ASSERT(tmp.subnr == 0);
              GBE_ASSERT(start_addr >= 0);
              new_a0[0] = start_addr + 7;
              new_a0[1] = start_addr + 6;
              new_a0[2] = start_addr + 5;
              new_a0[3] = start_addr + 4;
              new_a0[4] = start_addr + 3;
              new_a0[5] = start_addr + 2;
              new_a0[6] = start_addr + 1;
              new_a0[7] = start_addr;
              if(!uniform_src) {
                new_a0[8] = start_addr + 15;
                new_a0[9] = start_addr + 14;
                new_a0[10] = start_addr + 13;
                new_a0[11] = start_addr + 12;
                new_a0[12] = start_addr + 11;
                new_a0[13] = start_addr + 10;
                new_a0[14] = start_addr + 9;
                new_a0[15] = start_addr + 8;
              } else {
                new_a0[8] = start_addr + 7;
                new_a0[9] = start_addr + 6;
                new_a0[10] = start_addr + 5;
                new_a0[11] = start_addr + 4;
                new_a0[12] = start_addr + 3;
                new_a0[13] = start_addr + 2;
                new_a0[14] = start_addr + 1;
                new_a0[15] = start_addr;
              }
              this->setA0Content(new_a0, 56);

              p->push();
              p->curr.execWidth = 16;
              p->curr.predicate = GEN_PREDICATE_NONE;
              p->curr.noMask = 1;
              GenRegister ind_src = GenRegister::to_indirect1xN(GenRegister::retype(src, GEN_TYPE_UB), new_a0[0], 0);
              p->MOV(GenRegister::retype(tmp, GEN_TYPE_UB), ind_src);
              if(!uniform_src)
                ind_src.addr_imm += 16;
              p->MOV(GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_UB), 0, 16), ind_src);
              for (int i = 0; i < 2; i++) {
                if(!uniform_src)
                  ind_src.addr_imm += 16;
                p->MOV(GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_UB), 1, 16*i), ind_src);
              }
              if (simd == 16) {
                for (int i = 0; i < 2; i++) {
                  if(!uniform_src)
                    ind_src.addr_imm += 16;
                  p->MOV(GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_UB), 2, 16*i), ind_src);
                }
                for (int i = 0; i < 2; i++) {
                  if(!uniform_src)
                    ind_src.addr_imm += 16;
                  p->MOV(GenRegister::offset(GenRegister::retype(tmp, GEN_TYPE_UB), 3, 16*i), ind_src);
                }
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

  void Gen8Context::emitSimdShuffleInstruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src0 = ra->genReg(insn.src(0));
    const GenRegister src1 = ra->genReg(insn.src(1));
    assert(insn.opcode == SEL_OP_SIMD_SHUFFLE);
    assert (src1.file != GEN_IMMEDIATE_VALUE);

    uint32_t base = src0.nr * 32 + src0.subnr;
    GenRegister baseReg = GenRegister::immuw(base);
    const GenRegister a0 = GenRegister::addr8(0);
    p->ADD(a0, GenRegister::unpacked_uw(src1.nr, src1.subnr / typeSize(GEN_TYPE_UW)), baseReg);
    GenRegister indirect = GenRegister::to_indirect1xN(src0, 0, 0);
    p->MOV(dst, indirect);
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

  GenRegister Gen8Context::unpacked_ud(GenRegister reg, uint32_t offset)
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
  void Gen8Context::emitUntypedReadA64Instruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t elemNum = insn.extra.elem;
    p->UNTYPED_READA64(dst, src, elemNum);
  }

  void Gen8Context::emitUntypedWriteA64Instruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t elemNum = insn.extra.elem;
    p->UNTYPED_WRITEA64(src, elemNum);
  }

  void Gen8Context::emitByteGatherA64Instruction(const SelectionInstruction &insn) {
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t elemSize = insn.extra.elem;
    p->BYTE_GATHERA64(dst, src, elemSize);
  }

  void Gen8Context::emitByteScatterA64Instruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.src(0));
    const uint32_t elemSize = insn.extra.elem;
    p->BYTE_SCATTERA64(src, elemSize);
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
      const GenRegister btiTmp = ra->genReg(insn.dst(2*elemNum + 1));
      unsigned desc = p->generateUntypedReadMessageDesc(0, 2*elemNum);

      unsigned jip0 = beforeMessage(insn, bti, tmp, btiTmp, desc);

      //predicated load
      p->push();
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(insn.state.flag, insn.state.subFlag);
        p->UNTYPED_READ(dst, src, GenRegister::retype(GenRegister::addr1(0), GEN_TYPE_UD), 2*elemNum);
      p->pop();
      afterMessage(insn, bti, tmp, btiTmp, jip0);
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
    GenRegister data = ra->genReg(insn.src(elemNum+1));

    /* Because BDW's store and load send instructions for 64 bits require the bti to be surfaceless,
       which we can not accept. We just fallback to 2 DW untypewrite here. */
    for (uint32_t elemID = 0; elemID < elemNum; elemID++) {
      GenRegister the_long = ra->genReg(insn.src(elemID));
      GenRegister long_tmp = ra->genReg(insn.src(elemNum + 1 + elemID));
      this->unpackLongVec(the_long, long_tmp, p->curr.execWidth);
    }

    if (bti.file == GEN_IMMEDIATE_VALUE) {
      p->UNTYPED_WRITE(addr, data, bti, elemNum*2, insn.extra.splitSend);
    } else {
      const GenRegister tmp = ra->genReg(insn.dst(elemNum));
      const GenRegister btiTmp = ra->genReg(insn.dst(elemNum + 1));
      unsigned desc = 0;
      if (insn.extra.splitSend)
        desc = p->generateUntypedWriteSendsMessageDesc(0, elemNum*2);
      else
        desc = p->generateUntypedWriteMessageDesc(0, elemNum*2);

      unsigned jip0 = beforeMessage(insn, bti, tmp, btiTmp, desc);

      //predicated load
      p->push();
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.useFlag(insn.state.flag, insn.state.subFlag);
        p->UNTYPED_WRITE(addr, data, GenRegister::addr1(0), elemNum*2, insn.extra.splitSend);
      p->pop();
      afterMessage(insn, bti, tmp, btiTmp, jip0);
    }
  }
  void Gen8Context::emitRead64A64Instruction(const SelectionInstruction &insn) {
    const uint32_t elemNum = insn.extra.elem;
    GBE_ASSERT(elemNum == 1);

    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister src = ra->genReg(insn.src(0));

    /* Because BDW's store and load send instructions for 64 bits require the bti to be surfaceless,
       which we can not accept. We just fallback to 2 DW untyperead here. */
    p->UNTYPED_READA64(dst, src, 2*elemNum);

    for (uint32_t elemID = 0; elemID < elemNum; elemID++) {
      GenRegister long_tmp = ra->genReg(insn.dst(elemID));
      GenRegister the_long = ra->genReg(insn.dst(elemID + elemNum));
      this->packLongVec(long_tmp, the_long, p->curr.execWidth);
    }
  }

  void Gen8Context::emitWrite64A64Instruction(const SelectionInstruction &insn)
  {
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

    p->UNTYPED_WRITEA64(addr, elemNum*2);
  }
  void Gen8Context::emitAtomicA64Instruction(const SelectionInstruction &insn)
  {
    const GenRegister src = ra->genReg(insn.src(0));
    const GenRegister dst = ra->genReg(insn.dst(0));
    const uint32_t function = insn.extra.function;
    unsigned srcNum = insn.extra.elem;
    const GenRegister bti = ra->genReg(insn.src(srcNum));
    GBE_ASSERT(bti.value.ud == 0xff);
    p->ATOMICA64(dst, function, src, bti, srcNum);
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
    GBE_ASSERT(dst.hstride != GEN_HORIZONTAL_STRIDE_0);
    this->unpackLongVec(src, dst, p->curr.execWidth);
  }

  void Gen8Context::emitF64DIVInstruction(const SelectionInstruction &insn) {
    /* Macro for Double Precision IEEE Compliant fdiv

       Set Rounding Mode in CR to RNE
       GRF are initialized: r0 = 0, r6 = a, r7 = b, r1 = 1
       The default data type for the macro is :df

       math.eo.f0.0 (4) r8.acc2 r6.noacc r7.noacc 0xE
       (-f0.0) if
       madm (4) r9.acc3 r0.noacc r6.noacc r8.acc2       // Step(1), q0=a*y0
       madm (4) r10.acc4 r1.noacc -r7.noacc r8.acc2     // Step(2), e0=(1-b*y0)
       madm (4) r11.acc5 r6.noacc -r7.noacc r9.acc3     // Step(3), r0=a-b*q0
       madm (4) r12.acc6 r8.acc2 r10.acc4 r8.acc2       // Step(4), y1=y0+e0*y0
       madm (4) r13.acc7 r1.noacc -r7.noacc r12.acc6    // Step(5), e1=(1-b*y1)
       madm (4) r8.acc8 r8.acc2 r10.acc4 r12.acc6       // Step(6), y2=y0+e0*y1
       madm (4) r9.acc9 r9.acc3 r11.acc5 r12.acc6       // Step(7), q1=q0+r0*y1
       madm (4) r12.acc2 r12.acc6 r8.acc8 r13.acc7      // Step(8), y3=y1+e1*y2
       madm (4) r11.acc3 r6.noacc -r7.noacc r9.acc9     // Step(9), r1=a-b*q1

       Change Rounding Mode in CR if required
       Implicit Accumulator for destination is NULL

       madm (4) r8.noacc r9.acc9 r11.acc3 r12.acc2      // Step(10), q=q1+r1*y3
       endif */
    GenRegister src0 = GenRegister::retype(ra->genReg(insn.src(0)), GEN_TYPE_DF);
    GenRegister src1 = GenRegister::retype(ra->genReg(insn.src(1)), GEN_TYPE_DF);
    GenRegister dst = GenRegister::retype(ra->genReg(insn.dst(0)), GEN_TYPE_DF);
    GenRegister r6 , r7, r8;
    int src0Stride = 1;
    int src1Stride = 1;
    int tmpNum = 7;
    int loopNum = 0;

    if (dst.hstride == GEN_HORIZONTAL_STRIDE_0) {// dst is uniform
      loopNum = 1;
    } else if (p->curr.execWidth == 4) {
      loopNum = 1;
    } else if (p->curr.execWidth == 8) {
      loopNum = 2;
    } else if (p->curr.execWidth == 16) {
      loopNum = 4;
    } else
      GBE_ASSERT(0);

    r8 = GenRegister::retype(ra->genReg(insn.dst(tmpNum + 1)), GEN_TYPE_DF);
    tmpNum++;

    if (src0.vstride == GEN_HORIZONTAL_STRIDE_0) {
      r6 = GenRegister::retype(ra->genReg(insn.dst(tmpNum + 1)), GEN_TYPE_DF);
      tmpNum++;
      src0Stride = 0;
      p->push(); {
        p->curr.execWidth = 4;
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->curr.noMask= 1;
        p->MOV(r6, src0);
      } p->pop();
    } else {
      r6 = src0;
    }

    if (src1.vstride == GEN_HORIZONTAL_STRIDE_0) {
      r7 = GenRegister::retype(ra->genReg(insn.dst(tmpNum + 1)), GEN_TYPE_DF);
      tmpNum++;
      src1Stride = 0;
      p->push(); {
        p->curr.execWidth = 4;
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->curr.noMask = 1;
        p->MOV(r7, src1);
      } p->pop();
    } else {
      r7 = src1;
    }

    const GenRegister r0 = GenRegister::retype(ra->genReg(insn.dst(1)), GEN_TYPE_DF);
    const GenRegister r1 = GenRegister::retype(ra->genReg(insn.dst(2)), GEN_TYPE_DF);
    const GenRegister r9 = GenRegister::retype(ra->genReg(insn.dst(3)), GEN_TYPE_DF);
    const GenRegister r10 = GenRegister::retype(ra->genReg(insn.dst(4)), GEN_TYPE_DF);
    const GenRegister r11 = GenRegister::retype(ra->genReg(insn.dst(5)), GEN_TYPE_DF);
    const GenRegister r12 = GenRegister::retype(ra->genReg(insn.dst(6)), GEN_TYPE_DF);
    const GenRegister r13 = GenRegister::retype(ra->genReg(insn.dst(7)), GEN_TYPE_DF);
    Gen8Encoder *p8 = reinterpret_cast<Gen8Encoder *>(p);
    p->push(); {
      p->curr.execWidth = 4;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask= 1;
      p->MOV(r1, GenRegister::immdf(1.0));
      p->MOV(r0, GenRegister::immdf(0.0));
    } p->pop();

    for (int i = 0; i < loopNum; i++) {
      p->push(); {
        p->curr.noMask= 1;
        p->curr.execWidth = 4;
        p->curr.predicate = GEN_PREDICATE_NONE;
        p8->MATH_WITH_ACC(r8, GEN8_MATH_FUNCTION_INVM, r6, r7, GEN8_INSN_ACC2, GEN8_INSN_NOACC, GEN8_INSN_NOACC);
        p->curr.useFlag(insn.state.flag, insn.state.subFlag);
        p->curr.predicate = GEN_PREDICATE_NORMAL;
        p->curr.inversePredicate = 1;
        p8->MADM(r9, r0, r6, r8, GEN8_INSN_ACC3, GEN8_INSN_NOACC, GEN8_INSN_NOACC, GEN8_INSN_ACC2);
        p8->MADM(r10, r1, GenRegister::negate(r7), r8, GEN8_INSN_ACC4, GEN8_INSN_NOACC, GEN8_INSN_NOACC, GEN8_INSN_ACC2);
        p8->MADM(r11, r6, GenRegister::negate(r7), r9, GEN8_INSN_ACC5, GEN8_INSN_NOACC, GEN8_INSN_NOACC, GEN8_INSN_ACC3);
        p8->MADM(r12, r8, r10, r8, GEN8_INSN_ACC6, GEN8_INSN_ACC2, GEN8_INSN_ACC4, GEN8_INSN_ACC2);
        p8->MADM(r13, r1, GenRegister::negate(r7), r12, GEN8_INSN_ACC7, GEN8_INSN_NOACC, GEN8_INSN_NOACC, GEN8_INSN_ACC6);
        p8->MADM(r8, r8, r10, r12, GEN8_INSN_ACC8, GEN8_INSN_ACC2, GEN8_INSN_ACC4, GEN8_INSN_ACC6);
        p8->MADM(r9, r9, r11, r12, GEN8_INSN_ACC9, GEN8_INSN_ACC3, GEN8_INSN_ACC5, GEN8_INSN_ACC6);
        p8->MADM(r12, r12, r8, r13, GEN8_INSN_ACC2, GEN8_INSN_ACC6, GEN8_INSN_ACC8, GEN8_INSN_ACC7);
        p8->MADM(r11, r6, GenRegister::negate(r7), r9, GEN8_INSN_ACC3, GEN8_INSN_NOACC, GEN8_INSN_NOACC, GEN8_INSN_ACC9);

        p8->MADM(r8, r9, r11, r12, GEN8_INSN_NOACC, GEN8_INSN_ACC9, GEN8_INSN_ACC3, GEN8_INSN_ACC2);
      } p->pop();

      r6 = GenRegister::offset(r6, src0Stride);
      r7 = GenRegister::offset(r7, src1Stride);

      /* Move back the result. */
      if (dst.hstride == GEN_HORIZONTAL_STRIDE_0) {// dst is uniform
        p->push(); {
          p->curr.execWidth = 1;
          r8.hstride = GEN_HORIZONTAL_STRIDE_0;
          r8.vstride = GEN_VERTICAL_STRIDE_0;
          r8.width = GEN_WIDTH_1;
          p->MOV(dst, r8);
        } p->pop();
        break;
      } else {
        p->push(); {
          p->curr.execWidth = 4;
          if (i % 2 == 0)
            p->curr.nibControl = 0;
          else
            p->curr.nibControl = 1;

          if (i < 2)
            p->curr.quarterControl = GEN_COMPRESSION_Q1;
          else
            p->curr.quarterControl = GEN_COMPRESSION_Q2;

          p->MOV(GenRegister::offset(dst, i), r8);
        } p->pop();
      }
    }
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

  void Gen8Context::subTimestamps(GenRegister& t0, GenRegister& t1, GenRegister& tmp)
  {
    p->push(); {
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->ADD(GenRegister::retype(t0, GEN_TYPE_UL), GenRegister::retype(t0, GEN_TYPE_UL),
          GenRegister::negate(GenRegister::retype(t1, GEN_TYPE_UL)));
    } p->pop();
  }

  void Gen8Context::addTimestamps(GenRegister& t0, GenRegister& t1, GenRegister& tmp) {
    p->push(); {
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->curr.noMask = 1;
      p->ADD(GenRegister::retype(t0, GEN_TYPE_UL), GenRegister::retype(t0, GEN_TYPE_UL),
          GenRegister::retype(t1, GEN_TYPE_UL));
    } p->pop();
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

  void Gen8Context::emitPrintfLongInstruction(GenRegister& addr, GenRegister& data,
                                             GenRegister& src, uint32_t bti) {
    GenRegister tempSrc, tempDst;
    GenRegister nextSrc, nextDst;
    p->push();
      tempSrc = GenRegister::h2(GenRegister::retype(src, GEN_TYPE_UD));
      tempDst = GenRegister::retype(data, GEN_TYPE_UD);
      p->curr.execWidth = 8;
      p->curr.quarterControl = GEN_COMPRESSION_Q1;
      p->MOV(tempDst, tempSrc);

      p->curr.quarterControl = GEN_COMPRESSION_Q2;
      nextSrc = GenRegister::Qn(tempSrc, 1);
      nextDst = GenRegister::Qn(tempDst, 1);
      p->MOV(nextDst, nextSrc);
    p->pop();
    p->UNTYPED_WRITE(addr, addr, GenRegister::immud(bti), 1, false);
    p->ADD(addr, addr, GenRegister::immud(sizeof(uint32_t)));

    p->push();
      tempSrc = GenRegister::h2(
        GenRegister::retype(GenRegister::offset(src, 0, 4), GEN_TYPE_UD));
      tempDst = GenRegister::retype(data, GEN_TYPE_UD);
      p->curr.execWidth = 8;
      p->curr.quarterControl = GEN_COMPRESSION_Q1;
      p->MOV(tempDst, tempSrc);

      p->curr.quarterControl = GEN_COMPRESSION_Q2;
      nextSrc = GenRegister::Qn(tempSrc, 1);
      nextDst = GenRegister::Qn(tempDst, 1);
      p->MOV(nextDst, nextSrc);
    p->pop();
    p->UNTYPED_WRITE(addr, addr, GenRegister::immud(bti), 1, false);
    p->ADD(addr, addr, GenRegister::immud(sizeof(uint32_t)));
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

  void ChvContext::emitStackPointer(void) {
    using namespace ir;

    // Only emit stack pointer computation if we use a stack
    if (kernel->getStackSize() == 0)
      return;

    // Check that everything is consistent in the kernel code
    const uint32_t perLaneSize = kernel->getStackSize();
    GBE_ASSERT(perLaneSize > 0);

    const GenRegister selStatckPtr = this->simdWidth == 8 ?
      GenRegister::ud8grf(ir::ocl::stackptr) :
      GenRegister::ud16grf(ir::ocl::stackptr);
    const GenRegister stackptr = ra->genReg(selStatckPtr);
    // borrow block ip as temporary register as we will
    // initialize block ip latter.
    const GenRegister tmpReg = GenRegister::retype(GenRegister::vec1(getBlockIP()), GEN_TYPE_UW);
    const GenRegister tmpReg_ud = GenRegister::retype(tmpReg, GEN_TYPE_UD);

    loadLaneID(stackptr);

    // We compute the per-lane stack pointer here
    // threadId * perThreadSize + laneId*perLaneSize or
    // (threadId * simdWidth + laneId)*perLaneSize
    // let private address start from zero
    //p->MOV(stackptr, GenRegister::immud(0));
    p->push();
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->AND(tmpReg, GenRegister::ud1grf(0,5), GenRegister::immuw(0x1ff)); //threadId
      p->MUL(tmpReg, tmpReg, GenRegister::immuw(this->simdWidth));  //threadId * simdWidth
      p->curr.execWidth = this->simdWidth;
      p->ADD(stackptr, GenRegister::unpacked_uw(stackptr), tmpReg);  //threadId * simdWidth + laneId, must < 64K
      p->curr.execWidth = 1;
      p->MOV(tmpReg_ud, GenRegister::immud(perLaneSize));
      p->curr.execWidth = this->simdWidth;
      p->MUL(stackptr, tmpReg_ud, GenRegister::unpacked_uw(stackptr)); // (threadId * simdWidth + laneId)*perLaneSize
      if (fn.getPointerFamily() == ir::FAMILY_QWORD) {
        const GenRegister selStatckPtr2 = this->simdWidth == 8 ?
          GenRegister::ul8grf(ir::ocl::stackptr) :
          GenRegister::ul16grf(ir::ocl::stackptr);
        GenRegister stackptr2 = ra->genReg(selStatckPtr2);
        GenRegister sp = GenRegister::unpacked_ud(stackptr2.nr, stackptr2.subnr);
        int simdWidth = p->curr.execWidth;
        if (simdWidth == 16) {
          // we need do second quarter first, because the dst type is QW,
          // while the src is DW. If we do first quater first, the 1st
          // quarter's dst would contain the 2nd quarter's src.
          p->curr.execWidth = 8;
          p->curr.quarterControl = GEN_COMPRESSION_Q2;
          p->MOV(GenRegister::Qn(sp, 1), GenRegister::Qn(stackptr,1));
          p->MOV(GenRegister::Qn(stackptr2, 1), GenRegister::Qn(sp,1));
        }
        p->curr.quarterControl = GEN_COMPRESSION_Q1;
        p->MOV(sp, stackptr);
        p->MOV(stackptr2, sp);
      }
    p->pop();
  }

  /* Init value according to WORKGROUP OP
   * Emit assert is invalid combination operation - datatype */
  static void wgOpInitValue(GenEncoder *p, GenRegister dataReg, uint32_t wg_op)
  {

    if (wg_op == ir::WORKGROUP_OP_ALL)
    {
      if (dataReg.type == GEN_TYPE_D
          || dataReg.type == GEN_TYPE_UD)
        p->MOV(dataReg, GenRegister::immd(0xFFFFFFFF));
      else if(dataReg.type == GEN_TYPE_L ||
          dataReg.type == GEN_TYPE_UL)
        p->MOV(dataReg, GenRegister::immint64(0xFFFFFFFFFFFFFFFFL));
      else
        GBE_ASSERT(0); /* unsupported data-type */
    }

    else if(wg_op == ir::WORKGROUP_OP_ANY
      || wg_op == ir::WORKGROUP_OP_REDUCE_ADD
      || wg_op == ir::WORKGROUP_OP_INCLUSIVE_ADD
      || wg_op == ir::WORKGROUP_OP_EXCLUSIVE_ADD)
    {
      if (dataReg.type == GEN_TYPE_D)
        p->MOV(dataReg, GenRegister::immd(0x0));
      else if (dataReg.type == GEN_TYPE_UD)
        p->MOV(dataReg, GenRegister::immud(0x0));
      else if (dataReg.type == GEN_TYPE_HF)
        p->MOV(dataReg, GenRegister::immh(0x0));
      else if (dataReg.type == GEN_TYPE_F)
        p->MOV(dataReg, GenRegister::immf(0x0));
      else if (dataReg.type == GEN_TYPE_L)
        p->MOV(dataReg, GenRegister::immint64(0x0));
      else if (dataReg.type == GEN_TYPE_UL)
        p->MOV(dataReg, GenRegister::immuint64(0x0));
      else if (dataReg.type == GEN_TYPE_W)
        p->MOV(dataReg, GenRegister::immw(0x0));
      else if (dataReg.type == GEN_TYPE_UW)
        p->MOV(dataReg, GenRegister::immuw(0x0));
      else
        GBE_ASSERT(0); /* unsupported data-type */
    }

    else if(wg_op == ir::WORKGROUP_OP_REDUCE_MIN
      || wg_op == ir::WORKGROUP_OP_INCLUSIVE_MIN
      || wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MIN)
    {
      if (dataReg.type == GEN_TYPE_D)
        p->MOV(dataReg, GenRegister::immd(0x7FFFFFFF));
      else if (dataReg.type == GEN_TYPE_UD)
        p->MOV(dataReg, GenRegister::immud(0xFFFFFFFF));
      else if (dataReg.type == GEN_TYPE_HF)
        p->MOV(GenRegister::retype(dataReg, GEN_TYPE_UW), GenRegister::immuw(0x7C00));
      else if (dataReg.type == GEN_TYPE_F)
        p->MOV(GenRegister::retype(dataReg, GEN_TYPE_UD), GenRegister::immud(0x7F800000));
      else if (dataReg.type == GEN_TYPE_L)
        p->MOV(dataReg, GenRegister::immint64(0x7FFFFFFFFFFFFFFFL));
      else if (dataReg.type == GEN_TYPE_UL)
        p->MOV(dataReg, GenRegister::immuint64(0xFFFFFFFFFFFFFFFFL));
      else if (dataReg.type == GEN_TYPE_W)
        p->MOV(dataReg, GenRegister::immw(0x7FFF));
      else if (dataReg.type == GEN_TYPE_UW)
        p->MOV(dataReg, GenRegister::immuw(0xFFFF));
      else
        GBE_ASSERT(0); /* unsupported data-type */
    }

    else if(wg_op == ir::WORKGROUP_OP_REDUCE_MAX
      || wg_op == ir::WORKGROUP_OP_INCLUSIVE_MAX
      || wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MAX)
    {
      if (dataReg.type == GEN_TYPE_D)
        p->MOV(dataReg, GenRegister::immd(0x80000000));
      else if (dataReg.type == GEN_TYPE_UD)
        p->MOV(dataReg, GenRegister::immud(0x0));
      else if (dataReg.type == GEN_TYPE_HF)
        p->MOV(GenRegister::retype(dataReg, GEN_TYPE_UW), GenRegister::immuw(0xFC00));
      else if (dataReg.type == GEN_TYPE_F)
        p->MOV(GenRegister::retype(dataReg, GEN_TYPE_UD), GenRegister::immud(0xFF800000));
      else if (dataReg.type == GEN_TYPE_L)
        p->MOV(dataReg, GenRegister::immint64(0x8000000000000000L));
      else if (dataReg.type == GEN_TYPE_UL)
        p->MOV(dataReg, GenRegister::immuint64(0x0));
      else if (dataReg.type == GEN_TYPE_W)
        p->MOV(dataReg, GenRegister::immw(0x8000));
      else if (dataReg.type == GEN_TYPE_UW)
        p->MOV(dataReg, GenRegister::immuw(0x0));
      else
        GBE_ASSERT(0); /* unsupported data-type */
    }

    /* unsupported operation */
    else
      GBE_ASSERT(0);
  }

  /* Perform WORKGROUP OP on 2 input elements (registers) */
  static void wgOpPerform(GenRegister dst,
                         GenRegister src1,
                         GenRegister src2,
                         uint32_t wg_op,
                         GenEncoder *p)
  {
    /* perform OP REDUCE on 2 elements */
    if (wg_op == ir::WORKGROUP_OP_ANY)
      p->OR(dst, src1, src2);
    else if (wg_op == ir::WORKGROUP_OP_ALL)
      p->AND(dst, src1, src2);
    else if(wg_op == ir::WORKGROUP_OP_REDUCE_ADD)
      p->ADD(dst, src1, src2);
    else if(wg_op == ir::WORKGROUP_OP_REDUCE_MIN)
      p->SEL_CMP(GEN_CONDITIONAL_LE, dst, src1, src2);
    else if(wg_op == ir::WORKGROUP_OP_REDUCE_MAX)
      p->SEL_CMP(GEN_CONDITIONAL_GE, dst, src1, src2);

    /* perform OP SCAN INCLUSIVE on 2 elements */
    else if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_ADD)
      p->ADD(dst, src1, src2);
    else if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_MIN)
      p->SEL_CMP(GEN_CONDITIONAL_LE, dst, src1, src2);
    else if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_MAX)
      p->SEL_CMP(GEN_CONDITIONAL_GE, dst, src1, src2);

    /* perform OP SCAN EXCLUSIVE on 2 elements */
    else if(wg_op == ir::WORKGROUP_OP_EXCLUSIVE_ADD)
      p->ADD(dst, src1, src2);
    else if(wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MIN)
      p->SEL_CMP(GEN_CONDITIONAL_LE, dst, src1, src2);
    else if(wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MAX)
      p->SEL_CMP(GEN_CONDITIONAL_GE, dst, src1, src2);

    else
      GBE_ASSERT(0);
  }

  static void wgOpPerformThread(GenRegister threadDst,
                                  GenRegister inputVal,
                                  GenRegister threadExchangeData,
                                   GenRegister resultVal,
                                   uint32_t simd,
                                   uint32_t wg_op,
                                   GenEncoder *p)
  {
   p->push();
   p->curr.predicate = GEN_PREDICATE_NONE;
   p->curr.noMask = 1;
   p->curr.execWidth = 1;

   /* setting the type */
   resultVal = GenRegister::retype(resultVal, inputVal.type);
   threadDst = GenRegister::retype(threadDst, inputVal.type);
   threadExchangeData = GenRegister::retype(threadExchangeData, inputVal.type);

   vector<GenRegister> input;
   vector<GenRegister> result;

   /* for workgroup all and any we can use simd_all/any for each thread */
   if (wg_op == ir::WORKGROUP_OP_ALL || wg_op == ir::WORKGROUP_OP_ANY) {
     GenRegister constZero = GenRegister::immuw(0);
     GenRegister flag01 = GenRegister::flag(0, 1);

     p->push();
     {
       p->curr.predicate = GEN_PREDICATE_NONE;
       p->curr.noMask = 1;
       p->curr.execWidth = simd;
       p->MOV(resultVal, GenRegister::immud(1));
       p->curr.execWidth = 1;
       if (wg_op == ir::WORKGROUP_OP_ALL)
         p->MOV(flag01, GenRegister::immw(-1));
       else
         p->MOV(flag01, constZero);

       p->curr.execWidth = simd;
       p->curr.noMask = 0;

       p->curr.flag = 0;
       p->curr.subFlag = 1;
       p->CMP(GEN_CONDITIONAL_NEQ, inputVal, constZero);

       if (p->curr.execWidth == 16)
         if (wg_op == ir::WORKGROUP_OP_ALL)
           p->curr.predicate = GEN_PREDICATE_ALIGN1_ALL16H;
         else
           p->curr.predicate = GEN_PREDICATE_ALIGN1_ANY16H;
       else if (p->curr.execWidth == 8)
         if (wg_op == ir::WORKGROUP_OP_ALL)
           p->curr.predicate = GEN_PREDICATE_ALIGN1_ALL8H;
         else
          p->curr.predicate = GEN_PREDICATE_ALIGN1_ANY8H;
       else
         NOT_IMPLEMENTED;
       p->SEL(threadDst, resultVal, constZero);
       p->SEL(threadExchangeData, resultVal, constZero);
     }
     p->pop();
   } else {
     if (inputVal.hstride == GEN_HORIZONTAL_STRIDE_0) {
       p->MOV(threadExchangeData, inputVal);
       p->pop();
       return;
     }

     /* init thread data to min/max/null values */
     p->push(); {
       p->curr.execWidth = simd;
       wgOpInitValue(p, threadExchangeData, wg_op);
       p->MOV(resultVal, inputVal);
     } p->pop();

     GenRegister resultValSingle = resultVal;
     resultValSingle.hstride = GEN_HORIZONTAL_STRIDE_0;
     resultValSingle.vstride = GEN_VERTICAL_STRIDE_0;
     resultValSingle.width = GEN_WIDTH_1;

     GenRegister inputValSingle = inputVal;
     inputValSingle.hstride = GEN_HORIZONTAL_STRIDE_0;
     inputValSingle.vstride = GEN_VERTICAL_STRIDE_0;
     inputValSingle.width = GEN_WIDTH_1;


     /* make an array of registers for easy accesing */
     for(uint32_t i = 0; i < simd; i++){
       /* add all resultVal offset reg positions from list */
       result.push_back(resultValSingle);
       input.push_back(inputValSingle);

       /* move to next position */
       resultValSingle.subnr += typeSize(resultValSingle.type);
       if (resultValSingle.subnr == 32) {
           resultValSingle.subnr = 0;
           resultValSingle.nr++;
       }
       /* move to next position */
       inputValSingle.subnr += typeSize(inputValSingle.type);
       if (inputValSingle.subnr == 32) {
           inputValSingle.subnr = 0;
           inputValSingle.nr++;
       }
     }

     uint32_t start_i = 0;
     if( wg_op == ir::WORKGROUP_OP_REDUCE_ADD ||
         wg_op == ir::WORKGROUP_OP_REDUCE_MIN ||
         wg_op == ir::WORKGROUP_OP_REDUCE_MAX ||
         wg_op == ir::WORKGROUP_OP_INCLUSIVE_ADD ||
         wg_op == ir::WORKGROUP_OP_INCLUSIVE_MIN ||
         wg_op == ir::WORKGROUP_OP_INCLUSIVE_MAX) {
       p->MOV(result[0], input[0]);
       start_i = 1;
     }

     else if(wg_op == ir::WORKGROUP_OP_EXCLUSIVE_ADD ||
         wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MIN ||
         wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MAX) {
       p->MOV(result[1], input[0]);
       start_i = 2;
     }

     /* algorithm workgroup */
     for (uint32_t i = start_i; i < simd; i++)
     {
       if( wg_op == ir::WORKGROUP_OP_REDUCE_ADD ||
           wg_op == ir::WORKGROUP_OP_REDUCE_MIN ||
           wg_op == ir::WORKGROUP_OP_REDUCE_MAX)
         wgOpPerform(result[0], result[0], input[i], wg_op, p);

       else if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_ADD ||
           wg_op == ir::WORKGROUP_OP_INCLUSIVE_MIN ||
           wg_op == ir::WORKGROUP_OP_INCLUSIVE_MAX)
         wgOpPerform(result[i], result[i - 1], input[i], wg_op, p);

       else if(wg_op == ir::WORKGROUP_OP_EXCLUSIVE_ADD ||
           wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MIN ||
           wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MAX)
         wgOpPerform(result[i], result[i - 1], input[i - 1], wg_op, p);

       else
         GBE_ASSERT(0);
     }
   }

   if( wg_op == ir::WORKGROUP_OP_REDUCE_ADD ||
       wg_op == ir::WORKGROUP_OP_REDUCE_MIN ||
       wg_op == ir::WORKGROUP_OP_REDUCE_MAX)
   {
     p->curr.execWidth = simd;
     /* value exchanged with other threads */
     p->MOV(threadExchangeData, result[0]);
     /* partial result thread */
     p->MOV(threadDst, result[0]);
   }
   else if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_ADD ||
       wg_op == ir::WORKGROUP_OP_INCLUSIVE_MIN ||
       wg_op == ir::WORKGROUP_OP_INCLUSIVE_MAX)
   {
     p->curr.execWidth = simd;
     /* value exchanged with other threads */
     p->MOV(threadExchangeData, result[simd - 1]);
     /* partial result thread */
     p->MOV(threadDst, resultVal);
   }
   else if(wg_op == ir::WORKGROUP_OP_EXCLUSIVE_ADD ||
       wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MIN ||
       wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MAX)
   {
     p->curr.execWidth = 1;
     /* set result[0] to min/max/null */
     wgOpInitValue(p, result[0], wg_op);

     p->curr.execWidth = simd;
     /* value exchanged with other threads */
     wgOpPerform(threadExchangeData, result[simd - 1], input[simd - 1], wg_op, p);
     /* partial result thread */
     p->MOV(threadDst, resultVal);
   }

   p->pop();
 }

/**
 * WORKGROUP OP: ALL, ANY, REDUCE, SCAN INCLUSIVE, SCAN EXCLUSIVE
 *
 * Implementation:
 * 1. All the threads first perform the workgroup op value for the
 * allocated work-items. SIMD16=> 16 work-items allocated for each thread
 * 2. Each thread writes the partial result in shared local memory using threadId
 * 3. After a barrier, each thread will read in chunks of 1-4 elements,
 * the shared local memory region, using a loop based on the thread num value (threadN)
 * 4. Each thread computes the final value individually
 *
 * Optimizations:
 * Performance is given by chunk read. If threads read in chunks of 4 elements
 * the performance is increase 2-3x times compared to chunks of 1 element.
 */
  void Gen8Context::emitWorkGroupOpInstruction(const SelectionInstruction &insn){
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister tmp = GenRegister::retype(ra->genReg(insn.dst(1)), dst.type);
    const GenRegister theVal = GenRegister::retype(ra->genReg(insn.src(2)), dst.type);
    GenRegister threadData = ra->genReg(insn.src(3));
    GenRegister partialData = GenRegister::toUniform(threadData, dst.type);
    GenRegister threadId = ra->genReg(insn.src(0));
    GenRegister threadLoop = ra->genReg(insn.src(1));
    GenRegister barrierId = ra->genReg(GenRegister::ud1grf(ir::ocl::barrierid));
    GenRegister localBarrier = ra->genReg(insn.src(5));

    uint32_t wg_op = insn.extra.wgop.workgroupOp;
    uint32_t simd = p->curr.execWidth;
    int32_t jip0, jip1;

    /* masked elements should be properly set to init value */
    p->push(); {
      p->curr.noMask = 1;
      wgOpInitValue(p, tmp, wg_op);
      p->curr.noMask = 0;
      p->MOV(tmp, theVal);
      p->curr.noMask = 1;
      p->MOV(theVal, tmp);
    } p->pop();

    threadId = GenRegister::toUniform(threadId, GEN_TYPE_UD);

    /* use of continuous GRF allocation from insn selection */
    GenRegister msg = GenRegister::retype(ra->genReg(insn.dst(2)), dst.type);
    GenRegister msgSlmOff = GenRegister::retype(ra->genReg(insn.src(4)), GEN_TYPE_UD);
    GenRegister msgAddr = GenRegister::retype(msg, GEN_TYPE_UD);
    GenRegister msgData = GenRegister::retype(ra->genReg(insn.dst(3)), dst.type);

    /* do some calculation within each thread */
    wgOpPerformThread(dst, theVal, threadData, tmp, simd, wg_op, p);

    p->curr.execWidth = simd;
    p->MOV(theVal, dst);
    threadData = GenRegister::toUniform(threadData, dst.type);

    /* store thread count for future use on read/write to SLM */
    if (wg_op == ir::WORKGROUP_OP_ANY ||
      wg_op == ir::WORKGROUP_OP_ALL ||
      wg_op == ir::WORKGROUP_OP_REDUCE_ADD ||
      wg_op == ir::WORKGROUP_OP_REDUCE_MIN ||
      wg_op == ir::WORKGROUP_OP_REDUCE_MAX)
    {
      threadLoop = GenRegister::retype(tmp, GEN_TYPE_D);
      p->MOV(threadLoop, ra->genReg(GenRegister::ud1grf(ir::ocl::threadn)));
    }
    else if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_ADD ||
      wg_op == ir::WORKGROUP_OP_INCLUSIVE_MIN ||
      wg_op == ir::WORKGROUP_OP_INCLUSIVE_MAX ||
      wg_op == ir::WORKGROUP_OP_EXCLUSIVE_ADD ||
      wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MIN ||
      wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MAX)
    {
      threadLoop = GenRegister::retype(tmp, GEN_TYPE_D);
      p->MOV(threadLoop, ra->genReg(GenRegister::ud1grf(ir::ocl::threadid)));
    }

    /* all threads write the partial results to SLM memory */
    if(dst.type == GEN_TYPE_UL || dst.type == GEN_TYPE_L)
    {
      GenRegister threadDataL = GenRegister::retype(threadData, GEN_TYPE_D);
      GenRegister threadDataH = threadDataL.offset(threadDataL, 0, 4);
      GenRegister msgDataL = GenRegister::retype(msgData, GEN_TYPE_D);
      GenRegister msgDataH = msgDataL.offset(msgDataL, 1);
      p->curr.execWidth = 8;
      p->MOV(msgDataL, threadDataL);
      p->MOV(msgDataH, threadDataH);

      p->MUL(msgAddr, threadId, GenRegister::immd(0x8));
      p->ADD(msgAddr, msgAddr, msgSlmOff);
      p->UNTYPED_WRITE(msgAddr, msgData, GenRegister::immw(0xFE), 2, insn.extra.wgop.splitSend);
    }
    else
    {
      p->curr.execWidth = 8;
      p->MOV(msgData, threadData);
      p->MUL(msgAddr, threadId, GenRegister::immd(0x4));
      p->ADD(msgAddr, msgAddr, msgSlmOff);
      p->UNTYPED_WRITE(msgAddr, msgData, GenRegister::immw(0xFE), 1, insn.extra.wgop.splitSend);
    }

    /* init partialData register, it will hold the final result */
    wgOpInitValue(p, partialData, wg_op);

    /* add call to barrier */
    p->push();
      p->curr.execWidth = 8;
      p->curr.physicalFlag = 0;
      p->curr.noMask = 1;
      p->AND(localBarrier, barrierId, GenRegister::immud(0x0f000000));
      p->BARRIER(localBarrier);
      p->curr.execWidth = 1;
      p->WAIT();
    p->pop();

    /* perform a loop, based on thread count (which is now multiple of 4) */
    p->push();{
      jip0 = p->n_instruction();

      /* read in chunks of 4 to optimize SLM reads and reduce SEND messages */
      if(dst.type == GEN_TYPE_UL || dst.type == GEN_TYPE_L)
      {
        p->curr.execWidth = 8;
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->ADD(threadLoop, threadLoop, GenRegister::immd(-1));
        p->MUL(msgAddr, threadLoop, GenRegister::immd(0x8));
        p->ADD(msgAddr, msgAddr, msgSlmOff);
        p->UNTYPED_READ(msgData, msgAddr, GenRegister::immw(0xFE), 2);

        GenRegister msgDataL = msgData.retype(msgData.offset(msgData, 0, 4), GEN_TYPE_D);
        GenRegister msgDataH = msgData.retype(msgData.offset(msgData, 1, 4), GEN_TYPE_D);
        msgDataL.hstride = 2;
        msgDataH.hstride = 2;
        p->MOV(msgDataL, msgDataH);

        /* perform operation, partialData will hold result */
        wgOpPerform(partialData, partialData, msgData.offset(msgData, 0), wg_op, p);
      }
      else
      {
        p->curr.execWidth = 8;
        p->curr.predicate = GEN_PREDICATE_NONE;
        p->ADD(threadLoop, threadLoop, GenRegister::immd(-1));
        p->MUL(msgAddr, threadLoop, GenRegister::immd(0x4));
        p->ADD(msgAddr, msgAddr, msgSlmOff);
        p->UNTYPED_READ(msgData, msgAddr, GenRegister::immw(0xFE), 1);

        /* perform operation, partialData will hold result */
        wgOpPerform(partialData, partialData, msgData.offset(msgData, 0), wg_op, p);
      }

      /* while threadN is not 0, cycle read SLM / update value */
      p->curr.noMask = 1;
      p->curr.flag = 0;
      p->curr.subFlag = 1;
      p->CMP(GEN_CONDITIONAL_G, threadLoop, GenRegister::immd(0x0));
      p->curr.predicate = GEN_PREDICATE_NORMAL;
      jip1 = p->n_instruction();
      p->JMPI(GenRegister::immud(0));
      p->patchJMPI(jip1, jip0 - jip1, 0);
    } p->pop();

    if(wg_op == ir::WORKGROUP_OP_ANY ||
      wg_op == ir::WORKGROUP_OP_ALL ||
      wg_op == ir::WORKGROUP_OP_REDUCE_ADD ||
      wg_op == ir::WORKGROUP_OP_REDUCE_MIN ||
      wg_op == ir::WORKGROUP_OP_REDUCE_MAX)
    {
      /* save result to final register location dst */
      p->curr.execWidth = simd;
      p->MOV(dst, partialData);
    }
    else
    {
      /* save result to final register location dst */
      p->curr.execWidth = simd;

      if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_ADD
          || wg_op == ir::WORKGROUP_OP_EXCLUSIVE_ADD)
        p->ADD(dst, dst, partialData);
      else if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_MIN
        || wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MIN)
      {
        /* workaround QW datatype on CMP */
        if(dst.type == GEN_TYPE_UL || dst.type == GEN_TYPE_L){
          p->push();
            p->curr.execWidth = 8;
            p->SEL_CMP(GEN_CONDITIONAL_LE, dst, dst, partialData);
            if (simd == 16) {
              p->curr.execWidth = 8;
              p->curr.quarterControl = GEN_COMPRESSION_Q2;
              p->SEL_CMP(GEN_CONDITIONAL_LE, GenRegister::Qn(dst, 1),
                         GenRegister::Qn(dst, 1), GenRegister::Qn(partialData, 1));
            }
          p->pop();
        } else
          p->SEL_CMP(GEN_CONDITIONAL_LE, dst, dst, partialData);
      }
      else if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_MAX
        || wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MAX)
      {
        /* workaround QW datatype on CMP */
        if(dst.type == GEN_TYPE_UL || dst.type == GEN_TYPE_L){
          p->push();
            p->curr.execWidth = 8;
            p->SEL_CMP(GEN_CONDITIONAL_GE, dst, dst, partialData);
            if (simd == 16) {
              p->curr.execWidth = 8;
              p->curr.quarterControl = GEN_COMPRESSION_Q2;
              p->SEL_CMP(GEN_CONDITIONAL_GE, GenRegister::Qn(dst, 1),
                         GenRegister::Qn(dst, 1), GenRegister::Qn(partialData, 1));
            }
          p->pop();
        } else
          p->SEL_CMP(GEN_CONDITIONAL_GE, dst, dst, partialData);
      }
    }

    /* corner cases for threads 0 */
    if(wg_op == ir::WORKGROUP_OP_INCLUSIVE_ADD ||
      wg_op == ir::WORKGROUP_OP_INCLUSIVE_MIN ||
      wg_op == ir::WORKGROUP_OP_INCLUSIVE_MAX ||
      wg_op == ir::WORKGROUP_OP_EXCLUSIVE_ADD ||
      wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MIN ||
      wg_op == ir::WORKGROUP_OP_EXCLUSIVE_MAX)
    {
      p->push();{
        p->curr.flag = 0;
        p->curr.subFlag = 1;
        p->CMP(GEN_CONDITIONAL_EQ, threadId, GenRegister::immd(0x0));
        p->curr.predicate = GEN_PREDICATE_NORMAL;

        p->curr.execWidth = simd;
        p->MOV(dst, theVal);
      } p->pop();
    }
  }

  void Gen8Context::emitSubGroupOpInstruction(const SelectionInstruction &insn){
    const GenRegister dst = ra->genReg(insn.dst(0));
    const GenRegister tmp = GenRegister::retype(ra->genReg(insn.dst(1)), dst.type);
    const GenRegister theVal = GenRegister::retype(ra->genReg(insn.src(0)), dst.type);
    GenRegister threadData = ra->genReg(insn.src(1));

    uint32_t wg_op = insn.extra.wgop.workgroupOp;
    uint32_t simd = p->curr.execWidth;

    /* masked elements should be properly set to init value */
    p->push(); {
      p->curr.noMask = 1;
      wgOpInitValue(p, tmp, wg_op);
      p->curr.noMask = 0;
      p->MOV(tmp, theVal);
      p->curr.noMask = 1;
      p->MOV(theVal, tmp);
    } p->pop();

    /* do some calculation within each thread */
    wgOpPerformThread(dst, theVal, threadData, tmp, simd, wg_op, p);
  }

}

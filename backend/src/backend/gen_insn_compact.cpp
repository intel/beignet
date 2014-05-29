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
 * Author: Ruiling Song <ruiling.song@intel.com>
 */
#include "backend/gen_defs.hpp"
#include "backend/gen_encoder.hpp"
#include <cstring>

namespace gbe {

  struct compact_table_entry {
    uint32_t bit_pattern;
    uint32_t index;
  };

  static compact_table_entry control_table[] = {
    {0b0000000000000000010, 0},
    {0b0000100000000000000, 1},
    {0b0000100000000000001, 2},
    {0b0000100000000000010, 3},
    {0b0000100000000000011, 4},
    {0b0000100000000000100, 5},
    {0b0000100000000000101, 6},
    {0b0000100000000000111, 7},
    {0b0000100000000001000, 8},
    {0b0000100000000001001, 9},
    {0b0000100000000001101, 10},
    {0b0000110000000000000, 11},
    {0b0000110000000000001, 12},
    {0b0000110000000000010, 13},
    {0b0000110000000000011, 14},
    {0b0000110000000000100, 15},
    {0b0000110000000000101, 16},
    {0b0000110000000000111, 17},
    {0b0000110000000001001, 18},
    {0b0000110000000001101, 19},
    {0b0000110000000010000, 20},
    {0b0000110000100000000, 21},
    {0b0001000000000000000, 22},
    {0b0001000000000000010, 23},
    {0b0001000000000000100, 24},
    {0b0001000000100000000, 25},
    {0b0010110000000000000, 26},
    {0b0010110000000010000, 27},
    {0b0011000000000000000, 28},
    {0b0011000000100000000, 29},
    {0b0101000000000000000, 30},
    {0b0101000000100000000, 31},
  };

  static compact_table_entry data_type_table[] = {
    {0b000000001000001100, 20},
    {0b001000000000000001, 0},
    {0b001000000000100000, 1},
    {0b001000000000100001, 2},
    {0b001000000000111101, 21},
    {0b001000000001100001, 3},
    {0b001000000010100101, 22},
    {0b001000000010111101, 4},
    {0b001000001011111101, 5},
    {0b001000001110100001, 6},
    {0b001000001110100101, 7},
    {0b001000001110111101, 8},
    {0b001000010000100000, 23},
    {0b001000010000100001, 9},
    {0b001000110000100000, 10},
    {0b001000110000100001, 11},
    {0b001001010010100100, 24},
    {0b001001010010100101, 12},
    {0b001001110010000100, 25},
    {0b001001110010100100, 13},
    {0b001001110010100101, 14},
    {0b001010010100001001, 26},
    {0b001010010100101000, 30},
    {0b001010110100101000, 31},
    {0b001011110110101100, 29},
    {0b001101111110111101, 27},
    {0b001111001110111101, 15},
    {0b001111011110011101, 16},
    {0b001111011110111100, 17},
    {0b001111011110111101, 18},
    {0b001111111110111100, 19},
    {0b001111111110111101, 28},
  };

  static compact_table_entry data_type_decompact[] = {
    {0b001000000000000001, 0},
    {0b001000000000100000, 1},
    {0b001000000000100001, 2},
    {0b001000000001100001, 3},
    {0b001000000010111101, 4},
    {0b001000001011111101, 5},
    {0b001000001110100001, 6},
    {0b001000001110100101, 7},
    {0b001000001110111101, 8},
    {0b001000010000100001, 9},
    {0b001000110000100000, 10},
    {0b001000110000100001, 11},
    {0b001001010010100101, 12},
    {0b001001110010100100, 13},
    {0b001001110010100101, 14},
    {0b001111001110111101, 15},
    {0b001111011110011101, 16},
    {0b001111011110111100, 17},
    {0b001111011110111101, 18},
    {0b001111111110111100, 19},
    {0b000000001000001100, 20},
    {0b001000000000111101, 21},
    {0b001000000010100101, 22},
    {0b001000010000100000, 23},
    {0b001001010010100100, 24},
    {0b001001110010000100, 25},
    {0b001010010100001001, 26},
    {0b001101111110111101, 27},
    {0b001111111110111101, 28},
    {0b001011110110101100, 29},
    {0b001010010100101000, 30},
    {0b001010110100101000, 31},
  };

  static compact_table_entry subreg_table[] = {
    {0b000000000000000, 0},
    {0b000000000000001, 1},
    {0b000000000001000, 2},
    {0b000000000001111, 3},
    {0b000000000010000, 4},
    {0b000000010000000, 5},
    {0b000000100000000, 6},
    {0b000000110000000, 7},
    {0b000001000000000, 8},
    {0b000001000010000, 9},
    {0b000001010000000, 10},
    {0b001000000000000, 11},
    {0b001000000000001, 12},
    {0b001000010000001, 13},
    {0b001000010000010, 14},
    {0b001000010000011, 15},
    {0b001000010000100, 16},
    {0b001000010000111, 17},
    {0b001000010001000, 18},
    {0b001000010001110, 19},
    {0b001000010001111, 20},
    {0b001000110000000, 21},
    {0b001000111101000, 22},
    {0b010000000000000, 23},
    {0b010000110000000, 24},
    {0b011000000000000, 25},
    {0b011110010000111, 26},
    {0b100000000000000, 27},
    {0b101000000000000, 28},
    {0b110000000000000, 29},
    {0b111000000000000, 30},
    {0b111000000011100, 31},
  };

  static compact_table_entry srcreg_table[] = {
    {0b000000000000, 0},
    {0b000000000010, 1},
    {0b000000010000, 2},
    {0b000000010010, 3},
    {0b000000011000, 4},
    {0b000000100000, 5},
    {0b000000101000, 6},
    {0b000001001000, 7},
    {0b000001010000, 8},
    {0b000001110000, 9},
    {0b000001111000, 10},
    {0b001100000000, 11},
    {0b001100000010, 12},
    {0b001100001000, 13},
    {0b001100010000, 14},
    {0b001100010010, 15},
    {0b001100100000, 16},
    {0b001100101000, 17},
    {0b001100111000, 18},
    {0b001101000000, 19},
    {0b001101000010, 20},
    {0b001101001000, 21},
    {0b001101010000, 22},
    {0b001101100000, 23},
    {0b001101101000, 24},
    {0b001101110000, 25},
    {0b001101110001, 26},
    {0b001101111000, 27},
    {0b010001101000, 28},
    {0b010001101001, 29},
    {0b010001101010, 30},
    {0b010110001000, 31},
  };

  static int cmp_key(const void *p1, const void*p2) {
    const compact_table_entry * px = (compact_table_entry *)p1;
    const compact_table_entry * py = (compact_table_entry *)p2;
    return (px->bit_pattern) - py->bit_pattern;
  }
  union ControlBits{
    struct {
      uint32_t access_mode:1;
      uint32_t mask_control:1;
      uint32_t dependency_control:2;
      uint32_t quarter_control:2;
      uint32_t thread_control:2;
      uint32_t predicate_control:4;
      uint32_t predicate_inverse:1;
      uint32_t execution_size:3;
      uint32_t saturate:1;
      uint32_t flag_sub_reg_nr:1;
      uint32_t flag_reg_nr:1;
      uint32_t pad:23;
    };
    uint32_t data;
  };
  union DataTypeBits{
    struct {
      uint32_t dest_reg_file:2;
      uint32_t dest_reg_type:3;
      uint32_t src0_reg_file:2;
      uint32_t src0_reg_type:3;
      uint32_t src1_reg_file:2;
      uint32_t src1_reg_type:3;
      uint32_t dest_horiz_stride:2;
      uint32_t dest_address_mode:1;
      uint32_t pad:14;
    };
    uint32_t data;
  };
  union SubRegBits {
    struct {
      uint32_t dest_subreg_nr:5;
      uint32_t src0_subreg_nr:5;
      uint32_t src1_subreg_nr:5;
      uint32_t pad:17;
    };
    uint32_t data;
  };
  union SrcRegBits {
    struct {
      uint32_t src_abs:1;
      uint32_t src_negate:1;
      uint32_t src_address_mode:1;
      uint32_t src_horiz_stride:2;
      uint32_t src_width:3;
      uint32_t src_vert_stride:4;
      uint32_t pad:20;
    };
    uint32_t data;
  };

  void decompactInstruction(GenCompactInstruction * p, GenNativeInstruction *pOut) {

    memset(pOut, 0, sizeof(GenNativeInstruction));
    union ControlBits control_bits;
    control_bits.data = control_table[(uint32_t)p->bits1.control_index].bit_pattern;
    pOut->low.low = (uint32_t)p->bits1.opcode | ((control_bits.data & 0xffff) << 8);
    pOut->header.destreg_or_condmod = p->bits1.destreg_or_condmod;
    pOut->header.saturate = control_bits.saturate;
    pOut->header.acc_wr_control = p->bits1.acc_wr_control;
    pOut->header.cmpt_control = p->bits1.cmpt_control;
    pOut->header.debug_control = p->bits1.debug_control;

    union DataTypeBits data_type_bits;
    union SubRegBits subreg_bits;
    union SrcRegBits src0_bits;
    data_type_bits.data = data_type_decompact[(uint32_t)p->bits1.data_type_index].bit_pattern;
    subreg_bits.data = subreg_table[(uint32_t)p->bits1.sub_reg_index].bit_pattern;
    src0_bits.data = srcreg_table[p->bits1.src0_index_lo | p->bits2.src0_index_hi << 2].bit_pattern;

    pOut->low.high |= data_type_bits.data & 0x7fff;
    pOut->bits1.da1.dest_horiz_stride = data_type_bits.dest_horiz_stride;
    pOut->bits1.da1.dest_address_mode = data_type_bits.dest_address_mode;
    pOut->bits1.da1.dest_reg_nr = p->bits2.dest_reg_nr;
    pOut->bits1.da1.dest_subreg_nr = subreg_bits.dest_subreg_nr;

    pOut->bits2.da1.src0_subreg_nr = subreg_bits.src0_subreg_nr;
    pOut->bits2.da1.src0_reg_nr = p->bits2.src0_reg_nr;
    pOut->high.low |= (src0_bits.data << 13);
    pOut->bits2.da1.flag_sub_reg_nr = control_bits.flag_sub_reg_nr;
    pOut->bits2.da1.flag_reg_nr = control_bits.flag_reg_nr;

    if(data_type_bits.src1_reg_file == GEN_IMMEDIATE_VALUE) {
      uint32_t imm = (uint32_t)p->bits2.src1_reg_nr | (p->bits2.src1_index<<8);
      pOut->bits3.ud = imm & 0x1000 ? (imm | 0xfffff000) : imm;
    } else {
      union SrcRegBits src1_bits;
      src1_bits.data = srcreg_table[p->bits2.src1_index].bit_pattern;
      pOut->bits3.da1.src1_subreg_nr = subreg_bits.src1_subreg_nr;
      pOut->bits3.da1.src1_reg_nr = p->bits2.src1_reg_nr;
      pOut->high.high |= (src1_bits.data << 13);
    }
  }

  int compactControlBits(GenEncoder *p, uint32_t quarter, uint32_t execWidth) {

    const GenInstructionState *s = &p->curr;
    // some quick check
    if(s->nibControl != 0)
      return -1;
    if(s->predicate > GEN_PREDICATE_NORMAL)
      return -1;
    if(s->flag == 1)
      return -1;

    ControlBits b;
    b.data = 0;

    if (execWidth == 8)
      b.execution_size = GEN_WIDTH_8;
    else if (execWidth == 16)
      b.execution_size = GEN_WIDTH_16;
    else if (execWidth == 4)
      b.execution_size = GEN_WIDTH_4;
    else if (execWidth == 1)
      b.execution_size = GEN_WIDTH_1;
    else
      NOT_IMPLEMENTED;

    b.mask_control = s->noMask;
    b.quarter_control = quarter;
    b.predicate_control = s->predicate;
    b.predicate_inverse = s->inversePredicate;

    b.saturate = s->saturate;
    b.flag_sub_reg_nr = s->subFlag;
    b.flag_reg_nr = s->flag;

    compact_table_entry key;
    key.bit_pattern = b.data;

    compact_table_entry *r = (compact_table_entry *)bsearch(&key, control_table,
      sizeof(control_table)/sizeof(compact_table_entry), sizeof(compact_table_entry), cmp_key);
    if (r == NULL)
      return -1;
    return r->index;
  }

  int compactDataTypeBits(GenEncoder *p, GenRegister *dst, GenRegister *src0, GenRegister *src1) {

    // compact does not support any indirect acess
    if(dst->address_mode != GEN_ADDRESS_DIRECT)
      return -1;

    if(src0->file == GEN_IMMEDIATE_VALUE)
      return -1;

    DataTypeBits b;
    b.data = 0;

    b.dest_horiz_stride = dst->hstride == GEN_HORIZONTAL_STRIDE_0 ? GEN_HORIZONTAL_STRIDE_1 : dst->hstride;
    b.dest_address_mode = dst->address_mode;
    b.dest_reg_file = dst->file;
    b.dest_reg_type = dst->type;

    b.src0_reg_file = src0->file;
    b.src0_reg_type = src0->type;

    if(src1) {
      b.src1_reg_type = src1->type;
      b.src1_reg_file = src1->file;
    } else {
      // default to zero
      b.src1_reg_type = 0;
      b.src1_reg_file = 0;
    }

    compact_table_entry key;
    key.bit_pattern = b.data;

    compact_table_entry *r = (compact_table_entry *)bsearch(&key, data_type_table,
                             sizeof(data_type_table)/sizeof(compact_table_entry), sizeof(compact_table_entry), cmp_key);
    if (r == NULL)
      return -1;
    return r->index;
  }
  int compactSubRegBits(GenEncoder *p, GenRegister *dst, GenRegister *src0, GenRegister *src1) {
    SubRegBits b;
    b.data = 0;
    b.dest_subreg_nr = dst->subnr;
    b.src0_subreg_nr = src0->subnr;
    if(src1)
      b.src1_subreg_nr = src1->subnr;
    else
      b.src1_subreg_nr = 0;

    compact_table_entry key;
    key.bit_pattern = b.data;

    compact_table_entry *r = (compact_table_entry *)bsearch(&key, subreg_table,
                sizeof(subreg_table)/sizeof(compact_table_entry), sizeof(compact_table_entry), cmp_key);
    if (r == NULL)
      return -1;
    return r->index;
  }
  int compactSrcRegBits(GenEncoder *p, GenRegister *src) {
    // As we only use GEN_ALIGN_1 and compact only support direct register access,
    // we only need to verify [hstride, width, vstride]
    if(src->file == GEN_IMMEDIATE_VALUE)
      return -1;
    if(src->address_mode != GEN_ADDRESS_DIRECT)
      return -1;

    SrcRegBits b;
    b.data = 0;
    b.src_abs = src->absolute;
    b.src_negate = src->negation;
    b.src_address_mode = src->address_mode;
    if(p->curr.execWidth == 1 && src->width == GEN_WIDTH_1) {
      b.src_width = src->width;
      b.src_horiz_stride = GEN_HORIZONTAL_STRIDE_0;
      b.src_vert_stride = GEN_VERTICAL_STRIDE_0;
    }
    else {
      b.src_horiz_stride = src->hstride;
      b.src_width = src->width;
      b.src_vert_stride = src->vstride;
    }
    compact_table_entry key;
    key.bit_pattern = b.data;

    compact_table_entry *r = (compact_table_entry *)bsearch(&key, srcreg_table,
                    sizeof(srcreg_table)/sizeof(compact_table_entry), sizeof(compact_table_entry), cmp_key);
    if (r == NULL)
      return -1;
    return r->index;
  }

  bool compactAlu1(GenEncoder *p, uint32_t opcode, GenRegister dst, GenRegister src, uint32_t condition, bool split) {
    if(split) {
      // TODO support it
      return false;
    } else {
      int control_index = compactControlBits(p, p->curr.quarterControl, p->curr.execWidth);
      if(control_index == -1) return false;

      int data_type_index = compactDataTypeBits(p, &dst, &src, NULL);
      if(data_type_index == -1) return false;

      int sub_reg_index = compactSubRegBits(p, &dst, &src, NULL);
      if(sub_reg_index == -1) return false;

      int src_reg_index = compactSrcRegBits(p, &src);
      if(src_reg_index == -1) return false;

      GenCompactInstruction * insn = p->nextCompact(opcode);
      insn->bits1.control_index = control_index;
      insn->bits1.data_type_index = data_type_index;
      insn->bits1.sub_reg_index = sub_reg_index;
      insn->bits1.acc_wr_control = p->curr.accWrEnable;
      insn->bits1.destreg_or_condmod = condition;
      insn->bits1.cmpt_control = 1;
      insn->bits1.src0_index_lo = src_reg_index & 3;

      insn->bits2.src0_index_hi = src_reg_index >> 2;
      insn->bits2.src1_index = 0;
      insn->bits2.dest_reg_nr = dst.nr;
      insn->bits2.src0_reg_nr = src.nr;
      insn->bits2.src1_reg_nr = 0;
      return true;
    }
  }

  bool compactAlu2(GenEncoder *p, uint32_t opcode, GenRegister dst, GenRegister src0, GenRegister src1, uint32_t condition, bool split) {
    if(split) {
      // TODO support it
      return false;
    } else {
      if(opcode == GEN_OPCODE_IF  || opcode == GEN_OPCODE_ENDIF || opcode == GEN_OPCODE_JMPI) return false;

      int control_index = compactControlBits(p, p->curr.quarterControl, p->curr.execWidth);
      if(control_index == -1) return false;

      int data_type_index = compactDataTypeBits(p, &dst, &src0, &src1);
      if(data_type_index == -1) return false;

      int sub_reg_index = compactSubRegBits(p, &dst, &src0, &src1);
      if(sub_reg_index == -1) return false;

      int src0_reg_index = compactSrcRegBits(p, &src0);
      if(src0_reg_index == -1) return false;

      bool src1_imm = false;
      int src1_reg_index;
      if(src1.file == GEN_IMMEDIATE_VALUE) {
        if(src1.absolute != 0 || src1.negation != 0 || src1.type == GEN_TYPE_F)
          return false;
        if(src1.value.d < -4096 || src1.value.d > 4095) // 13bit signed imm
          return false;
        src1_imm = true;
      } else {
        src1_reg_index = compactSrcRegBits(p, &src1);
        if(src1_reg_index == -1) return false;
      }
      GenCompactInstruction * insn = p->nextCompact(opcode);
      insn->bits1.control_index = control_index;
      insn->bits1.data_type_index = data_type_index;
      insn->bits1.sub_reg_index = sub_reg_index;
      insn->bits1.acc_wr_control = p->curr.accWrEnable;
      insn->bits1.destreg_or_condmod = condition;
      insn->bits1.cmpt_control = 1;
      insn->bits1.src0_index_lo = src0_reg_index & 3;

      insn->bits2.src0_index_hi = src0_reg_index >> 2;
      insn->bits2.src1_index = src1_imm ? (src1.value.ud & 8191)>> 8 : src1_reg_index;
      insn->bits2.dest_reg_nr = dst.nr;
      insn->bits2.src0_reg_nr = src0.nr;
      insn->bits2.src1_reg_nr = src1_imm ? (src1.value.ud & 0xff): src1.nr;
      return true;
    }
  }
};

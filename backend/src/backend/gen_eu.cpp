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
  void GenEmitter::guess_execution_size(GenInstruction *insn, GenReg reg)
  {
     if (reg.width == GEN_WIDTH_8 && this->compressed)
        insn->header.execution_size = GEN_EXECUTE_16;
     else
        insn->header.execution_size = reg.width;
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

  /* How does predicate control work when execution_size != 8?  Do I
   * need to test/set for 0xffff when execution_size is 16?
   */
  void GenEmitter::set_predicate_control_flag_value(uint32_t value)
  {
     if (value != 0xff) {
        if (value != this->flag_value) {
           this->MOV(brw_flag_reg(), brw_imm_uw(value));
           this->flag_value = value;
        }
     }
  }

  void GenEmitter::set_predicate_control(uint32_t pc)
  {
    // p->current->header.predicate_control = pc;
  }

  void GenEmitter::set_predicate_inverse(bool predicate_inverse)
  {
    // p->current->header.predicate_inverse = predicate_inverse;
  }

  void GenEmitter::set_conditionalmod(uint32_t conditional)
  {
   //  p->current->header.destreg__conditionalmod = conditional;
  }

  void GenEmitter::set_access_mode(uint32_t access_mode)
  {
    // p->current->header.access_mode = access_mode;
  }

  void
  GenEmitter::set_compression_control(enum brw_compression compression_control)
  {
#if 0
     p->compressed = (compression_control == GEN_COMPRESSION_COMPRESSED);

     if (p->gen >= 6) {
        /* Since we don't use the 32-wide support in gen6, we translate
         * the pre-gen6 compression control here.
         */
        switch (compression_control) {
        case GEN_COMPRESSION_NONE:
           /* This is the "use the first set of bits of dmask/vmask/arf
            * according to execsize" option.
            */
           p->current->header.compression_control = GEN6_COMPRESSION_1Q;
           break;
        case GEN_COMPRESSION_2NDHALF:
           /* For 8-wide, this is "use the second set of 8 bits." */
           p->current->header.compression_control = GEN6_COMPRESSION_2Q;
           break;
        case GEN_COMPRESSION_COMPRESSED:
           /* For 16-wide instruction compression, use the first set of 16 bits
            * since we don't do 32-wide dispatch.
            */
           p->current->header.compression_control = GEN6_COMPRESSION_1H;
           break;
        default:
           assert(!"not reached");
           p->current->header.compression_control = GEN6_COMPRESSION_1H;
           break;
        }
     } else {
        p->current->header.compression_control = compression_control;
     }
#endif
  }

  void GenEmitter::set_dest(GenInstruction *insn, GenReg dest)
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

     /* NEW: Set the execution size based on dest.width and
      * insn->compression_control:
      */
     guess_execution_size(insn, dest);
  }

  static const int reg_type_size[8] = { 4, 4, 2, 2, 1, 1, 4 };

  static void
  validate_reg(GenInstruction *insn, GenReg reg)
  {
     int hstride_for_reg[] = {0, 1, 2, 4};
     int vstride_for_reg[] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256};
     int width_for_reg[] = {1, 2, 4, 8, 16};
     int execsize_for_reg[] = {1, 2, 4, 8, 16};
     int width, hstride, vstride, execsize;

     if (reg.file == GEN_IMMEDIATE_VALUE) {
        /* 3.3.6: Region Parameters.  Restriction: Immediate vectors
         * mean the destination has to be 128-bit aligned and the
         * destination horiz stride has to be a word.
         */
        if (reg.type == GEN_REGISTER_TYPE_V) {
           assert(hstride_for_reg[insn->bits1.da1.dest_horiz_stride] *
                  reg_type_size[insn->bits1.da1.dest_reg_type] == 2);
        }

        return;
     }

     if (reg.file == GEN_ARCHITECTURE_REGISTER_FILE &&
         reg.file == GEN_ARF_NULL)
        return;

     assert(reg.hstride >= 0 && reg.hstride < ARRAY_ELEM_NUM(hstride_for_reg));
     hstride = hstride_for_reg[reg.hstride];

     if (reg.vstride == 0xf) {
        vstride = -1;
     } else {
        assert(reg.vstride >= 0 && reg.vstride < ARRAY_ELEM_NUM(vstride_for_reg));
        vstride = vstride_for_reg[reg.vstride];
     }

     assert(reg.width >= 0 && reg.width < ARRAY_ELEM_NUM(width_for_reg));
     width = width_for_reg[reg.width];

     assert(insn->header.execution_size >= 0 &&
            insn->header.execution_size < ARRAY_ELEM_NUM(execsize_for_reg));
     execsize = execsize_for_reg[insn->header.execution_size];

     /* Restrictions from 3.3.10: Register Region Restrictions. */
     /* 3. */
     assert(execsize >= width);

     /* 4. */
     if (execsize == width && hstride != 0) {
        assert(vstride == -1 || vstride == width * hstride);
     }

     /* 5. */
     if (execsize == width && hstride == 0) {
        /* no restriction on vstride. */
     }

     /* 6. */
     if (width == 1) {
        assert(hstride == 0);
     }

     /* 7. */
     if (execsize == 1 && width == 1) {
        assert(hstride == 0);
        assert(vstride == 0);
     }

     /* 8. */
     if (vstride == 0 && hstride == 0) {
        assert(width == 1);
     }

     /* 10. Check destination issues. */
  }

  void
  GenEmitter::set_src0(GenInstruction *insn, GenReg reg)
  {
     if (reg.type != GEN_ARCHITECTURE_REGISTER_FILE)
        assert(reg.nr < 128);

     validate_reg(insn, reg);

     insn->bits1.da1.src0_reg_file = reg.file;
     insn->bits1.da1.src0_reg_type = reg.type;
     insn->bits2.da1.src0_abs = reg.abs;
     insn->bits2.da1.src0_negate = reg.negate;
     insn->bits2.da1.src0_address_mode = reg.address_mode;

     if (reg.file == GEN_IMMEDIATE_VALUE) {
        insn->bits3.ud = reg.dw1.ud;
     
        /* Required to set some fields in src1 as well:
         */
        insn->bits1.da1.src1_reg_file = 0; /* arf */
        insn->bits1.da1.src1_reg_type = reg.type;
     }
     else 
     {
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
               insn->header.execution_size == GEN_EXECUTE_1) {
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


  void GenEmitter::set_src1(GenInstruction *insn, GenReg reg)
  {
     assert(reg.nr < 128);

     validate_reg(insn, reg);

     insn->bits1.da1.src1_reg_file = reg.file;
     insn->bits1.da1.src1_reg_type = reg.type;
     insn->bits3.da1.src1_abs = reg.abs;
     insn->bits3.da1.src1_negate = reg.negate;

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
               insn->header.execution_size == GEN_EXECUTE_1) {
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
                             enum brw_message_target sfid,
                             unsigned msg_length,
                             unsigned response_length,
                             bool header_present,
                             bool end_of_thread)
  {
     p->set_src1(inst, brw_imm_d(0));
     inst->bits3.generic_gen5.header_present = header_present;
     inst->bits3.generic_gen5.response_length = response_length;
     inst->bits3.generic_gen5.msg_length = msg_length;
     inst->bits3.generic_gen5.end_of_thread = end_of_thread;
     inst->header.destreg__conditionalmod = sfid;
  }

  static void brw_set_math_message(GenEmitter *p,
                                   GenInstruction *insn,
                                   uint32_t function,
                                   uint32_t integer_type,
                                   bool low_precision,
                                   bool saturate,
                                   uint32_t dataType)
  {
     unsigned msg_length;
     unsigned response_length;

     /* Infer message length from the function */
     switch (function) {
     case GEN_MATH_FUNCTION_POW:
     case GEN_MATH_FUNCTION_INT_DIV_QUOTIENT:
     case GEN_MATH_FUNCTION_INT_DIV_REMAINDER:
     case GEN_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER:
        msg_length = 2;
        break;
     default:
        msg_length = 1;
        break;
     }

     /* Infer response length from the function */
     switch (function) {
     case GEN_MATH_FUNCTION_SINCOS:
     case GEN_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER:
        response_length = 2;
        break;
     default:
        response_length = 1;
        break;
     }

     brw_set_message_descriptor(p, insn, GEN_SFID_MATH,
                                msg_length, response_length, false, false);
     if (p->gen == 5) {
        insn->bits3.math_gen5.function = function;
        insn->bits3.math_gen5.int_type = integer_type;
        insn->bits3.math_gen5.precision = low_precision;
        insn->bits3.math_gen5.saturate = saturate;
        insn->bits3.math_gen5.data_type = dataType;
        insn->bits3.math_gen5.snapshot = 0;
     } else {
        insn->bits3.math.function = function;
        insn->bits3.math.int_type = integer_type;
        insn->bits3.math.precision = low_precision;
        insn->bits3.math.saturate = saturate;
        insn->bits3.math.data_type = dataType;
     }
  }

  void
  GenEmitter::set_dp_write_message(GenInstruction *insn,
                                   uint32_t bti,
                                   uint32_t msg_control,
                                   uint32_t msg_type,
                                   uint32_t msg_length,
                                   bool header_present,
                                   uint32_t last_render_target,
                                   uint32_t response_length,
                                   uint32_t end_of_thread,
                                   uint32_t send_commit_msg)
  {
     unsigned sfid;

     /* Use the Render Cache for RT writes; otherwise use the Data Cache */
     if (msg_type == GEN6_DATAPORT_WRITE_MESSAGE_RENDER_TARGET_WRITE)
      sfid = GEN6_SFID_DATAPORT_RENDER_CACHE;
     else
      sfid = GEN7_SFID_DATAPORT_DATA_CACHE;
     brw_set_message_descriptor(this, insn, brw_message_target(sfid),
                                msg_length, response_length,
                                header_present, end_of_thread);

     insn->bits3.gen7_dp.bti = bti;
     insn->bits3.gen7_dp.msg_control = msg_control;
     insn->bits3.gen7_dp.last_render_target = last_render_target;
     insn->bits3.gen7_dp.msg_type = msg_type;
  }

  void
  GenEmitter::set_dp_read_message(GenInstruction *insn,
                                       uint32_t bti,
                                       uint32_t msg_control,
                                       uint32_t msg_type,
                                       uint32_t target_cache,
                                       uint32_t msg_length,
                                       uint32_t response_length)
  {
     unsigned sfid;

     sfid = GEN7_SFID_DATAPORT_DATA_CACHE;
     brw_set_message_descriptor(this, insn, brw_message_target(sfid), msg_length, response_length,
                                true, false);

     insn->bits3.gen7_dp.bti = bti;
     insn->bits3.gen7_dp.msg_control = msg_control;
     insn->bits3.gen7_dp.last_render_target = 0;
     insn->bits3.gen7_dp.msg_type = msg_type;
  }

  void
  GenEmitter::set_sampler_message(GenInstruction *insn,
                                  uint32_t bti,
                                  uint32_t sampler,
                                  uint32_t msg_type,
                                  uint32_t response_length,
                                  uint32_t msg_length,
                                  uint32_t header_present,
                                  uint32_t simd_mode,
                                  uint32_t return_format)
  {
     brw_set_message_descriptor(this, insn, GEN_SFID_SAMPLER, msg_length,
                                response_length, header_present, false);
     insn->bits3.sampler_gen7.bti = bti;
     insn->bits3.sampler_gen7.sampler = sampler;
     insn->bits3.sampler_gen7.msg_type = msg_type;
     insn->bits3.sampler_gen7.simd_mode = simd_mode;
  }

  GenInstruction *GenEmitter::next(uint32_t opcode)
  {
     GenInstruction *insn;
     insn = &this->store[this->nr_insn++];
     insn->header.opcode = opcode;
     return insn;
  }

  static GenInstruction *brw_alu1(GenEmitter *p,
                                   uint32_t opcode,
                                   GenReg dest,
                                   GenReg src)
  {
     GenInstruction *insn = p->next(opcode);
     p->set_dest(insn, dest);
     p->set_src0(insn, src);
     return insn;
  }

  static GenInstruction *brw_alu2(GenEmitter *p,
                                   uint32_t opcode,
                                   GenReg dest,
                                   GenReg src0,
                                   GenReg src1)
  {
     GenInstruction *insn = p->next(opcode);
     p->set_dest(insn, dest);
     p->set_src0(insn, src0);
     p->set_src1(insn, src1);
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
     assert(dest.type = GEN_REGISTER_TYPE_F);
     insn->bits1.da3src.dest_reg_file = 0;
     insn->bits1.da3src.dest_reg_nr = dest.nr;
     insn->bits1.da3src.dest_subreg_nr = dest.subnr / 16;
     insn->bits1.da3src.dest_writemask = dest.dw1.bits.writemask;
     p->guess_execution_size(insn, dest);

     assert(src0.file == GEN_GENERAL_REGISTER_FILE);
     assert(src0.address_mode == GEN_ADDRESS_DIRECT);
     assert(src0.nr < 128);
     assert(src0.type == GEN_REGISTER_TYPE_F);
     insn->bits2.da3src.src0_swizzle = src0.dw1.bits.swizzle;
     insn->bits2.da3src.src0_subreg_nr = get_3src_subreg_nr(src0);
     insn->bits2.da3src.src0_reg_nr = src0.nr;
     insn->bits1.da3src.src0_abs = src0.abs;
     insn->bits1.da3src.src0_negate = src0.negate;
     insn->bits2.da3src.src0_rep_ctrl = src0.vstride == GEN_VERTICAL_STRIDE_0;

     assert(src1.file == GEN_GENERAL_REGISTER_FILE);
     assert(src1.address_mode == GEN_ADDRESS_DIRECT);
     assert(src1.nr < 128);
     assert(src1.type == GEN_REGISTER_TYPE_F);
     insn->bits2.da3src.src1_swizzle = src1.dw1.bits.swizzle;
     insn->bits2.da3src.src1_subreg_nr_low = get_3src_subreg_nr(src1) & 0x3;
     insn->bits3.da3src.src1_subreg_nr_high = get_3src_subreg_nr(src1) >> 2;
     insn->bits2.da3src.src1_rep_ctrl = src1.vstride == GEN_VERTICAL_STRIDE_0;
     insn->bits3.da3src.src1_reg_nr = src1.nr;
     insn->bits1.da3src.src1_abs = src1.abs;
     insn->bits1.da3src.src1_negate = src1.negate;

     assert(src2.file == GEN_GENERAL_REGISTER_FILE);
     assert(src2.address_mode == GEN_ADDRESS_DIRECT);
     assert(src2.nr < 128);
     assert(src2.type == GEN_REGISTER_TYPE_F);
     insn->bits3.da3src.src2_swizzle = src2.dw1.bits.swizzle;
     insn->bits3.da3src.src2_subreg_nr = get_3src_subreg_nr(src2);
     insn->bits3.da3src.src2_rep_ctrl = src2.vstride == GEN_VERTICAL_STRIDE_0;
     insn->bits3.da3src.src2_reg_nr = src2.nr;
     insn->bits1.da3src.src2_abs = src2.abs;
     insn->bits1.da3src.src2_negate = src2.negate;

     return insn;
  }


  /***********************************************************************
   * Convenience routines.
   */
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

  /* Rounding operations (other than RNDD) require two instructions - the first
   * stores a rounded value (possibly the wrong way) in the dest register, but
   * also sets a per-channel "increment bit" in the flag register.  A predicated
   * add of 1.0 fixes dest to contain the desired result.
   *
   * Sandybridge and later appear to round correctly without an ADD.
   */
#define ROUND(OP) \
  void GenEmitter::OP(GenReg dest, GenReg src) \
  { \
     GenInstruction *rnd; \
     rnd = this->next(GEN_OPCODE_##OP);  \
     this->set_dest(rnd, dest); \
     this->set_src0(rnd, src); \
  }

  ALU1(MOV)
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
  ALU2(MACH)
  ALU1(LZD)
  ALU2(DP4)
  ALU2(DPH)
  ALU2(DP3)
  ALU2(DP2)
  ALU2(LINE)
  ALU2(PLN)
  ALU3(MAD)

  ROUND(RNDZ)
  ROUND(RNDE)

  GenInstruction *GenEmitter::ADD(GenReg dest, GenReg src0, GenReg src1)
  {
     /* 6.2.2: add */
     if (src0.type == GEN_REGISTER_TYPE_F ||
         (src0.file == GEN_IMMEDIATE_VALUE &&
          src0.type == GEN_REGISTER_TYPE_VF)) {
        assert(src1.type != GEN_REGISTER_TYPE_UD);
        assert(src1.type != GEN_REGISTER_TYPE_D);
     }

     if (src1.type == GEN_REGISTER_TYPE_F ||
         (src1.file == GEN_IMMEDIATE_VALUE &&
          src1.type == GEN_REGISTER_TYPE_VF)) {
        assert(src0.type != GEN_REGISTER_TYPE_UD);
        assert(src0.type != GEN_REGISTER_TYPE_D);
     }

     return brw_alu2(this, GEN_OPCODE_ADD, dest, src0, src1);
  }

  GenInstruction *GenEmitter::MUL(GenReg dest, GenReg src0, GenReg src1)
  {
     /* 6.32.38: mul */
     if (src0.type == GEN_REGISTER_TYPE_D ||
         src0.type == GEN_REGISTER_TYPE_UD ||
         src1.type == GEN_REGISTER_TYPE_D ||
         src1.type == GEN_REGISTER_TYPE_UD) {
        assert(dest.type != GEN_REGISTER_TYPE_F);
     }

     if (src0.type == GEN_REGISTER_TYPE_F ||
         (src0.file == GEN_IMMEDIATE_VALUE &&
          src0.type == GEN_REGISTER_TYPE_VF)) {
        assert(src1.type != GEN_REGISTER_TYPE_UD);
        assert(src1.type != GEN_REGISTER_TYPE_D);
     }

     if (src1.type == GEN_REGISTER_TYPE_F ||
         (src1.file == GEN_IMMEDIATE_VALUE &&
          src1.type == GEN_REGISTER_TYPE_VF)) {
        assert(src0.type != GEN_REGISTER_TYPE_UD);
        assert(src0.type != GEN_REGISTER_TYPE_D);
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
     this->set_dest(insn, retype(brw_vec4_grf(0,0), GEN_REGISTER_TYPE_UD));
     this->set_src0(insn, retype(brw_vec4_grf(0,0), GEN_REGISTER_TYPE_UD));
     this->set_src1(insn, brw_imm_ud(0x0));
  }

  GenInstruction *GenEmitter::JMPI(GenReg dest, GenReg src0, GenReg src1)
  {
     GenInstruction *insn = brw_alu2(this, GEN_OPCODE_JMPI, dest, src0, src1);
     insn->header.execution_size = 1;
     insn->header.compression_control = GEN_COMPRESSION_NONE;
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
     this->set_dest(insn, dest);
     this->set_src0(insn, src0);
     this->set_src1(insn, src1);

  /*    guess_execution_size(insn, src0); */
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
     GenReg src = brw_notification_1_reg();

     this->set_dest(insn, src);
     this->set_src0(insn, src);
     this->set_src1(insn, brw_null_reg());
     insn->header.execution_size = 0; /* must */
     insn->header.predicate_control = 0;
     insn->header.compression_control = 0;
  }

  /* Extended math function, float[8] */
  void GenEmitter::math(
                GenReg dest,
                uint32_t function,
                uint32_t saturate,
                uint32_t msg_reg_nr,
                GenReg src,
                uint32_t data_type,
                uint32_t precision)
  {
     if (this->gen >= 6) {
        GenInstruction *insn = this->next(GEN_OPCODE_MATH);

        assert(dest.file == GEN_GENERAL_REGISTER_FILE);
        assert(src.file == GEN_GENERAL_REGISTER_FILE);

        assert(dest.hstride == GEN_HORIZONTAL_STRIDE_1);
        if (this->gen == 6)
           assert(src.hstride == GEN_HORIZONTAL_STRIDE_1);

        /* Source modifiers are ignored for extended math instructions on Gen6. */
        if (this->gen == 6) {
           assert(!src.negate);
           assert(!src.abs);
        }

        if (function == GEN_MATH_FUNCTION_INT_DIV_QUOTIENT ||
            function == GEN_MATH_FUNCTION_INT_DIV_REMAINDER ||
            function == GEN_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER) {
           assert(src.type != GEN_REGISTER_TYPE_F);
        } else {
           assert(src.type == GEN_REGISTER_TYPE_F);
        }

        /* Math is the same ISA format as other opcodes, except that CondModifier
         * becomes FC[3:0] and ThreadCtrl becomes FC[5:4].
         */
        insn->header.destreg__conditionalmod = function;
        insn->header.saturate = saturate;

        this->set_dest(insn, dest);
        this->set_src0(insn, src);
        this->set_src1(insn, brw_null_reg());
     } else {
        GenInstruction *insn = this->next(GEN_OPCODE_SEND);

        /* Example code doesn't set predicate_control for send
         * instructions.
         */
        insn->header.predicate_control = 0;
        insn->header.destreg__conditionalmod = msg_reg_nr;

        this->set_dest(insn, dest);
        this->set_src0(insn, src);
        brw_set_math_message(this,
                             insn,
                             function,
                             src.type == GEN_REGISTER_TYPE_D,
                             precision,
                             saturate,
                             data_type);
     }
  }

  /* Extended math function, float[8] */
  void GenEmitter::math2(GenReg dest, uint32_t function, GenReg src0, GenReg src1)
  {
     GenInstruction *insn = this->next(GEN_OPCODE_MATH);

     assert(this->gen >= 6);


     assert(dest.file == GEN_GENERAL_REGISTER_FILE);
     assert(src0.file == GEN_GENERAL_REGISTER_FILE);
     assert(src1.file == GEN_GENERAL_REGISTER_FILE);

     assert(dest.hstride == GEN_HORIZONTAL_STRIDE_1);
     if (this->gen == 6) {
        assert(src0.hstride == GEN_HORIZONTAL_STRIDE_1);
        assert(src1.hstride == GEN_HORIZONTAL_STRIDE_1);
     }

     if (function == GEN_MATH_FUNCTION_INT_DIV_QUOTIENT ||
         function == GEN_MATH_FUNCTION_INT_DIV_REMAINDER ||
         function == GEN_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER) {
        assert(src0.type != GEN_REGISTER_TYPE_F);
        assert(src1.type != GEN_REGISTER_TYPE_F);
     } else {
        assert(src0.type == GEN_REGISTER_TYPE_F);
        assert(src1.type == GEN_REGISTER_TYPE_F);
     }

     /* Source modifiers are ignored for extended math instructions on Gen6. */
     if (this->gen == 6) {
        assert(!src0.negate);
        assert(!src0.abs);
        assert(!src1.negate);
        assert(!src1.abs);
     }

     /* Math is the same ISA format as other opcodes, except that CondModifier
      * becomes FC[3:0] and ThreadCtrl becomes FC[5:4].
      */
     insn->header.destreg__conditionalmod = function;

     this->set_dest(insn, dest);
     this->set_src0(insn, src0);
     this->set_src1(insn, src1);
  }

  /* Extended math function, float[16] */
  void GenEmitter::math_16(GenReg dest,
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

     this->set_dest(insn, dest);
     this->set_src0(insn, src);
     this->set_src1(insn, brw_null_reg());
  }

  /**
   * Read a set of dwords from the data port Data Cache (const buffer).
   *
   * Location (in buffer) appears as UD offsets in the register after
   * the provided mrf header reg.
   */
  void GenEmitter::dword_scattered_read(GenReg dest, GenReg mrf, uint32_t bti)
  {
     mrf = retype(mrf, GEN_REGISTER_TYPE_UD);

     this->set_predicate_control(GEN_PREDICATE_NONE);
     this->set_compression_control(GEN_COMPRESSION_NONE);
     this->MOV(mrf, retype(brw_vec8_grf(0, 0), GEN_REGISTER_TYPE_UD));

     GenInstruction *insn = this->next(GEN_OPCODE_SEND);
     insn->header.destreg__conditionalmod = mrf.nr;

     /* cast dest to a uword[8] vector */
     dest = retype(vec8(dest), GEN_REGISTER_TYPE_UW);

     this->set_dest(insn, dest);
     this->set_src0(insn, brw_null_reg());

     this->set_dp_read_message(insn,
                             bti,
                             GEN_DATAPORT_DWORD_SCATTERED_BLOCK_8DWORDS,
                             GEN_DATAPORT_READ_MESSAGE_DWORD_SCATTERED_READ,
                             GEN_DATAPORT_READ_TARGET_DATA_CACHE,
                             2, /* msg_length */
                             1); /* response_length */
  }

  /**
   * Texture sample instruction.
   * Note: the msg_type plus msg_length values determine exactly what kind
   * of sampling operation is performed.  See volume 4, page 161 of docs.
   */
  void GenEmitter::SAMPLE(
                  GenReg dest,
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

     /* Hardware doesn't do destination dependency checking on send
      * instructions properly.  Add a workaround which generates the
      * dependency by other means.  In practice it seems like this bug
      * only crops up for texture samples, and only where registers are
      * written by the send and then written again later without being
      * read in between.  Luckily for us, we already track that
      * information and use it to modify the writemask for the
      * instruction, so that is a guide for whether a workaround is
      * needed.
      */
     assert (writemask == WRITEMASK_XYZW);

     {
        GenInstruction *insn;

        insn = this->next(GEN_OPCODE_SEND);
        insn->header.predicate_control = 0; /* XXX */
        insn->header.compression_control = GEN_COMPRESSION_NONE;

        this->set_dest(insn, dest);
        this->set_src0(insn, src0);
        this->set_sampler_message(insn,
                                bti,
                                sampler,
                                msg_type,
                                response_length, 
                                msg_length,
                                header_present,
                                simd_mode,
                                return_format);
     }
  }

  void GenEmitter::EOT(uint32_t msg_nr)
  {
    GenInstruction *insn = NULL;

    insn = this->MOV(brw_vec8_grf(msg_nr,0), brw_vec8_grf(0,0));
    insn->header.mask_control = GEN_MASK_DISABLE;
    insn = this->next(GEN_OPCODE_SEND);
    this->set_dest(insn, brw_null_reg());
    this->set_src0(insn, brw_vec8_grf(msg_nr,0));
    this->set_src1(insn, brw_imm_ud(0));
    insn->header.execution_size = GEN_EXECUTE_8;
    insn->header.compression_control = GEN_COMPRESSION_NONE;
    insn->bits3.spawner_gen5.resource = GEN_DO_NOT_DEREFERENCE_URB;
    insn->bits3.spawner_gen5.msg_length = 1;
    insn->bits3.spawner_gen5.end_of_thread = 1;
    insn->header.destreg__conditionalmod = GEN_SFID_THREAD_SPAWNER;
  }
} /* namespace gbe */


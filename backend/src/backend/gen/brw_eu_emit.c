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

// #include "brw_context.h"
#include "brw_defines.h"
#include "brw_eu.h"

#include <string.h>

#define Elements(x) (sizeof(x) / sizeof(*(x)))

/***********************************************************************
 * Internal helper for constructing instructions
 */

static void guess_execution_size(struct brw_compile *p,
                                 struct brw_instruction *insn,
                                 struct brw_reg reg)
{
   if (reg.width == BRW_WIDTH_8 && p->compressed)
      insn->header.execution_size = BRW_EXECUTE_16;
   else
      insn->header.execution_size = reg.width;        /* note - definitions are compatible */
}

void
brw_set_dest(struct brw_compile *p, struct brw_instruction *insn,
             struct brw_reg dest)
{
   if (dest.file != BRW_ARCHITECTURE_REGISTER_FILE)
      assert(dest.nr < 128);

   insn->bits1.da1.dest_reg_file = dest.file;
   insn->bits1.da1.dest_reg_type = dest.type;
   insn->bits1.da1.dest_address_mode = dest.address_mode;

   if (dest.address_mode == BRW_ADDRESS_DIRECT) {   
      insn->bits1.da1.dest_reg_nr = dest.nr;

      if (insn->header.access_mode == BRW_ALIGN_1) {
         insn->bits1.da1.dest_subreg_nr = dest.subnr;
         if (dest.hstride == BRW_HORIZONTAL_STRIDE_0)
            dest.hstride = BRW_HORIZONTAL_STRIDE_1;
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
      if (insn->header.access_mode == BRW_ALIGN_1) {
         insn->bits1.ia1.dest_indirect_offset = dest.dw1.bits.indirect_offset;
         if (dest.hstride == BRW_HORIZONTAL_STRIDE_0)
            dest.hstride = BRW_HORIZONTAL_STRIDE_1;
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
   guess_execution_size(p, insn, dest);
}

extern int reg_type_size[];

static void
validate_reg(struct brw_instruction *insn, struct brw_reg reg)
{
   int hstride_for_reg[] = {0, 1, 2, 4};
   int vstride_for_reg[] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256};
   int width_for_reg[] = {1, 2, 4, 8, 16};
   int execsize_for_reg[] = {1, 2, 4, 8, 16};
   int width, hstride, vstride, execsize;

   if (reg.file == BRW_IMMEDIATE_VALUE) {
      /* 3.3.6: Region Parameters.  Restriction: Immediate vectors
       * mean the destination has to be 128-bit aligned and the
       * destination horiz stride has to be a word.
       */
      if (reg.type == BRW_REGISTER_TYPE_V) {
         assert(hstride_for_reg[insn->bits1.da1.dest_horiz_stride] *
                reg_type_size[insn->bits1.da1.dest_reg_type] == 2);
      }

      return;
   }

   if (reg.file == BRW_ARCHITECTURE_REGISTER_FILE &&
       reg.file == BRW_ARF_NULL)
      return;

   assert(reg.hstride >= 0 && reg.hstride < Elements(hstride_for_reg));
   hstride = hstride_for_reg[reg.hstride];

   if (reg.vstride == 0xf) {
      vstride = -1;
   } else {
      assert(reg.vstride >= 0 && reg.vstride < Elements(vstride_for_reg));
      vstride = vstride_for_reg[reg.vstride];
   }

   assert(reg.width >= 0 && reg.width < Elements(width_for_reg));
   width = width_for_reg[reg.width];

   assert(insn->header.execution_size >= 0 &&
          insn->header.execution_size < Elements(execsize_for_reg));
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
brw_set_src0(struct brw_compile *p, struct brw_instruction *insn,
             struct brw_reg reg)
{
   if (reg.type != BRW_ARCHITECTURE_REGISTER_FILE)
      assert(reg.nr < 128);

   validate_reg(insn, reg);

   insn->bits1.da1.src0_reg_file = reg.file;
   insn->bits1.da1.src0_reg_type = reg.type;
   insn->bits2.da1.src0_abs = reg.abs;
   insn->bits2.da1.src0_negate = reg.negate;
   insn->bits2.da1.src0_address_mode = reg.address_mode;

   if (reg.file == BRW_IMMEDIATE_VALUE) {
      insn->bits3.ud = reg.dw1.ud;
   
      /* Required to set some fields in src1 as well:
       */
      insn->bits1.da1.src1_reg_file = 0; /* arf */
      insn->bits1.da1.src1_reg_type = reg.type;
   }
   else 
   {
      if (reg.address_mode == BRW_ADDRESS_DIRECT) {
         if (insn->header.access_mode == BRW_ALIGN_1) {
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

         if (insn->header.access_mode == BRW_ALIGN_1) {
            insn->bits2.ia1.src0_indirect_offset = reg.dw1.bits.indirect_offset; 
         }
         else {
            insn->bits2.ia16.src0_subreg_nr = reg.dw1.bits.indirect_offset;
         }
      }

      if (insn->header.access_mode == BRW_ALIGN_1) {
         if (reg.width == BRW_WIDTH_1 && 
             insn->header.execution_size == BRW_EXECUTE_1) {
            insn->bits2.da1.src0_horiz_stride = BRW_HORIZONTAL_STRIDE_0;
            insn->bits2.da1.src0_width = BRW_WIDTH_1;
            insn->bits2.da1.src0_vert_stride = BRW_VERTICAL_STRIDE_0;
         }
         else {
            insn->bits2.da1.src0_horiz_stride = reg.hstride;
            insn->bits2.da1.src0_width = reg.width;
            insn->bits2.da1.src0_vert_stride = reg.vstride;
         }
      }
      else {
         insn->bits2.da16.src0_swz_x = BRW_GET_SWZ(reg.dw1.bits.swizzle, BRW_CHANNEL_X);
         insn->bits2.da16.src0_swz_y = BRW_GET_SWZ(reg.dw1.bits.swizzle, BRW_CHANNEL_Y);
         insn->bits2.da16.src0_swz_z = BRW_GET_SWZ(reg.dw1.bits.swizzle, BRW_CHANNEL_Z);
         insn->bits2.da16.src0_swz_w = BRW_GET_SWZ(reg.dw1.bits.swizzle, BRW_CHANNEL_W);

         /* This is an oddity of the fact we're using the same
          * descriptions for registers in align_16 as align_1:
          */
         if (reg.vstride == BRW_VERTICAL_STRIDE_8)
            insn->bits2.da16.src0_vert_stride = BRW_VERTICAL_STRIDE_4;
         else
            insn->bits2.da16.src0_vert_stride = reg.vstride;
      }
   }
}


void brw_set_src1(struct brw_compile *p,
                  struct brw_instruction *insn,
                  struct brw_reg reg)
{
   assert(reg.nr < 128);

   validate_reg(insn, reg);

   insn->bits1.da1.src1_reg_file = reg.file;
   insn->bits1.da1.src1_reg_type = reg.type;
   insn->bits3.da1.src1_abs = reg.abs;
   insn->bits3.da1.src1_negate = reg.negate;

   /* Only src1 can be immediate in two-argument instructions.
    */
   assert(insn->bits1.da1.src0_reg_file != BRW_IMMEDIATE_VALUE);

   if (reg.file == BRW_IMMEDIATE_VALUE) {
      insn->bits3.ud = reg.dw1.ud;
   }
   else {
      /* This is a hardware restriction, which may or may not be lifted
       * in the future:
       */
      assert (reg.address_mode == BRW_ADDRESS_DIRECT);
      /* assert (reg.file == BRW_GENERAL_REGISTER_FILE); */

      if (insn->header.access_mode == BRW_ALIGN_1) {
         insn->bits3.da1.src1_subreg_nr = reg.subnr;
         insn->bits3.da1.src1_reg_nr = reg.nr;
      }
      else {
         insn->bits3.da16.src1_subreg_nr = reg.subnr / 16;
         insn->bits3.da16.src1_reg_nr = reg.nr;
      }

      if (insn->header.access_mode == BRW_ALIGN_1) {
         if (reg.width == BRW_WIDTH_1 && 
             insn->header.execution_size == BRW_EXECUTE_1) {
            insn->bits3.da1.src1_horiz_stride = BRW_HORIZONTAL_STRIDE_0;
            insn->bits3.da1.src1_width = BRW_WIDTH_1;
            insn->bits3.da1.src1_vert_stride = BRW_VERTICAL_STRIDE_0;
         }
         else {
            insn->bits3.da1.src1_horiz_stride = reg.hstride;
            insn->bits3.da1.src1_width = reg.width;
            insn->bits3.da1.src1_vert_stride = reg.vstride;
         }
      }
      else {
         insn->bits3.da16.src1_swz_x = BRW_GET_SWZ(reg.dw1.bits.swizzle, BRW_CHANNEL_X);
         insn->bits3.da16.src1_swz_y = BRW_GET_SWZ(reg.dw1.bits.swizzle, BRW_CHANNEL_Y);
         insn->bits3.da16.src1_swz_z = BRW_GET_SWZ(reg.dw1.bits.swizzle, BRW_CHANNEL_Z);
         insn->bits3.da16.src1_swz_w = BRW_GET_SWZ(reg.dw1.bits.swizzle, BRW_CHANNEL_W);

         /* This is an oddity of the fact we're using the same
          * descriptions for registers in align_16 as align_1:
          */
         if (reg.vstride == BRW_VERTICAL_STRIDE_8)
            insn->bits3.da16.src1_vert_stride = BRW_VERTICAL_STRIDE_4;
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
brw_set_message_descriptor(struct brw_compile *p,
                           struct brw_instruction *inst,
                           enum brw_message_target sfid,
                           unsigned msg_length,
                           unsigned response_length,
                           bool header_present,
                           bool end_of_thread)
{
   brw_set_src1(p, inst, brw_imm_d(0));

   if (p->gen >= 5) {
      inst->bits3.generic_gen5.header_present = header_present;
      inst->bits3.generic_gen5.response_length = response_length;
      inst->bits3.generic_gen5.msg_length = msg_length;
      inst->bits3.generic_gen5.end_of_thread = end_of_thread;

      if (p->gen >= 6) {
         /* On Gen6+ Message target/SFID goes in bits 27:24 of the header */
         inst->header.destreg__conditionalmod = sfid;
      } else {
         /* Set Extended Message Descriptor (ex_desc) */
         inst->bits2.send_gen5.sfid = sfid;
         inst->bits2.send_gen5.end_of_thread = end_of_thread;
      }
   } else {
      inst->bits3.generic.response_length = response_length;
      inst->bits3.generic.msg_length = msg_length;
      inst->bits3.generic.msg_target = sfid;
      inst->bits3.generic.end_of_thread = end_of_thread;
   }
}

static void brw_set_math_message(struct brw_compile *p,
                                 struct brw_instruction *insn,
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
   case BRW_MATH_FUNCTION_POW:
   case BRW_MATH_FUNCTION_INT_DIV_QUOTIENT:
   case BRW_MATH_FUNCTION_INT_DIV_REMAINDER:
   case BRW_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER:
      msg_length = 2;
      break;
   default:
      msg_length = 1;
      break;
   }

   /* Infer response length from the function */
   switch (function) {
   case BRW_MATH_FUNCTION_SINCOS:
   case BRW_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER:
      response_length = 2;
      break;
   default:
      response_length = 1;
      break;
   }

   brw_set_message_descriptor(p, insn, BRW_SFID_MATH,
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
brw_set_dp_write_message(struct brw_compile *p,
                         struct brw_instruction *insn,
                         uint32_t binding_table_index,
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
   brw_set_message_descriptor(p, insn, sfid, msg_length, response_length,
                              header_present, end_of_thread);

   insn->bits3.gen7_dp.binding_table_index = binding_table_index;
   insn->bits3.gen7_dp.msg_control = msg_control;
   insn->bits3.gen7_dp.last_render_target = last_render_target;
   insn->bits3.gen7_dp.msg_type = msg_type;
}

void
brw_set_dp_read_message(struct brw_compile *p,
                        struct brw_instruction *insn,
                        uint32_t binding_table_index,
                        uint32_t msg_control,
                        uint32_t msg_type,
                        uint32_t target_cache,
                        uint32_t msg_length,
                        uint32_t response_length)
{
   unsigned sfid;

   sfid = GEN7_SFID_DATAPORT_DATA_CACHE;
   brw_set_message_descriptor(p, insn, sfid, msg_length, response_length,
                              true, false);

   insn->bits3.gen7_dp.binding_table_index = binding_table_index;
   insn->bits3.gen7_dp.msg_control = msg_control;
   insn->bits3.gen7_dp.last_render_target = 0;
   insn->bits3.gen7_dp.msg_type = msg_type;
}

void
brw_set_sampler_message(struct brw_compile *p,
                        struct brw_instruction *insn,
                        uint32_t binding_table_index,
                        uint32_t sampler,
                        uint32_t msg_type,
                        uint32_t response_length,
                        uint32_t msg_length,
                        uint32_t header_present,
                        uint32_t simd_mode,
                        uint32_t return_format)
{
   brw_set_message_descriptor(p, insn, BRW_SFID_SAMPLER, msg_length,
                              response_length, header_present, false);
   insn->bits3.sampler_gen7.binding_table_index = binding_table_index;
   insn->bits3.sampler_gen7.sampler = sampler;
   insn->bits3.sampler_gen7.msg_type = msg_type;
   insn->bits3.sampler_gen7.simd_mode = simd_mode;
}

#define next_insn brw_next_insn
struct brw_instruction *
brw_next_insn(struct brw_compile *p, uint32_t opcode)
{
   struct brw_instruction *insn;
   insn = &p->store[p->nr_insn++];
   insn->header.opcode = opcode;
   return insn;
}

static struct brw_instruction *brw_alu1(struct brw_compile *p,
                                        uint32_t opcode,
                                        struct brw_reg dest,
                                        struct brw_reg src)
{
   struct brw_instruction *insn = next_insn(p, opcode);
   brw_set_dest(p, insn, dest);
   brw_set_src0(p, insn, src);
   return insn;
}

static struct brw_instruction *brw_alu2(struct brw_compile *p,
                                        uint32_t opcode,
                                        struct brw_reg dest,
                                        struct brw_reg src0,
                                        struct brw_reg src1)
{
   struct brw_instruction *insn = next_insn(p, opcode);   
   brw_set_dest(p, insn, dest);
   brw_set_src0(p, insn, src0);
   brw_set_src1(p, insn, src1);
   return insn;
}

static int
get_3src_subreg_nr(struct brw_reg reg)
{
   if (reg.vstride == BRW_VERTICAL_STRIDE_0) {
      assert(brw_is_single_value_swizzle(reg.dw1.bits.swizzle));
      return reg.subnr / 4 + BRW_GET_SWZ(reg.dw1.bits.swizzle, 0);
   } else {
      return reg.subnr / 4;
   }
}

static struct brw_instruction *brw_alu3(struct brw_compile *p,
                                        uint32_t opcode,
                                        struct brw_reg dest,
                                        struct brw_reg src0,
                                        struct brw_reg src1,
                                        struct brw_reg src2)
{
   struct brw_instruction *insn = next_insn(p, opcode);

   assert(insn->header.access_mode == BRW_ALIGN_16);

   assert(dest.file == BRW_GENERAL_REGISTER_FILE);
   assert(dest.nr < 128);
   assert(dest.address_mode == BRW_ADDRESS_DIRECT);
   assert(dest.type = BRW_REGISTER_TYPE_F);
   insn->bits1.da3src.dest_reg_file = 0;
   insn->bits1.da3src.dest_reg_nr = dest.nr;
   insn->bits1.da3src.dest_subreg_nr = dest.subnr / 16;
   insn->bits1.da3src.dest_writemask = dest.dw1.bits.writemask;
   guess_execution_size(p, insn, dest);

   assert(src0.file == BRW_GENERAL_REGISTER_FILE);
   assert(src0.address_mode == BRW_ADDRESS_DIRECT);
   assert(src0.nr < 128);
   assert(src0.type == BRW_REGISTER_TYPE_F);
   insn->bits2.da3src.src0_swizzle = src0.dw1.bits.swizzle;
   insn->bits2.da3src.src0_subreg_nr = get_3src_subreg_nr(src0);
   insn->bits2.da3src.src0_reg_nr = src0.nr;
   insn->bits1.da3src.src0_abs = src0.abs;
   insn->bits1.da3src.src0_negate = src0.negate;
   insn->bits2.da3src.src0_rep_ctrl = src0.vstride == BRW_VERTICAL_STRIDE_0;

   assert(src1.file == BRW_GENERAL_REGISTER_FILE);
   assert(src1.address_mode == BRW_ADDRESS_DIRECT);
   assert(src1.nr < 128);
   assert(src1.type == BRW_REGISTER_TYPE_F);
   insn->bits2.da3src.src1_swizzle = src1.dw1.bits.swizzle;
   insn->bits2.da3src.src1_subreg_nr_low = get_3src_subreg_nr(src1) & 0x3;
   insn->bits3.da3src.src1_subreg_nr_high = get_3src_subreg_nr(src1) >> 2;
   insn->bits2.da3src.src1_rep_ctrl = src1.vstride == BRW_VERTICAL_STRIDE_0;
   insn->bits3.da3src.src1_reg_nr = src1.nr;
   insn->bits1.da3src.src1_abs = src1.abs;
   insn->bits1.da3src.src1_negate = src1.negate;

   assert(src2.file == BRW_GENERAL_REGISTER_FILE);
   assert(src2.address_mode == BRW_ADDRESS_DIRECT);
   assert(src2.nr < 128);
   assert(src2.type == BRW_REGISTER_TYPE_F);
   insn->bits3.da3src.src2_swizzle = src2.dw1.bits.swizzle;
   insn->bits3.da3src.src2_subreg_nr = get_3src_subreg_nr(src2);
   insn->bits3.da3src.src2_rep_ctrl = src2.vstride == BRW_VERTICAL_STRIDE_0;
   insn->bits3.da3src.src2_reg_nr = src2.nr;
   insn->bits1.da3src.src2_abs = src2.abs;
   insn->bits1.da3src.src2_negate = src2.negate;

   return insn;
}


/***********************************************************************
 * Convenience routines.
 */
#define ALU1(OP)                                          \
struct brw_instruction *brw_##OP(struct brw_compile *p,   \
              struct brw_reg dest,                        \
              struct brw_reg src0)                        \
{                                                         \
   return brw_alu1(p, BRW_OPCODE_##OP, dest, src0);       \
}

#define ALU2(OP)                                          \
struct brw_instruction *brw_##OP(struct brw_compile *p,   \
              struct brw_reg dest,                        \
              struct brw_reg src0,                        \
              struct brw_reg src1)                        \
{                                                         \
   return brw_alu2(p, BRW_OPCODE_##OP, dest, src0, src1); \
}

#define ALU3(OP)                                          \
struct brw_instruction *brw_##OP(struct brw_compile *p,   \
              struct brw_reg dest,                        \
              struct brw_reg src0,                        \
              struct brw_reg src1,                        \
              struct brw_reg src2)                        \
{                                                         \
   return brw_alu3(p, BRW_OPCODE_##OP, dest,              \
                   src0, src1, src2);                     \
}

/* Rounding operations (other than RNDD) require two instructions - the first
 * stores a rounded value (possibly the wrong way) in the dest register, but
 * also sets a per-channel "increment bit" in the flag register.  A predicated
 * add of 1.0 fixes dest to contain the desired result.
 *
 * Sandybridge and later appear to round correctly without an ADD.
 */
#define ROUND(OP)                        \
void brw_##OP(struct brw_compile *p,     \
              struct brw_reg dest,       \
              struct brw_reg src)        \
{                                        \
   struct brw_instruction *rnd;          \
   rnd = next_insn(p, BRW_OPCODE_##OP);  \
   brw_set_dest(p, rnd, dest);           \
   brw_set_src0(p, rnd, src);            \
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

struct brw_instruction *brw_ADD(struct brw_compile *p,
                                struct brw_reg dest,
                                struct brw_reg src0,
                                struct brw_reg src1)
{
   /* 6.2.2: add */
   if (src0.type == BRW_REGISTER_TYPE_F ||
       (src0.file == BRW_IMMEDIATE_VALUE &&
        src0.type == BRW_REGISTER_TYPE_VF)) {
      assert(src1.type != BRW_REGISTER_TYPE_UD);
      assert(src1.type != BRW_REGISTER_TYPE_D);
   }

   if (src1.type == BRW_REGISTER_TYPE_F ||
       (src1.file == BRW_IMMEDIATE_VALUE &&
        src1.type == BRW_REGISTER_TYPE_VF)) {
      assert(src0.type != BRW_REGISTER_TYPE_UD);
      assert(src0.type != BRW_REGISTER_TYPE_D);
   }

   return brw_alu2(p, BRW_OPCODE_ADD, dest, src0, src1);
}

struct brw_instruction *brw_MUL(struct brw_compile *p,
                                struct brw_reg dest,
                                struct brw_reg src0,
                                struct brw_reg src1)
{
   /* 6.32.38: mul */
   if (src0.type == BRW_REGISTER_TYPE_D ||
       src0.type == BRW_REGISTER_TYPE_UD ||
       src1.type == BRW_REGISTER_TYPE_D ||
       src1.type == BRW_REGISTER_TYPE_UD) {
      assert(dest.type != BRW_REGISTER_TYPE_F);
   }

   if (src0.type == BRW_REGISTER_TYPE_F ||
       (src0.file == BRW_IMMEDIATE_VALUE &&
        src0.type == BRW_REGISTER_TYPE_VF)) {
      assert(src1.type != BRW_REGISTER_TYPE_UD);
      assert(src1.type != BRW_REGISTER_TYPE_D);
   }

   if (src1.type == BRW_REGISTER_TYPE_F ||
       (src1.file == BRW_IMMEDIATE_VALUE &&
        src1.type == BRW_REGISTER_TYPE_VF)) {
      assert(src0.type != BRW_REGISTER_TYPE_UD);
      assert(src0.type != BRW_REGISTER_TYPE_D);
   }

   assert(src0.file != BRW_ARCHITECTURE_REGISTER_FILE ||
          src0.nr != BRW_ARF_ACCUMULATOR);
   assert(src1.file != BRW_ARCHITECTURE_REGISTER_FILE ||
          src1.nr != BRW_ARF_ACCUMULATOR);

   return brw_alu2(p, BRW_OPCODE_MUL, dest, src0, src1);
}


void brw_NOP(struct brw_compile *p)
{
   struct brw_instruction *insn = next_insn(p, BRW_OPCODE_NOP);   
   brw_set_dest(p, insn, retype(brw_vec4_grf(0,0), BRW_REGISTER_TYPE_UD));
   brw_set_src0(p, insn, retype(brw_vec4_grf(0,0), BRW_REGISTER_TYPE_UD));
   brw_set_src1(p, insn, brw_imm_ud(0x0));
}

/***********************************************************************
 * Comparisons, if/else/endif
 */

struct brw_instruction *brw_JMPI(struct brw_compile *p, 
                                 struct brw_reg dest,
                                 struct brw_reg src0,
                                 struct brw_reg src1)
{
   struct brw_instruction *insn = brw_alu2(p, BRW_OPCODE_JMPI, dest, src0, src1);

   insn->header.execution_size = 1;
   insn->header.compression_control = BRW_COMPRESSION_NONE;
   insn->header.mask_control = BRW_MASK_DISABLE;

   // p->current->header.predicate_control = BRW_PREDICATE_NONE;

   return insn;
}

/* To integrate with the above, it makes sense that the comparison
 * instruction should populate the flag register.  It might be simpler
 * just to use the flag reg for most WM tasks?
 */
void brw_CMP(struct brw_compile *p,
             struct brw_reg dest,
             uint32_t conditional,
             struct brw_reg src0,
             struct brw_reg src1)
{
   struct brw_instruction *insn = next_insn(p, BRW_OPCODE_CMP);

   insn->header.destreg__conditionalmod = conditional;
   brw_set_dest(p, insn, dest);
   brw_set_src0(p, insn, src0);
   brw_set_src1(p, insn, src1);

/*    guess_execution_size(insn, src0); */
#if 0

   /* Make it so that future instructions will use the computed flag
    * value until brw_set_predicate_control_flag_value() is called
    * again.  
    */
   if (dest.file == BRW_ARCHITECTURE_REGISTER_FILE &&
       dest.nr == 0) {
      p->current->header.predicate_control = BRW_PREDICATE_NORMAL;
      p->flag_value = 0xff;
   }
#endif
}

/* Issue 'wait' instruction for n1, host could program MMIO
   to wake up thread. */
void brw_WAIT (struct brw_compile *p)
{
   struct brw_instruction *insn = next_insn(p, BRW_OPCODE_WAIT);
   struct brw_reg src = brw_notification_1_reg();

   brw_set_dest(p, insn, src);
   brw_set_src0(p, insn, src);
   brw_set_src1(p, insn, brw_null_reg());
   insn->header.execution_size = 0; /* must */
   insn->header.predicate_control = 0;
   insn->header.compression_control = 0;
}


/***********************************************************************
 * Helpers for the various SEND message types:
 */

/** Extended math function, float[8].  */
void brw_math(struct brw_compile *p,
              struct brw_reg dest,
              uint32_t function,
              uint32_t saturate,
              uint32_t msg_reg_nr,
              struct brw_reg src,
              uint32_t data_type,
              uint32_t precision)
{
   if (p->gen >= 6) {
      struct brw_instruction *insn = next_insn(p, BRW_OPCODE_MATH);

      assert(dest.file == BRW_GENERAL_REGISTER_FILE);
      assert(src.file == BRW_GENERAL_REGISTER_FILE);

      assert(dest.hstride == BRW_HORIZONTAL_STRIDE_1);
      if (p->gen == 6)
         assert(src.hstride == BRW_HORIZONTAL_STRIDE_1);

      /* Source modifiers are ignored for extended math instructions on Gen6. */
      if (p->gen == 6) {
         assert(!src.negate);
         assert(!src.abs);
      }

      if (function == BRW_MATH_FUNCTION_INT_DIV_QUOTIENT ||
          function == BRW_MATH_FUNCTION_INT_DIV_REMAINDER ||
          function == BRW_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER) {
         assert(src.type != BRW_REGISTER_TYPE_F);
      } else {
         assert(src.type == BRW_REGISTER_TYPE_F);
      }

      /* Math is the same ISA format as other opcodes, except that CondModifier
       * becomes FC[3:0] and ThreadCtrl becomes FC[5:4].
       */
      insn->header.destreg__conditionalmod = function;
      insn->header.saturate = saturate;

      brw_set_dest(p, insn, dest);
      brw_set_src0(p, insn, src);
      brw_set_src1(p, insn, brw_null_reg());
   } else {
      struct brw_instruction *insn = next_insn(p, BRW_OPCODE_SEND);

      /* Example code doesn't set predicate_control for send
       * instructions.
       */
      insn->header.predicate_control = 0;
      insn->header.destreg__conditionalmod = msg_reg_nr;

      brw_set_dest(p, insn, dest);
      brw_set_src0(p, insn, src);
      brw_set_math_message(p,
                           insn,
                           function,
                           src.type == BRW_REGISTER_TYPE_D,
                           precision,
                           saturate,
                           data_type);
   }
}

/** Extended math function, float[8].
 */
void brw_math2(struct brw_compile *p,
               struct brw_reg dest,
               uint32_t function,
               struct brw_reg src0,
               struct brw_reg src1)
{
   struct brw_instruction *insn = next_insn(p, BRW_OPCODE_MATH);

   assert(p->gen >= 6);


   assert(dest.file == BRW_GENERAL_REGISTER_FILE);
   assert(src0.file == BRW_GENERAL_REGISTER_FILE);
   assert(src1.file == BRW_GENERAL_REGISTER_FILE);

   assert(dest.hstride == BRW_HORIZONTAL_STRIDE_1);
   if (p->gen == 6) {
      assert(src0.hstride == BRW_HORIZONTAL_STRIDE_1);
      assert(src1.hstride == BRW_HORIZONTAL_STRIDE_1);
   }

   if (function == BRW_MATH_FUNCTION_INT_DIV_QUOTIENT ||
       function == BRW_MATH_FUNCTION_INT_DIV_REMAINDER ||
       function == BRW_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER) {
      assert(src0.type != BRW_REGISTER_TYPE_F);
      assert(src1.type != BRW_REGISTER_TYPE_F);
   } else {
      assert(src0.type == BRW_REGISTER_TYPE_F);
      assert(src1.type == BRW_REGISTER_TYPE_F);
   }

   /* Source modifiers are ignored for extended math instructions on Gen6. */
   if (p->gen == 6) {
      assert(!src0.negate);
      assert(!src0.abs);
      assert(!src1.negate);
      assert(!src1.abs);
   }

   /* Math is the same ISA format as other opcodes, except that CondModifier
    * becomes FC[3:0] and ThreadCtrl becomes FC[5:4].
    */
   insn->header.destreg__conditionalmod = function;

   brw_set_dest(p, insn, dest);
   brw_set_src0(p, insn, src0);
   brw_set_src1(p, insn, src1);
}

/**
 * Extended math function, float[16].
 * Use 2 send instructions.
 */
void brw_math_16(struct brw_compile *p,
                 struct brw_reg dest,
                 uint32_t function,
                 uint32_t saturate,
                 uint32_t msg_reg_nr,
                 struct brw_reg src,
                 uint32_t precision)
{
   struct brw_instruction *insn;

   if (p->gen >= 6) {
      insn = next_insn(p, BRW_OPCODE_MATH);

      /* Math is the same ISA format as other opcodes, except that CondModifier
       * becomes FC[3:0] and ThreadCtrl becomes FC[5:4].
       */
      insn->header.destreg__conditionalmod = function;
      insn->header.saturate = saturate;

      /* Source modifiers are ignored for extended math instructions. */
      assert(!src.negate);
      assert(!src.abs);

      brw_set_dest(p, insn, dest);
      brw_set_src0(p, insn, src);
      brw_set_src1(p, insn, brw_null_reg());
      return;
   }

   /* First instruction:
    */
   brw_push_insn_state(p);
   brw_set_predicate_control_flag_value(p, 0xff);
   brw_set_compression_control(p, BRW_COMPRESSION_NONE);

   insn = next_insn(p, BRW_OPCODE_SEND);
   insn->header.destreg__conditionalmod = msg_reg_nr;

   brw_set_dest(p, insn, dest);
   brw_set_src0(p, insn, src);
   brw_set_math_message(p,
                        insn, 
                        function,
                        BRW_MATH_INTEGER_UNSIGNED,
                        precision,
                        saturate,
                        BRW_MATH_DATA_VECTOR);

   /* Second instruction:
    */
   insn = next_insn(p, BRW_OPCODE_SEND);
   insn->header.compression_control = BRW_COMPRESSION_2NDHALF;
   insn->header.destreg__conditionalmod = msg_reg_nr+1;

   brw_set_dest(p, insn, offset(dest,1));
   brw_set_src0(p, insn, src);
   brw_set_math_message(p, 
                        insn, 
                        function,
                        BRW_MATH_INTEGER_UNSIGNED,
                        precision,
                        saturate,
                        BRW_MATH_DATA_VECTOR);

   brw_pop_insn_state(p);
}


/**
 * Write a block of OWORDs (half a GRF each) from the scratch buffer,
 * using a constant offset per channel.
 *
 * The offset must be aligned to oword size (16 bytes).  Used for
 * register spilling.
 */
void brw_oword_block_write_scratch(struct brw_compile *p,
                                   struct brw_reg mrf,
                                   int num_regs,
                                   uint32_t offset)
{
   uint32_t msg_control, msg_type;
   int mlen;

   if (p->gen >= 6)
      offset /= 16;

   mrf = retype(mrf, BRW_REGISTER_TYPE_UD);

   if (num_regs == 1) {
      msg_control = BRW_DATAPORT_OWORD_BLOCK_2_OWORDS;
      mlen = 2;
   } else {
      msg_control = BRW_DATAPORT_OWORD_BLOCK_4_OWORDS;
      mlen = 3;
   }

   /* Set up the message header.  This is g0, with g0.2 filled with
    * the offset.  We don't want to leave our offset around in g0 or
    * it'll screw up texture samples, so set it up inside the message
    * reg.
    */
   {
      brw_push_insn_state(p);
      brw_set_mask_control(p, BRW_MASK_DISABLE);
      brw_set_compression_control(p, BRW_COMPRESSION_NONE);

      brw_MOV(p, mrf, retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UD));

      /* set message header global offset field (reg 0, element 2) */
      brw_MOV(p,
              retype(brw_vec1_reg(BRW_GENERAL_REGISTER_FILE,
                                  mrf.nr,
                                  2), BRW_REGISTER_TYPE_UD),
              brw_imm_ud(offset));

      brw_pop_insn_state(p);
   }

   {
      struct brw_reg dest;
      struct brw_instruction *insn = next_insn(p, BRW_OPCODE_SEND);
      int send_commit_msg;
      struct brw_reg src_header = retype(brw_vec8_grf(0, 0),
                                         BRW_REGISTER_TYPE_UW);

      if (insn->header.compression_control != BRW_COMPRESSION_NONE) {
         insn->header.compression_control = BRW_COMPRESSION_NONE;
         src_header = vec16(src_header);
      }
      assert(insn->header.predicate_control == BRW_PREDICATE_NONE);
      insn->header.destreg__conditionalmod = mrf.nr;

      /* Until gen6, writes followed by reads from the same location
       * are not guaranteed to be ordered unless write_commit is set.
       * If set, then a no-op write is issued to the destination
       * register to set a dependency, and a read from the destination
       * can be used to ensure the ordering.
       *
       * For gen6, only writes between different threads need ordering
       * protection.  Our use of DP writes is all about register
       * spilling within a thread.
       */
      if (p->gen >= 6) {
         dest = retype(vec16(brw_null_reg()), BRW_REGISTER_TYPE_UW);
         send_commit_msg = 0;
      } else {
         dest = src_header;
         send_commit_msg = 1;
      }

      brw_set_dest(p, insn, dest);
      if (p->gen >= 6) {
         brw_set_src0(p, insn, mrf);
      } else {
         brw_set_src0(p, insn, brw_null_reg());
      }

      if (p->gen >= 6)
         msg_type = GEN6_DATAPORT_WRITE_MESSAGE_OWORD_BLOCK_WRITE;
      else
         msg_type = BRW_DATAPORT_WRITE_MESSAGE_OWORD_BLOCK_WRITE;

      brw_set_dp_write_message(p,
                               insn,
                               255, /* binding table index (255=stateless) */
                               msg_control,
                               msg_type,
                               mlen,
                               true, /* header_present */
                               0, /* not a render target */
                               send_commit_msg, /* response_length */
                               0, /* eot */
                               send_commit_msg);
   }
}


/**
 * Read a block of owords (half a GRF each) from the scratch buffer
 * using a constant index per channel.
 *
 * Offset must be aligned to oword size (16 bytes).  Used for register
 * spilling.
 */
void
brw_oword_block_read_scratch(struct brw_compile *p,
                             struct brw_reg dest,
                             struct brw_reg mrf,
                             int num_regs,
                             uint32_t offset)
{
   uint32_t msg_control;
   int rlen;

   if (p->gen >= 6)
      offset /= 16;

   mrf = retype(mrf, BRW_REGISTER_TYPE_UD);
   dest = retype(dest, BRW_REGISTER_TYPE_UW);

   if (num_regs == 1) {
      msg_control = BRW_DATAPORT_OWORD_BLOCK_2_OWORDS;
      rlen = 1;
   } else {
      msg_control = BRW_DATAPORT_OWORD_BLOCK_4_OWORDS;
      rlen = 2;
   }

   {
      brw_push_insn_state(p);
      brw_set_compression_control(p, BRW_COMPRESSION_NONE);
      brw_set_mask_control(p, BRW_MASK_DISABLE);

      brw_MOV(p, mrf, retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UD));

      /* set message header global offset field (reg 0, element 2) */
      brw_MOV(p,
              retype(brw_vec1_reg(BRW_GENERAL_REGISTER_FILE,
                                  mrf.nr,
                                  2), BRW_REGISTER_TYPE_UD),
              brw_imm_ud(offset));

      brw_pop_insn_state(p);
   }

   {
      struct brw_instruction *insn = next_insn(p, BRW_OPCODE_SEND);

      assert(insn->header.predicate_control == 0);
      insn->header.compression_control = BRW_COMPRESSION_NONE;
      insn->header.destreg__conditionalmod = mrf.nr;

      brw_set_dest(p, insn, dest);        /* UW? */
      if (p->gen >= 6) {
         brw_set_src0(p, insn, mrf);
      } else {
         brw_set_src0(p, insn, brw_null_reg());
      }

      brw_set_dp_read_message(p,
                              insn,
                              255, /* binding table index (255=stateless) */
                              msg_control,
                              BRW_DATAPORT_READ_MESSAGE_OWORD_BLOCK_READ, /* msg_type */
                              BRW_DATAPORT_READ_TARGET_RENDER_CACHE,
                              1, /* msg_length */
                              rlen);
   }
}

/**
 * Read a float[4] vector from the data port Data Cache (const buffer).
 * Location (in buffer) should be a multiple of 16.
 * Used for fetching shader constants.
 */
void brw_oword_block_read(struct brw_compile *p,
                          struct brw_reg dest,
                          struct brw_reg mrf,
                          uint32_t offset,
                          uint32_t bind_table_index)
{

   /* On newer hardware, offset is in units of owords. */
   if (p->gen >= 6)
      offset /= 16;

   mrf = retype(mrf, BRW_REGISTER_TYPE_UD);

   brw_push_insn_state(p);
   brw_set_predicate_control(p, BRW_PREDICATE_NONE);
   brw_set_compression_control(p, BRW_COMPRESSION_NONE);
   brw_set_mask_control(p, BRW_MASK_DISABLE);

   brw_MOV(p, mrf, retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UD));

   /* set message header global offset field (reg 0, element 2) */
   brw_MOV(p,
           retype(brw_vec1_reg(BRW_GENERAL_REGISTER_FILE,
                               mrf.nr,
                               2), BRW_REGISTER_TYPE_UD),
           brw_imm_ud(offset));

   struct brw_instruction *insn = next_insn(p, BRW_OPCODE_SEND);
   insn->header.destreg__conditionalmod = mrf.nr;

   /* cast dest to a uword[8] vector */
   dest = retype(vec8(dest), BRW_REGISTER_TYPE_UW);

   brw_set_dest(p, insn, dest);
   if (p->gen >= 6) {
      brw_set_src0(p, insn, mrf);
   } else {
      brw_set_src0(p, insn, brw_null_reg());
   }

   brw_set_dp_read_message(p,
                           insn,
                           bind_table_index,
                           BRW_DATAPORT_OWORD_BLOCK_1_OWORDLOW,
                           BRW_DATAPORT_READ_MESSAGE_OWORD_BLOCK_READ,
                           BRW_DATAPORT_READ_TARGET_DATA_CACHE,
                           1, /* msg_length */
                           1); /* response_length (1 reg, 2 owords!) */

   brw_pop_insn_state(p);
}

/**
 * Read a set of dwords from the data port Data Cache (const buffer).
 *
 * Location (in buffer) appears as UD offsets in the register after
 * the provided mrf header reg.
 */
void brw_dword_scattered_read(struct brw_compile *p,
                              struct brw_reg dest,
                              struct brw_reg mrf,
                              uint32_t bind_table_index)
{
   mrf = retype(mrf, BRW_REGISTER_TYPE_UD);

   brw_push_insn_state(p);
   brw_set_predicate_control(p, BRW_PREDICATE_NONE);
   brw_set_compression_control(p, BRW_COMPRESSION_NONE);
   brw_set_mask_control(p, BRW_MASK_DISABLE);
   brw_MOV(p, mrf, retype(brw_vec8_grf(0, 0), BRW_REGISTER_TYPE_UD));
   brw_pop_insn_state(p);

   struct brw_instruction *insn = next_insn(p, BRW_OPCODE_SEND);
   insn->header.destreg__conditionalmod = mrf.nr;

   /* cast dest to a uword[8] vector */
   dest = retype(vec8(dest), BRW_REGISTER_TYPE_UW);

   brw_set_dest(p, insn, dest);
   brw_set_src0(p, insn, brw_null_reg());

   brw_set_dp_read_message(p,
                           insn,
                           bind_table_index,
                           BRW_DATAPORT_DWORD_SCATTERED_BLOCK_8DWORDS,
                           BRW_DATAPORT_READ_MESSAGE_DWORD_SCATTERED_READ,
                           BRW_DATAPORT_READ_TARGET_DATA_CACHE,
                           2, /* msg_length */
                           1); /* response_length */
}

/**
 * Texture sample instruction.
 * Note: the msg_type plus msg_length values determine exactly what kind
 * of sampling operation is performed.  See volume 4, page 161 of docs.
 */
void brw_SAMPLE(struct brw_compile *p,
                struct brw_reg dest,
                uint32_t msg_reg_nr,
                struct brw_reg src0,
                uint32_t binding_table_index,
                uint32_t sampler,
                uint32_t writemask,
                uint32_t msg_type,
                uint32_t response_length,
                uint32_t msg_length,
                uint32_t header_present,
                uint32_t simd_mode,
                uint32_t return_format)
{
   bool need_stall = 0;

   if (writemask == 0) {
      /*printf("%s: zero writemask??\n", __FUNCTION__); */
      return;
   }
   
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
      struct brw_instruction *insn;

      insn = next_insn(p, BRW_OPCODE_SEND);
      insn->header.predicate_control = 0; /* XXX */
      insn->header.compression_control = BRW_COMPRESSION_NONE;
      if (p->gen < 6)
          insn->header.destreg__conditionalmod = msg_reg_nr;

      brw_set_dest(p, insn, dest);
      brw_set_src0(p, insn, src0);
      brw_set_sampler_message(p, insn,
                              binding_table_index,
                              sampler,
                              msg_type,
                              response_length, 
                              msg_length,
                              header_present,
                              simd_mode,
                              return_format);
   }

   if (need_stall) {
      struct brw_reg reg = vec8(offset(dest, response_length-1));

      /*  mov (8) r9.0<1>:f    r9.0<8;8,1>:f    { Align1 }
       */
      brw_push_insn_state(p);
      brw_set_compression_control(p, BRW_COMPRESSION_NONE);
      brw_MOV(p, retype(reg, BRW_REGISTER_TYPE_UD),
              retype(reg, BRW_REGISTER_TYPE_UD));
      brw_pop_insn_state(p);
   }

}

void
brw_EOT(struct brw_compile *p, uint32_t msg_nr)
{
  struct brw_instruction *insn = NULL;

  insn = brw_MOV(p, brw_vec8_grf(msg_nr,0), brw_vec8_grf(0,0));
  insn->header.mask_control = BRW_MASK_DISABLE;
  insn = next_insn(p, BRW_OPCODE_SEND);
  brw_set_dest(p, insn, brw_null_reg());
  brw_set_src0(p, insn, brw_vec8_grf(msg_nr,0));
  brw_set_src1(p, insn, brw_imm_ud(0));
  insn->header.execution_size = BRW_EXECUTE_8;
  insn->header.compression_control = BRW_COMPRESSION_NONE;
  insn->bits3.spawner_gen5.resource = BRW_DO_NOT_DEREFERENCE_URB;
  insn->bits3.spawner_gen5.msg_length = 1;
  insn->bits3.spawner_gen5.end_of_thread = 1;
  insn->header.destreg__conditionalmod = BRW_SFID_THREAD_SPAWNER;
}


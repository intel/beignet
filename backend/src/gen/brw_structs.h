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
#ifndef BRW_STRUCTS_H
#define BRW_STRUCTS_H

#include <stdint.h>

/** Number of general purpose registers (VS, WM, etc) */
#define BRW_MAX_GRF 128

/**
 * First GRF used for the MRF hack.
 *
 * On gen7, MRFs are no longer used, and contiguous GRFs are used instead.  We
 * haven't converted our compiler to be aware of this, so it asks for MRFs and
 * brw_eu_emit.c quietly converts them to be accesses of the top GRFs.  The
 * register allocators have to be careful of this to avoid corrupting the "MRF"s
 * with actual GRF allocations.
 */
#define GEN7_MRF_HACK_START 112.

/** Number of message register file registers */
#define BRW_MAX_MRF 16

struct brw_urb_immediate {
   uint32_t opcode:4;
   uint32_t offset:6;
   uint32_t swizzle_control:2; 
   uint32_t pad:1;
   uint32_t allocate:1;
   uint32_t used:1;
   uint32_t complete:1;
   uint32_t response_length:4;
   uint32_t msg_length:4;
   uint32_t msg_target:4;
   uint32_t pad1:3;
   uint32_t end_of_thread:1;
};

struct brw_sampler_state
{
   struct
   {
      uint32_t shadow_function:3;
      uint32_t lod_bias:11;
      uint32_t min_filter:3;
      uint32_t mag_filter:3;
      uint32_t mip_filter:2;
      uint32_t base_level:5;
      uint32_t min_mag_neq:1;
      uint32_t lod_preclamp:1;
      uint32_t default_color_mode:1;
      uint32_t pad0:1;
      uint32_t disable:1;
   } ss0;

   struct
   {
      uint32_t r_wrap_mode:3;
      uint32_t t_wrap_mode:3;
      uint32_t s_wrap_mode:3;
      uint32_t cube_control_mode:1;
      uint32_t pad:2;
      uint32_t max_lod:10;
      uint32_t min_lod:10;
   } ss1;

   struct
   {
      uint32_t pad:5;
      uint32_t default_color_pointer:27;
   } ss2;

   struct
   {
      uint32_t non_normalized_coord:1;
      uint32_t pad:12;
      uint32_t address_round:6;
      uint32_t max_aniso:3;
      uint32_t chroma_key_mode:1;
      uint32_t chroma_key_index:2;
      uint32_t chroma_key_enable:1;
      uint32_t monochrome_filter_width:3;
      uint32_t monochrome_filter_height:3;
   } ss3;
};

struct gen7_sampler_state
{
   struct
   {
      uint32_t aniso_algorithm:1;
      uint32_t lod_bias:13;
      uint32_t min_filter:3;
      uint32_t mag_filter:3;
      uint32_t mip_filter:2;
      uint32_t base_level:5;
      uint32_t pad1:1;
      uint32_t lod_preclamp:1;
      uint32_t default_color_mode:1;
      uint32_t pad0:1;
      uint32_t disable:1;
   } ss0;

   struct
   {
      uint32_t cube_control_mode:1;
      uint32_t shadow_function:3;
      uint32_t pad:4;
      uint32_t max_lod:12;
      uint32_t min_lod:12;
   } ss1;

   struct
   {
      uint32_t pad:5;
      uint32_t default_color_pointer:27;
   } ss2;

   struct
   {
      uint32_t r_wrap_mode:3;
      uint32_t t_wrap_mode:3;
      uint32_t s_wrap_mode:3;
      uint32_t pad:1;
      uint32_t non_normalized_coord:1;
      uint32_t trilinear_quality:2;
      uint32_t address_round:6;
      uint32_t max_aniso:3;
      uint32_t chroma_key_mode:1;
      uint32_t chroma_key_index:2;
      uint32_t chroma_key_enable:1;
      uint32_t pad0:6;
   } ss3;
};

/* Instruction format for the execution units */
struct brw_instruction
{
   struct
   {
      uint32_t opcode:7;
      uint32_t pad:1;
      uint32_t access_mode:1;
      uint32_t mask_control:1;
      uint32_t dependency_control:2;
      uint32_t compression_control:2; /* gen6: quater control */
      uint32_t thread_control:2;
      uint32_t predicate_control:4;
      uint32_t predicate_inverse:1;
      uint32_t execution_size:3;
      /**
       * Conditional Modifier for most instructions.  On Gen6+, this is also
       * used for the SEND instruction's Message Target/SFID.
       */
      uint32_t destreg__conditionalmod:4;
      uint32_t acc_wr_control:1;
      uint32_t cmpt_control:1;
      uint32_t debug_control:1;
      uint32_t saturate:1;
   } header;

   union {
      struct
      {
         uint32_t dest_reg_file:2;
         uint32_t dest_reg_type:3;
         uint32_t src0_reg_file:2;
         uint32_t src0_reg_type:3;
         uint32_t src1_reg_file:2;
         uint32_t src1_reg_type:3;
         uint32_t pad:1;
         uint32_t dest_subreg_nr:5;
         uint32_t dest_reg_nr:8;
         uint32_t dest_horiz_stride:2;
         uint32_t dest_address_mode:1;
      } da1;

      struct
      {
         uint32_t dest_reg_file:2;
         uint32_t dest_reg_type:3;
         uint32_t src0_reg_file:2;
         uint32_t src0_reg_type:3;
         uint32_t src1_reg_file:2;        /* 0x00000c00 */
         uint32_t src1_reg_type:3;        /* 0x00007000 */
         uint32_t pad:1;
         int dest_indirect_offset:10;        /* offset against the deref'd address reg */
         uint32_t dest_subreg_nr:3; /* subnr for the address reg a0.x */
         uint32_t dest_horiz_stride:2;
         uint32_t dest_address_mode:1;
      } ia1;

      struct
      {
         uint32_t dest_reg_file:2;
         uint32_t dest_reg_type:3;
         uint32_t src0_reg_file:2;
         uint32_t src0_reg_type:3;
         uint32_t src1_reg_file:2;
         uint32_t src1_reg_type:3;
         uint32_t pad:1;
         uint32_t dest_writemask:4;
         uint32_t dest_subreg_nr:1;
         uint32_t dest_reg_nr:8;
         uint32_t dest_horiz_stride:2;
         uint32_t dest_address_mode:1;
      } da16;

      struct
      {
         uint32_t dest_reg_file:2;
         uint32_t dest_reg_type:3;
         uint32_t src0_reg_file:2;
         uint32_t src0_reg_type:3;
         uint32_t pad0:6;
         uint32_t dest_writemask:4;
         int dest_indirect_offset:6;
         uint32_t dest_subreg_nr:3;
         uint32_t dest_horiz_stride:2;
         uint32_t dest_address_mode:1;
      } ia16;

      struct {
         uint32_t dest_reg_file:2;
         uint32_t dest_reg_type:3;
         uint32_t src0_reg_file:2;
         uint32_t src0_reg_type:3;
         uint32_t src1_reg_file:2;
         uint32_t src1_reg_type:3;
         uint32_t pad:1;

         int jump_count:16;
      } branch_gen6;

      struct {
         uint32_t dest_reg_file:1;
         uint32_t flag_subreg_num:1;
         uint32_t pad0:2;
         uint32_t src0_abs:1;
         uint32_t src0_negate:1;
         uint32_t src1_abs:1;
         uint32_t src1_negate:1;
         uint32_t src2_abs:1;
         uint32_t src2_negate:1;
         uint32_t pad1:7;
         uint32_t dest_writemask:4;
         uint32_t dest_subreg_nr:3;
         uint32_t dest_reg_nr:8;
      } da3src;
   } bits1;


   union {
      struct
      {
         uint32_t src0_subreg_nr:5;
         uint32_t src0_reg_nr:8;
         uint32_t src0_abs:1;
         uint32_t src0_negate:1;
         uint32_t src0_address_mode:1;
         uint32_t src0_horiz_stride:2;
         uint32_t src0_width:3;
         uint32_t src0_vert_stride:4;
         uint32_t flag_reg_nr:1;
         uint32_t pad:6;
      } da1;

      struct
      {
         int src0_indirect_offset:10;
         uint32_t src0_subreg_nr:3;
         uint32_t src0_abs:1;
         uint32_t src0_negate:1;
         uint32_t src0_address_mode:1;
         uint32_t src0_horiz_stride:2;
         uint32_t src0_width:3;
         uint32_t src0_vert_stride:4;
         uint32_t flag_reg_nr:1;
         uint32_t pad:6;
      } ia1;

      struct
      {
         uint32_t src0_swz_x:2;
         uint32_t src0_swz_y:2;
         uint32_t src0_subreg_nr:1;
         uint32_t src0_reg_nr:8;
         uint32_t src0_abs:1;
         uint32_t src0_negate:1;
         uint32_t src0_address_mode:1;
         uint32_t src0_swz_z:2;
         uint32_t src0_swz_w:2;
         uint32_t pad0:1;
         uint32_t src0_vert_stride:4;
         uint32_t flag_reg_nr:1;
         uint32_t pad1:6;
      } da16;

      struct
      {
         uint32_t src0_swz_x:2;
         uint32_t src0_swz_y:2;
         int src0_indirect_offset:6;
         uint32_t src0_subreg_nr:3;
         uint32_t src0_abs:1;
         uint32_t src0_negate:1;
         uint32_t src0_address_mode:1;
         uint32_t src0_swz_z:2;
         uint32_t src0_swz_w:2;
         uint32_t pad0:1;
         uint32_t src0_vert_stride:4;
         uint32_t flag_reg_nr:1;
         uint32_t pad1:6;
      } ia16;

      /* Extended Message Descriptor for Ironlake (Gen5) SEND instruction.
       *
       * Does not apply to Gen6+.  The SFID/message target moved to bits
       * 27:24 of the header (destreg__conditionalmod); EOT is in bits3.
       */
       struct
       {
           uint32_t pad:26;
           uint32_t end_of_thread:1;
           uint32_t pad1:1;
           uint32_t sfid:4;
       } send_gen5;  /* for Ironlake only */

      struct {
         uint32_t src0_rep_ctrl:1;
         uint32_t src0_swizzle:8;
         uint32_t src0_subreg_nr:3;
         uint32_t src0_reg_nr:8;
         uint32_t pad0:1;
         uint32_t src1_rep_ctrl:1;
         uint32_t src1_swizzle:8;
         uint32_t src1_subreg_nr_low:2;
      } da3src;
   } bits2;

   union
   {
      struct
      {
         uint32_t src1_subreg_nr:5;
         uint32_t src1_reg_nr:8;
         uint32_t src1_abs:1;
         uint32_t src1_negate:1;
         uint32_t src1_address_mode:1;
         uint32_t src1_horiz_stride:2;
         uint32_t src1_width:3;
         uint32_t src1_vert_stride:4;
         uint32_t pad0:7;
      } da1;

      struct
      {
         uint32_t src1_swz_x:2;
         uint32_t src1_swz_y:2;
         uint32_t src1_subreg_nr:1;
         uint32_t src1_reg_nr:8;
         uint32_t src1_abs:1;
         uint32_t src1_negate:1;
         uint32_t src1_address_mode:1;
         uint32_t src1_swz_z:2;
         uint32_t src1_swz_w:2;
         uint32_t pad1:1;
         uint32_t src1_vert_stride:4;
         uint32_t pad2:7;
      } da16;

      struct
      {
         int  src1_indirect_offset:10;
         uint32_t src1_subreg_nr:3;
         uint32_t src1_abs:1;
         uint32_t src1_negate:1;
         uint32_t src1_address_mode:1;
         uint32_t src1_horiz_stride:2;
         uint32_t src1_width:3;
         uint32_t src1_vert_stride:4;
         uint32_t flag_reg_nr:1;
         uint32_t pad1:6;
      } ia1;

      struct
      {
         uint32_t src1_swz_x:2;
         uint32_t src1_swz_y:2;
         int  src1_indirect_offset:6;
         uint32_t src1_subreg_nr:3;
         uint32_t src1_abs:1;
         uint32_t src1_negate:1;
         uint32_t pad0:1;
         uint32_t src1_swz_z:2;
         uint32_t src1_swz_w:2;
         uint32_t pad1:1;
         uint32_t src1_vert_stride:4;
         uint32_t flag_reg_nr:1;
         uint32_t pad2:6;
      } ia16;


      struct
      {
         int  jump_count:16;        /* note: signed */
         uint32_t  pop_count:4;
         uint32_t  pad0:12;
      } if_else;

      /* This is also used for gen7 IF/ELSE instructions */
      struct
      {
         /* Signed jump distance to the ip to jump to if all channels
          * are disabled after the break or continue.  It should point
          * to the end of the innermost control flow block, as that's
          * where some channel could get re-enabled.
          */
         int jip:16;

         /* Signed jump distance to the location to resume execution
          * of this channel if it's enabled for the break or continue.
          */
         int uip:16;
      } break_cont;

      /**
       * \defgroup SEND instructions / Message Descriptors
       *
       * @{
       */

      /**
       * Generic Message Descriptor for Gen4 SEND instructions.  The structs
       * below expand function_control to something specific for their
       * message.  Due to struct packing issues, they duplicate these bits.
       *
       * See the G45 PRM, Volume 4, Table 14-15.
       */
      struct {
         uint32_t function_control:16;
         uint32_t response_length:4;
         uint32_t msg_length:4;
         uint32_t msg_target:4;
         uint32_t pad1:3;
         uint32_t end_of_thread:1;
      } generic;

      /**
       * Generic Message Descriptor for Gen5-7 SEND instructions.
       *
       * See the Sandybridge PRM, Volume 2 Part 2, Table 8-15.  (Sadly, most
       * of the information on the SEND instruction is missing from the public
       * Ironlake PRM.)
       *
       * The table claims that bit 31 is reserved/MBZ on Gen6+, but it lies.
       * According to the SEND instruction description:
       * "The MSb of the message description, the EOT field, always comes from
       *  bit 127 of the instruction word"...which is bit 31 of this field.
       */
      struct {
         uint32_t function_control:19;
         uint32_t header_present:1;
         uint32_t response_length:5;
         uint32_t msg_length:4;
         uint32_t pad1:2;
         uint32_t end_of_thread:1;
      } generic_gen5;

      struct {
        uint32_t opcode:1;
        uint32_t request:1;
        uint32_t pad0:2;
        uint32_t resource:1;
        uint32_t pad1:14;
        uint32_t header:1;
        uint32_t response_length:5;
        uint32_t msg_length:4;
        uint32_t pad2:2;
        uint32_t end_of_thread:1;
      } spawner_gen5;

      /** G45 PRM, Volume 4, Section 6.1.1.1 */
      struct {
         uint32_t function:4;
         uint32_t int_type:1;
         uint32_t precision:1;
         uint32_t saturate:1;
         uint32_t data_type:1;
         uint32_t pad0:8;
         uint32_t response_length:4;
         uint32_t msg_length:4;
         uint32_t msg_target:4;
         uint32_t pad1:3;
         uint32_t end_of_thread:1;
      } math;

      /** Ironlake PRM, Volume 4 Part 1, Section 6.1.1.1 */
      struct {
         uint32_t function:4;
         uint32_t int_type:1;
         uint32_t precision:1;
         uint32_t saturate:1;
         uint32_t data_type:1;
         uint32_t snapshot:1;
         uint32_t pad0:10;
         uint32_t header_present:1;
         uint32_t response_length:5;
         uint32_t msg_length:4;
         uint32_t pad1:2;
         uint32_t end_of_thread:1;
      } math_gen5;

      /** G45 PRM, Volume 4, Section 4.8.1.1.1 [DevBW] and [DevCL] */
      struct {
         uint32_t binding_table_index:8;
         uint32_t sampler:4;
         uint32_t return_format:2;
         uint32_t msg_type:2;
         uint32_t response_length:4;
         uint32_t msg_length:4;
         uint32_t msg_target:4;
         uint32_t pad1:3;
         uint32_t end_of_thread:1;
      } sampler;

      /** G45 PRM, Volume 4, Section 4.8.1.1.2 [DevCTG] */
      struct {
         uint32_t binding_table_index:8;
         uint32_t sampler:4;
         uint32_t msg_type:4;
         uint32_t response_length:4;
         uint32_t msg_length:4;
         uint32_t msg_target:4;
         uint32_t pad1:3;
         uint32_t end_of_thread:1;
      } sampler_g4x;

      /** Ironlake PRM, Volume 4 Part 1, Section 4.11.1.1.3 */
      struct {
         uint32_t binding_table_index:8;
         uint32_t sampler:4;
         uint32_t msg_type:4;
         uint32_t simd_mode:2;
         uint32_t pad0:1;
         uint32_t header_present:1;
         uint32_t response_length:5;
         uint32_t msg_length:4;
         uint32_t pad1:2;
         uint32_t end_of_thread:1;
      } sampler_gen5;

      struct {
         uint32_t binding_table_index:8;
         uint32_t sampler:4;
         uint32_t msg_type:5;
         uint32_t simd_mode:2;
         uint32_t header_present:1;
         uint32_t response_length:5;
         uint32_t msg_length:4;
         uint32_t pad1:2;
         uint32_t end_of_thread:1;
      } sampler_gen7;

      struct brw_urb_immediate urb;

      struct {
         uint32_t opcode:4;
         uint32_t offset:6;
         uint32_t swizzle_control:2;
         uint32_t pad:1;
         uint32_t allocate:1;
         uint32_t used:1;
         uint32_t complete:1;
         uint32_t pad0:3;
         uint32_t header_present:1;
         uint32_t response_length:5;
         uint32_t msg_length:4;
         uint32_t pad1:2;
         uint32_t end_of_thread:1;
      } urb_gen5;

      struct {
         uint32_t opcode:3;
         uint32_t offset:11;
         uint32_t swizzle_control:1;
         uint32_t complete:1;
         uint32_t per_slot_offset:1;
         uint32_t pad0:2;
         uint32_t header_present:1;
         uint32_t response_length:5;
         uint32_t msg_length:4;
         uint32_t pad1:2;
         uint32_t end_of_thread:1;
      } urb_gen7;

      /** 965 PRM, Volume 4, Section 5.10.1.1: Message Descriptor */
      struct {
         uint32_t binding_table_index:8;
         uint32_t msg_control:4;
         uint32_t msg_type:2;
         uint32_t target_cache:2;
         uint32_t response_length:4;
         uint32_t msg_length:4;
         uint32_t msg_target:4;
         uint32_t pad1:3;
         uint32_t end_of_thread:1;
      } dp_read;

      /** G45 PRM, Volume 4, Section 5.10.1.1.2 */
      struct {
         uint32_t binding_table_index:8;
         uint32_t msg_control:3;
         uint32_t msg_type:3;
         uint32_t target_cache:2;
         uint32_t response_length:4;
         uint32_t msg_length:4;
         uint32_t msg_target:4;
         uint32_t pad1:3;
         uint32_t end_of_thread:1;
      } dp_read_g4x;

      /** Ironlake PRM, Volume 4 Part 1, Section 5.10.2.1.2. */
      struct {
         uint32_t binding_table_index:8;
         uint32_t msg_control:3;
         uint32_t msg_type:3;
         uint32_t target_cache:2;
         uint32_t pad0:3;
         uint32_t header_present:1;
         uint32_t response_length:5;
         uint32_t msg_length:4;
         uint32_t pad1:2;
         uint32_t end_of_thread:1;
      } dp_read_gen5;

      /** G45 PRM, Volume 4, Section 5.10.1.1.2.  For both Gen4 and G45. */
      struct {
         uint32_t binding_table_index:8;
         uint32_t msg_control:3;
         uint32_t last_render_target:1;
         uint32_t msg_type:3;
         uint32_t send_commit_msg:1;
         uint32_t response_length:4;
         uint32_t msg_length:4;
         uint32_t msg_target:4;
         uint32_t pad1:3;
         uint32_t end_of_thread:1;
      } dp_write;

      /** Ironlake PRM, Volume 4 Part 1, Section 5.10.2.1.2. */
      struct {
         uint32_t binding_table_index:8;
         uint32_t msg_control:3;
         uint32_t last_render_target:1;
         uint32_t msg_type:3;
         uint32_t send_commit_msg:1;
         uint32_t pad0:3;
         uint32_t header_present:1;
         uint32_t response_length:5;
         uint32_t msg_length:4;
         uint32_t pad1:2;
         uint32_t end_of_thread:1;
      } dp_write_gen5;

      /**
       * Message for the Sandybridge Sampler Cache or Constant Cache Data Port.
       *
       * See the Sandybridge PRM, Volume 4 Part 1, Section 3.9.2.1.1.
       **/
      struct {
         uint32_t binding_table_index:8;
         uint32_t msg_control:5;
         uint32_t msg_type:3;
         uint32_t pad0:3;
         uint32_t header_present:1;
         uint32_t response_length:5;
         uint32_t msg_length:4;
         uint32_t pad1:2;
         uint32_t end_of_thread:1;
      } gen6_dp_sampler_const_cache;

      /**
       * Message for the Sandybridge Render Cache Data Port.
       *
       * Most fields are defined in the Sandybridge PRM, Volume 4 Part 1,
       * Section 3.9.2.1.1: Message Descriptor.
       *
       * "Slot Group Select" and "Last Render Target" are part of the
       * 5-bit message control for Render Target Write messages.  See
       * Section 3.9.9.2.1 of the same volume.
       */
      struct {
         uint32_t binding_table_index:8;
         uint32_t msg_control:3;
         uint32_t slot_group_select:1;
         uint32_t last_render_target:1;
         uint32_t msg_type:4;
         uint32_t send_commit_msg:1;
         uint32_t pad0:1;
         uint32_t header_present:1;
         uint32_t response_length:5;
         uint32_t msg_length:4;
         uint32_t pad1:2;
         uint32_t end_of_thread:1;
      } gen6_dp;

      /**
       * Message for any of the Gen7 Data Port caches.
       *
       * Most fields are defined in BSpec volume 5c.2 Data Port / Messages /
       * Data Port Messages / Message Descriptor.  Once again, "Slot Group
       * Select" and "Last Render Target" are part of the 6-bit message
       * control for Render Target Writes.
       */
      struct {
         uint32_t binding_table_index:8;
         uint32_t msg_control:3;
         uint32_t slot_group_select:1;
         uint32_t last_render_target:1;
         uint32_t msg_control_pad:1;
         uint32_t msg_type:4;
         uint32_t pad1:1;
         uint32_t header_present:1;
         uint32_t response_length:5;
         uint32_t msg_length:4;
         uint32_t pad2:2;
         uint32_t end_of_thread:1;
      } gen7_dp;
      /** @} */

      struct {
         uint32_t src1_subreg_nr_high:1;
         uint32_t src1_reg_nr:8;
         uint32_t pad0:1;
         uint32_t src2_rep_ctrl:1;
         uint32_t src2_swizzle:8;
         uint32_t src2_subreg_nr:3;
         uint32_t src2_reg_nr:8;
         uint32_t pad1:2;
      } da3src;

      int d;
      uint32_t ud;
      float f;
   } bits3;
};

#endif

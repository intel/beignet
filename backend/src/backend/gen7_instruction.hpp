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
 * Author: Rong Yang <rong.r.yang@intel.com>
 */

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
 /*
  * Authors:
  *   Keith Whitwell <keith@tungstengraphics.com>
  */

#ifndef __GEN7_INSTRUCTION_HPP__
#define __GEN7_INSTRUCTION_HPP__

union Gen7NativeInstruction
{
  struct {
    struct {
      uint32_t opcode:7;
      uint32_t pad:1;
      uint32_t access_mode:1;
      uint32_t mask_control:1;
      uint32_t dependency_control:2;
      uint32_t quarter_control:2;
      uint32_t thread_control:2;
      uint32_t predicate_control:4;
      uint32_t predicate_inverse:1;
      uint32_t execution_size:3;
      uint32_t destreg_or_condmod:4;
      uint32_t acc_wr_control:1;
      uint32_t cmpt_control:1;
      uint32_t debug_control:1;
      uint32_t saturate:1;
    } header;

    union {
      struct {
        uint32_t dest_reg_file:2;
        uint32_t dest_reg_type:3;
        uint32_t src0_reg_file:2;
        uint32_t src0_reg_type:3;
        uint32_t src1_reg_file:2;
        uint32_t src1_reg_type:3;
        uint32_t nib_ctrl:1;
        uint32_t dest_subreg_nr:5;
        uint32_t dest_reg_nr:8;
        uint32_t dest_horiz_stride:2;
        uint32_t dest_address_mode:1;
      } da1;

      struct {
        uint32_t dest_reg_file:2;
        uint32_t dest_reg_type:3;
        uint32_t src0_reg_file:2;
        uint32_t src0_reg_type:3;
        uint32_t src1_reg_file:2;        /* 0x00000c00 */
        uint32_t src1_reg_type:3;        /* 0x00007000 */
        uint32_t nib_ctrl:1;
        int dest_indirect_offset:10;        /* offset against the deref'd address reg */
        uint32_t dest_subreg_nr:3; /* subnr for the address reg a0.x */
        uint32_t dest_horiz_stride:2;
        uint32_t dest_address_mode:1;
      } ia1;

      struct {
        uint32_t dest_reg_file:2;
        uint32_t dest_reg_type:3;
        uint32_t src0_reg_file:2;
        uint32_t src0_reg_type:3;
        uint32_t src1_reg_file:2;
        uint32_t src1_reg_type:3;
        uint32_t nib_ctrl:1;
        uint32_t dest_writemask:4;
        uint32_t dest_subreg_nr:1;
        uint32_t dest_reg_nr:8;
        uint32_t dest_horiz_stride:2;
        uint32_t dest_address_mode:1;
      } da16;

      struct {
        uint32_t dest_reg_file:2;
        uint32_t dest_reg_type:3;
        uint32_t src0_reg_file:2;
        uint32_t src0_reg_type:3;
        uint32_t nib_ctrl:1;
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
      struct {
        uint32_t src0_subreg_nr:5;
        uint32_t src0_reg_nr:8;
        uint32_t src0_abs:1;
        uint32_t src0_negate:1;
        uint32_t src0_address_mode:1;
        uint32_t src0_horiz_stride:2;
        uint32_t src0_width:3;
        uint32_t src0_vert_stride:4;
        uint32_t flag_sub_reg_nr:1;
        uint32_t flag_reg_nr:1;
        uint32_t pad:5;
      } da1;

      struct {
        int src0_indirect_offset:10;
        uint32_t src0_subreg_nr:3;
        uint32_t src0_abs:1;
        uint32_t src0_negate:1;
        uint32_t src0_address_mode:1;
        uint32_t src0_horiz_stride:2;
        uint32_t src0_width:3;
        uint32_t src0_vert_stride:4;
        uint32_t flag_sub_reg_nr:1;
        uint32_t flag_reg_nr:1;
        uint32_t pad:5;
      } ia1;

      struct {
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
        uint32_t flag_sub_reg_nr:1;
        uint32_t flag_reg_nr:1;
        uint32_t pad:5;
      } da16;

      struct {
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
        uint32_t flag_sub_reg_nr:1;
        uint32_t flag_reg_nr:1;
        uint32_t pad:5;
      } ia16;

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

    union {
      struct {
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

      struct {
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

      struct {
        int  src1_indirect_offset:10;
        uint32_t src1_subreg_nr:3;
        uint32_t src1_abs:1;
        uint32_t src1_negate:1;
        uint32_t src1_address_mode:1;
        uint32_t src1_horiz_stride:2;
        uint32_t src1_width:3;
        uint32_t src1_vert_stride:4;
        uint32_t pad1:7;
      } ia1;

      struct {
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
        uint32_t pad2:7;
      } ia16;

      struct {
        uint32_t function_control:19;
        uint32_t header_present:1;
        uint32_t response_length:5;
        uint32_t msg_length:4;
        uint32_t pad1:2;
        uint32_t end_of_thread:1;
      } generic_gen5;

      struct {
        uint32_t sub_function_id:3;
        uint32_t pad0:11;
        uint32_t ack_req:1;
        uint32_t notify:2;
        uint32_t pad1:2;
        uint32_t header:1;
        uint32_t response_length:5;
        uint32_t msg_length:4;
        uint32_t pad2:2;
        uint32_t end_of_thread:1;
      } msg_gateway;

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

      struct {
        uint32_t bti:8;
        uint32_t sampler:4;
        uint32_t msg_type:5;
        uint32_t simd_mode:2;
        uint32_t header_present:1;
        uint32_t response_length:5;
        uint32_t msg_length:4;
        uint32_t pad1:2;
        uint32_t end_of_thread:1;
      } sampler_gen7;

      /**
       * Message for the Sandybridge Sampler Cache or Constant Cache Data Port.
       *
       * See the Sandybridge PRM, Volume 4 Part 1, Section 3.9.2.1.1.
       **/
      struct {
        uint32_t bti:8;
        uint32_t msg_control:5;
        uint32_t msg_type:3;
        uint32_t pad0:3;
        uint32_t header_present:1;
        uint32_t response_length:5;
        uint32_t msg_length:4;
        uint32_t pad1:2;
        uint32_t end_of_thread:1;
      } gen6_dp_sampler_const_cache;

      /*! Data port untyped read / write messages */
      struct {
        uint32_t bti:8;
        uint32_t rgba:4;
        uint32_t simd_mode:2;
        uint32_t msg_type:4;
        uint32_t category:1;
        uint32_t header_present:1;
        uint32_t response_length:5;
        uint32_t msg_length:4;
        uint32_t pad2:2;
        uint32_t end_of_thread:1;
      } gen7_untyped_rw;

      /*! Data port byte scatter / gather */
      struct {
        uint32_t bti:8;
        uint32_t simd_mode:1;
        uint32_t ignored0:1;
        uint32_t data_size:2;
        uint32_t ignored1:2;
        uint32_t msg_type:4;
        uint32_t category:1;
        uint32_t header_present:1;
        uint32_t response_length:5;
        uint32_t msg_length:4;
        uint32_t pad2:2;
        uint32_t end_of_thread:1;
      } gen7_byte_rw;

      /*! Data port Scratch Read/ write */
      struct {
        uint32_t offset:12;
        uint32_t block_size:2;
        uint32_t ignored0:1;
        uint32_t invalidate_after_read:1;
        uint32_t channel_mode:1;
        uint32_t msg_type:1;
        uint32_t category:1;
        uint32_t header_present:1;
        uint32_t response_length:5;
        uint32_t msg_length:4;
        uint32_t pad2:2;
        uint32_t end_of_thread:1;
      } gen7_scratch_rw;

      /*! Data port OBlock read / write */
      struct {
        uint32_t bti:8;
        uint32_t block_size:3;
        uint32_t ignored:2;
        uint32_t invalidate_after_read:1;
        uint32_t msg_type:4;
        uint32_t category:1;
        uint32_t header_present:1;
        uint32_t response_length:5;
        uint32_t msg_length:4;
        uint32_t pad2:2;
        uint32_t end_of_thread:1;
      } gen7_oblock_rw;

      /*! Data port dword scatter / gather */
      struct {
        uint32_t bti:8;
        uint32_t block_size:2;
        uint32_t ignored0:3;
        uint32_t invalidate_after_read:1;
        uint32_t msg_type:4;
        uint32_t ignored1:1;
        uint32_t header_present:1;
        uint32_t response_length:5;
        uint32_t msg_length:4;
        uint32_t pad2:2;
        uint32_t end_of_thread:1;
      } gen7_dword_rw;

      /*! Data port typed read / write messages */
      struct {
        uint32_t bti:8;
        uint32_t chan_mask:4;
        uint32_t slot:2;
        uint32_t msg_type:4;
        uint32_t pad2:1;
        uint32_t header_present:1;
        uint32_t response_length:5;
        uint32_t msg_length:4;
        uint32_t pad3:2;
        uint32_t end_of_thread:1;
      } gen7_typed_rw;

      /*! Memory fence */
      struct {
        uint32_t bti:8;
        uint32_t pad:5;
        uint32_t commit_enable:1;
        uint32_t msg_type:4;
        uint32_t pad2:1;
        uint32_t header_present:1;
        uint32_t response_length:5;
        uint32_t msg_length:4;
        uint32_t pad3:2;
        uint32_t end_of_thread:1;
      } gen7_memory_fence;

      /*! atomic messages */
      struct {
        uint32_t bti:8;
        uint32_t aop_type:4;
        uint32_t simd_mode:1;
        uint32_t return_data:1;
        uint32_t msg_type:4;
        uint32_t category:1;
        uint32_t header_present:1;
        uint32_t response_length:5;
        uint32_t msg_length:4;
        uint32_t pad3:2;
        uint32_t end_of_thread:1;
      } gen7_atomic_op;

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

      /*! Message gateway */
      struct {
        uint32_t subfunc:3;
        uint32_t pad:11;
        uint32_t ackreq:1;
        uint32_t notify:2;
        uint32_t pad2:2;
        uint32_t header_present:1;
        uint32_t response_length:5;
        uint32_t msg_length:4;
        uint32_t pad3:2;
        uint32_t end_of_thread:1;
      } gen7_msg_gw;

      struct {
        uint32_t jip:16;
        uint32_t uip:16;
      } gen7_branch;

      int d;
      uint32_t ud;
      float f;
    } bits3;
  };
};
#endif

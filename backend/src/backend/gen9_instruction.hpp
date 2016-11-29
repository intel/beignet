/*
 * Copyright © 2016 Intel Corporation
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
 * Author: Guo, Yejun <yejun.guo@intel.com>
 */


#ifndef __GEN9_INSTRUCTION_HPP__
#define __GEN9_INSTRUCTION_HPP__

union Gen9NativeInstruction
{
  struct {
    struct {
      uint32_t opcode:7;
      uint32_t pad:1;
      uint32_t access_mode:1;
      uint32_t dependency_control:2;
      uint32_t nib_ctrl:1;
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
        uint32_t flag_sub_reg_nr:1;
        uint32_t flag_reg_nr:1;
        uint32_t mask_control:1;
        uint32_t dest_reg_file_0:1;
        uint32_t src1_reg_file_0:1;
        uint32_t dest_reg_type:4;
        uint32_t pad0:3;
        uint32_t src1_reg_nr:8;
        uint32_t dest_subreg_nr:1;
        uint32_t dest_reg_nr:8;
        uint32_t pad1:1;
        uint32_t pad2:1;    //direct mode is used
        uint32_t dest_address_mode:1;
      } sends;

      uint32_t ud;
    }bits1;

    union {
      struct {
        uint32_t src1_length:4;     //exdesc_9_6
        uint32_t src0_subreg_nr:1;
        uint32_t src0_reg_nr:8;
        uint32_t sel_reg32_desc:1;
        uint32_t pad0:1;
        uint32_t src0_address_mode:1;
        uint32_t exdesc_31_16:16;
      } sends;

      uint32_t ud;
    } bits2;

    union {
      uint32_t ud;
    } bits3;
  };
};
#endif

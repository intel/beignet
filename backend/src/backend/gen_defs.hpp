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

#ifndef __GEN_DEFS_HPP__
#define __GEN_DEFS_HPP__

#include <cstdint>

/////////////////////////////////////////////////////////////////////////////
// Gen EU defines
/////////////////////////////////////////////////////////////////////////////

/* Execution Unit (EU) defines */
#define GEN_ALIGN_1   0
#define GEN_ALIGN_16  1

#define GEN_REG_SIZE 32

#define GEN_ADDRESS_DIRECT                        0
#define GEN_ADDRESS_REGISTER_INDIRECT_REGISTER    1

#define GEN_CHANNEL_X     0
#define GEN_CHANNEL_Y     1
#define GEN_CHANNEL_Z     2
#define GEN_CHANNEL_W     3

#define GEN_COMPRESSION_Q1  0
#define GEN_COMPRESSION_Q2  1
#define GEN_COMPRESSION_Q3  2
#define GEN_COMPRESSION_Q4  3
#define GEN_COMPRESSION_H1  0
#define GEN_COMPRESSION_H2  2

#define GEN_CONDITIONAL_NONE  0
#define GEN_CONDITIONAL_Z     1
#define GEN_CONDITIONAL_NZ    2
#define GEN_CONDITIONAL_EQ    1 /* Z */
#define GEN_CONDITIONAL_NEQ   2 /* NZ */
#define GEN_CONDITIONAL_G     3
#define GEN_CONDITIONAL_GE    4
#define GEN_CONDITIONAL_L     5
#define GEN_CONDITIONAL_LE    6
#define GEN_CONDITIONAL_R     7
#define GEN_CONDITIONAL_O     8
#define GEN_CONDITIONAL_U     9

#define GEN_DEBUG_NONE        0
#define GEN_DEBUG_BREAKPOINT  1

#define GEN_DEPENDENCY_NORMAL         0
#define GEN_DEPENDENCY_NOTCLEARED     1
#define GEN_DEPENDENCY_NOTCHECKED     2
#define GEN_DEPENDENCY_DISABLE        3

#define GEN_HORIZONTAL_STRIDE_0   0
#define GEN_HORIZONTAL_STRIDE_1   1
#define GEN_HORIZONTAL_STRIDE_2   2
#define GEN_HORIZONTAL_STRIDE_4   3

#define GEN_INSTRUCTION_NORMAL    0
#define GEN_INSTRUCTION_SATURATE  1

#define GEN_MASK_ENABLE   0
#define GEN_MASK_DISABLE  1

/*! Gen opcode */
enum opcode {
  GEN_OPCODE_MOV = 1,
  GEN_OPCODE_SEL = 2,
  GEN_OPCODE_NOT = 4,
  GEN_OPCODE_AND = 5,
  GEN_OPCODE_OR = 6,
  GEN_OPCODE_XOR = 7,
  GEN_OPCODE_SHR = 8,
  GEN_OPCODE_SHL = 9,
  GEN_OPCODE_RSR = 10,
  GEN_OPCODE_RSL = 11,
  GEN_OPCODE_ASR = 12,
  GEN_OPCODE_CMP = 16,
  GEN_OPCODE_CMPN = 17,
  GEN_OPCODE_JMPI = 32,
  GEN_OPCODE_IF = 34,
  GEN_OPCODE_IFF = 35,
  GEN_OPCODE_ELSE = 36,
  GEN_OPCODE_ENDIF = 37,
  GEN_OPCODE_DO = 38,
  GEN_OPCODE_WHILE = 39,
  GEN_OPCODE_BREAK = 40,
  GEN_OPCODE_CONTINUE = 41,
  GEN_OPCODE_HALT = 42,
  GEN_OPCODE_MSAVE = 44,
  GEN_OPCODE_MRESTORE = 45,
  GEN_OPCODE_PUSH = 46,
  GEN_OPCODE_POP = 47,
  GEN_OPCODE_WAIT = 48,
  GEN_OPCODE_SEND = 49,
  GEN_OPCODE_SENDC = 50,
  GEN_OPCODE_MATH = 56,
  GEN_OPCODE_ADD = 64,
  GEN_OPCODE_MUL = 65,
  GEN_OPCODE_AVG = 66,
  GEN_OPCODE_FRC = 67,
  GEN_OPCODE_RNDU = 68,
  GEN_OPCODE_RNDD = 69,
  GEN_OPCODE_RNDE = 70,
  GEN_OPCODE_RNDZ = 71,
  GEN_OPCODE_MAC = 72,
  GEN_OPCODE_MACH = 73,
  GEN_OPCODE_LZD = 74,
  GEN_OPCODE_SAD2 = 80,
  GEN_OPCODE_SADA2 = 81,
  GEN_OPCODE_DP4 = 84,
  GEN_OPCODE_DPH = 85,
  GEN_OPCODE_DP3 = 86,
  GEN_OPCODE_DP2 = 87,
  GEN_OPCODE_DPA2 = 88,
  GEN_OPCODE_LINE = 89,
  GEN_OPCODE_PLN = 90,
  GEN_OPCODE_MAD = 91,
  GEN_OPCODE_NOP = 126,
};

/*! Gen SFID */
enum GenMessageTarget {
  GEN_SFID_NULL                     = 0,
  GEN_SFID_MATH                     = 1,
  GEN_SFID_SAMPLER                  = 2,
  GEN_SFID_MESSAGE_GATEWAY          = 3,
  GEN_SFID_DATAPORT_READ            = 4,
  GEN_SFID_DATAPORT_WRITE           = 5,
  GEN_SFID_URB                      = 6,
  GEN_SFID_THREAD_SPAWNER           = 7,
  GEN6_SFID_DATAPORT_SAMPLER_CACHE  = 4,
  GEN6_SFID_DATAPORT_RENDER_CACHE   = 5,
  GEN6_SFID_DATAPORT_CONSTANT_CACHE = 9,
  GEN_SFID_DATAPORT_DATA_CACHE     = 10,
};

#define GEN_PREDICATE_NONE                    0
#define GEN_PREDICATE_NORMAL                  1
#define GEN_PREDICATE_ALIGN1_ANYV             2
#define GEN_PREDICATE_ALIGN1_ALLV             3
#define GEN_PREDICATE_ALIGN1_ANY2H            4
#define GEN_PREDICATE_ALIGN1_ALL2H            5
#define GEN_PREDICATE_ALIGN1_ANY4H            6
#define GEN_PREDICATE_ALIGN1_ALL4H            7
#define GEN_PREDICATE_ALIGN1_ANY8H            8
#define GEN_PREDICATE_ALIGN1_ALL8H            9
#define GEN_PREDICATE_ALIGN1_ANY16H           10
#define GEN_PREDICATE_ALIGN1_ALL16H           11
#define GEN_PREDICATE_ALIGN16_REPLICATE_X     2
#define GEN_PREDICATE_ALIGN16_REPLICATE_Y     3
#define GEN_PREDICATE_ALIGN16_REPLICATE_Z     4
#define GEN_PREDICATE_ALIGN16_REPLICATE_W     5
#define GEN_PREDICATE_ALIGN16_ANY4H           6
#define GEN_PREDICATE_ALIGN16_ALL4H           7

#define GEN_ARCHITECTURE_REGISTER_FILE        0
#define GEN_GENERAL_REGISTER_FILE             1
#define GEN_IMMEDIATE_VALUE                   3

#define GEN_TYPE_UD  0
#define GEN_TYPE_D   1
#define GEN_TYPE_UW  2
#define GEN_TYPE_W   3
#define GEN_TYPE_UB  4
#define GEN_TYPE_B   5
#define GEN_TYPE_VF  5 /* packed float vector, immediates only? */
#define GEN_TYPE_HF  6
#define GEN_TYPE_V   6 /* packed int vector, immediates only, uword dest only */
#define GEN_TYPE_F   7

#define GEN_ARF_NULL                  0x00
#define GEN_ARF_ADDRESS               0x10
#define GEN_ARF_ACCUMULATOR           0x20
#define GEN_ARF_FLAG                  0x30
#define GEN_ARF_MASK                  0x40
#define GEN_ARF_MASK_STACK            0x50
#define GEN_ARF_MASK_STACK_DEPTH      0x60
#define GEN_ARF_STATE                 0x70
#define GEN_ARF_CONTROL               0x80
#define GEN_ARF_NOTIFICATION_COUNT    0x90
#define GEN_ARF_IP                    0xA0

#define GEN_MRF_COMPR4   (1 << 7)

#define GEN_AMASK   0
#define GEN_IMASK   1
#define GEN_LMASK   2
#define GEN_CMASK   3

#define GEN_THREAD_NORMAL     0
#define GEN_THREAD_ATOMIC     1
#define GEN_THREAD_SWITCH     2

#define GEN_VERTICAL_STRIDE_0                 0
#define GEN_VERTICAL_STRIDE_1                 1
#define GEN_VERTICAL_STRIDE_2                 2
#define GEN_VERTICAL_STRIDE_4                 3
#define GEN_VERTICAL_STRIDE_8                 4
#define GEN_VERTICAL_STRIDE_16                5
#define GEN_VERTICAL_STRIDE_32                6
#define GEN_VERTICAL_STRIDE_64                7
#define GEN_VERTICAL_STRIDE_128               8
#define GEN_VERTICAL_STRIDE_256               9
#define GEN_VERTICAL_STRIDE_ONE_DIMENSIONAL   0xF

/* Execution width */
#define GEN_WIDTH_1       0
#define GEN_WIDTH_2       1
#define GEN_WIDTH_4       2
#define GEN_WIDTH_8       3
#define GEN_WIDTH_16      4
#define GEN_WIDTH_32      5

/* Channels to enable for the untyped reads and writes */
#define GEN_UNTYPED_RED   (1 << 0)
#define GEN_UNTYPED_GREEN (1 << 1)
#define GEN_UNTYPED_BLUE  (1 << 2)
#define GEN_UNTYPED_ALPHA (1 << 3)

/* SIMD mode for untyped reads and writes */
#define GEN_UNTYPED_SIMD4x2 0
#define GEN_UNTYPED_SIMD16  1
#define GEN_UNTYPED_SIMD8   2

/* SIMD mode for byte scatters / gathers */
#define GEN_BYTE_SCATTER_SIMD8    0
#define GEN_BYTE_SCATTER_SIMD16   1

/* Data port message type */
#define GEN_UNTYPED_READ    5
#define GEN_UNTYPED_WRITE  13
#define GEN_BYTE_GATHER     4
#define GEN_BYTE_SCATTER   12
#define GEN_OBLOCK_READ     0
#define GEN_OBLOCK_WRITE    8

/* For byte scatters and gathers, the element to write */
#define GEN_BYTE_SCATTER_BYTE   0
#define GEN_BYTE_SCATTER_WORD   1
#define GEN_BYTE_SCATTER_DWORD  2

#define GEN_SAMPLER_RETURN_FORMAT_FLOAT32     0
#define GEN_SAMPLER_RETURN_FORMAT_UINT32      2
#define GEN_SAMPLER_RETURN_FORMAT_SINT32      3

#define GEN_SAMPLER_MESSAGE_SIMD8_SAMPLE              0
#define GEN_SAMPLER_MESSAGE_SIMD16_SAMPLE             0
#define GEN_SAMPLER_MESSAGE_SIMD16_SAMPLE_BIAS        0
#define GEN_SAMPLER_MESSAGE_SIMD8_KILLPIX             1
#define GEN_SAMPLER_MESSAGE_SIMD4X2_SAMPLE_LOD        1
#define GEN_SAMPLER_MESSAGE_SIMD16_SAMPLE_LOD         1
#define GEN_SAMPLER_MESSAGE_SIMD4X2_SAMPLE_GRADIENTS  2
#define GEN_SAMPLER_MESSAGE_SIMD8_SAMPLE_GRADIENTS    2
#define GEN_SAMPLER_MESSAGE_SIMD4X2_SAMPLE_COMPARE    0
#define GEN_SAMPLER_MESSAGE_SIMD16_SAMPLE_COMPARE     2
#define GEN_SAMPLER_MESSAGE_SIMD8_SAMPLE_BIAS_COMPARE 0
#define GEN_SAMPLER_MESSAGE_SIMD4X2_SAMPLE_LOD_COMPARE 1
#define GEN_SAMPLER_MESSAGE_SIMD8_SAMPLE_LOD_COMPARE  1
#define GEN_SAMPLER_MESSAGE_SIMD4X2_RESINFO           2
#define GEN_SAMPLER_MESSAGE_SIMD16_RESINFO            2
#define GEN_SAMPLER_MESSAGE_SIMD4X2_LD                3
#define GEN_SAMPLER_MESSAGE_SIMD8_LD                  3
#define GEN_SAMPLER_MESSAGE_SIMD16_LD                 3

#define GEN5_SAMPLER_MESSAGE_SAMPLE              0
#define GEN5_SAMPLER_MESSAGE_SAMPLE_BIAS         1
#define GEN5_SAMPLER_MESSAGE_SAMPLE_LOD          2
#define GEN5_SAMPLER_MESSAGE_SAMPLE_COMPARE      3
#define GEN5_SAMPLER_MESSAGE_SAMPLE_DERIVS       4
#define GEN5_SAMPLER_MESSAGE_SAMPLE_BIAS_COMPARE 5
#define GEN5_SAMPLER_MESSAGE_SAMPLE_LOD_COMPARE  6
#define GEN5_SAMPLER_MESSAGE_SAMPLE_LD           7
#define GEN5_SAMPLER_MESSAGE_SAMPLE_RESINFO      10

/* for GEN5 only */
#define GEN_SAMPLER_SIMD_MODE_SIMD4X2                   0
#define GEN_SAMPLER_SIMD_MODE_SIMD8                     1
#define GEN_SAMPLER_SIMD_MODE_SIMD16                    2
#define GEN_SAMPLER_SIMD_MODE_SIMD32_64                 3

#define GEN_MATH_FUNCTION_INV                              1
#define GEN_MATH_FUNCTION_LOG                              2
#define GEN_MATH_FUNCTION_EXP                              3
#define GEN_MATH_FUNCTION_SQRT                             4
#define GEN_MATH_FUNCTION_RSQ                              5
#define GEN_MATH_FUNCTION_SIN                              6 /* was 7 */
#define GEN_MATH_FUNCTION_COS                              7 /* was 8 */
#define GEN_MATH_FUNCTION_SINCOS                           8 /* was 6 */
#define GEN_MATH_FUNCTION_TAN                              9 /* gen4 */
#define GEN_MATH_FUNCTION_FDIV                             9 /* gen6+ */
#define GEN_MATH_FUNCTION_POW                              10
#define GEN_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER   11
#define GEN_MATH_FUNCTION_INT_DIV_QUOTIENT                 12
#define GEN_MATH_FUNCTION_INT_DIV_REMAINDER                13

#define GEN_MATH_INTEGER_UNSIGNED     0
#define GEN_MATH_INTEGER_SIGNED       1

#define GEN_MATH_PRECISION_FULL        0
#define GEN_MATH_PRECISION_PARTIAL     1

#define GEN_MATH_SATURATE_NONE         0
#define GEN_MATH_SATURATE_SATURATE     1

#define GEN_MATH_DATA_VECTOR  0
#define GEN_MATH_DATA_SCALAR  1

#define GEN_DEREFERENCE_URB 0
#define GEN_DO_NOT_DEREFERENCE_URB 1

#define GEN_MAX_NUM_BUFFER_ENTRIES (1 << 27)

/////////////////////////////////////////////////////////////////////////////
// Gen EU structures
/////////////////////////////////////////////////////////////////////////////

/** Number of general purpose registers (VS, WM, etc) */
#define GEN_MAX_GRF 128

/* Instruction format for the execution units */
struct GenInstruction
{
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
      uint32_t pad:1;
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
      uint32_t pad:1;
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
      uint32_t pad:1;
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

#endif /* __GEN_DEFS_HPP__ */


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

#ifndef BRW_DEFINES_H
#define BRW_DEFINES_H

#include <cstdint>

/////////////////////////////////////////////////////////////////////////////
// Gen EU defines
/////////////////////////////////////////////////////////////////////////////

#define INTEL_MASK(high, low) (((1<<((high)-(low)+1))-1)<<(low))
#define SET_FIELD(value, field) (((value) << field ## _SHIFT) & field ## _MASK)
#define GET_FIELD(word, field) (((word)  & field ## _MASK) >> field ## _SHIFT)

/* Execution Unit (EU) defines */
#define BRW_ALIGN_1   0
#define BRW_ALIGN_16  1

#define BRW_ADDRESS_DIRECT                        0
#define BRW_ADDRESS_REGISTER_INDIRECT_REGISTER    1

#define BRW_CHANNEL_X     0
#define BRW_CHANNEL_Y     1
#define BRW_CHANNEL_Z     2
#define BRW_CHANNEL_W     3

enum brw_compression {
   BRW_COMPRESSION_NONE       = 0,
   BRW_COMPRESSION_2NDHALF    = 1,
   BRW_COMPRESSION_COMPRESSED = 2,
};

#define GEN6_COMPRESSION_1Q  0
#define GEN6_COMPRESSION_2Q  1
#define GEN6_COMPRESSION_3Q  2
#define GEN6_COMPRESSION_4Q  3
#define GEN6_COMPRESSION_1H  0
#define GEN6_COMPRESSION_2H  2

#define BRW_CONDITIONAL_NONE  0
#define BRW_CONDITIONAL_Z     1
#define BRW_CONDITIONAL_NZ    2
#define BRW_CONDITIONAL_EQ    1 /* Z */
#define BRW_CONDITIONAL_NEQ   2 /* NZ */
#define BRW_CONDITIONAL_G     3
#define BRW_CONDITIONAL_GE    4
#define BRW_CONDITIONAL_L     5
#define BRW_CONDITIONAL_LE    6
#define BRW_CONDITIONAL_R     7
#define BRW_CONDITIONAL_O     8
#define BRW_CONDITIONAL_U     9

#define BRW_DEBUG_NONE        0
#define BRW_DEBUG_BREAKPOINT  1

#define BRW_DEPENDENCY_NORMAL         0
#define BRW_DEPENDENCY_NOTCLEARED     1
#define BRW_DEPENDENCY_NOTCHECKED     2
#define BRW_DEPENDENCY_DISABLE        3

#define BRW_EXECUTE_1     0
#define BRW_EXECUTE_2     1
#define BRW_EXECUTE_4     2
#define BRW_EXECUTE_8     3
#define BRW_EXECUTE_16    4
#define BRW_EXECUTE_32    5

#define BRW_HORIZONTAL_STRIDE_0   0
#define BRW_HORIZONTAL_STRIDE_1   1
#define BRW_HORIZONTAL_STRIDE_2   2
#define BRW_HORIZONTAL_STRIDE_4   3

#define BRW_INSTRUCTION_NORMAL    0
#define BRW_INSTRUCTION_SATURATE  1

#define BRW_MASK_ENABLE   0
#define BRW_MASK_DISABLE  1

/** @{
 *
 * Gen6 has replaced "mask enable/disable" with WECtrl, which is
 * effectively the same but much simpler to think about.  Now, there
 * are two contributors ANDed together to whether channels are
 * executed: The predication on the instruction, and the channel write
 * enable.
 */
/**
 * This is the default value.  It means that a channel's write enable is set
 * if the per-channel IP is pointing at this instruction.
 */
#define BRW_WE_NORMAL  0
/**
 * This is used like BRW_MASK_DISABLE, and causes all channels to have
 * their write enable set.  Note that predication still contributes to
 * whether the channel actually gets written.
 */
#define BRW_WE_ALL  1
/** @} */

enum opcode {
   /* These are the actual hardware opcodes. */
   BRW_OPCODE_MOV = 1,
   BRW_OPCODE_SEL = 2,
   BRW_OPCODE_NOT = 4,
   BRW_OPCODE_AND = 5,
   BRW_OPCODE_OR = 6,
   BRW_OPCODE_XOR = 7,
   BRW_OPCODE_SHR = 8,
   BRW_OPCODE_SHL = 9,
   BRW_OPCODE_RSR = 10,
   BRW_OPCODE_RSL = 11,
   BRW_OPCODE_ASR = 12,
   BRW_OPCODE_CMP = 16,
   BRW_OPCODE_CMPN = 17,
   BRW_OPCODE_JMPI = 32,
   BRW_OPCODE_IF = 34,
   BRW_OPCODE_IFF = 35,
   BRW_OPCODE_ELSE = 36,
   BRW_OPCODE_ENDIF = 37,
   BRW_OPCODE_DO = 38,
   BRW_OPCODE_WHILE = 39,
   BRW_OPCODE_BREAK = 40,
   BRW_OPCODE_CONTINUE = 41,
   BRW_OPCODE_HALT = 42,
   BRW_OPCODE_MSAVE = 44,
   BRW_OPCODE_MRESTORE = 45,
   BRW_OPCODE_PUSH = 46,
   BRW_OPCODE_POP = 47,
   BRW_OPCODE_WAIT = 48,
   BRW_OPCODE_SEND = 49,
   BRW_OPCODE_SENDC = 50,
   BRW_OPCODE_MATH = 56,
   BRW_OPCODE_ADD = 64,
   BRW_OPCODE_MUL = 65,
   BRW_OPCODE_AVG = 66,
   BRW_OPCODE_FRC = 67,
   BRW_OPCODE_RNDU = 68,
   BRW_OPCODE_RNDD = 69,
   BRW_OPCODE_RNDE = 70,
   BRW_OPCODE_RNDZ = 71,
   BRW_OPCODE_MAC = 72,
   BRW_OPCODE_MACH = 73,
   BRW_OPCODE_LZD = 74,
   BRW_OPCODE_SAD2 = 80,
   BRW_OPCODE_SADA2 = 81,
   BRW_OPCODE_DP4 = 84,
   BRW_OPCODE_DPH = 85,
   BRW_OPCODE_DP3 = 86,
   BRW_OPCODE_DP2 = 87,
   BRW_OPCODE_DPA2 = 88,
   BRW_OPCODE_LINE = 89,
   BRW_OPCODE_PLN = 90,
   BRW_OPCODE_MAD = 91,
   BRW_OPCODE_NOP = 126,

   /* These are compiler backend opcodes that get translated into other
    * instructions.
    */
   FS_OPCODE_FB_WRITE = 128,
   SHADER_OPCODE_RCP,
   SHADER_OPCODE_RSQ,
   SHADER_OPCODE_SQRT,
   SHADER_OPCODE_EXP2,
   SHADER_OPCODE_LOG2,
   SHADER_OPCODE_POW,
   SHADER_OPCODE_INT_QUOTIENT,
   SHADER_OPCODE_INT_REMAINDER,
   SHADER_OPCODE_SIN,
   SHADER_OPCODE_COS,

   SHADER_OPCODE_TEX,
   SHADER_OPCODE_TXD,
   SHADER_OPCODE_TXF,
   SHADER_OPCODE_TXL,
   SHADER_OPCODE_TXS,
   FS_OPCODE_TXB,

   FS_OPCODE_DDX,
   FS_OPCODE_DDY,
   FS_OPCODE_PIXEL_X,
   FS_OPCODE_PIXEL_Y,
   FS_OPCODE_CINTERP,
   FS_OPCODE_LINTERP,
   FS_OPCODE_DISCARD,
   FS_OPCODE_SPILL,
   FS_OPCODE_UNSPILL,
   FS_OPCODE_PULL_CONSTANT_LOAD,

   VS_OPCODE_URB_WRITE,
   VS_OPCODE_SCRATCH_READ,
   VS_OPCODE_SCRATCH_WRITE,
   VS_OPCODE_PULL_CONSTANT_LOAD,
};

#define BRW_PREDICATE_NONE                    0
#define BRW_PREDICATE_NORMAL                  1
#define BRW_PREDICATE_ALIGN1_ANYV             2
#define BRW_PREDICATE_ALIGN1_ALLV             3
#define BRW_PREDICATE_ALIGN1_ANY2H            4
#define BRW_PREDICATE_ALIGN1_ALL2H            5
#define BRW_PREDICATE_ALIGN1_ANY4H            6
#define BRW_PREDICATE_ALIGN1_ALL4H            7
#define BRW_PREDICATE_ALIGN1_ANY8H            8
#define BRW_PREDICATE_ALIGN1_ALL8H            9
#define BRW_PREDICATE_ALIGN1_ANY16H           10
#define BRW_PREDICATE_ALIGN1_ALL16H           11
#define BRW_PREDICATE_ALIGN16_REPLICATE_X     2
#define BRW_PREDICATE_ALIGN16_REPLICATE_Y     3
#define BRW_PREDICATE_ALIGN16_REPLICATE_Z     4
#define BRW_PREDICATE_ALIGN16_REPLICATE_W     5
#define BRW_PREDICATE_ALIGN16_ANY4H           6
#define BRW_PREDICATE_ALIGN16_ALL4H           7

#define BRW_ARCHITECTURE_REGISTER_FILE        0
#define BRW_GENERAL_REGISTER_FILE             1
#define BRW_IMMEDIATE_VALUE                   3

#define BRW_REGISTER_TYPE_UD  0
#define BRW_REGISTER_TYPE_D   1
#define BRW_REGISTER_TYPE_UW  2
#define BRW_REGISTER_TYPE_W   3
#define BRW_REGISTER_TYPE_UB  4
#define BRW_REGISTER_TYPE_B   5
#define BRW_REGISTER_TYPE_VF  5 /* packed float vector, immediates only? */
#define BRW_REGISTER_TYPE_HF  6
#define BRW_REGISTER_TYPE_V   6 /* packed int vector, immediates only, uword dest only */
#define BRW_REGISTER_TYPE_F   7

#define BRW_ARF_NULL                  0x00
#define BRW_ARF_ADDRESS               0x10
#define BRW_ARF_ACCUMULATOR           0x20
#define BRW_ARF_FLAG                  0x30
#define BRW_ARF_MASK                  0x40
#define BRW_ARF_MASK_STACK            0x50
#define BRW_ARF_MASK_STACK_DEPTH      0x60
#define BRW_ARF_STATE                 0x70
#define BRW_ARF_CONTROL               0x80
#define BRW_ARF_NOTIFICATION_COUNT    0x90
#define BRW_ARF_IP                    0xA0

#define BRW_MRF_COMPR4   (1 << 7)

#define BRW_AMASK   0
#define BRW_IMASK   1
#define BRW_LMASK   2
#define BRW_CMASK   3



#define BRW_THREAD_NORMAL     0
#define BRW_THREAD_ATOMIC     1
#define BRW_THREAD_SWITCH     2

#define BRW_VERTICAL_STRIDE_0                 0
#define BRW_VERTICAL_STRIDE_1                 1
#define BRW_VERTICAL_STRIDE_2                 2
#define BRW_VERTICAL_STRIDE_4                 3
#define BRW_VERTICAL_STRIDE_8                 4
#define BRW_VERTICAL_STRIDE_16                5
#define BRW_VERTICAL_STRIDE_32                6
#define BRW_VERTICAL_STRIDE_64                7
#define BRW_VERTICAL_STRIDE_128               8
#define BRW_VERTICAL_STRIDE_256               9
#define BRW_VERTICAL_STRIDE_ONE_DIMENSIONAL   0xF

#define BRW_WIDTH_1       0
#define BRW_WIDTH_2       1
#define BRW_WIDTH_4       2
#define BRW_WIDTH_8       3
#define BRW_WIDTH_16      4

#define BRW_STATELESS_BUFFER_BOUNDARY_1K      0
#define BRW_STATELESS_BUFFER_BOUNDARY_2K      1
#define BRW_STATELESS_BUFFER_BOUNDARY_4K      2
#define BRW_STATELESS_BUFFER_BOUNDARY_8K      3
#define BRW_STATELESS_BUFFER_BOUNDARY_16K     4
#define BRW_STATELESS_BUFFER_BOUNDARY_32K     5
#define BRW_STATELESS_BUFFER_BOUNDARY_64K     6
#define BRW_STATELESS_BUFFER_BOUNDARY_128K    7
#define BRW_STATELESS_BUFFER_BOUNDARY_256K    8
#define BRW_STATELESS_BUFFER_BOUNDARY_512K    9
#define BRW_STATELESS_BUFFER_BOUNDARY_1M      10
#define BRW_STATELESS_BUFFER_BOUNDARY_2M      11

#define BRW_POLYGON_FACING_FRONT      0
#define BRW_POLYGON_FACING_BACK       1

/**
 * Message target: Shared Function ID for where to SEND a message.
 *
 * These are enumerated in the ISA reference under "send - Send Message".
 * In particular, see the following tables:
 * - G45 PRM, Volume 4, Table 14-15 "Message Descriptor Definition"
 * - Sandybridge PRM, Volume 4 Part 2, Table 8-16 "Extended Message Descriptor"
 * - BSpec, Volume 1a (GPU Overview) / Graphics Processing Engine (GPE) /
 *   Overview / GPE Function IDs
 */
enum brw_message_target {
   BRW_SFID_NULL                     = 0,
   BRW_SFID_MATH                     = 1, /* Only valid on Gen4-5 */
   BRW_SFID_SAMPLER                  = 2,
   BRW_SFID_MESSAGE_GATEWAY          = 3,
   BRW_SFID_DATAPORT_READ            = 4,
   BRW_SFID_DATAPORT_WRITE           = 5,
   BRW_SFID_URB                      = 6,
   BRW_SFID_THREAD_SPAWNER           = 7,
   GEN6_SFID_DATAPORT_SAMPLER_CACHE  = 4,
   GEN6_SFID_DATAPORT_RENDER_CACHE   = 5,
   GEN6_SFID_DATAPORT_CONSTANT_CACHE = 9,
   GEN7_SFID_DATAPORT_DATA_CACHE     = 10,
};

#define GEN7_MESSAGE_TARGET_DP_DATA_CACHE     10

#define BRW_SAMPLER_RETURN_FORMAT_FLOAT32     0
#define BRW_SAMPLER_RETURN_FORMAT_UINT32      2
#define BRW_SAMPLER_RETURN_FORMAT_SINT32      3

#define BRW_SAMPLER_MESSAGE_SIMD8_SAMPLE              0
#define BRW_SAMPLER_MESSAGE_SIMD16_SAMPLE             0
#define BRW_SAMPLER_MESSAGE_SIMD16_SAMPLE_BIAS        0
#define BRW_SAMPLER_MESSAGE_SIMD8_KILLPIX             1
#define BRW_SAMPLER_MESSAGE_SIMD4X2_SAMPLE_LOD        1
#define BRW_SAMPLER_MESSAGE_SIMD16_SAMPLE_LOD         1
#define BRW_SAMPLER_MESSAGE_SIMD4X2_SAMPLE_GRADIENTS  2
#define BRW_SAMPLER_MESSAGE_SIMD8_SAMPLE_GRADIENTS    2
#define BRW_SAMPLER_MESSAGE_SIMD4X2_SAMPLE_COMPARE    0
#define BRW_SAMPLER_MESSAGE_SIMD16_SAMPLE_COMPARE     2
#define BRW_SAMPLER_MESSAGE_SIMD8_SAMPLE_BIAS_COMPARE 0
#define BRW_SAMPLER_MESSAGE_SIMD4X2_SAMPLE_LOD_COMPARE 1
#define BRW_SAMPLER_MESSAGE_SIMD8_SAMPLE_LOD_COMPARE  1
#define BRW_SAMPLER_MESSAGE_SIMD4X2_RESINFO           2
#define BRW_SAMPLER_MESSAGE_SIMD16_RESINFO            2
#define BRW_SAMPLER_MESSAGE_SIMD4X2_LD                3
#define BRW_SAMPLER_MESSAGE_SIMD8_LD                  3
#define BRW_SAMPLER_MESSAGE_SIMD16_LD                 3

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
#define BRW_SAMPLER_SIMD_MODE_SIMD4X2                   0
#define BRW_SAMPLER_SIMD_MODE_SIMD8                     1
#define BRW_SAMPLER_SIMD_MODE_SIMD16                    2
#define BRW_SAMPLER_SIMD_MODE_SIMD32_64                 3

#define BRW_DATAPORT_OWORD_BLOCK_1_OWORDLOW   0
#define BRW_DATAPORT_OWORD_BLOCK_1_OWORDHIGH  1
#define BRW_DATAPORT_OWORD_BLOCK_2_OWORDS     2
#define BRW_DATAPORT_OWORD_BLOCK_4_OWORDS     3
#define BRW_DATAPORT_OWORD_BLOCK_8_OWORDS     4

#define BRW_DATAPORT_OWORD_DUAL_BLOCK_1OWORD     0
#define BRW_DATAPORT_OWORD_DUAL_BLOCK_4OWORDS    2

#define BRW_DATAPORT_DWORD_SCATTERED_BLOCK_8DWORDS   2
#define BRW_DATAPORT_DWORD_SCATTERED_BLOCK_16DWORDS  3

/* This one stays the same across generations. */
#define BRW_DATAPORT_READ_MESSAGE_OWORD_BLOCK_READ          0
/* GEN4 */
#define BRW_DATAPORT_READ_MESSAGE_OWORD_DUAL_BLOCK_READ     1
#define BRW_DATAPORT_READ_MESSAGE_MEDIA_BLOCK_READ          2
#define BRW_DATAPORT_READ_MESSAGE_DWORD_SCATTERED_READ      3
/* G45, GEN5 */
#define G45_DATAPORT_READ_MESSAGE_RENDER_UNORM_READ     1
#define G45_DATAPORT_READ_MESSAGE_OWORD_DUAL_BLOCK_READ     2
#define G45_DATAPORT_READ_MESSAGE_AVC_LOOP_FILTER_READ     3
#define G45_DATAPORT_READ_MESSAGE_MEDIA_BLOCK_READ          4
#define G45_DATAPORT_READ_MESSAGE_DWORD_SCATTERED_READ      6
/* GEN6 */
#define GEN6_DATAPORT_READ_MESSAGE_RENDER_UNORM_READ     1
#define GEN6_DATAPORT_READ_MESSAGE_OWORD_DUAL_BLOCK_READ     2
#define GEN6_DATAPORT_READ_MESSAGE_MEDIA_BLOCK_READ          4
#define GEN6_DATAPORT_READ_MESSAGE_OWORD_UNALIGN_BLOCK_READ  5
#define GEN6_DATAPORT_READ_MESSAGE_DWORD_SCATTERED_READ      6

#define BRW_DATAPORT_READ_TARGET_DATA_CACHE      0
#define BRW_DATAPORT_READ_TARGET_RENDER_CACHE    1
#define BRW_DATAPORT_READ_TARGET_SAMPLER_CACHE   2

#define BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD16_SINGLE_SOURCE                0
#define BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD16_SINGLE_SOURCE_REPLICATED     1
#define BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD8_DUAL_SOURCE_SUBSPAN01         2
#define BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD8_DUAL_SOURCE_SUBSPAN23         3
#define BRW_DATAPORT_RENDER_TARGET_WRITE_SIMD8_SINGLE_SOURCE_SUBSPAN01       4

#define BRW_DATAPORT_WRITE_MESSAGE_OWORD_BLOCK_WRITE                0
#define BRW_DATAPORT_WRITE_MESSAGE_OWORD_DUAL_BLOCK_WRITE           1
#define BRW_DATAPORT_WRITE_MESSAGE_MEDIA_BLOCK_WRITE                2
#define BRW_DATAPORT_WRITE_MESSAGE_DWORD_SCATTERED_WRITE            3
#define BRW_DATAPORT_WRITE_MESSAGE_RENDER_TARGET_WRITE              4
#define BRW_DATAPORT_WRITE_MESSAGE_STREAMED_VERTEX_BUFFER_WRITE     5
#define BRW_DATAPORT_WRITE_MESSAGE_FLUSH_RENDER_CACHE               7

/* GEN6 */
#define GEN6_DATAPORT_WRITE_MESSAGE_DWORD_ATOMIC_WRITE              7
#define GEN6_DATAPORT_WRITE_MESSAGE_OWORD_BLOCK_WRITE               8
#define GEN6_DATAPORT_WRITE_MESSAGE_OWORD_DUAL_BLOCK_WRITE          9
#define GEN6_DATAPORT_WRITE_MESSAGE_MEDIA_BLOCK_WRITE               10
#define GEN6_DATAPORT_WRITE_MESSAGE_DWORD_SCATTERED_WRITE           11
#define GEN6_DATAPORT_WRITE_MESSAGE_RENDER_TARGET_WRITE             12
#define GEN6_DATAPORT_WRITE_MESSAGE_STREAMED_VB_WRITE               13
#define GEN6_DATAPORT_WRITE_MESSAGE_RENDER_TARGET_UNORM_WRITE       14

/* GEN7 */
#define GEN7_DATAPORT_WRITE_MESSAGE_OWORD_DUAL_BLOCK_WRITE          10

#define BRW_MATH_FUNCTION_INV                              1
#define BRW_MATH_FUNCTION_LOG                              2
#define BRW_MATH_FUNCTION_EXP                              3
#define BRW_MATH_FUNCTION_SQRT                             4
#define BRW_MATH_FUNCTION_RSQ                              5
#define BRW_MATH_FUNCTION_SIN                              6 /* was 7 */
#define BRW_MATH_FUNCTION_COS                              7 /* was 8 */
#define BRW_MATH_FUNCTION_SINCOS                           8 /* was 6 */
#define BRW_MATH_FUNCTION_TAN                              9 /* gen4 */
#define BRW_MATH_FUNCTION_FDIV                             9 /* gen6+ */
#define BRW_MATH_FUNCTION_POW                              10
#define BRW_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER   11
#define BRW_MATH_FUNCTION_INT_DIV_QUOTIENT                 12
#define BRW_MATH_FUNCTION_INT_DIV_REMAINDER                13

#define BRW_MATH_INTEGER_UNSIGNED     0
#define BRW_MATH_INTEGER_SIGNED       1

#define BRW_MATH_PRECISION_FULL        0
#define BRW_MATH_PRECISION_PARTIAL     1

#define BRW_MATH_SATURATE_NONE         0
#define BRW_MATH_SATURATE_SATURATE     1

#define BRW_MATH_DATA_VECTOR  0
#define BRW_MATH_DATA_SCALAR  1

#define BRW_URB_OPCODE_WRITE  0

#define BRW_URB_SWIZZLE_NONE          0
#define BRW_URB_SWIZZLE_INTERLEAVE    1
#define BRW_URB_SWIZZLE_TRANSPOSE     2

#define CMD_URB_FENCE                 0x6000
#define CMD_CS_URB_STATE              0x6001
#define CMD_CONST_BUFFER              0x6002

#define CMD_STATE_BASE_ADDRESS        0x6101
#define CMD_STATE_SIP                 0x6102
#define CMD_PIPELINE_SELECT_965       0x6104
#define CMD_PIPELINE_SELECT_GM45      0x6904

/* DW2 */
# define GEN6_VS_SPF_MODE    (1 << 31)
# define GEN6_VS_VECTOR_MASK_ENABLE   (1 << 30)
# define GEN6_VS_SAMPLER_COUNT_SHIFT   27
# define GEN6_VS_BINDING_TABLE_ENTRY_COUNT_SHIFT 18
# define GEN6_VS_FLOATING_POINT_MODE_IEEE_754  (0 << 16)
# define GEN6_VS_FLOATING_POINT_MODE_ALT  (1 << 16)
/* DW4 */
# define GEN6_VS_DISPATCH_START_GRF_SHIFT  20
# define GEN6_VS_URB_READ_LENGTH_SHIFT   11
# define GEN6_VS_URB_ENTRY_READ_OFFSET_SHIFT  4
/* DW5 */
# define GEN6_VS_MAX_THREADS_SHIFT   25
# define GEN6_VS_STATISTICS_ENABLE   (1 << 10)
# define GEN6_VS_CACHE_DISABLE    (1 << 1)
# define GEN6_VS_ENABLE     (1 << 0)

#define BRW_DEREFERENCE_URB 0
#define BRW_DO_NOT_DEREFERENCE_URB 1

/* Maximum number of entries that can be addressed using a binding table
 * pointer of type SURFTYPE_BUFFER
 */
#define BRW_MAX_NUM_BUFFER_ENTRIES (1 << 27)

/////////////////////////////////////////////////////////////////////////////
// Gen EU structures
/////////////////////////////////////////////////////////////////////////////

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

#endif /* BRW_DEFINES_H */


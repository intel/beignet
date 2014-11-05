/*
 * Copyright Â© 2012 Intel Corporation
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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

/*
 * Copyright © 2008 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdint.h>
#include <assert.h>

#include "backend/gen_defs.hpp"
#include "backend/gen7_instruction.hpp"
#include "src/cl_device_data.h"

static const struct {
  const char    *name;
  int	    nsrc;
  int	    ndst;
} opcode[128] = {
  [GEN_OPCODE_MOV] = { .name = "mov", .nsrc = 1, .ndst = 1 },
  [GEN_OPCODE_FRC] = { .name = "frc", .nsrc = 1, .ndst = 1 },
  [GEN_OPCODE_RNDU] = { .name = "rndu", .nsrc = 1, .ndst = 1 },
  [GEN_OPCODE_RNDD] = { .name = "rndd", .nsrc = 1, .ndst = 1 },
  [GEN_OPCODE_RNDE] = { .name = "rnde", .nsrc = 1, .ndst = 1 },
  [GEN_OPCODE_RNDZ] = { .name = "rndz", .nsrc = 1, .ndst = 1 },
  [GEN_OPCODE_NOT] = { .name = "not", .nsrc = 1, .ndst = 1 },
  [GEN_OPCODE_LZD] = { .name = "lzd", .nsrc = 1, .ndst = 1 },
  [GEN_OPCODE_FBH] = { .name = "fbh", .nsrc = 1, .ndst = 1 },
  [GEN_OPCODE_FBL] = { .name = "fbl", .nsrc = 1, .ndst = 1 },
  [GEN_OPCODE_F16TO32] = { .name = "f16to32", .nsrc = 1, .ndst = 1 },
  [GEN_OPCODE_F32TO16] = { .name = "f32to16", .nsrc = 1, .ndst = 1 },

  [GEN_OPCODE_MUL] = { .name = "mul", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_MAC] = { .name = "mac", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_MACH] = { .name = "mach", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_LINE] = { .name = "line", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_PLN] = { .name = "pln", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_MAD] = { .name = "mad", .nsrc = 3, .ndst = 1 },
  [GEN_OPCODE_SAD2] = { .name = "sad2", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_SADA2] = { .name = "sada2", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_DP4] = { .name = "dp4", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_DPH] = { .name = "dph", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_DP3] = { .name = "dp3", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_DP2] = { .name = "dp2", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_MATH] = { .name = "math", .nsrc = 2, .ndst = 1 },

  [GEN_OPCODE_AVG] = { .name = "avg", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_ADD] = { .name = "add", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_ADDC] = { .name = "addc", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_SUBB] = { .name = "subb", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_SEL] = { .name = "sel", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_AND] = { .name = "and", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_OR] = { .name = "or", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_XOR] = { .name = "xor", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_SHR] = { .name = "shr", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_SHL] = { .name = "shl", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_ASR] = { .name = "asr", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_CMP] = { .name = "cmp", .nsrc = 2, .ndst = 1 },
  [GEN_OPCODE_CMPN] = { .name = "cmpn", .nsrc = 2, .ndst = 1 },

  [GEN_OPCODE_SEND] = { .name = "send", .nsrc = 1, .ndst = 1 },
  [GEN_OPCODE_SENDC] = { .name = "sendc", .nsrc = 1, .ndst = 1 },
  [GEN_OPCODE_NOP] = { .name = "nop", .nsrc = 0, .ndst = 0 },
  [GEN_OPCODE_JMPI] = { .name = "jmpi", .nsrc = 0, .ndst = 0 },
  [GEN_OPCODE_BRD] = { .name = "brd", .nsrc = 0, .ndst = 0 },
  [GEN_OPCODE_IF] = { .name = "if", .nsrc = 0, .ndst = 0 },
  [GEN_OPCODE_BRC] = { .name = "brc", .nsrc = 0, .ndst = 0 },
  [GEN_OPCODE_WHILE] = { .name = "while", .nsrc = 0, .ndst = 0 },
  [GEN_OPCODE_ELSE] = { .name = "else", .nsrc = 0, .ndst = 0 },
  [GEN_OPCODE_BREAK] = { .name = "break", .nsrc = 0, .ndst = 0 },
  [GEN_OPCODE_CONTINUE] = { .name = "cont", .nsrc = 0, .ndst = 0 },
  [GEN_OPCODE_HALT] = { .name = "halt", .nsrc = 1, .ndst = 0 },
  [GEN_OPCODE_MSAVE] = { .name = "msave", .nsrc = 1, .ndst = 1 },
  [GEN_OPCODE_PUSH] = { .name = "push", .nsrc = 1, .ndst = 1 },
  [GEN_OPCODE_MRESTORE] = { .name = "mrest", .nsrc = 1, .ndst = 1 },
  [GEN_OPCODE_POP] = { .name = "pop", .nsrc = 2, .ndst = 0 },
  [GEN_OPCODE_WAIT] = { .name = "wait", .nsrc = 1, .ndst = 0 },
  [GEN_OPCODE_DO] = { .name = "do", .nsrc = 0, .ndst = 0 },
  [GEN_OPCODE_ENDIF] = { .name = "endif", .nsrc = 1, .ndst = 0 },
};

static const char *conditional_modifier[16] = {
  [GEN_CONDITIONAL_NONE] = "",
  [GEN_CONDITIONAL_Z] = ".e",
  [GEN_CONDITIONAL_NZ] = ".ne",
  [GEN_CONDITIONAL_G] = ".g",
  [GEN_CONDITIONAL_GE] = ".ge",
  [GEN_CONDITIONAL_L] = ".l",
  [GEN_CONDITIONAL_LE] = ".le",
  [GEN_CONDITIONAL_R] = ".r",
  [GEN_CONDITIONAL_O] = ".o",
  [GEN_CONDITIONAL_U] = ".u",
};

static const char *negate[2] = {
  [0] = "",
  [1] = "-",
};

static const char *_abs[2] = {
  [0] = "",
  [1] = "(abs)",
};

static const char *vert_stride_gen7[16] = {
  [0] = "0",
  [1] = "1",
  [2] = "2",
  [3] = "4",
  [4] = "8",
  [5] = "16",
  [6] = "32",
  [15] = "VxH",
};
static const char *vert_stride_gen8[16] = {
  [0] = "0",
  [1] = "1",
  [2] = "2",
  [3] = "4",
  [4] = "8",
  [5] = "16",
  [6] = "32",
};

static const char *width[8] = {
  [0] = "1",
  [1] = "2",
  [2] = "4",
  [3] = "8",
  [4] = "16",
};

static const char *horiz_stride[4] = {
  [0] = "0",
  [1] = "1",
  [2] = "2",
  [3] = "4"
};

static const char *chan_sel[4] = {
  [0] = "x",
  [1] = "y",
  [2] = "z",
  [3] = "w",
};

static const char *debug_ctrl[2] = {
  [0] = "",
  [1] = ".breakpoint"
};

static const char *saturate[2] = {
  [0] = "",
  [1] = ".sat"
};

static const char *accwr[2] = {
  [0] = "",
  [1] = "AccWrEnable"
};

static const char *wectrl[2] = {
  [0] = "WE_normal",
  [1] = "WE_all"
};

static const char *exec_size[8] = {
  [0] = "1",
  [1] = "2",
  [2] = "4",
  [3] = "8",
  [4] = "16",
  [5] = "32"
};

static const char *pred_inv[2] = {
  [0] = "+",
  [1] = "-"
};

static const char *pred_ctrl_align16[16] = {
  [1] = "",
  [2] = ".x",
  [3] = ".y",
  [4] = ".z",
  [5] = ".w",
  [6] = ".any4h",
  [7] = ".all4h",
};

static const char *pred_ctrl_align1[16] = {
  [1] = "",
  [2] = ".anyv",
  [3] = ".allv",
  [4] = ".any2h",
  [5] = ".all2h",
  [6] = ".any4h",
  [7] = ".all4h",
  [8] = ".any8h",
  [9] = ".all8h",
  [10] = ".any16h",
  [11] = ".all16h",
};

static const char *thread_ctrl_gen7[4] = {
  [0] = "",
  [2] = "switch"
};
static const char *thread_ctrl_gen8[4] = {
  [0] = "",
  [1] = "atomic",
  [2] = "switch"
};


static const char *dep_ctrl[4] = {
  [0] = "",
  [1] = "NoDDClr",
  [2] = "NoDDChk",
  [3] = "NoDDClr,NoDDChk",
};

static const char *access_mode[2] = {
  [0] = "align1",
  [1] = "align16",
};

static const char *reg_encoding[8] = {
  [0] = ":UD",
  [1] = ":D",
  [2] = ":UW",
  [3] = ":W",
  [4] = ":UB",
  [5] = ":B",
  [6] = ":DF",
  [7] = ":F"
};

int reg_type_size[8] = {
  [0] = 4,
  [1] = 4,
  [2] = 2,
  [3] = 2,
  [4] = 1,
  [5] = 1,
  [6] = 8,
  [7] = 4
};

static const char *reg_file[4] = {
  [0] = "A",
  [1] = "g",
  [2] = "m",
  [3] = "imm",
};

static const char *writemask[16] = {
  [0x0] = ".",
  [0x1] = ".x",
  [0x2] = ".y",
  [0x3] = ".xy",
  [0x4] = ".z",
  [0x5] = ".xz",
  [0x6] = ".yz",
  [0x7] = ".xyz",
  [0x8] = ".w",
  [0x9] = ".xw",
  [0xa] = ".yw",
  [0xb] = ".xyw",
  [0xc] = ".zw",
  [0xd] = ".xzw",
  [0xe] = ".yzw",
  [0xf] = "",
};

static const char *end_of_thread[2] = {
  [0] = "",
  [1] = "EOT"
};

static const char *target_function_gen7[16] = {
  [GEN_SFID_NULL] = "null",
  [GEN_SFID_RESERVED] = NULL,
  [GEN_SFID_SAMPLER] = "sampler",
  [GEN_SFID_MESSAGE_GATEWAY] = "gateway",
  [GEN_SFID_DATAPORT_SAMPLER] = "dataport_sampler",
  [GEN_SFID_DATAPORT_RENDER] = "render",
  [GEN_SFID_URB] = "urb",
  [GEN_SFID_THREAD_SPAWNER] = "thread_spawner",
  [GEN_SFID_VIDEO_MOTION_EST] = "video_motion_estimation",
  [GEN_SFID_DATAPORT_CONSTANT] = "const",
  [GEN_SFID_DATAPORT_DATA] = "data",
  [GEN_SFID_PIXEL_INTERPOLATOR] = "pix_interpolator",
};

static const char *target_function_gen75[16] = {
  [GEN_SFID_NULL] = "null",
  [GEN_SFID_RESERVED] = NULL,
  [GEN_SFID_SAMPLER] = "sampler",
  [GEN_SFID_MESSAGE_GATEWAY] = "gateway",
  [GEN_SFID_DATAPORT_SAMPLER] = "dataport_sampler",
  [GEN_SFID_DATAPORT_RENDER] = "render",
  [GEN_SFID_URB] = "urb",
  [GEN_SFID_THREAD_SPAWNER] = "thread_spawner",
  [GEN_SFID_VIDEO_MOTION_EST] = "video_motion_estimation",
  [GEN_SFID_DATAPORT_CONSTANT] = "const",
  [GEN_SFID_DATAPORT_DATA] = "data (0)",
  [GEN_SFID_PIXEL_INTERPOLATOR] = "pix_interpolator",
  [GEN_SFID_DATAPORT1_DATA] = "data (1)",
};

static const char *gateway_sub_function[8] = {
  [0] = "open gateway",
  [1] = "close gateway",
  [2] = "forward gateway",
  [3] = "get time stamp",
  [4] = "barrier",
  [5] = "update gateway state",
  [6] = "MMIO R/W",
  [7] = "reserved"
};

static const char *math_function_gen7[16] = {
  [GEN_MATH_FUNCTION_INV] = "inv",
  [GEN_MATH_FUNCTION_LOG] = "log",
  [GEN_MATH_FUNCTION_EXP] = "exp",
  [GEN_MATH_FUNCTION_SQRT] = "sqrt",
  [GEN_MATH_FUNCTION_RSQ] = "rsq",
  [GEN_MATH_FUNCTION_SIN] = "sin",
  [GEN_MATH_FUNCTION_COS] = "cos",
  [GEN_MATH_FUNCTION_FDIV] = "fdiv",
  [GEN_MATH_FUNCTION_POW] = "pow",
  [GEN_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER] = "intdivmod",
  [GEN_MATH_FUNCTION_INT_DIV_QUOTIENT] = "intdiv",
  [GEN_MATH_FUNCTION_INT_DIV_REMAINDER] = "intmod",
};
static const char *math_function_gen8[16] = {
  [GEN_MATH_FUNCTION_INV] = "inv",
  [GEN_MATH_FUNCTION_LOG] = "log",
  [GEN_MATH_FUNCTION_EXP] = "exp",
  [GEN_MATH_FUNCTION_SQRT] = "sqrt",
  [GEN_MATH_FUNCTION_RSQ] = "rsq",
  [GEN_MATH_FUNCTION_SIN] = "sin",
  [GEN_MATH_FUNCTION_COS] = "cos",
  [GEN_MATH_FUNCTION_FDIV] = "fdiv",
  [GEN_MATH_FUNCTION_POW] = "pow",
  [GEN_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER] = "intdivmod",
  [GEN8_MATH_FUNCTION_INVM] = "invm",
  [GEN8_MATH_FUNCTION_RSQRTM] = "rsqrtm",
};

static const char *data_port_data_cache_simd_mode[] = {
  "SIMD4x2",
  "SIMD16",
  "SIMD8",
};

static const char *data_port_data_cache_category[] = {
  "legacy",
  "scratch",
};

static const char *data_port_scratch_block_size[] = {
  "1 register",
  "2 registers",
  "Reserve",
  "4 registers",
};

static const char *data_port_scratch_invalidate[] = {
  "no invalidate",
  "invalidate cache line",
};

static const char *data_port_scratch_channel_mode[] = {
  "Oword",
  "Dword",
};

static const char *data_port_scratch_msg_type[] = {
  "Scratch Read",
  "Scratch Write",
};

static const char *data_port_data_cache_msg_type[] = {
  [0] = "OWord Block Read",
  [1] = "Unaligned OWord Block Read",
  [2] = "OWord Dual Block Read",
  [3] = "DWord Scattered Read",
  [4] = "Byte Scattered Read",
  [5] = "Untyped Surface Read",
  [6] = "Untyped Atomic Operation",
  [7] = "Memory Fence",
  [8] = "OWord Block Write",
  [10] = "OWord Dual Block Write",
  [11] = "DWord Scattered Write",
  [12] = "Byte Scattered Write",
  [13] = "Untyped Surface Write",
};

static const char *data_port1_data_cache_msg_type[] = {
  [1] = "Untyped Surface Read",
  [2] = "Untyped Atomic Operation",
  [3] = "Untyped Atomic Operation SIMD4x2",
  [4] = "Media Block Read",
  [5] = "Typed Surface Read",
  [6] = "Typed Atomic Operation",
  [7] = "Typed Atomic Operation SIMD4x2",
  [9] = "Untyped Surface Write",
  [10] = "Media Block Write",
  [11] = "Atomic Counter Operation",
  [12] = "Atomic Counter Operation 4X2",
  [13] = "Typed Surface Write",
};

static int column;

static int gen_version;

#define GEN_BITS_FIELD(inst, gen)                               \
  ({                                                            \
    int bits;                                                   \
    if (gen_version < 80)                                       \
      bits = ((const union Gen7NativeInstruction *)inst)->gen;  \
    else                                                        \
      bits = ((const union Gen8NativeInstruction *)inst)->gen;  \
    bits;                                                       \
  })

#define GEN_BITS_FIELD2(inst, gen7, gen8)                       \
  ({                                                            \
    int bits;                                                   \
    if (gen_version < 80)                                       \
      bits = ((const union Gen7NativeInstruction *)inst)->gen7; \
    else                                                        \
      bits = ((const union Gen8NativeInstruction *)inst)->gen8; \
    bits;                                                       \
  })

#define PRED_CTRL(inst)            GEN_BITS_FIELD(inst, header.predicate_control)
#define PRED_INV(inst)             GEN_BITS_FIELD(inst, header.predicate_inverse)
#define FLAG_REG_NR(inst)          GEN_BITS_FIELD2(inst, bits2.da1.flag_reg_nr, bits1.da1.flag_reg_nr)
#define FLAG_SUB_REG_NR(inst)      GEN_BITS_FIELD2(inst, bits2.da1.flag_sub_reg_nr, bits1.da1.flag_sub_reg_nr)
#define ACCESS_MODE(inst)          GEN_BITS_FIELD(inst, header.access_mode)
#define MASK_CONTROL(inst)         GEN_BITS_FIELD2(inst, header.mask_control, bits1.da1.mask_control)
#define DEPENDENCY_CONTROL(inst)   GEN_BITS_FIELD(inst, header.dependency_control)
#define THREAD_CONTROL(inst)       GEN_BITS_FIELD(inst, header.thread_control)
#define ACC_WR_CONTROL(inst)       GEN_BITS_FIELD(inst, header.acc_wr_control)
#define QUARTER_CONTROL(inst)      GEN_BITS_FIELD(inst, header.quarter_control)
#define END_OF_THREAD(inst)        GEN_BITS_FIELD(inst, bits3.generic_gen5.end_of_thread)
#define OPCODE(inst)               GEN_BITS_FIELD(inst, header.opcode)
#define SATURATE(inst)             GEN_BITS_FIELD(inst, header.saturate)
#define DEBUG_CONTROL(inst)        GEN_BITS_FIELD(inst, header.debug_control)
#define MATH_FUNCTION(inst)        GEN_BITS_FIELD(inst, header.destreg_or_condmod)
#define MATH_SATURATE(inst)        GEN_BITS_FIELD(inst, bits3.math_gen5.saturate)
#define MATH_SIGNED(inst)          GEN_BITS_FIELD(inst, bits3.math_gen5.int_type)
#define MATH_SCALAR(inst)          GEN_BITS_FIELD(inst, bits3.math_gen5.data_type)
#define MATH_PRECISION(inst)       GEN_BITS_FIELD(inst, bits3.math_gen5.precision)
#define COND_DST_OR_MODIFIER(inst) GEN_BITS_FIELD(inst, header.destreg_or_condmod)
#define EXECUTION_SIZE(inst)       GEN_BITS_FIELD(inst, header.execution_size)
#define BRANCH_JIP(inst)           GEN_BITS_FIELD2(inst, bits3.gen7_branch.jip, bits3.gen8_branch.jip/8)
#define BRANCH_UIP(inst)           GEN_BITS_FIELD2(inst, bits3.gen7_branch.uip, bits2.gen8_branch.uip/8)
#define SAMPLE_BTI(inst)           GEN_BITS_FIELD(inst, bits3.sampler_gen7.bti)
#define SAMPLER(inst)              GEN_BITS_FIELD(inst, bits3.sampler_gen7.sampler)
#define SAMPLER_MSG_TYPE(inst)     GEN_BITS_FIELD(inst, bits3.sampler_gen7.msg_type)
#define SAMPLER_SIMD_MODE(inst)    GEN_BITS_FIELD(inst, bits3.sampler_gen7.simd_mode)
#define UNTYPED_RW_BTI(inst)       GEN_BITS_FIELD(inst, bits3.gen7_untyped_rw.bti)
#define UNTYPED_RW_RGBA(inst)      GEN_BITS_FIELD(inst, bits3.gen7_untyped_rw.rgba)
#define UNTYPED_RW_SIMD_MODE(inst) GEN_BITS_FIELD(inst, bits3.gen7_untyped_rw.simd_mode)
#define UNTYPED_RW_CATEGORY(inst)  GEN_BITS_FIELD(inst, bits3.gen7_untyped_rw.category)
#define UNTYPED_RW_MSG_TYPE(inst)  GEN_BITS_FIELD(inst, bits3.gen7_untyped_rw.msg_type)
#define SCRATCH_RW_OFFSET(inst)    GEN_BITS_FIELD(inst, bits3.gen7_scratch_rw.offset)
#define SCRATCH_RW_BLOCK_SIZE(inst) GEN_BITS_FIELD(inst, bits3.gen7_scratch_rw.block_size)
#define SCRATCH_RW_INVALIDATE_AFTER_READ(inst) GEN_BITS_FIELD(inst, bits3.gen7_scratch_rw.invalidate_after_read)
#define SCRATCH_RW_BLOCK_SIZE(inst) GEN_BITS_FIELD(inst, bits3.gen7_scratch_rw.block_size)
#define SCRATCH_RW_CHANNEL_MODE(inst) GEN_BITS_FIELD(inst, bits3.gen7_scratch_rw.channel_mode)
#define SCRATCH_RW_MSG_TYPE(inst)  GEN_BITS_FIELD(inst, bits3.gen7_scratch_rw.msg_type)
#define DWORD_RW_BTI(inst)         GEN_BITS_FIELD(inst, bits3.gen7_dword_rw.msg_type)
#define DWORD_RW_MSG_TYPE(inst)    GEN_BITS_FIELD(inst, bits3.gen7_dword_rw.bti)
#define MSG_GW_SUBFUNC(inst)       GEN_BITS_FIELD(inst, bits3.gen7_msg_gw.subfunc)
#define MSG_GW_NOTIFY(inst)        GEN_BITS_FIELD(inst, bits3.gen7_msg_gw.notify)
#define MSG_GW_ACKREQ(inst)        GEN_BITS_FIELD(inst, bits3.gen7_msg_gw.ackreq)
#define GENERIC_MSG_LENGTH(inst)   GEN_BITS_FIELD(inst, bits3.generic_gen5.msg_length)
#define GENERIC_RESPONSE_LENGTH(inst) GEN_BITS_FIELD(inst, bits3.generic_gen5.response_length)

static int string(FILE *file, const char *string)
{
  fputs (string, file);
  column += strlen (string);
  return 0;
}

static int format(FILE *f, const char *format, ...)
{
  char    buf[1024];
  va_list	args;
  va_start (args, format);

  vsnprintf (buf, sizeof (buf) - 1, format, args);
  va_end (args);
  string(f, buf);
  return 0;
}

static int newline(FILE *f)
{
  putc ('\n', f);
  column = 0;
  return 0;
}

static int pad(FILE *f, int c)
{
  do
    string(f, " ");
  while (column < c);
  return 0;
}

static int flag_reg(FILE *file, const int flag_nr, const int flag_sub_reg_nr)
{
  if (flag_nr || flag_sub_reg_nr)
    return format(file, ".f%d.%d", flag_nr, flag_sub_reg_nr);
  return 0;
}

static int control(FILE *file, const char *name, const char *ctrl[], uint32_t id, int *space)
{
  if (!ctrl[id]) {
    fprintf (file, "*** invalid %s value %d ",
             name, id);
    return 1;
  }
  if (ctrl[id][0]) {
    if (space && *space)
      string(file, " ");
    string(file, ctrl[id]);
    if (space)
      *space = 1;
  }
  return 0;
}

static int print_opcode(FILE *file, int id)
{
  if (!opcode[id].name) {
    format(file, "*** invalid opcode value %d ", id);
    return 1;
  }
  string(file, opcode[id].name);
  return 0;
}

static int reg(FILE *file, uint32_t _reg_file, uint32_t _reg_nr)
{
  int err = 0;

  if (_reg_file == GEN_ARCHITECTURE_REGISTER_FILE) {
    switch (_reg_nr & 0xf0) {
      case GEN_ARF_NULL:
        string(file, "null");
        return -1;
      case GEN_ARF_ADDRESS:
        format(file, "a%d", _reg_nr & 0x0f);
        break;
      case GEN_ARF_ACCUMULATOR:
        format(file, "acc%d", _reg_nr & 0x0f);
        break;
      case GEN_ARF_FLAG:
        format(file, "f%d", _reg_nr & 0x0f);
        break;
      case GEN_ARF_MASK:
        format(file, "mask%d", _reg_nr & 0x0f);
        break;
      case GEN_ARF_MASK_STACK:
        format(file, "msd%d", _reg_nr & 0x0f);
        break;
      case GEN_ARF_STATE:
        format(file, "sr%d", _reg_nr & 0x0f);
        break;
      case GEN_ARF_CONTROL:
        format(file, "cr%d", _reg_nr & 0x0f);
        break;
      case GEN_ARF_NOTIFICATION_COUNT:
        format(file, "n%d", _reg_nr & 0x0f);
        break;
      case GEN_ARF_IP:
        string(file, "ip");
        return -1;
        break;
      case GEN_ARF_TM:
        format(file, "tm%d", _reg_nr & 0x0f);
        break;
      default:
        format(file, "ARF%d", _reg_nr);
        break;
    }
  } else {
    err |= control(file, "src reg file", reg_file, _reg_file, NULL);
    format(file, "%d", _reg_nr);
  }
  return err;
}

static int dest(FILE *file, const void* inst)
{
  int err = 0;

  if (ACCESS_MODE(inst) == GEN_ALIGN_1) {
    if (GEN_BITS_FIELD(inst, bits1.da1.dest_address_mode) == GEN_ADDRESS_DIRECT) {
      err |= reg(file, GEN_BITS_FIELD(inst, bits1.da1.dest_reg_file),
                 GEN_BITS_FIELD(inst, bits1.da1.dest_reg_nr));
      if (err == -1) {
        control(file, "dest reg encoding", reg_encoding, GEN_BITS_FIELD(inst, bits1.da1.dest_reg_type), NULL);
        return 0;
      }
      if (GEN_BITS_FIELD(inst, bits1.da1.dest_subreg_nr))
        format(file, ".%d", GEN_BITS_FIELD(inst, bits1.da1.dest_subreg_nr) /
               reg_type_size[GEN_BITS_FIELD(inst, bits1.da1.dest_reg_type)]);
      format(file, "<%s>", horiz_stride[GEN_BITS_FIELD(inst, bits1.da1.dest_horiz_stride)]);
      err |= control(file, "dest reg encoding", reg_encoding, GEN_BITS_FIELD(inst, bits1.da1.dest_reg_type), NULL);
    } else {
      string(file, "g[a0");
      if (GEN_BITS_FIELD(inst, bits1.ia1.dest_subreg_nr))
        format(file, ".%d", GEN_BITS_FIELD(inst, bits1.ia1.dest_subreg_nr) /
               reg_type_size[GEN_BITS_FIELD(inst, bits1.ia1.dest_reg_type)]);
      if (GEN_BITS_FIELD(inst, bits1.ia1.dest_indirect_offset))
        format(file, " %d", GEN_BITS_FIELD(inst, bits1.ia1.dest_indirect_offset));
      string(file, "]");
      format(file, "<%s>", horiz_stride[GEN_BITS_FIELD(inst, bits1.ia1.dest_horiz_stride)]);
      err |= control(file, "dest reg encoding", reg_encoding, GEN_BITS_FIELD(inst, bits1.ia1.dest_reg_type), NULL);
    }
  } else {
    if (GEN_BITS_FIELD(inst, bits1.da16.dest_address_mode) == GEN_ADDRESS_DIRECT) {
      err |= reg(file, GEN_BITS_FIELD(inst, bits1.da16.dest_reg_file), GEN_BITS_FIELD(inst, bits1.da16.dest_reg_nr));
      if (err == -1)
        return 0;
      if (GEN_BITS_FIELD(inst, bits1.da16.dest_subreg_nr))
        format(file, ".%d", GEN_BITS_FIELD(inst, bits1.da16.dest_subreg_nr) /
               reg_type_size[GEN_BITS_FIELD(inst, bits1.da16.dest_reg_type)]);
      string(file, "<1>");
      err |= control(file, "writemask", writemask, GEN_BITS_FIELD(inst, bits1.da16.dest_writemask), NULL);
      err |= control(file, "dest reg encoding", reg_encoding, GEN_BITS_FIELD(inst, bits1.da16.dest_reg_type), NULL);
    } else {
      err = 1;
      string(file, "Indirect align16 address mode not supported");
    }
  }

  return 0;
}

static int dest_3src(FILE *file, const void *inst)
{
  int err = 0;
  const uint32_t reg_file = GEN_GENERAL_REGISTER_FILE;

  err |= reg(file, reg_file, GEN_BITS_FIELD(inst, bits1.da3src.dest_reg_nr));
  if (err == -1)
    return 0;
  if (GEN_BITS_FIELD(inst, bits1.da3src.dest_subreg_nr))
    format(file, ".%d", GEN_BITS_FIELD(inst, bits1.da3src.dest_subreg_nr));
  string(file, "<1>");
  err |= control(file, "writemask", writemask, GEN_BITS_FIELD(inst, bits1.da3src.dest_writemask), NULL);
  err |= control(file, "dest reg encoding", reg_encoding, GEN_TYPE_F, NULL);

  return 0;
}

static int src_align1_region(FILE *file,
                             uint32_t _vert_stride, uint32_t _width, uint32_t _horiz_stride)
{
  int err = 0;
  string(file, "<");
  if (gen_version < 80) {
    err |= control(file, "vert stride", vert_stride_gen7, _vert_stride, NULL);
  } else {
    err |= control(file, "vert stride", vert_stride_gen8, _vert_stride, NULL);
  }
  string(file, ",");
  err |= control(file, "width", width, _width, NULL);
  string(file, ",");
  err |= control(file, "horiz_stride", horiz_stride, _horiz_stride, NULL);
  string(file, ">");
  return err;
}

static int src_da1(FILE *file, uint32_t type, uint32_t _reg_file,
                   uint32_t _vert_stride, uint32_t _width, uint32_t _horiz_stride,
                   uint32_t reg_num, uint32_t sub_reg_num, uint32_t __abs, uint32_t _negate)
{
  int err = 0;
  err |= control(file, "negate", negate, _negate, NULL);
  err |= control(file, "abs", _abs, __abs, NULL);

  err |= reg(file, _reg_file, reg_num);
  if (err == -1)
    return 0;
  if (sub_reg_num)
    format(file, ".%d", sub_reg_num / reg_type_size[type]); /* use formal style like spec */
  src_align1_region(file, _vert_stride, _width, _horiz_stride);
  err |= control(file, "src reg encoding", reg_encoding, type, NULL);
  return err;
}

static int src_ia1(FILE *file,
                   uint32_t type,
                   uint32_t _reg_file,
                   int32_t _addr_imm,
                   uint32_t _addr_subreg_nr,
                   uint32_t _negate,
                   uint32_t __abs,
                   uint32_t _addr_mode,
                   uint32_t _horiz_stride,
                   uint32_t _width,
                   uint32_t _vert_stride)
{
  int err = 0;
  err |= control(file, "negate", negate, _negate, NULL);
  err |= control(file, "abs", _abs, __abs, NULL);

  string(file, "g[a0");
  if (_addr_subreg_nr)
    format(file, ".%d", _addr_subreg_nr);
  if (_addr_imm)
    format(file, " %d", _addr_imm);
  string(file, "]");
  src_align1_region(file, _vert_stride, _width, _horiz_stride);
  err |= control(file, "src reg encoding", reg_encoding, type, NULL);
  return err;
}

static int src_da16(FILE *file,
                    uint32_t _reg_type,
                    uint32_t _reg_file,
                    uint32_t _vert_stride,
                    uint32_t _reg_nr,
                    uint32_t _subreg_nr,
                    uint32_t __abs,
                    uint32_t _negate,
                    uint32_t swz_x,
                    uint32_t swz_y,
                    uint32_t swz_z,
                    uint32_t swz_w)
{
  int err = 0;
  err |= control(file, "negate", negate, _negate, NULL);
  err |= control(file, "abs", _abs, __abs, NULL);

  err |= reg(file, _reg_file, _reg_nr);
  if (err == -1)
    return 0;
  if (_subreg_nr)
    /* bit4 for subreg number byte addressing. Make this same meaning as
       in da1 case, so output looks consistent. */
    format(file, ".%d", 16 / reg_type_size[_reg_type]);
  string(file, "<");

  if (gen_version < 80) {
    err |= control(file, "vert stride", vert_stride_gen7, _vert_stride, NULL);
  } else {
    err |= control(file, "vert stride", vert_stride_gen8, _vert_stride, NULL);
  }
  string(file, ",4,1>");
  /*
   * Three kinds of swizzle display:
   *  identity - nothing printed
   *  1->all	 - print the single channel
   *  1->1     - print the mapping
   */
  if (swz_x == GEN_CHANNEL_X &&
      swz_y == GEN_CHANNEL_Y &&
      swz_z == GEN_CHANNEL_Z &&
      swz_w == GEN_CHANNEL_W) {
    ;
  } else if (swz_x == swz_y && swz_x == swz_z && swz_x == swz_w) {
    string(file, ".");
    err |= control(file, "channel select", chan_sel, swz_x, NULL);
  } else {
    string(file, ".");
    err |= control(file, "channel select", chan_sel, swz_x, NULL);
    err |= control(file, "channel select", chan_sel, swz_y, NULL);
    err |= control(file, "channel select", chan_sel, swz_z, NULL);
    err |= control(file, "channel select", chan_sel, swz_w, NULL);
  }
  err |= control(file, "src da16 reg type", reg_encoding, _reg_type, NULL);
  return err;
}

static int src0_3src(FILE *file, const void* inst)
{
  int err = 0;
  uint32_t swz_x = (GEN_BITS_FIELD(inst, bits2.da3src.src0_swizzle) >> 0) & 0x3;
  uint32_t swz_y = (GEN_BITS_FIELD(inst, bits2.da3src.src0_swizzle) >> 2) & 0x3;
  uint32_t swz_z = (GEN_BITS_FIELD(inst, bits2.da3src.src0_swizzle) >> 4) & 0x3;
  uint32_t swz_w = (GEN_BITS_FIELD(inst, bits2.da3src.src0_swizzle) >> 6) & 0x3;

  err |= control(file, "negate", negate, GEN_BITS_FIELD(inst, bits1.da3src.src0_negate), NULL);
  err |= control(file, "abs", _abs, GEN_BITS_FIELD(inst, bits1.da3src.src0_abs), NULL);

  err |= reg(file, GEN_GENERAL_REGISTER_FILE, GEN_BITS_FIELD(inst, bits2.da3src.src0_reg_nr));
  if (err == -1)
    return 0;
  if (GEN_BITS_FIELD(inst, bits2.da3src.src0_subreg_nr))
    format(file, ".%d", GEN_BITS_FIELD(inst, bits2.da3src.src0_subreg_nr));
  string(file, "<4,1,1>");
  err |= control(file, "src da16 reg type", reg_encoding,
                 GEN_TYPE_F, NULL);
  /*
   * Three kinds of swizzle display:
   *  identity - nothing printed
   *  1->all	 - print the single channel
   *  1->1     - print the mapping
   */
  if (swz_x == GEN_CHANNEL_X &&
      swz_y == GEN_CHANNEL_Y &&
      swz_z == GEN_CHANNEL_Z &&
      swz_w == GEN_CHANNEL_W) {
    ;
  } else if (swz_x == swz_y && swz_x == swz_z && swz_x == swz_w) {
    string(file, ".");
    err |= control(file, "channel select", chan_sel, swz_x, NULL);
  } else {
    string(file, ".");
    err |= control(file, "channel select", chan_sel, swz_x, NULL);
    err |= control(file, "channel select", chan_sel, swz_y, NULL);
    err |= control(file, "channel select", chan_sel, swz_z, NULL);
    err |= control(file, "channel select", chan_sel, swz_w, NULL);
  }
  return err;
}

static int src1_3src(FILE *file, const void* inst)
{
  int err = 0;
  uint32_t swz_x = (GEN_BITS_FIELD(inst, bits2.da3src.src1_swizzle) >> 0) & 0x3;
  uint32_t swz_y = (GEN_BITS_FIELD(inst, bits2.da3src.src1_swizzle) >> 2) & 0x3;
  uint32_t swz_z = (GEN_BITS_FIELD(inst, bits2.da3src.src1_swizzle) >> 4) & 0x3;
  uint32_t swz_w = (GEN_BITS_FIELD(inst, bits2.da3src.src1_swizzle) >> 6) & 0x3;
  uint32_t src1_subreg_nr = (GEN_BITS_FIELD(inst, bits2.da3src.src1_subreg_nr_low) |
                             (GEN_BITS_FIELD(inst, bits3.da3src.src1_subreg_nr_high) << 2));

  err |= control(file, "negate", negate, GEN_BITS_FIELD(inst, bits1.da3src.src1_negate), NULL);
  err |= control(file, "abs", _abs, GEN_BITS_FIELD(inst, bits1.da3src.src1_abs), NULL);

  err |= reg(file, GEN_GENERAL_REGISTER_FILE, GEN_BITS_FIELD(inst, bits3.da3src.src1_reg_nr));
  if (err == -1)
    return 0;
  if (src1_subreg_nr)
    format(file, ".%d", src1_subreg_nr);
  string(file, "<4,1,1>");
  err |= control(file, "src da16 reg type", reg_encoding,
                 GEN_TYPE_F, NULL);
  /*
   * Three kinds of swizzle display:
   *  identity - nothing printed
   *  1->all	 - print the single channel
   *  1->1     - print the mapping
   */
  if (swz_x == GEN_CHANNEL_X &&
      swz_y == GEN_CHANNEL_Y &&
      swz_z == GEN_CHANNEL_Z &&
      swz_w == GEN_CHANNEL_W) {
    ;
  } else if (swz_x == swz_y && swz_x == swz_z && swz_x == swz_w) {
    string(file, ".");
    err |= control(file, "channel select", chan_sel, swz_x, NULL);
  } else {
    string(file, ".");
    err |= control(file, "channel select", chan_sel, swz_x, NULL);
    err |= control(file, "channel select", chan_sel, swz_y, NULL);
    err |= control(file, "channel select", chan_sel, swz_z, NULL);
    err |= control(file, "channel select", chan_sel, swz_w, NULL);
  }
  return err;
}

static int src2_3src(FILE *file, const void* inst)
{
  int err = 0;
  uint32_t swz_x = (GEN_BITS_FIELD(inst, bits3.da3src.src2_swizzle) >> 0) & 0x3;
  uint32_t swz_y = (GEN_BITS_FIELD(inst, bits3.da3src.src2_swizzle) >> 2) & 0x3;
  uint32_t swz_z = (GEN_BITS_FIELD(inst, bits3.da3src.src2_swizzle) >> 4) & 0x3;
  uint32_t swz_w = (GEN_BITS_FIELD(inst, bits3.da3src.src2_swizzle) >> 6) & 0x3;

  err |= control(file, "negate", negate, GEN_BITS_FIELD(inst, bits1.da3src.src2_negate), NULL);
  err |= control(file, "abs", _abs, GEN_BITS_FIELD(inst, bits1.da3src.src2_abs), NULL);
  err |= reg(file, GEN_GENERAL_REGISTER_FILE, GEN_BITS_FIELD(inst, bits3.da3src.src2_reg_nr));
  if (err == -1)
    return 0;
  if (GEN_BITS_FIELD(inst, bits3.da3src.src2_subreg_nr))
    format(file, ".%d", GEN_BITS_FIELD(inst, bits3.da3src.src2_subreg_nr));
  string(file, "<4,1,1>");
  err |= control(file, "src da16 reg type", reg_encoding,
                 GEN_TYPE_F, NULL);
  /*
   * Three kinds of swizzle display:
   *  identity - nothing printed
   *  1->all	 - print the single channel
   *  1->1     - print the mapping
   */
  if (swz_x == GEN_CHANNEL_X &&
      swz_y == GEN_CHANNEL_Y &&
      swz_z == GEN_CHANNEL_Z &&
      swz_w == GEN_CHANNEL_W) {
    ;
  } else if (swz_x == swz_y && swz_x == swz_z && swz_x == swz_w) {
    string(file, ".");
    err |= control(file, "channel select", chan_sel, swz_x, NULL);
  } else {
    string(file, ".");
    err |= control(file, "channel select", chan_sel, swz_x, NULL);
    err |= control(file, "channel select", chan_sel, swz_y, NULL);
    err |= control(file, "channel select", chan_sel, swz_z, NULL);
    err |= control(file, "channel select", chan_sel, swz_w, NULL);
  }
  return err;
}

static int imm(FILE *file, uint32_t type, const void* inst)
{
  switch (type) {
    case GEN_TYPE_UD:
      format(file, "0x%xUD", GEN_BITS_FIELD(inst, bits3.ud));
      break;
    case GEN_TYPE_D:
      format(file, "%dD", GEN_BITS_FIELD(inst, bits3.d));
      break;
    case GEN_TYPE_UW:
      format(file, "0x%xUW", (uint16_t) GEN_BITS_FIELD(inst, bits3.ud));
      break;
    case GEN_TYPE_W:
      format(file, "%dW", (int16_t) GEN_BITS_FIELD(inst, bits3.d));
      break;
    case GEN_TYPE_UB:
      format(file, "0x%xUB", (int8_t) GEN_BITS_FIELD(inst, bits3.ud));
      break;
    case GEN_TYPE_VF:
      format(file, "Vector Float");
      break;
    case GEN_TYPE_V:
      format(file, "0x%xV", GEN_BITS_FIELD(inst, bits3.ud));
      break;
    case GEN_TYPE_F:
      format(file, "%-gF", GEN_BITS_FIELD(inst, bits3.f));
  }
  return 0;
}

static int src0(FILE *file, const void* inst)
{
  if (GEN_BITS_FIELD(inst, bits1.da1.src0_reg_file) == GEN_IMMEDIATE_VALUE)
    return imm(file, GEN_BITS_FIELD(inst, bits1.da1.src0_reg_type), inst);
  else if (ACCESS_MODE(inst) == GEN_ALIGN_1) {
    if (GEN_BITS_FIELD(inst, bits2.da1.src0_address_mode) == GEN_ADDRESS_DIRECT) {
      return src_da1(file,
                     GEN_BITS_FIELD(inst, bits1.da1.src0_reg_type),
                     GEN_BITS_FIELD(inst, bits1.da1.src0_reg_file),
                     GEN_BITS_FIELD(inst, bits2.da1.src0_vert_stride),
                     GEN_BITS_FIELD(inst, bits2.da1.src0_width),
                     GEN_BITS_FIELD(inst, bits2.da1.src0_horiz_stride),
                     GEN_BITS_FIELD(inst, bits2.da1.src0_reg_nr),
                     GEN_BITS_FIELD(inst, bits2.da1.src0_subreg_nr),
                     GEN_BITS_FIELD(inst, bits2.da1.src0_abs),
                     GEN_BITS_FIELD(inst, bits2.da1.src0_negate));
    } else {
      return src_ia1(file,
                     GEN_BITS_FIELD(inst, bits1.ia1.src0_reg_type),
                     GEN_BITS_FIELD(inst, bits1.ia1.src0_reg_file),
                     GEN_BITS_FIELD(inst, bits2.ia1.src0_indirect_offset),
                     GEN_BITS_FIELD(inst, bits2.ia1.src0_subreg_nr),
                     GEN_BITS_FIELD(inst, bits2.ia1.src0_negate),
                     GEN_BITS_FIELD(inst, bits2.ia1.src0_abs),
                     GEN_BITS_FIELD(inst, bits2.ia1.src0_address_mode),
                     GEN_BITS_FIELD(inst, bits2.ia1.src0_horiz_stride),
                     GEN_BITS_FIELD(inst, bits2.ia1.src0_width),
                     GEN_BITS_FIELD(inst, bits2.ia1.src0_vert_stride));
    }
  } else {
    if (GEN_BITS_FIELD(inst, bits2.da16.src0_address_mode) == GEN_ADDRESS_DIRECT) {
      return src_da16(file,
                      GEN_BITS_FIELD(inst, bits1.da16.src0_reg_type),
                      GEN_BITS_FIELD(inst, bits1.da16.src0_reg_file),
                      GEN_BITS_FIELD(inst, bits2.da16.src0_vert_stride),
                      GEN_BITS_FIELD(inst, bits2.da16.src0_reg_nr),
                      GEN_BITS_FIELD(inst, bits2.da16.src0_subreg_nr),
                      GEN_BITS_FIELD(inst, bits2.da16.src0_abs),
                      GEN_BITS_FIELD(inst, bits2.da16.src0_negate),
                      GEN_BITS_FIELD(inst, bits2.da16.src0_swz_x),
                      GEN_BITS_FIELD(inst, bits2.da16.src0_swz_y),
                      GEN_BITS_FIELD(inst, bits2.da16.src0_swz_z),
                      GEN_BITS_FIELD(inst, bits2.da16.src0_swz_w));
    } else {
      string(file, "Indirect align16 address mode not supported");
      return 1;
    }
  }
}

static int src1(FILE *file, const void* inst)
{
  if (GEN_BITS_FIELD2(inst, bits1.da1.src1_reg_file, bits2.da1.src1_reg_file) == GEN_IMMEDIATE_VALUE)
    return imm(file, GEN_BITS_FIELD2(inst, bits1.da1.src1_reg_type, bits2.da1.src1_reg_type),
               inst);
  else if (ACCESS_MODE(inst) == GEN_ALIGN_1) {
    if (GEN_BITS_FIELD(inst, bits3.da1.src1_address_mode) == GEN_ADDRESS_DIRECT) {
      return src_da1(file,
                     GEN_BITS_FIELD2(inst, bits1.da1.src1_reg_type, bits2.da1.src1_reg_type),
                     GEN_BITS_FIELD2(inst, bits1.da1.src1_reg_file, bits2.da1.src1_reg_file),
                     GEN_BITS_FIELD(inst, bits3.da1.src1_vert_stride),
                     GEN_BITS_FIELD(inst, bits3.da1.src1_width),
                     GEN_BITS_FIELD(inst, bits3.da1.src1_horiz_stride),
                     GEN_BITS_FIELD(inst, bits3.da1.src1_reg_nr),
                     GEN_BITS_FIELD(inst, bits3.da1.src1_subreg_nr),
                     GEN_BITS_FIELD(inst, bits3.da1.src1_abs),
                     GEN_BITS_FIELD(inst, bits3.da1.src1_negate));
    } else {
      return src_ia1(file,
                     GEN_BITS_FIELD2(inst, bits1.ia1.src1_reg_type, bits2.ia1.src1_reg_type),
                     GEN_BITS_FIELD2(inst, bits1.ia1.src1_reg_file, bits2.ia1.src1_reg_file),
                     GEN_BITS_FIELD(inst, bits3.ia1.src1_indirect_offset),
                     GEN_BITS_FIELD(inst, bits3.ia1.src1_subreg_nr),
                     GEN_BITS_FIELD(inst, bits3.ia1.src1_negate),
                     GEN_BITS_FIELD(inst, bits3.ia1.src1_abs),
                     GEN_BITS_FIELD(inst, bits3.ia1.src1_address_mode),
                     GEN_BITS_FIELD(inst, bits3.ia1.src1_horiz_stride),
                     GEN_BITS_FIELD(inst, bits3.ia1.src1_width),
                     GEN_BITS_FIELD(inst, bits3.ia1.src1_vert_stride));
    }
  } else {
    if (GEN_BITS_FIELD(inst, bits3.da16.src1_address_mode) == GEN_ADDRESS_DIRECT) {
      return src_da16(file,
                      GEN_BITS_FIELD2(inst, bits1.da16.src1_reg_type, bits2.da16.src1_reg_type),
                      GEN_BITS_FIELD2(inst, bits1.da16.src1_reg_file, bits2.da16.src1_reg_file),
                      GEN_BITS_FIELD(inst, bits3.da16.src1_vert_stride),
                      GEN_BITS_FIELD(inst, bits3.da16.src1_reg_nr),
                      GEN_BITS_FIELD(inst, bits3.da16.src1_subreg_nr),
                      GEN_BITS_FIELD(inst, bits3.da16.src1_abs),
                      GEN_BITS_FIELD(inst, bits3.da16.src1_negate),
                      GEN_BITS_FIELD(inst, bits3.da16.src1_swz_x),
                      GEN_BITS_FIELD(inst, bits3.da16.src1_swz_y),
                      GEN_BITS_FIELD(inst, bits3.da16.src1_swz_z),
                      GEN_BITS_FIELD(inst, bits3.da16.src1_swz_w));
    } else {
      string(file, "Indirect align16 address mode not supported");
      return 1;
    }
  }
}

static const int esize[6] = {
  [0] = 1,
  [1] = 2,
  [2] = 4,
  [3] = 8,
  [4] = 16,
  [5] = 32,
};

static int qtr_ctrl(FILE *file, const void* inst)
{
  int qtr_ctl = QUARTER_CONTROL(inst);
  int exec_size = esize[EXECUTION_SIZE(inst)];

  if (exec_size == 8) {
    switch (qtr_ctl) {
      case 0:
        string(file, " 1Q");
        break;
      case 1:
        string(file, " 2Q");
        break;
      case 2:
        string(file, " 3Q");
        break;
      case 3:
        string(file, " 4Q");
        break;
    }
  } else if (exec_size == 16) {
    if (qtr_ctl < 2)
      string(file, " 1H");
    else
      string(file, " 2H");
  }
  return 0;
}

int gen_disasm (FILE *file, const void *inst, uint32_t deviceID, uint32_t compacted)
{
  int err = 0;
  int space = 0;
  if (IS_IVYBRIDGE(deviceID)) {
    gen_version = 70;
  } else if (IS_HASWELL(deviceID)) {
    gen_version = 75;
  } else if (IS_BROADWELL(deviceID)) {
    gen_version = 80;
  }

  if (PRED_CTRL(inst)) {
    string(file, "(");
    err |= control(file, "predicate inverse", pred_inv, PRED_INV(inst), NULL);
    format(file, "f%d", FLAG_REG_NR(inst));
    if (FLAG_SUB_REG_NR(inst))
      format(file, ".%d", FLAG_SUB_REG_NR(inst));
    if (ACCESS_MODE(inst) == GEN_ALIGN_1)
      err |= control(file, "predicate control align1", pred_ctrl_align1,
                     PRED_CTRL(inst), NULL);
    else
      err |= control(file, "predicate control align16", pred_ctrl_align16,
                     PRED_CTRL(inst), NULL);
    string(file, ") ");
  }

  err |= print_opcode(file, OPCODE(inst));
  err |= control(file, "saturate", saturate, SATURATE(inst), NULL);
  err |= control(file, "debug control", debug_ctrl, DEBUG_CONTROL(inst), NULL);

  if (OPCODE(inst) == GEN_OPCODE_MATH) {
    string(file, " ");
    if (gen_version < 80) {
      err |= control(file, "function", math_function_gen7,
                     MATH_FUNCTION(inst), &space);
    } else {
      err |= control(file, "function", math_function_gen8,
                     MATH_FUNCTION(inst), &space);
    }

  } else if (OPCODE(inst) != GEN_OPCODE_SEND &&
             OPCODE(inst) != GEN_OPCODE_SENDC) {
    err |= control(file, "conditional modifier", conditional_modifier,
                   COND_DST_OR_MODIFIER(inst), NULL);
    if (COND_DST_OR_MODIFIER(inst))
      err |= flag_reg(file,
                      FLAG_REG_NR(inst),
                      FLAG_SUB_REG_NR(inst));
  }

  if (OPCODE(inst) != GEN_OPCODE_NOP) {
    string(file, "(");
    err |= control(file, "execution size", exec_size, EXECUTION_SIZE(inst), NULL);
    string(file, ")");
  }

  if (opcode[OPCODE(inst)].nsrc == 3) {
    pad(file, 16);
    err |= dest_3src(file, inst);

    pad(file, 32);
    err |= src0_3src(file, inst);

    pad(file, 48);
    err |= src1_3src(file, inst);

    pad(file, 64);
    err |= src2_3src(file, inst);
  } else {
    if (opcode[OPCODE(inst)].ndst > 0) {
      pad(file, 16);
      err |= dest(file, inst);
    } else if (OPCODE(inst) == GEN_OPCODE_IF ||
               OPCODE(inst) == GEN_OPCODE_ELSE ||
               OPCODE(inst) == GEN_OPCODE_ENDIF ||
               OPCODE(inst) == GEN_OPCODE_WHILE ||
               OPCODE(inst) == GEN_OPCODE_BRD ||
               OPCODE(inst) == GEN_OPCODE_JMPI) {
      format(file, " %d", (int16_t)BRANCH_JIP(inst));
    } else if (OPCODE(inst) == GEN_OPCODE_BREAK ||
               OPCODE(inst) == GEN_OPCODE_CONTINUE ||
               OPCODE(inst) == GEN_OPCODE_HALT ||
               OPCODE(inst) == GEN_OPCODE_BRC) {
      format(file, " %d %d", BRANCH_JIP(inst), BRANCH_UIP(inst));
    }/* else if (inst->header.opcode == GEN_OPCODE_JMPI) {
      format(file, " %d", inst->bits3.d);
    }*/

    if (opcode[OPCODE(inst)].nsrc > 0) {
      pad(file, 32);
      err |= src0(file, inst);
    }
    if (opcode[OPCODE(inst)].nsrc > 1) {
      pad(file, 48);
      err |= src1(file, inst);
    }
  }

  if (OPCODE(inst) == GEN_OPCODE_SEND ||
      OPCODE(inst) == GEN_OPCODE_SENDC) {
    enum GenMessageTarget target = COND_DST_OR_MODIFIER(inst);

    newline(file);
    pad(file, 16);
    space = 0;

    if(gen_version >= 75) {
      err |= control(file, "target function", target_function_gen75,
                     target, &space);
    } else {
      err |= control(file, "target function", target_function_gen7,
                     target, &space);
    }

    switch (target) {
      case GEN_SFID_SAMPLER:
        format(file, " (%d, %d, %d, %d)",
               SAMPLE_BTI(inst),
               SAMPLER(inst),
               SAMPLER_MSG_TYPE(inst),
               SAMPLER_SIMD_MODE(inst));
        break;
      case GEN_SFID_DATAPORT_DATA:
        if(UNTYPED_RW_CATEGORY(inst) == 0) {
          format(file, " (bti: %d, rgba: %d, %s, %s, %s)",
                 UNTYPED_RW_BTI(inst),
                 UNTYPED_RW_RGBA(inst),
                 data_port_data_cache_simd_mode[UNTYPED_RW_SIMD_MODE(inst)],
                 data_port_data_cache_category[UNTYPED_RW_CATEGORY(inst)],
                 data_port_data_cache_msg_type[UNTYPED_RW_MSG_TYPE(inst)]);
        } else {
          format(file, " (addr: %d, blocks: %s, %s, mode: %s, %s)",
                 SCRATCH_RW_OFFSET(inst),
                 data_port_scratch_block_size[SCRATCH_RW_BLOCK_SIZE(inst)],
                 data_port_scratch_invalidate[SCRATCH_RW_INVALIDATE_AFTER_READ(inst)],
                 data_port_scratch_channel_mode[SCRATCH_RW_CHANNEL_MODE(inst)],
                 data_port_scratch_msg_type[SCRATCH_RW_MSG_TYPE(inst)]);
        }
        break;
      case GEN_SFID_DATAPORT1_DATA:
        format(file, " (bti: %d, rgba: %d, %s, %s, %s)",
               UNTYPED_RW_BTI(inst),
               UNTYPED_RW_RGBA(inst),
               data_port_data_cache_simd_mode[UNTYPED_RW_SIMD_MODE(inst)],
               data_port_data_cache_category[UNTYPED_RW_CATEGORY(inst)],
               data_port1_data_cache_msg_type[UNTYPED_RW_MSG_TYPE(inst)]);
        break;
      case GEN_SFID_DATAPORT_CONSTANT:
        format(file, " (bti: %d, %s)",
               DWORD_RW_BTI(inst),
               data_port_data_cache_msg_type[DWORD_RW_MSG_TYPE(inst)]);
        break;
      case GEN_SFID_MESSAGE_GATEWAY:
        format(file, " (subfunc: %s, notify: %d, ackreq: %d)",
               gateway_sub_function[MSG_GW_SUBFUNC(inst)],
               MSG_GW_NOTIFY(inst),
               MSG_GW_ACKREQ(inst));
        break;

      default:
        format(file, "unsupported target %d", target);
        break;
    }
    if (space)
      string(file, " ");
    format(file, "mlen %d", GENERIC_MSG_LENGTH(inst));
    format(file, " rlen %d", GENERIC_RESPONSE_LENGTH(inst));
  }
  pad(file, 64);
  if (OPCODE(inst) != GEN_OPCODE_NOP) {
    string(file, "{");
    space = 1;
    err |= control(file, "access mode", access_mode, ACCESS_MODE(inst), &space);
    err |= control(file, "write enable control", wectrl, MASK_CONTROL(inst), &space);
    err |= control(file, "dependency control", dep_ctrl, DEPENDENCY_CONTROL(inst), &space);

    err |= qtr_ctrl(file, inst);
    if (gen_version < 80) {
      err |= control(file, "thread control", thread_ctrl_gen7, THREAD_CONTROL(inst), &space);
    } else {
      err |= control(file, "thread control", thread_ctrl_gen8, THREAD_CONTROL(inst), &space);
    }
    err |= control(file, "acc write control", accwr, ACC_WR_CONTROL(inst), &space);
    if (OPCODE(inst) == GEN_OPCODE_SEND ||
        OPCODE(inst) == GEN_OPCODE_SENDC)
      err |= control(file, "end of thread", end_of_thread,
                     END_OF_THREAD(inst), &space);

    if(compacted) {
      string(file, " Compacted");
    }
    if (space)
      string(file, " ");
    string(file, "}");
  }
  string(file, ";");
  newline(file);
  return err;
}


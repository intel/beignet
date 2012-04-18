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
#ifndef GEN_EU_H
#define GEN_EU_H

#include "backend/gen_defs.hpp"
#include "sys/platform.hpp"
#include <cassert>

#define GEN_SWIZZLE4(a,b,c,d) (((a)<<0) | ((b)<<2) | ((c)<<4) | ((d)<<6))
#define GEN_GET_SWZ(swz, idx) (((swz) >> ((idx)*2)) & 0x3)

#define GEN_SWIZZLE_NOOP      GEN_SWIZZLE4(0,1,2,3)
#define GEN_SWIZZLE_XYZW      GEN_SWIZZLE4(0,1,2,3)
#define GEN_SWIZZLE_XXXX      GEN_SWIZZLE4(0,0,0,0)
#define GEN_SWIZZLE_YYYY      GEN_SWIZZLE4(1,1,1,1)
#define GEN_SWIZZLE_ZZZZ      GEN_SWIZZLE4(2,2,2,2)
#define GEN_SWIZZLE_WWWW      GEN_SWIZZLE4(3,3,3,3)
#define GEN_SWIZZLE_XYXY      GEN_SWIZZLE4(0,1,0,1)

#define WRITEMASK_X     0x1
#define WRITEMASK_Y     0x2
#define WRITEMASK_XY    0x3
#define WRITEMASK_Z     0x4
#define WRITEMASK_XZ    0x5
#define WRITEMASK_YZ    0x6
#define WRITEMASK_XYZ   0x7
#define WRITEMASK_W     0x8
#define WRITEMASK_XW    0x9
#define WRITEMASK_YW    0xa
#define WRITEMASK_XYW   0xb
#define WRITEMASK_ZW    0xc
#define WRITEMASK_XZW   0xd
#define WRITEMASK_YZW   0xe
#define WRITEMASK_XYZW  0xf

#define GEN_REG_SIZE (8*4)
#define GEN_GRF_SIZE (GEN_REG_SIZE*128)
#define GEN_EU_MAX_INSN_STACK 5

#define VF_ZERO 0x0
#define VF_ONE  0x30
#define VF_NEG  (1<<7)

namespace gbe
{
  /* These aren't hardware structs, just something useful for us to pass around:
   *
   * Align1 operation has a lot of control over input ranges.  Used in
   * WM programs to implement shaders decomposed into "channel serial"
   * or "structure of array" form:
   */
  struct GenReg
  {
     uint32_t type:4;
     uint32_t file:2;
     uint32_t nr:8;
     uint32_t subnr:5;                /* :1 in align16 */
     uint32_t negate:1;                /* source only */
     uint32_t abs:1;                /* source only */
     uint32_t vstride:4;                /* source only */
     uint32_t width:3;                /* src only, align1 only */
     uint32_t hstride:2;                   /* align1 only */
     uint32_t address_mode:1;        /* relative addressing, hopefully! */
     uint32_t pad0:1;

     union {
        struct {
           uint32_t swizzle:8;                /* src only, align16 only */
           uint32_t writemask:4;                /* dest only, align16 only */
           int  indirect_offset:10;        /* relative addressing offset */
           uint32_t pad1:10;                /* two dwords total */
        } bits;

        float f;
        int   d;
        uint32_t ud;
     } dw1;
  };

  struct GenEmitter
  {
    int gen;
    GenInstruction store[8192];
    int store_size;
    uint32_t nr_insn;

    /* Allow clients to push/pop instruction state */
    GenInstruction stack[GEN_EU_MAX_INSN_STACK];
    bool compressed_stack[GEN_EU_MAX_INSN_STACK];

    uint32_t flag_value;
    bool single_program_flow;
    bool compressed;

    INLINE GenInstruction *current_insn(void) { return &this->store[this->nr_insn]; }

    void guess_execution_size(GenInstruction *insn, GenReg reg);
    void set_mask_control(uint32_t value);
    void set_saturate(uint32_t value);
    void set_access_mode(uint32_t access_mode);
    void set_compression_control(enum brw_compression c);
    void set_predicate_control_flag_value(uint32_t value);
    void set_predicate_control(uint32_t pc);
    void set_predicate_inverse(bool predicate_inverse);
    void set_conditionalmod(uint32_t conditional);
    void set_acc_write_control(uint32_t value);

    void init_compile(struct context *, void *mem_ctx);
    const uint32_t *get_program(uint32_t *sz);

    GenInstruction *next(uint32_t opcode);
    void set_dest(GenInstruction *insn, GenReg dest);
    void set_src0(GenInstruction *insn, GenReg reg);

#define ALU1(OP) GenInstruction *OP(GenReg dest, GenReg src0);
#define ALU2(OP) GenInstruction *OP(GenReg dest, GenReg src0, GenReg src1);
#define ALU3(OP) GenInstruction *OP(GenReg dest, GenReg src0, GenReg src1, GenReg src2);
#define ROUND(OP) void OP(GenReg dest, GenReg src0);

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
    ALU2(JMPI)
    ALU2(ADD)
    ALU2(MUL)
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

#undef ALU1
#undef ALU2
#undef ALU3
#undef ROUND

    /* Helpers for SEND instruction */
    void set_sampler_message(GenInstruction *insn,
                             uint32_t binding_table_index,
                             uint32_t sampler,
                             uint32_t msg_type,
                             uint32_t response_length,
                             uint32_t msg_length,
                             uint32_t header_present,
                             uint32_t simd_mode,
                             uint32_t return_format);

    void set_dp_read_message(GenInstruction *insn,
                             uint32_t binding_table_index,
                             uint32_t msg_control,
                             uint32_t msg_type,
                             uint32_t target_cache,
                             uint32_t msg_length,
                             uint32_t response_length);

    void set_dp_write_message(GenInstruction *insn,
                              uint32_t binding_table_index,
                              uint32_t msg_control,
                              uint32_t msg_type,
                              uint32_t msg_length,
                              bool header_present,
                              uint32_t last_render_target,
                              uint32_t response_length,
                              uint32_t end_of_thread,
                              uint32_t send_commit_msg);

    void SAMPLE(GenReg dest,
                uint32_t msg_reg_nr,
                GenReg src0,
                uint32_t binding_table_index,
                uint32_t sampler,
                uint32_t writemask,
                uint32_t msg_type,
                uint32_t response_length,
                uint32_t msg_length,
                uint32_t header_present,
                uint32_t simd_mode,
                uint32_t return_format);

    void math_16(GenReg dest,
                 uint32_t function,
                 uint32_t saturate,
                 uint32_t msg_reg_nr,
                 GenReg src,
                 uint32_t precision);

    void math(GenReg dest,
              uint32_t function,
              uint32_t saturate,
              uint32_t msg_reg_nr,
              GenReg src,
              uint32_t data_type,
              uint32_t precision);

    void math2(GenReg dest, uint32_t function, GenReg src0, GenReg src1);
    void EOT(uint32_t msg_nr);

    void dword_scattered_read(GenReg dest, GenReg mrf, uint32_t bind_table_index);

    void land_fwd_jump(int jmp_insn_idx);
    void NOP(void);
    void WAIT(void);

    void CMP(GenReg dest, uint32_t conditional, GenReg src0, GenReg src1);
    void copy4(GenReg dst, GenReg src, uint32_t count);
    void copy8(GenReg dst, GenReg src, uint32_t count);
    void math_invert(GenReg dst, GenReg src);
    void set_src1(GenInstruction *insn, GenReg reg);
    void set_uip_jip(void);
  };

  static INLINE int type_sz(uint32_t type)
  {
     switch(type) {
     case GEN_REGISTER_TYPE_UD:
     case GEN_REGISTER_TYPE_D:
     case GEN_REGISTER_TYPE_F:
        return 4;
     case GEN_REGISTER_TYPE_HF:
     case GEN_REGISTER_TYPE_UW:
     case GEN_REGISTER_TYPE_W:
        return 2;
     case GEN_REGISTER_TYPE_UB:
     case GEN_REGISTER_TYPE_B:
        return 1;
     default:
        return 0;
     }
  }

  static INLINE bool brw_is_single_value_swizzle(int swiz)
  {
     return (swiz == GEN_SWIZZLE_XXXX ||
             swiz == GEN_SWIZZLE_YYYY ||
             swiz == GEN_SWIZZLE_ZZZZ ||
             swiz == GEN_SWIZZLE_WWWW);
  }

  /**
   * Construct a GenReg.
   * \param file  one of the GEN_x_REGISTER_FILE values
   * \param nr  register number/index
   * \param subnr  register sub number
   * \param type  one of GEN_REGISTER_TYPE_x
   * \param vstride  one of GEN_VERTICAL_STRIDE_x
   * \param width  one of GEN_WIDTH_x
   * \param hstride  one of GEN_HORIZONTAL_STRIDE_x
   * \param swizzle  one of GEN_SWIZZLE_x
   * \param writemask  WRITEMASK_X/Y/Z/W bitfield
   */
  static INLINE GenReg makeGenReg(uint32_t file,
                                     uint32_t nr,
                                     uint32_t subnr,
                                     uint32_t type,
                                     uint32_t vstride,
                                     uint32_t width,
                                     uint32_t hstride,
                                     uint32_t swizzle,
                                     uint32_t writemask)
  {
     GenReg reg;
     if (file == GEN_GENERAL_REGISTER_FILE)
        assert(nr < GEN_MAX_GRF);
     else if (file == GEN_ARCHITECTURE_REGISTER_FILE)
        assert(nr <= GEN_ARF_IP);

     reg.type = type;
     reg.file = file;
     reg.nr = nr;
     reg.subnr = subnr * type_sz(type);
     reg.negate = 0;
     reg.abs = 0;
     reg.vstride = vstride;
     reg.width = width;
     reg.hstride = hstride;
     reg.address_mode = GEN_ADDRESS_DIRECT;
     reg.pad0 = 0;

     /* Could do better: If the reg is r5.3<0;1,0>, we probably want to
      * set swizzle and writemask to W, as the lower bits of subnr will
      * be lost when converted to align16.  This is probably too much to
      * keep track of as you'd want it adjusted by suboffset(), etc.
      * Perhaps fix up when converting to align16?
      */
     reg.dw1.bits.swizzle = swizzle;
     reg.dw1.bits.writemask = writemask;
     reg.dw1.bits.indirect_offset = 0;
     reg.dw1.bits.pad1 = 0;
     return reg;
  }

  /** Construct float[16] register */
  static INLINE GenReg brw_vec16_reg(uint32_t file, uint32_t nr, uint32_t subnr)
  {
     return makeGenReg(file,
                       nr,
                       subnr,
                       GEN_REGISTER_TYPE_F,
                       GEN_VERTICAL_STRIDE_16,
                       GEN_WIDTH_16,
                       GEN_HORIZONTAL_STRIDE_1,
                       GEN_SWIZZLE_XYZW,
                       WRITEMASK_XYZW);
  }

  /** Construct float[8] register */
  static INLINE GenReg brw_vec8_reg(uint32_t file, uint32_t nr, uint32_t subnr)
  {
     return makeGenReg(file,
                         nr,
                         subnr,
                         GEN_REGISTER_TYPE_F,
                         GEN_VERTICAL_STRIDE_8,
                         GEN_WIDTH_8,
                         GEN_HORIZONTAL_STRIDE_1,
                         GEN_SWIZZLE_XYZW,
                         WRITEMASK_XYZW);
  }

  /** Construct float[4] register */
  static INLINE GenReg brw_vec4_reg(uint32_t file, uint32_t nr, uint32_t subnr)
  {
     return makeGenReg(file,
                       nr,
                       subnr,
                       GEN_REGISTER_TYPE_F,
                       GEN_VERTICAL_STRIDE_4,
                       GEN_WIDTH_4,
                       GEN_HORIZONTAL_STRIDE_1,
                       GEN_SWIZZLE_XYZW,
                       WRITEMASK_XYZW);
  }

  /** Construct float[2] register */
  static INLINE GenReg brw_vec2_reg(uint32_t file, uint32_t nr, uint32_t subnr)
  {
     return makeGenReg(file,
                         nr,
                         subnr,
                         GEN_REGISTER_TYPE_F,
                         GEN_VERTICAL_STRIDE_2,
                         GEN_WIDTH_2,
                         GEN_HORIZONTAL_STRIDE_1,
                         GEN_SWIZZLE_XYXY,
                         WRITEMASK_XY);
  }

  /** Construct float[1] register */
  static INLINE GenReg brw_vec1_reg(uint32_t file, uint32_t nr, uint32_t subnr)
  {
     return makeGenReg(file,
                       nr,
                       subnr,
                       GEN_REGISTER_TYPE_F,
                       GEN_VERTICAL_STRIDE_0,
                       GEN_WIDTH_1,
                       GEN_HORIZONTAL_STRIDE_0,
                       GEN_SWIZZLE_XXXX,
                       WRITEMASK_X);
  }

  static INLINE GenReg retype(GenReg reg, uint32_t type)
  {
     reg.type = type;
     return reg;
  }

  static INLINE GenReg sechalf(GenReg reg)
  {
     if (reg.vstride)
        reg.nr++;
     return reg;
  }

  static INLINE GenReg suboffset(GenReg reg, uint32_t delta)
  {
     reg.subnr += delta * type_sz(reg.type);
     return reg;
  }

  static INLINE GenReg offset(GenReg reg, uint32_t delta)
  {
     reg.nr += delta;
     return reg;
  }

  static INLINE GenReg byte_offset(GenReg reg, uint32_t bytes)
  {
     uint32_t newoffset = reg.nr * GEN_REG_SIZE + reg.subnr + bytes;
     reg.nr = newoffset / GEN_REG_SIZE;
     reg.subnr = newoffset % GEN_REG_SIZE;
     return reg;
  }


  /** Construct unsigned word[16] register */
  static INLINE GenReg brw_uw16_reg(uint32_t file, uint32_t nr, uint32_t subnr)
  {
     return suboffset(retype(brw_vec16_reg(file, nr, 0), GEN_REGISTER_TYPE_UW), subnr);
  }

  /** Construct unsigned word[8] register */
  static INLINE GenReg brw_uw8_reg(uint32_t file, uint32_t nr, uint32_t subnr)
  {
     return suboffset(retype(brw_vec8_reg(file, nr, 0), GEN_REGISTER_TYPE_UW), subnr);
  }

  /** Construct unsigned word[1] register */
  static INLINE GenReg brw_uw1_reg(uint32_t file, uint32_t nr, uint32_t subnr)
  {
     return suboffset(retype(brw_vec1_reg(file, nr, 0), GEN_REGISTER_TYPE_UW), subnr);
  }

  static INLINE GenReg brw_imm_reg(uint32_t type)
  {
     return makeGenReg(GEN_IMMEDIATE_VALUE,
                         0,
                         0,
                         type,
                         GEN_VERTICAL_STRIDE_0,
                         GEN_WIDTH_1,
                         GEN_HORIZONTAL_STRIDE_0,
                         0,
                         0);
  }

  /** Construct float immediate register */
  static INLINE GenReg brw_imm_f(float f)
  {
     GenReg imm = brw_imm_reg(GEN_REGISTER_TYPE_F);
     imm.dw1.f = f;
     return imm;
  }

  /** Construct integer immediate register */
  static INLINE GenReg brw_imm_d(int d)
  {
     GenReg imm = brw_imm_reg(GEN_REGISTER_TYPE_D);
     imm.dw1.d = d;
     return imm;
  }

  /** Construct uint immediate register */
  static INLINE GenReg brw_imm_ud(uint32_t ud)
  {
     GenReg imm = brw_imm_reg(GEN_REGISTER_TYPE_UD);
     imm.dw1.ud = ud;
     return imm;
  }

  /** Construct ushort immediate register */
  static INLINE GenReg brw_imm_uw(uint16_t uw)
  {
     GenReg imm = brw_imm_reg(GEN_REGISTER_TYPE_UW);
     imm.dw1.ud = uw | (uw << 16);
     return imm;
  }

  /** Construct short immediate register */
  static INLINE GenReg brw_imm_w(short w)
  {
     GenReg imm = brw_imm_reg(GEN_REGISTER_TYPE_W);
     imm.dw1.d = w | (w << 16);
     return imm;
  }

  /* brw_imm_b and brw_imm_ub aren't supported by hardware - the type
   * numbers alias with _V and _VF below:
   */

  /** Construct vector of eight signed half-byte values */
  static INLINE GenReg brw_imm_v(uint32_t v)
  {
     GenReg imm = brw_imm_reg(GEN_REGISTER_TYPE_V);
     imm.vstride = GEN_VERTICAL_STRIDE_0;
     imm.width = GEN_WIDTH_8;
     imm.hstride = GEN_HORIZONTAL_STRIDE_1;
     imm.dw1.ud = v;
     return imm;
  }

  /** Construct vector of four 8-bit float values */
  static INLINE GenReg brw_imm_vf(uint32_t v)
  {
     GenReg imm = brw_imm_reg(GEN_REGISTER_TYPE_VF);
     imm.vstride = GEN_VERTICAL_STRIDE_0;
     imm.width = GEN_WIDTH_4;
     imm.hstride = GEN_HORIZONTAL_STRIDE_1;
     imm.dw1.ud = v;
     return imm;
  }

  static INLINE GenReg brw_imm_vf4(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
  {
     GenReg imm = brw_imm_reg(GEN_REGISTER_TYPE_VF);
     imm.vstride = GEN_VERTICAL_STRIDE_0;
     imm.width = GEN_WIDTH_4;
     imm.hstride = GEN_HORIZONTAL_STRIDE_1;
     imm.dw1.ud = ((v0 << 0) | (v1 << 8) | (v2 << 16) | (v3 << 24));
     return imm;
  }

  static INLINE GenReg brw_address(GenReg reg)
  {
     return brw_imm_uw(reg.nr * GEN_REG_SIZE + reg.subnr);
  }

  /** Construct float[1] general-purpose register */
  static INLINE GenReg brw_vec1_grf(uint32_t nr, uint32_t subnr)
  {
     return brw_vec1_reg(GEN_GENERAL_REGISTER_FILE, nr, subnr);
  }

  /** Construct float[2] general-purpose register */
  static INLINE GenReg brw_vec2_grf(uint32_t nr, uint32_t subnr)
  {
     return brw_vec2_reg(GEN_GENERAL_REGISTER_FILE, nr, subnr);
  }

  /** Construct float[4] general-purpose register */
  static INLINE GenReg brw_vec4_grf(uint32_t nr, uint32_t subnr)
  {
     return brw_vec4_reg(GEN_GENERAL_REGISTER_FILE, nr, subnr);
  }

  /** Construct float[8] general-purpose register */
  static INLINE GenReg brw_vec8_grf(uint32_t nr, uint32_t subnr)
  {
     return brw_vec8_reg(GEN_GENERAL_REGISTER_FILE, nr, subnr);
  }

  static INLINE GenReg brw_uw8_grf(uint32_t nr, uint32_t subnr)
  {
     return brw_uw8_reg(GEN_GENERAL_REGISTER_FILE, nr, subnr);
  }

  static INLINE GenReg brw_uw16_grf(uint32_t nr, uint32_t subnr)
  {
     return brw_uw16_reg(GEN_GENERAL_REGISTER_FILE, nr, subnr);
  }

  /** Construct null register (usually used for setting condition codes) */
  static INLINE GenReg brw_null_reg(void)
  {
     return brw_vec8_reg(GEN_ARCHITECTURE_REGISTER_FILE,
                         GEN_ARF_NULL,
                         0);
  }

  static INLINE GenReg brw_address_reg(uint32_t subnr)
  {
     return brw_uw1_reg(GEN_ARCHITECTURE_REGISTER_FILE,
                        GEN_ARF_ADDRESS,
                        subnr);
  }

  /* If/else instructions break in align16 mode if writemask & swizzle
   * aren't xyzw.  This goes against the convention for other scalar
   * regs:
   */
  static INLINE GenReg brw_ip_reg(void)
  {
     return makeGenReg(GEN_ARCHITECTURE_REGISTER_FILE,
                         GEN_ARF_IP,
                         0,
                         GEN_REGISTER_TYPE_UD,
                         GEN_VERTICAL_STRIDE_4, /* ? */
                         GEN_WIDTH_1,
                         GEN_HORIZONTAL_STRIDE_0,
                         GEN_SWIZZLE_XYZW, /* NOTE! */
                         WRITEMASK_XYZW); /* NOTE! */
  }

  static INLINE GenReg brw_acc_reg(void)
  {
     return brw_vec8_reg(GEN_ARCHITECTURE_REGISTER_FILE,
                         GEN_ARF_ACCUMULATOR,
                         0);
  }

  static INLINE GenReg brw_notification_1_reg(void)
  {

     return makeGenReg(GEN_ARCHITECTURE_REGISTER_FILE,
                         GEN_ARF_NOTIFICATION_COUNT,
                         1,
                         GEN_REGISTER_TYPE_UD,
                         GEN_VERTICAL_STRIDE_0,
                         GEN_WIDTH_1,
                         GEN_HORIZONTAL_STRIDE_0,
                         GEN_SWIZZLE_XXXX,
                         WRITEMASK_X);
  }


  static INLINE GenReg brw_flag_reg(void)
  {
     return brw_uw1_reg(GEN_ARCHITECTURE_REGISTER_FILE, GEN_ARF_FLAG, 0);
  }

  static INLINE GenReg brw_mask_reg(uint32_t subnr)
  {
     return brw_uw1_reg(GEN_ARCHITECTURE_REGISTER_FILE, GEN_ARF_MASK, subnr);
  }

  /* This is almost always called with a numeric constant argument, so
   * make things easy to evaluate at compile time:
   */
  static INLINE uint32_t cvt(uint32_t val)
  {
     switch (val) {
     case 0: return 0;
     case 1: return 1;
     case 2: return 2;
     case 4: return 3;
     case 8: return 4;
     case 16: return 5;
     case 32: return 6;
     }
     return 0;
  }

  static INLINE GenReg stride(GenReg reg,
                              uint32_t vstride,
                              uint32_t width,
                              uint32_t hstride)
  {
     reg.vstride = cvt(vstride);
     reg.width = cvt(width) - 1;
     reg.hstride = cvt(hstride);
     return reg;
  }


  static INLINE GenReg vec16(GenReg reg)
  {
     return stride(reg, 16,16,1);
  }

  static INLINE GenReg vec8(GenReg reg)
  {
     return stride(reg, 8,8,1);
  }

  static INLINE GenReg vec4(GenReg reg)
  {
     return stride(reg, 4,4,1);
  }

  static INLINE GenReg vec2(GenReg reg)
  {
     return stride(reg, 2,2,1);
  }

  static INLINE GenReg vec1(GenReg reg)
  {
     return stride(reg, 0,1,0);
  }

  static INLINE GenReg get_element(GenReg reg, uint32_t elt)
  {
     return vec1(suboffset(reg, elt));
  }

  static INLINE GenReg get_element_ud(GenReg reg, uint32_t elt)
  {
     return vec1(suboffset(retype(reg, GEN_REGISTER_TYPE_UD), elt));
  }

  static INLINE GenReg get_element_d(GenReg reg, uint32_t elt)
  {
     return vec1(suboffset(retype(reg, GEN_REGISTER_TYPE_D), elt));
  }

  static INLINE GenReg brw_swizzle(GenReg reg, uint32_t x, uint32_t y, uint32_t z, uint32_t w)
  {
     assert(reg.file != GEN_IMMEDIATE_VALUE);
     reg.dw1.bits.swizzle = GEN_SWIZZLE4(GEN_GET_SWZ(reg.dw1.bits.swizzle, x),
                                         GEN_GET_SWZ(reg.dw1.bits.swizzle, y),
                                         GEN_GET_SWZ(reg.dw1.bits.swizzle, z),
                                         GEN_GET_SWZ(reg.dw1.bits.swizzle, w));
     return reg;
  }


  static INLINE GenReg brw_swizzle1(GenReg reg, uint32_t x)
  {
     return brw_swizzle(reg, x, x, x, x);
  }

  static INLINE GenReg brw_writemask(GenReg reg, uint32_t mask)
  {
     assert(reg.file != GEN_IMMEDIATE_VALUE);
     reg.dw1.bits.writemask &= mask;
     return reg;
  }

  static INLINE GenReg brw_set_writemask(GenReg reg, uint32_t mask)
  {
     assert(reg.file != GEN_IMMEDIATE_VALUE);
     reg.dw1.bits.writemask = mask;
     return reg;
  }

  static INLINE GenReg negate(GenReg reg)
  {
     reg.negate ^= 1;
     return reg;
  }

  static INLINE GenReg brw_abs(GenReg reg)
  {
     reg.abs = 1;
     reg.negate = 0;
     return reg;
  }

  static INLINE GenReg brw_vec4_indirect(uint32_t subnr, int offset)
  {
     GenReg reg =  brw_vec4_grf(0, 0);
     reg.subnr = subnr;
     reg.address_mode = GEN_ADDRESS_REGISTER_INDIRECT_REGISTER;
     reg.dw1.bits.indirect_offset = offset;
     return reg;
  }

  static INLINE GenReg brw_vec1_indirect(uint32_t subnr, int offset)
  {
     GenReg reg =  brw_vec1_grf(0, 0);
     reg.subnr = subnr;
     reg.address_mode = GEN_ADDRESS_REGISTER_INDIRECT_REGISTER;
     reg.dw1.bits.indirect_offset = offset;
     return reg;
  }

  static INLINE bool brw_same_reg(GenReg r1, GenReg r2)
  {
     return r1.file == r2.file && r1.nr == r2.nr;
  }

  uint32_t brw_swap_cmod(uint32_t cmod);

} /* namespace gbe */

#endif /* GEN_EU_H */


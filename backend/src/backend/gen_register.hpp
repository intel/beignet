/*
 * Copyright 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
/**
 * \file gen_register.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __GEN_REGISTER_HPP__
#define __GEN_REGISTER_HPP__

#include "backend/gen_defs.hpp"
#include "ir/register.hpp"
#include "sys/platform.hpp"

namespace gbe
{

  /*! Type size in bytes for each Gen type */
  INLINE int typeSize(uint32_t type) {
    switch(type) {
      case GEN_TYPE_DF:
      case GEN_TYPE_UL:
      case GEN_TYPE_L:
        return 8;
      case GEN_TYPE_UD:
      case GEN_TYPE_D:
      case GEN_TYPE_F:
        return 4;
      case GEN_TYPE_UW:
      case GEN_TYPE_W:
      case GEN_TYPE_HF:
      case GEN_TYPE_HF_IMM:
        return 2;
      case GEN_TYPE_UB:
      case GEN_TYPE_B:
        return 1;
      default:
        assert(0);
        return 0;
    }
  }

  /*! Convert a hstride to a number of element */
  INLINE uint32_t stride(uint32_t stride) {
    switch (stride) {
      case 0: return 0;
      case 1: return 1;
      case 2: return 2;
      case 3: return 4;
      case 4: return 8;
      case 5: return 16;
      default: assert(0); return 0;
    }
  }

  /*! Encode the instruction state. Note that the flag register can be either
   *  physical (i.e. a real Gen flag) or a virtual boolean register. The flag
   *  register allocation will turn all virtual boolean registers into flag
   *  registers
   */
  class GenInstructionState
  {
  public:
    INLINE GenInstructionState(uint32_t simdWidth = 8) {
      this->execWidth = simdWidth;
      this->quarterControl = GEN_COMPRESSION_Q1;
      this->nibControl = 0;
      this->accWrEnable = 0;
      this->noMask = 0;
      this->flag = 0;
      this->subFlag = 0;
      this->grfFlag = 1;
      this->externFlag = 0;
      this->modFlag = 0;
      this->flagGen = 0;
      this->predicate = GEN_PREDICATE_NONE;
      this->inversePredicate = 0;
      this->physicalFlag = 1;
      this->flagIndex = 0;
      this->saturate = GEN_MATH_SATURATE_NONE;
    }
    uint32_t physicalFlag:1; //!< Physical or virtual flag register
    uint32_t flag:1;         //!< Only if physical flag,
    uint32_t subFlag:1;      //!< Only if physical flag
    uint32_t grfFlag:1;      //!< Only if virtual flag, 0 means we do not need to allocate GRF.
    uint32_t externFlag:1;   //!< Only if virtual flag, 1 means this flag is from external BB.
    uint32_t modFlag:1;      //!< Only if virtual flag, 1 means will modify flag.
    uint32_t flagGen:1;      //!< Only if virtual flag, 1 means the gen_context stage may need to
                             //!< generate the flag.
    uint32_t execWidth:5;
    uint32_t quarterControl:1;
    uint32_t nibControl:1;
    uint32_t accWrEnable:1;
    uint32_t noMask:1;
    uint32_t predicate:4;
    uint32_t inversePredicate:1;
    uint32_t saturate:1;
    uint32_t flagIndex;   //!< Only if virtual flag (index of the register)
    void chooseNib(int nib) {
      switch (nib) {
        case 0:
          quarterControl = 0;
          nibControl = 0;
          break;
        case 1:
          quarterControl = 0;
          nibControl = 1;
          break;
        case 2:
          quarterControl = 1;
          nibControl = 0;
          break;
        case 3:
          quarterControl = 1;
          nibControl = 1;
          break;
        default:
          NOT_IMPLEMENTED;
      }
    }
    void useVirtualFlag(ir::Register flag, unsigned pred) {
      modFlag = 0;
      physicalFlag = 0;
      flagIndex = flag;
      predicate = pred;
    }
    void useFlag(int nr, int subnr) {
      flag = nr;
      subFlag = subnr;
      physicalFlag = 1;
    }
  };

  /*! This is a book-keeping structure used to encode both virtual and physical
   *  registers
   */
  class GenRegister
  {
  public:
    /*! Empty constructor */
    INLINE GenRegister(void) {}

    /*! General constructor */
    INLINE GenRegister(uint32_t file,
                       ir::Register reg,
                       uint32_t type,
                       uint32_t vstride,
                       uint32_t width,
                       uint32_t hstride)
    {
      this->type = type;
      this->file = file;
      this->physical = 0;
      this->subphysical = 0;
      this->value.reg = reg;
      this->negation = 0;
      this->absolute = 0;
      this->vstride = vstride;
      this->width = width;
      this->hstride = hstride;
      this->quarter = 0;
      this->nr = this->subnr = 0;
      this->address_mode = GEN_ADDRESS_DIRECT;
      this->a0_subnr = 0;
      this->addr_imm = 0;
    }

    /*! For specific physical registers only */
    INLINE GenRegister(uint32_t file,
                       uint32_t nr,
                       uint32_t subnr,
                       uint32_t type,
                       uint32_t vstride,
                       uint32_t width,
                       uint32_t hstride)
    {
      this->type = type;
      this->file = file;
      this->nr = nr;
      this->physical = 1;
      this->subphysical = 1;
      this->subnr = subnr * typeSize(type);
      this->negation = 0;
      this->absolute = 0;
      this->vstride = vstride;
      this->width = width;
      this->hstride = hstride;
      this->quarter = 0;
      this->address_mode = GEN_ADDRESS_DIRECT;
      this->a0_subnr = 0;
      this->addr_imm = 0;
    }

    /*! Return the IR virtual register */
    INLINE ir::Register reg(void) const {
      if (this->physical)
        return ir::Register();
      else
        return ir::Register(value.reg);
    }

    /*! For immediates or virtual register */
    union {
      double df;
      float f;
      int32_t d;
      uint32_t ud;
      uint32_t reg;
      int64_t i64;
      uint64_t u64;
    } value;

    uint32_t nr:8;         //!< Just for some physical registers (acc, null)
    uint32_t subnr:8;      //!< Idem
    uint32_t physical:1;   //!< 1 if physical, 0 otherwise
    uint32_t subphysical:1;//!< 1 if subnr is physical, 0 otherwise
    uint32_t type:4;       //!< Gen type
    uint32_t file:2;       //!< Register file
    uint32_t negation:1;   //!< For source
    uint32_t absolute:1;   //!< For source
    uint32_t vstride:4;    //!< Vertical stride
    uint32_t width:3;        //!< Width
    uint32_t hstride:2;      //!< Horizontal stride
    uint32_t quarter:1;      //!< To choose which part we want (Q1 / Q2)
    uint32_t address_mode:1; //!< direct or indirect
    uint32_t a0_subnr:4;     //!< In indirect mode, use a0.nr as the base.
    int32_t addr_imm:10;     //!< In indirect mode, the imm as address offset from a0.

    static INLINE GenRegister offset(GenRegister reg, int nr, int subnr = 0) {
      GenRegister r = reg;
      if(subnr >= 32){
        nr += subnr / 32;
        subnr = subnr % 32;
      }
      r.nr += nr;
      r.subnr += subnr;
      r.subphysical = 1;
      return r;
    }

    static INLINE GenRegister toUniform(GenRegister reg, uint32_t type) {
      GenRegister r = reg;
      r.type = type;
      r.hstride = GEN_HORIZONTAL_STRIDE_0;
      r.vstride = GEN_VERTICAL_STRIDE_0;
      r.width = GEN_WIDTH_1;
      return r;
    }

    INLINE bool isSameRegion(GenRegister reg) const {
      return reg.file == file &&
             typeSize(reg.type) == typeSize(type) &&
             reg.vstride == vstride &&
             reg.width == width &&
             reg.hstride == hstride;
    }

    static INLINE uint32_t grfOffset(GenRegister reg) {
      return reg.nr * GEN_REG_SIZE + reg.subnr;
    }

    // split a DWORD register into unpacked Byte or Short register
    static INLINE GenRegister splitReg(GenRegister reg, uint32_t count, uint32_t sub_part) {
      GenRegister r = reg;
      GBE_ASSERT(count == 4 || count == 2);
      GBE_ASSERT(reg.type == GEN_TYPE_UD || reg.type == GEN_TYPE_D);

      if(reg.hstride != GEN_HORIZONTAL_STRIDE_0) {
        GBE_ASSERT(reg.hstride == GEN_HORIZONTAL_STRIDE_1);
        r.hstride = count == 4 ? GEN_HORIZONTAL_STRIDE_4 : GEN_HORIZONTAL_STRIDE_2;
      }
      if(count == 4) {
        r.type = reg.type == GEN_TYPE_UD ? GEN_TYPE_UB : GEN_TYPE_B;
        r.vstride = GEN_VERTICAL_STRIDE_32;
      } else {
        r.type = reg.type == GEN_TYPE_UD ? GEN_TYPE_UW : GEN_TYPE_W;
        r.vstride = GEN_VERTICAL_STRIDE_16;
      }

      r.subnr += sub_part*typeSize(r.type);
      r.nr += r.subnr / 32;
      r.subnr %= 32;

      return r;
    }

    INLINE bool isint64(void) const {
      if ((type == GEN_TYPE_UL || type == GEN_TYPE_L) && file == GEN_GENERAL_REGISTER_FILE)
        return true;
      return false;
    }

    /* Besides long and double, there are also some cases which can also stride
       several registers, eg. unpacked ud for long<8,4:2> and unpacked uw for
       long<16,4:4> */
    INLINE bool is_unpacked_long(void) const {
      if (file != GEN_GENERAL_REGISTER_FILE) return false;
      if (width == GEN_WIDTH_4 && hstride > GEN_HORIZONTAL_STRIDE_1) return true;
      return false;
    }

    INLINE bool isimmdf(void) const {
      if (type == GEN_TYPE_DF && file == GEN_IMMEDIATE_VALUE)
        return true;
      return false;
    }

    INLINE GenRegister top_half(int simdWidth) const {
      GBE_ASSERT(isint64());
      GenRegister reg = retype(*this, type == GEN_TYPE_UL ? GEN_TYPE_UD : GEN_TYPE_D);

      if (reg.hstride != GEN_HORIZONTAL_STRIDE_0) {
        reg.subnr += simdWidth * typeSize(reg.type) * hstride_size(reg);
        reg.nr += reg.subnr / 32;
        reg.subnr %= 32;
      } else {
        reg.subnr += typeSize(reg.type);
        reg.nr += reg.subnr/32;
        reg.subnr %= 32;
      }
      return reg;
    }

    INLINE GenRegister bottom_half(void) const {
      GBE_ASSERT(isint64());
      GenRegister r = retype(*this, type == GEN_TYPE_UL ? GEN_TYPE_UD : GEN_TYPE_D);
      return r;
    }

    INLINE bool is_signed_int(void) const {
      if ((type == GEN_TYPE_B || type == GEN_TYPE_W || type == GEN_TYPE_D || type == GEN_TYPE_L) && file == GEN_GENERAL_REGISTER_FILE)
        return true;
      return false;
    }

    INLINE bool isdf(void) const {
      if (type == GEN_TYPE_DF && file == GEN_GENERAL_REGISTER_FILE)
        return true;
      return false;
    }

    INLINE int flag_nr(void) const {
      assert(file == GEN_ARCHITECTURE_REGISTER_FILE);
      assert(nr >= GEN_ARF_FLAG && nr < GEN_ARF_FLAG + 2);
      return nr & 15;
    }

    INLINE int flag_subnr(void) const {
      return subnr / typeSize(type);
    }

    static INLINE GenRegister h2(GenRegister reg) {
      GenRegister r = reg;
      if(r.hstride != GEN_HORIZONTAL_STRIDE_0)
        r.hstride = GEN_HORIZONTAL_STRIDE_2;
      return r;
    }

    static INLINE GenRegister QnVirtual(GenRegister reg, uint32_t quarter) {
      GBE_ASSERT(reg.physical == 0);
      if (reg.hstride == GEN_HORIZONTAL_STRIDE_0) // scalar register
        return reg;
      else {
        reg.quarter = quarter;
        return reg;
      }
    }

    static INLINE GenRegister QnPhysical(GenRegister reg, uint32_t quarter) {
      GBE_ASSERT(reg.physical);
      if (reg.hstride == GEN_HORIZONTAL_STRIDE_0) // scalar register
        return reg;
      else {
        const uint32_t typeSz = typeSize(reg.type);
        const uint32_t horizontal = stride(reg.hstride);
        const uint32_t grfOffset = reg.nr*GEN_REG_SIZE + reg.subnr;
        const uint32_t nextOffset = grfOffset + 8*quarter*horizontal*typeSz;
        reg.nr = nextOffset / GEN_REG_SIZE;
        reg.subnr = (nextOffset % GEN_REG_SIZE);
        return reg;
      }
    }

    static INLINE GenRegister Qn(GenRegister reg, uint32_t quarter) {
      if (reg.physical)
        return QnPhysical(reg, quarter);
      else
        return QnVirtual(reg, quarter);
    }

    static INLINE GenRegister vec16(uint32_t file, ir::Register reg) {
      return GenRegister(file,
                         reg,
                         GEN_TYPE_F,
                         GEN_VERTICAL_STRIDE_8,
                         GEN_WIDTH_8,
                         GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenRegister vec8(uint32_t file, ir::Register reg) {
      return GenRegister(file,
                         reg,
                         GEN_TYPE_F,
                         GEN_VERTICAL_STRIDE_8,
                         GEN_WIDTH_8,
                         GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenRegister vec4(uint32_t file, ir::Register reg) {
      return GenRegister(file,
                         reg,
                         GEN_TYPE_F,
                         GEN_VERTICAL_STRIDE_4,
                         GEN_WIDTH_4,
                         GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenRegister vec2(uint32_t file, ir::Register reg) {
      return GenRegister(file,
                         reg,
                         GEN_TYPE_F,
                         GEN_VERTICAL_STRIDE_2,
                         GEN_WIDTH_2,
                         GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenRegister vec1(uint32_t file, ir::Register reg) {
      return GenRegister(file,
                         reg,
                         GEN_TYPE_F,
                         GEN_VERTICAL_STRIDE_0,
                         GEN_WIDTH_1,
                         GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE GenRegister retype(GenRegister reg, uint32_t type) {
      reg.type = type;
      return reg;
    }

    static INLINE GenRegister df16(uint32_t file, ir::Register reg) {
      return retype(vec4(file, reg), GEN_TYPE_DF);
    }

    static INLINE GenRegister df8(uint32_t file, ir::Register reg) {
      return retype(vec4(file, reg), GEN_TYPE_DF);
    }

    static INLINE GenRegister df1(uint32_t file, ir::Register reg) {
      return retype(vec1(file, reg), GEN_TYPE_DF);
    }

    /* Because we can not crossing row with horizontal stride, so for long
       type, we need to set it to <4,4:1>:UQ */
    static INLINE GenRegister ul16(uint32_t file, ir::Register reg) {
      return retype(vec4(file, reg), GEN_TYPE_UL);
    }

    static INLINE GenRegister ul8(uint32_t file, ir::Register reg) {
      return retype(vec4(file, reg), GEN_TYPE_UL);
    }

    static INLINE GenRegister ul1(uint32_t file, ir::Register reg) {
      return retype(vec1(file, reg), GEN_TYPE_UL);
    }

    static INLINE GenRegister ud16(uint32_t file, ir::Register reg) {
      return retype(vec16(file, reg), GEN_TYPE_UD);
    }

    static INLINE GenRegister ud8(uint32_t file, ir::Register reg) {
      return retype(vec8(file, reg), GEN_TYPE_UD);
    }

    static INLINE GenRegister ud1(uint32_t file, ir::Register reg) {
      return retype(vec1(file, reg), GEN_TYPE_UD);
    }

    static INLINE GenRegister d8(uint32_t file, ir::Register reg) {
      return retype(vec8(file, reg), GEN_TYPE_D);
    }

    static INLINE GenRegister uw16(uint32_t file, ir::Register reg) {
      return retype(vec16(file, reg), GEN_TYPE_UW);
    }

    static INLINE GenRegister uw8(uint32_t file, ir::Register reg) {
      return retype(vec8(file, reg), GEN_TYPE_UW);
    }

    static INLINE GenRegister uw1(uint32_t file, ir::Register reg) {
      return retype(vec1(file, reg), GEN_TYPE_UW);
    }

    static INLINE GenRegister ub16(uint32_t file, ir::Register reg) {
      return GenRegister(file,
                         reg,
                         GEN_TYPE_UB,
                         GEN_VERTICAL_STRIDE_16,
                         GEN_WIDTH_8,
                         GEN_HORIZONTAL_STRIDE_2);
    }

    static INLINE GenRegister ub8(uint32_t file, ir::Register reg) {
      return GenRegister(file,
                         reg,
                         GEN_TYPE_UB,
                         GEN_VERTICAL_STRIDE_16,
                         GEN_WIDTH_8,
                         GEN_HORIZONTAL_STRIDE_2);
    }

    static INLINE GenRegister ub1(uint32_t file, ir::Register reg) {
      return retype(vec1(file, reg), GEN_TYPE_UB);
    }

    static INLINE GenRegister unpacked_ud(ir::Register reg, bool uniform = false) {
      uint32_t width;
      uint32_t vstride;
      uint32_t hstride;

      if (uniform) {
        width = GEN_WIDTH_1;
        vstride = GEN_VERTICAL_STRIDE_0;
        hstride = GEN_HORIZONTAL_STRIDE_0;
      } else {
        width = GEN_WIDTH_4;
        vstride = GEN_VERTICAL_STRIDE_8;
        hstride = GEN_HORIZONTAL_STRIDE_2;
      }

      return GenRegister(GEN_GENERAL_REGISTER_FILE, reg,
                         GEN_TYPE_UD, vstride, width, hstride);
    }

    static INLINE GenRegister unpacked_uw(ir::Register reg, bool uniform = false,
                                          bool islong = false) {
      uint32_t width;
      uint32_t vstride;
      uint32_t hstride;

      if (uniform) {
        width = GEN_WIDTH_1;
        vstride = GEN_VERTICAL_STRIDE_0;
        hstride = GEN_HORIZONTAL_STRIDE_0;
      } else if (islong) {
        width = GEN_WIDTH_4;
        vstride = GEN_VERTICAL_STRIDE_16;
        hstride = GEN_HORIZONTAL_STRIDE_4;
      } else {
        width = GEN_WIDTH_8;
        vstride = GEN_VERTICAL_STRIDE_16;
        hstride = GEN_HORIZONTAL_STRIDE_2;
      }

      return GenRegister(GEN_GENERAL_REGISTER_FILE, reg,
                         GEN_TYPE_UW, vstride, width, hstride);
    }

    static INLINE GenRegister unpacked_ub(ir::Register reg, bool uniform = false) {
      return GenRegister(GEN_GENERAL_REGISTER_FILE,
                         reg,
                         GEN_TYPE_UB,
                         uniform ? GEN_VERTICAL_STRIDE_0 : GEN_VERTICAL_STRIDE_32,
                         uniform ? GEN_WIDTH_1 : GEN_WIDTH_8,
                         uniform ? GEN_HORIZONTAL_STRIDE_0 : GEN_HORIZONTAL_STRIDE_4);
    }

    static INLINE GenRegister imm(uint32_t type) {
      return GenRegister(GEN_IMMEDIATE_VALUE,
                         0,
                         0,
                         type,
                         GEN_VERTICAL_STRIDE_0,
                         GEN_WIDTH_1,
                         GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE GenRegister immuint64(uint64_t i) {
      GenRegister immediate = imm(GEN_TYPE_UL);
      immediate.value.u64 = i;
      return immediate;
    }

    static INLINE GenRegister immint64(int64_t i) {
      GenRegister immediate = imm(GEN_TYPE_L);
      immediate.value.i64 = i;
      return immediate;
    }

    static INLINE GenRegister immdf(double df) {
      GenRegister immediate = imm(GEN_TYPE_DF_IMM);
      immediate.value.df = df;
      return immediate;
    }

    static INLINE GenRegister immf(float f) {
      GenRegister immediate = imm(GEN_TYPE_F);
      immediate.value.f = f;
      return immediate;
    }

    static INLINE GenRegister immd(int d) {
      GenRegister immediate = imm(GEN_TYPE_D);
      immediate.value.d = d;
      return immediate;
    }

    static INLINE GenRegister immud(uint32_t ud) {
      GenRegister immediate = imm(GEN_TYPE_UD);
      immediate.value.ud = ud;
      return immediate;
    }

    static INLINE GenRegister immuw(uint16_t uw) {
      GenRegister immediate = imm(GEN_TYPE_UW);
      immediate.value.ud = uw;
      return immediate;
    }

    static INLINE GenRegister immw(int16_t w) {
      GenRegister immediate = imm(GEN_TYPE_W);
      immediate.value.d = w;
      return immediate;
    }

    static INLINE GenRegister immh(uint16_t uw) {
      GenRegister immediate = imm(GEN_TYPE_HF_IMM);
      immediate.value.ud = uw;
      return immediate;
    }

    static INLINE GenRegister immv(uint32_t v) {
      GenRegister immediate = imm(GEN_TYPE_V);
      immediate.vstride = GEN_VERTICAL_STRIDE_0;
      immediate.width = GEN_WIDTH_8;
      immediate.hstride = GEN_HORIZONTAL_STRIDE_1;
      immediate.value.ud = v;
      return immediate;
    }

    static INLINE GenRegister immvf(uint32_t v) {
      GenRegister immediate = imm(GEN_TYPE_VF);
      immediate.vstride = GEN_VERTICAL_STRIDE_0;
      immediate.width = GEN_WIDTH_4;
      immediate.hstride = GEN_HORIZONTAL_STRIDE_1;
      immediate.value.ud = v;
      return immediate;
    }

    static INLINE GenRegister immvf4(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3) {
      GenRegister immediate = imm(GEN_TYPE_VF);
      immediate.vstride = GEN_VERTICAL_STRIDE_0;
      immediate.width = GEN_WIDTH_4;
      immediate.hstride = GEN_HORIZONTAL_STRIDE_1;
      immediate.value.ud = ((v0 << 0) | (v1 << 8) | (v2 << 16) | (v3 << 24));
      return immediate;
    }

    static INLINE GenRegister f1grf(ir::Register reg) {
      return vec1(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister f2grf(ir::Register reg) {
      return vec2(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister f4grf(ir::Register reg) {
      return vec4(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister f8grf(ir::Register reg) {
      return vec8(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister f16grf(ir::Register reg) {
      return vec16(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister df1grf(ir::Register reg) {
      return df1(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister df8grf(ir::Register reg) {
      return df8(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister df16grf(ir::Register reg) {
      return df16(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister ul16grf(ir::Register reg) {
      return ul16(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister ul8grf(ir::Register reg) {
      return ul8(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister ul1grf(ir::Register reg) {
      return ul1(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister ud16grf(ir::Register reg) {
      return ud16(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister ud8grf(ir::Register reg) {
      return ud8(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister ud1grf(ir::Register reg) {
      return ud1(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister uw1grf(ir::Register reg) {
      return uw1(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister uw8grf(ir::Register reg) {
      return uw8(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister uw16grf(ir::Register reg) {
      return uw16(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister ub1grf(ir::Register reg) {
      return ub1(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister ub8grf(ir::Register reg) {
      return ub8(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister ub16grf(ir::Register reg) {
      return ub16(GEN_GENERAL_REGISTER_FILE, reg);
    }

    static INLINE GenRegister null(void) {
      return GenRegister(GEN_ARCHITECTURE_REGISTER_FILE,
                         GEN_ARF_NULL,
                         0,
                         GEN_TYPE_F,
                         GEN_VERTICAL_STRIDE_8,
                         GEN_WIDTH_8,
                         GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenRegister nullud(void) {
      return GenRegister(GEN_ARCHITECTURE_REGISTER_FILE,
                         GEN_ARF_NULL,
                         0,
                         GEN_TYPE_UD,
                         GEN_VERTICAL_STRIDE_8,
                         GEN_WIDTH_8,
                         GEN_HORIZONTAL_STRIDE_1);
    }


    static INLINE bool isNull(GenRegister reg) {
      return (reg.file == GEN_ARCHITECTURE_REGISTER_FILE
              && reg.nr == GEN_ARF_NULL);
    }

    static INLINE GenRegister vec1(GenRegister reg) {
      reg.width = GEN_WIDTH_1;
      reg.hstride = GEN_HORIZONTAL_STRIDE_0;
      reg.vstride = GEN_VERTICAL_STRIDE_0;
      return reg;
    }

    static INLINE GenRegister tm0(void) {
      return GenRegister(GEN_ARCHITECTURE_REGISTER_FILE,
                         0xc0,
                         0,
                         GEN_TYPE_UW,
                         GEN_VERTICAL_STRIDE_4,
                         GEN_WIDTH_4,
                         GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenRegister acc(void) {
      return GenRegister(GEN_ARCHITECTURE_REGISTER_FILE,
                         GEN_ARF_ACCUMULATOR,
                         0,
                         GEN_TYPE_F,
                         GEN_VERTICAL_STRIDE_8,
                         GEN_WIDTH_8,
                         GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenRegister ip(void) {
      return GenRegister(GEN_ARCHITECTURE_REGISTER_FILE,
                         GEN_ARF_IP,
                         0,
                         GEN_TYPE_D,
                         GEN_VERTICAL_STRIDE_4,
                         GEN_WIDTH_1,
                         GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE GenRegister sr(uint32_t nr, uint32_t subnr = 0) {
      return GenRegister(GEN_ARCHITECTURE_REGISTER_FILE,
                         GEN_ARF_STATE | nr,
                         subnr,
                         GEN_TYPE_UD,
                         GEN_VERTICAL_STRIDE_8,
                         GEN_WIDTH_8,
                         GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenRegister notification0(uint32_t subnr) {
      return GenRegister(GEN_ARCHITECTURE_REGISTER_FILE,
                         GEN_ARF_NOTIFICATION_COUNT,
                         subnr,
                         GEN_TYPE_UD,
                         GEN_VERTICAL_STRIDE_0,
                         GEN_WIDTH_1,
                         GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE GenRegister flag(uint32_t nr, uint32_t subnr) {
      return GenRegister(GEN_ARCHITECTURE_REGISTER_FILE,
                         GEN_ARF_FLAG | nr,
                         subnr,
                         GEN_TYPE_UW,
                         GEN_VERTICAL_STRIDE_0,
                         GEN_WIDTH_1,
                         GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE GenRegister next(GenRegister reg) {
      if (reg.physical)
        reg.nr++;
      else
        reg.quarter++;
      return reg;
    }

    /*! Build an indirectly addressed source */
    static INLINE GenRegister indirect(uint32_t type, uint32_t subnr, uint32_t width,
                                        uint32_t vstride, uint32_t hstride) {
      GenRegister reg;
      reg.type = type;
      reg.file = GEN_GENERAL_REGISTER_FILE;
      reg.address_mode = GEN_ADDRESS_REGISTER_INDIRECT_REGISTER;
      reg.width = width;
      reg.a0_subnr = subnr;
      reg.nr = 0;
      reg.addr_imm = 0;
      reg.negation = 0;
      reg.absolute = 0;
      reg.vstride = vstride;
      reg.hstride = hstride;
      return reg;
    }

    /*! convert one register to indirectly mode */
    static INLINE GenRegister to_indirect1xN(GenRegister reg, uint32_t base_addr,
                                          int32_t imm_off = 4096, int a0_subnr = 0) {
      GenRegister r = reg;
      int32_t offset;
      if (imm_off > 4095) {
        offset = (r.nr*32 + r.subnr) - base_addr;
      } else {
        offset = imm_off;
      }

      GBE_ASSERT(offset <= 511 && offset>=-512);
      r.a0_subnr = a0_subnr;
      r.addr_imm = offset;
      r.address_mode = GEN_ADDRESS_REGISTER_INDIRECT_REGISTER;

      r.width = GEN_WIDTH_1;
      r.vstride = GEN_VERTICAL_STRIDE_ONE_DIMENSIONAL;
      r.hstride = GEN_HORIZONTAL_STRIDE_0;
      return r;
    }

    static INLINE GenRegister vec16(uint32_t file, uint32_t nr, uint32_t subnr) {
      return GenRegister(file,
                         nr,
                         subnr,
                         GEN_TYPE_F,
                         GEN_VERTICAL_STRIDE_8,
                         GEN_WIDTH_8,
                         GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenRegister vec8(uint32_t file, uint32_t nr, uint32_t subnr) {
      return GenRegister(file,
                         nr,
                         subnr,
                         GEN_TYPE_F,
                         GEN_VERTICAL_STRIDE_8,
                         GEN_WIDTH_8,
                         GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenRegister vec4(uint32_t file, uint32_t nr, uint32_t subnr) {
      return GenRegister(file,
                         nr,
                         subnr,
                         GEN_TYPE_F,
                         GEN_VERTICAL_STRIDE_4,
                         GEN_WIDTH_4,
                         GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenRegister vec2(uint32_t file, uint32_t nr, uint32_t subnr) {
      return GenRegister(file,
                    nr,
                    subnr,
                    GEN_TYPE_F,
                    GEN_VERTICAL_STRIDE_2,
                    GEN_WIDTH_2,
                    GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenRegister vec1(uint32_t file, uint32_t nr, uint32_t subnr) {
      return GenRegister(file,
                    nr,
                    subnr,
                    GEN_TYPE_F,
                    GEN_VERTICAL_STRIDE_0,
                    GEN_WIDTH_1,
                    GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE uint32_t hstrideFromSize(int size) {
      switch (size) {
        case 0: return GEN_HORIZONTAL_STRIDE_0;
        case 1: return GEN_HORIZONTAL_STRIDE_1;
        case 2: return GEN_HORIZONTAL_STRIDE_2;
        case 4: return GEN_HORIZONTAL_STRIDE_4;
        default: NOT_IMPLEMENTED; return GEN_HORIZONTAL_STRIDE_0;
      }
    }

    static INLINE int hstride_size(GenRegister reg) {
      switch (reg.hstride) {
        case GEN_HORIZONTAL_STRIDE_0: return 0;
        case GEN_HORIZONTAL_STRIDE_1: return 1;
        case GEN_HORIZONTAL_STRIDE_2: return 2;
        case GEN_HORIZONTAL_STRIDE_4: return 4;
        default: NOT_IMPLEMENTED; return 0;
      }
    }

    static INLINE int vstride_size(GenRegister reg) {
      switch (reg.vstride) {
        case GEN_VERTICAL_STRIDE_0: return 0;
        case GEN_VERTICAL_STRIDE_1: return 1;
        case GEN_VERTICAL_STRIDE_2: return 2;
        case GEN_VERTICAL_STRIDE_4: return 4;
        case GEN_VERTICAL_STRIDE_8: return 8;
        case GEN_VERTICAL_STRIDE_16: return 16;
        case GEN_VERTICAL_STRIDE_32: return 32;
        case GEN_VERTICAL_STRIDE_64: return 64;
        case GEN_VERTICAL_STRIDE_128: return 128;
        case GEN_VERTICAL_STRIDE_256: return 256;
        default: NOT_IMPLEMENTED; return 0;
      }
    }

    static INLINE int width_size(GenRegister reg) {
      switch (reg.width) {
        case GEN_WIDTH_1: return 1;
        case GEN_WIDTH_2: return 2;
        case GEN_WIDTH_4: return 4;
        case GEN_WIDTH_8: return 8;
        case GEN_WIDTH_16: return 16;
        case GEN_WIDTH_32: return 32;
        default: NOT_IMPLEMENTED; return 0;
      }
    }

    static INLINE GenRegister suboffset(GenRegister reg, uint32_t delta) {
      if (reg.hstride != GEN_HORIZONTAL_STRIDE_0) {
        reg.subnr += delta * typeSize(reg.type) * hstride_size(reg);
        reg.nr += reg.subnr / 32;
        reg.subnr %= 32;
      }
      return reg;
    }

    static INLINE GenRegister subphysicaloffset(GenRegister reg, uint32_t delta) {
      if (reg.hstride != GEN_HORIZONTAL_STRIDE_0) {
        reg.subnr += delta * typeSize(reg.type) * hstride_size(reg);
        reg.subphysical = 1;
      }
      return reg;
    }

    static INLINE GenRegister df16(uint32_t file, uint32_t nr, uint32_t subnr) {
      return retype(vec16(file, nr, subnr), GEN_TYPE_DF);
    }

    static INLINE GenRegister df8(uint32_t file, uint32_t nr, uint32_t subnr) {
      return retype(vec8(file, nr, subnr), GEN_TYPE_DF);
    }

    static INLINE GenRegister df1(uint32_t file, uint32_t nr, uint32_t subnr) {
      return retype(vec1(file, nr, subnr), GEN_TYPE_DF);
    }

    static INLINE GenRegister ul16(uint32_t file, uint32_t nr, uint32_t subnr) {
      return retype(vec4(file, nr, subnr), GEN_TYPE_UL);
    }

    static INLINE GenRegister ul8(uint32_t file, uint32_t nr, uint32_t subnr) {
      return retype(vec4(file, nr, subnr), GEN_TYPE_UL);
    }

    static INLINE GenRegister ul1(uint32_t file, uint32_t nr, uint32_t subnr) {
      return retype(vec1(file, nr, subnr), GEN_TYPE_UL);
    }

    static INLINE GenRegister ud16(uint32_t file, uint32_t nr, uint32_t subnr) {
      return retype(vec16(file, nr, subnr), GEN_TYPE_UD);
    }

    static INLINE GenRegister ud8(uint32_t file, uint32_t nr, uint32_t subnr) {
      return retype(vec8(file, nr, subnr), GEN_TYPE_UD);
    }

    static INLINE GenRegister ud1(uint32_t file, uint32_t nr, uint32_t subnr) {
      return retype(vec1(file, nr, subnr), GEN_TYPE_UD);
    }

    static INLINE GenRegister d8(uint32_t file, uint32_t nr, uint32_t subnr) {
      return retype(vec8(file, nr, subnr), GEN_TYPE_D);
    }

    static INLINE GenRegister uw16(uint32_t file, uint32_t nr, uint32_t subnr) {
      return suboffset(retype(vec16(file, nr, 0), GEN_TYPE_UW), subnr);
    }

    static INLINE GenRegister uw8(uint32_t file, uint32_t nr, uint32_t subnr) {
      return suboffset(retype(vec8(file, nr, 0), GEN_TYPE_UW), subnr);
    }

    static INLINE GenRegister uw1(uint32_t file, uint32_t nr, uint32_t subnr) {
      return GenRegister(file,
                         nr,
                         subnr,
                         GEN_TYPE_UW,
                         GEN_VERTICAL_STRIDE_0,
                         GEN_WIDTH_1,
                         GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE GenRegister ub16(uint32_t file, uint32_t nr, uint32_t subnr) {
      return GenRegister(file,
                         nr,
                         subnr,
                         GEN_TYPE_UB,
                         GEN_VERTICAL_STRIDE_16,
                         GEN_WIDTH_8,
                         GEN_HORIZONTAL_STRIDE_2);
    }

    static INLINE GenRegister ub8(uint32_t file, uint32_t nr, uint32_t subnr) {
      return GenRegister(file,
                         nr,
                         subnr,
                         GEN_TYPE_UB,
                         GEN_VERTICAL_STRIDE_16,
                         GEN_WIDTH_8,
                         GEN_HORIZONTAL_STRIDE_2);
    }

    static INLINE GenRegister ub1(uint32_t file, uint32_t nr, uint32_t subnr) {
      return GenRegister(file,
                         nr,
                         subnr,
                         GEN_TYPE_UB,
                         GEN_VERTICAL_STRIDE_0,
                         GEN_WIDTH_1,
                         GEN_HORIZONTAL_STRIDE_0);
    }

    static INLINE GenRegister f1grf(uint32_t nr, uint32_t subnr) {
      return vec1(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister f2grf(uint32_t nr, uint32_t subnr) {
      return vec2(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister f4grf(uint32_t nr, uint32_t subnr) {
      return vec4(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister f8grf(uint32_t nr, uint32_t subnr) {
      return vec8(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister f16grf(uint32_t nr, uint32_t subnr) {
      return vec16(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister df16grf(uint32_t nr, uint32_t subnr) {
      return df16(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister df8grf(uint32_t nr, uint32_t subnr) {
      return df8(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister df1grf(uint32_t nr, uint32_t subnr) {
      return df1(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister ul16grf(uint32_t nr, uint32_t subnr) {
      return ul16(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister ul8grf(uint32_t nr, uint32_t subnr) {
      return ul8(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister ul1grf(uint32_t nr, uint32_t subnr) {
      return ul1(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister ud16grf(uint32_t nr, uint32_t subnr) {
      return ud16(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister ud8grf(uint32_t nr, uint32_t subnr) {
      return ud8(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister ud1grf(uint32_t nr, uint32_t subnr) {
      return ud1(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister ud1arf(uint32_t nr, uint32_t subnr) {
      return ud1(GEN_ARCHITECTURE_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister uw1grf(uint32_t nr, uint32_t subnr) {
      return uw1(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister uw8grf(uint32_t nr, uint32_t subnr) {
      return uw8(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister uw16grf(uint32_t nr, uint32_t subnr) {
      return uw16(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister ub1grf(uint32_t nr, uint32_t subnr) {
      return ub1(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister ub8grf(uint32_t nr, uint32_t subnr) {
      return ub8(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister ub16grf(uint32_t nr, uint32_t subnr) {
      return ub16(GEN_GENERAL_REGISTER_FILE, nr, subnr);
    }

    static INLINE GenRegister unpacked_uw(uint32_t nr, uint32_t subnr) {
      return GenRegister(GEN_GENERAL_REGISTER_FILE,
                         nr,
                         subnr,
                         GEN_TYPE_UW,
                         GEN_VERTICAL_STRIDE_16,
                         GEN_WIDTH_8,
                         GEN_HORIZONTAL_STRIDE_2);
    }

    static INLINE GenRegister unpacked_uw(const GenRegister& reg) {
      uint32_t nr = reg.nr;
      uint32_t subnr = reg.subnr / typeSize(GEN_TYPE_UW);
      uint32_t width = reg.width;
      int hstrideSize = GenRegister::hstride_size(reg) * typeSize(reg.type) / typeSize(GEN_TYPE_UW);
      uint32_t hstride = GenRegister::hstrideFromSize(hstrideSize);

      return GenRegister(GEN_GENERAL_REGISTER_FILE,
                         nr,
                         subnr,
                         GEN_TYPE_UW,
                         GEN_VERTICAL_STRIDE_16,
                         width,
                         hstride);
    }

    static INLINE GenRegister packed_ud(uint32_t nr, uint32_t subnr) {
      return GenRegister(GEN_GENERAL_REGISTER_FILE,
                         nr,
                         subnr,
                         GEN_TYPE_UD,
                         GEN_VERTICAL_STRIDE_8,
                         GEN_WIDTH_4,
                         GEN_HORIZONTAL_STRIDE_1);
    }

    static INLINE GenRegister unpacked_ud(uint32_t nr, uint32_t subnr) {
      return GenRegister(GEN_GENERAL_REGISTER_FILE,
                         nr,
                         subnr,
                         GEN_TYPE_UD,
                         GEN_VERTICAL_STRIDE_8,
                         GEN_WIDTH_4,
                         GEN_HORIZONTAL_STRIDE_2);
    }

    static INLINE GenRegister mask(uint32_t subnr) {
      return uw1(GEN_ARCHITECTURE_REGISTER_FILE, GEN_ARF_MASK, subnr);
    }

    static INLINE GenRegister addr1(uint32_t subnr) {
      return uw1(GEN_ARCHITECTURE_REGISTER_FILE, GEN_ARF_ADDRESS, subnr);
    }

    static INLINE GenRegister addr8(uint32_t subnr) {
      return uw8(GEN_ARCHITECTURE_REGISTER_FILE, GEN_ARF_ADDRESS, subnr);
    }

    static INLINE GenRegister negate(GenRegister reg) {
      if (reg.file != GEN_IMMEDIATE_VALUE)
        reg.negation ^= 1;
      else {
        if (reg.type == GEN_TYPE_F)
          reg.value.f = -reg.value.f;
        else if (reg.type == GEN_TYPE_UD)
          reg.value.ud = -reg.value.ud;
        else if (reg.type == GEN_TYPE_D)
          reg.value.d = -reg.value.d;
        else if (reg.type == GEN_TYPE_UW) {
          const uint16_t uw = reg.value.ud & 0xffff;
          reg = GenRegister::immuw(-uw);
        } else if (reg.type == GEN_TYPE_W) {
          const uint16_t uw = reg.value.ud & 0xffff;
          reg = GenRegister::immw(-(int16_t)uw);
        } else if (reg.type == GEN_TYPE_HF_IMM) {
          const uint16_t uw = reg.value.ud & 0xffff;
          reg = GenRegister::immh(uw ^ 0x8000);
        } else if (reg.type == GEN_TYPE_DF_IMM) {
          reg.value.df = -reg.value.df;
        } else
          NOT_SUPPORTED;
      }
      return reg;
    }

    static INLINE GenRegister abs(GenRegister reg) {
      reg.absolute = 1;
      reg.negation = 0;
      return reg;
    }

    static INLINE void propagateRegister(GenRegister& dst, const GenRegister& src)
    {
      dst.type = src.type;
      dst.file = src.file;
      dst.physical = src.physical;
      dst.subphysical = src.subphysical;
      dst.value.reg = src.value.reg;
      dst.vstride = src.vstride;
      dst.width = src.width;
      dst.hstride = src.hstride;
      dst.quarter = src.quarter;
      dst.nr = src.nr;
      dst.subnr = src.subnr;
      dst.address_mode = src.address_mode;
      dst.a0_subnr = src.a0_subnr;
      dst.addr_imm = src.addr_imm;

      dst.negation = dst.negation ^ src.negation;
      dst.absolute = dst.absolute | src.absolute;
    }

    /*! Generate register encoding with run-time simdWidth */
#define DECL_REG_ENCODER(NAME, SIMD16, SIMD8, SIMD1) \
    template <typename... Args> \
    static INLINE GenRegister NAME(uint32_t simdWidth, Args... values) { \
      if (simdWidth == 16) \
        return SIMD16(values...); \
      else if (simdWidth == 8) \
        return SIMD8(values...); \
      else if (simdWidth == 1) \
        return SIMD1(values...); \
      else { \
        NOT_IMPLEMENTED; \
        return SIMD1(values...); \
      } \
    }
    // TODO: Should add native long type here.
    DECL_REG_ENCODER(dfxgrf, df16grf, df8grf, df1grf);
    DECL_REG_ENCODER(fxgrf, f16grf, f8grf, f1grf);
    DECL_REG_ENCODER(uwxgrf, uw16grf, uw8grf, uw1grf);
    DECL_REG_ENCODER(udxgrf, ud16grf, ud8grf, ud1grf);
#undef DECL_REG_ENCODER
  };
} /* namespace gbe */

#endif /* __GEN_REGISTER_HPP__ */


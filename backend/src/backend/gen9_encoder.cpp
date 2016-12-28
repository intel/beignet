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

#include "backend/gen9_encoder.hpp"
#include "backend/gen9_instruction.hpp"
static const uint32_t untypedRWMask[] = {
  GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE|GEN_UNTYPED_GREEN|GEN_UNTYPED_RED,
  GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE|GEN_UNTYPED_GREEN,
  GEN_UNTYPED_ALPHA|GEN_UNTYPED_BLUE,
  GEN_UNTYPED_ALPHA,
  0
};

namespace gbe
{
  void Gen9Encoder::SAMPLE(GenRegister dest,
                          GenRegister msg,
                          unsigned int msg_len,
                          bool header_present,
                          unsigned char bti,
                          unsigned char sampler,
                          uint32_t simdWidth,
                          uint32_t writemask,
                          uint32_t return_format,
                          bool isLD,
                          bool isUniform)
  {
     if (writemask == 0) return;
     uint32_t msg_type = isLD ? GEN_SAMPLER_MESSAGE_SIMD8_LD :
                                GEN_SAMPLER_MESSAGE_SIMD8_SAMPLE;
     uint32_t response_length = (4 * (simdWidth / 8));
     uint32_t msg_length = (msg_len * (simdWidth / 8));
     if (header_present)
       msg_length++;
     uint32_t simd_mode = (simdWidth == 16) ?
                            GEN_SAMPLER_SIMD_MODE_SIMD16 : GEN_SAMPLER_SIMD_MODE_SIMD8;
    if(isUniform) {
      response_length = 1;
      msg_type = GEN_SAMPLER_MESSAGE_SIMD4X2_LD;
      msg_length = 1;
      simd_mode = GEN_SAMPLER_SIMD_MODE_SIMD8;
    }
     GenNativeInstruction *insn = this->next(GEN_OPCODE_SEND);
     this->setHeader(insn);
     this->setDst(insn, dest);
     this->setSrc0(insn, msg);
     this->setSrc1(insn, GenRegister::immud(0));
     setSamplerMessage(insn, bti, sampler, msg_type,
                       response_length, msg_length,
                       header_present,
                       simd_mode, return_format);
  }

  void Gen9Encoder::setSendsOperands(Gen9NativeInstruction *gen9_insn, GenRegister dst, GenRegister src0, GenRegister src1)
  {
    assert(dst.subnr == 0 && src0.subnr == 0 && src1.subnr == 0);

    if (dst.file == GEN_ARCHITECTURE_REGISTER_FILE)
      gen9_insn->bits1.sends.dest_reg_file_0 = 0;
    else if (dst.file == GEN_GENERAL_REGISTER_FILE)
      gen9_insn->bits1.sends.dest_reg_file_0 = 1;
    else
      NOT_SUPPORTED;

    gen9_insn->bits1.sends.src1_reg_file_0 = 1;
    gen9_insn->bits1.sends.src1_reg_nr = src1.nr;
    gen9_insn->bits1.sends.dest_subreg_nr = 0;
    gen9_insn->bits1.sends.dest_reg_nr = dst.nr;
    gen9_insn->bits1.sends.dest_address_mode = 0;  //direct mode
    gen9_insn->bits2.sends.src0_subreg_nr = 0;
    gen9_insn->bits2.sends.src0_reg_nr = src0.nr;
    gen9_insn->bits2.sends.src0_address_mode = 0;
  }

  unsigned Gen9Encoder::setUntypedWriteSendsMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemNum)
  {
    uint32_t msg_length = 0;
    uint32_t response_length = 0;
    if (this->curr.execWidth == 8) {
      msg_length = 1;
    } else if (this->curr.execWidth == 16) {
      msg_length = 2;
    }
    else
      NOT_IMPLEMENTED;
    setDPUntypedRW(insn,
                   bti,
                   untypedRWMask[elemNum],
                   GEN75_P1_UNTYPED_SURFACE_WRITE,
                   msg_length,
                   response_length);
    return insn->bits3.ud;
  }

  void Gen9Encoder::UNTYPED_WRITE(GenRegister addr, GenRegister data, GenRegister bti, uint32_t elemNum, bool useSends)
  {
    if (!useSends)
      Gen8Encoder::UNTYPED_WRITE(addr, data, bti, elemNum, false);
    else {
      GBE_ASSERT(addr.reg() != data.reg());

      GenNativeInstruction *insn = this->next(GEN_OPCODE_SENDS);
      Gen9NativeInstruction *gen9_insn = &insn->gen9_insn;
      assert(elemNum >= 1 || elemNum <= 4);

      this->setHeader(insn);
      insn->header.destreg_or_condmod = GEN_SFID_DATAPORT1_DATA;

      setSendsOperands(gen9_insn, GenRegister::null(), addr, data);
      if (this->curr.execWidth == 8)
        gen9_insn->bits2.sends.src1_length = elemNum;
      else if (this->curr.execWidth == 16)
        gen9_insn->bits2.sends.src1_length = 2 * elemNum;
      else
        NOT_SUPPORTED;

      if (bti.file == GEN_IMMEDIATE_VALUE) {
        gen9_insn->bits2.sends.sel_reg32_desc = 0;
        setUntypedWriteSendsMessageDesc(insn, bti.value.ud, elemNum);
      } else
        gen9_insn->bits2.sends.sel_reg32_desc = 1;
    }
  }

  void Gen9Encoder::TYPED_WRITE(GenRegister header, GenRegister data, bool header_present, unsigned char bti, bool useSends)
  {
    if (!useSends)
      Gen8Encoder::TYPED_WRITE(header, data, header_present, bti, false);
    else {
      GBE_ASSERT(header.reg() != data.reg());

      GenNativeInstruction *insn = this->next(GEN_OPCODE_SENDS);
      Gen9NativeInstruction *gen9_insn = &insn->gen9_insn;
      assert(header_present);

      this->setHeader(insn);
      insn->header.destreg_or_condmod = GEN_SFID_DATAPORT1_DATA;

      setSendsOperands(gen9_insn, GenRegister::null(), header, data);
      gen9_insn->bits2.sends.src1_length = 4;   //src0_length: 5(header+u+v+w+lod), src1_length: 4(data)

      gen9_insn->bits2.sends.sel_reg32_desc = 0;
      setTypedWriteMessage(insn, bti, GEN_TYPED_WRITE, 5, header_present);
    }
  }

  unsigned Gen9Encoder::setByteScatterSendsMessageDesc(GenNativeInstruction *insn, unsigned bti, unsigned elemSize)
  {
    uint32_t msg_length = 0;
    uint32_t response_length = 0;
    if (this->curr.execWidth == 8) {
      msg_length = 1;
    } else if (this->curr.execWidth == 16) {
      msg_length = 2;
    } else
      NOT_IMPLEMENTED;

    setDPByteScatterGather(insn,
                           bti,
                           elemSize,
                           GEN7_BYTE_SCATTER,
                           msg_length,
                           response_length);
    return insn->bits3.ud;
  }

  void Gen9Encoder::BYTE_SCATTER(GenRegister addr, GenRegister data, GenRegister bti, uint32_t elemSize, bool useSends)
  {
    if (!useSends)
      Gen8Encoder::BYTE_SCATTER(addr, data, bti, elemSize, false);
    else {
      GBE_ASSERT(addr.reg() != data.reg());

      GenNativeInstruction *insn = this->next(GEN_OPCODE_SENDS);
      Gen9NativeInstruction *gen9_insn = &insn->gen9_insn;

      this->setHeader(insn);
      insn->header.destreg_or_condmod = GEN_SFID_DATAPORT_DATA;

      setSendsOperands(gen9_insn, GenRegister::null(), addr, data);
      if (this->curr.execWidth == 8)
        gen9_insn->bits2.sends.src1_length = 1;
      else if (this->curr.execWidth == 16)
        gen9_insn->bits2.sends.src1_length = 2;
      else
        NOT_SUPPORTED;

      if (bti.file == GEN_IMMEDIATE_VALUE) {
        gen9_insn->bits2.sends.sel_reg32_desc = 0;
        setByteScatterSendsMessageDesc(insn, bti.value.ud, elemSize);
      } else
        gen9_insn->bits2.sends.sel_reg32_desc = 1;
    }
  }

  void Gen9Encoder::ATOMIC(GenRegister dst, uint32_t function, GenRegister addr, GenRegister data, GenRegister bti, uint32_t srcNum, bool useSends)
  {
    if (!useSends)
      Gen8Encoder::ATOMIC(dst, function, addr, data, bti, srcNum, false);
    else {
      GBE_ASSERT(addr.reg() != data.reg());

      GenNativeInstruction *insn = this->next(GEN_OPCODE_SENDS);
      Gen9NativeInstruction *gen9_insn = &insn->gen9_insn;
      this->setHeader(insn);
      insn->header.destreg_or_condmod = GEN_SFID_DATAPORT1_DATA;

      setSendsOperands(gen9_insn, dst, addr, data);
      if (this->curr.execWidth == 8)
        gen9_insn->bits2.sends.src1_length = srcNum - 1;
      else if (this->curr.execWidth == 16)
        gen9_insn->bits2.sends.src1_length = 2 * (srcNum - 1);
      else
        NOT_SUPPORTED;

      if (bti.file == GEN_IMMEDIATE_VALUE) {
        gen9_insn->bits2.sends.sel_reg32_desc = 0;
        setAtomicMessageDesc(insn, function, bti.value.ud, 1);
      } else
        gen9_insn->bits2.sends.sel_reg32_desc = 1;
    }
  }

  void Gen9Encoder::OBWRITE(GenRegister header, GenRegister data, uint32_t bti, uint32_t ow_size, bool useSends)
  {
    if (!useSends)
      Gen8Encoder::OBWRITE(header, data, bti, ow_size, false);
    else {
      GBE_ASSERT(data.reg() != header.reg());
      GenNativeInstruction *insn = this->next(GEN_OPCODE_SENDS);
      Gen9NativeInstruction *gen9_insn = &insn->gen9_insn;

      this->setHeader(insn);
      insn->header.destreg_or_condmod = GEN_SFID_DATAPORT_DATA;

      setSendsOperands(gen9_insn, GenRegister::null(), header, data);

      uint32_t dataRegs = ow_size / 2;
      // half reg should also have size 1
      if (dataRegs == 0)
        dataRegs = 1;
      gen9_insn->bits2.sends.src1_length = dataRegs;

      const uint32_t block_size = getOBlockSize(ow_size);
      const uint32_t msg_length = 1;
      const uint32_t response_length = 0;
      setOBlockRW(insn,
                bti,
                block_size,
                GEN7_OBLOCK_WRITE,
                msg_length,
                response_length);
    }
  }

  void Gen9Encoder::MBWRITE(GenRegister header, GenRegister data, uint32_t bti, uint32_t data_size, bool useSends)
  {
    if (!useSends)
      Gen8Encoder::MBWRITE(header, data, bti, data_size, false);
    else {
      GBE_ASSERT(data.reg() != header.reg());
      GenNativeInstruction *insn = this->next(GEN_OPCODE_SENDS);
      Gen9NativeInstruction *gen9_insn = &insn->gen9_insn;

      this->setHeader(insn);
      insn->header.destreg_or_condmod = GEN_SFID_DATAPORT_DATA;

      setSendsOperands(gen9_insn, GenRegister::null(), header, data);
      gen9_insn->bits2.sends.src1_length = data_size;

      const uint32_t msg_length = 1;
      const uint32_t response_length = 0;
      setMBlockRW(insn,
                bti,
                GEN75_P1_MEDIA_TYPED_BWRITE,
                msg_length,
                response_length);
    }
  }
} /* End of the name space. */

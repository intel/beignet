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
 */

/**
 * \file gen9_context.cpp
 */

#include "backend/gen9_context.hpp"
#include "backend/gen_insn_selection.hpp"
#include "backend/gen_program.hpp"

namespace gbe
{
  void Gen9Context::newSelection(void) {
    this->sel = GBE_NEW(Selection9, *this);
  }

  void Gen9Context::emitBarrierInstruction(const SelectionInstruction &insn) {
    const GenRegister src = ra->genReg(insn.src(0));
    const GenRegister fenceDst = ra->genReg(insn.dst(0));
    uint32_t barrierType = insn.extra.barrierType;
    const GenRegister barrierId = ra->genReg(GenRegister::ud1grf(ir::ocl::barrierid));
    bool imageFence = barrierType & ir::SYNC_IMAGE_FENCE;

    if (barrierType & ir::SYNC_GLOBAL_READ_FENCE) {
      p->FENCE(fenceDst, imageFence);
      p->MOV(fenceDst, fenceDst);
    }
    p->push();
      // As only the payload.2 is used and all the other regions are ignored
      // SIMD8 mode here is safe.
      p->curr.execWidth = 8;
      p->curr.physicalFlag = 0;
      p->curr.noMask = 1;
      // Copy barrier id from r0.
      p->AND(src, barrierId, GenRegister::immud(0x8f000000));
      // A barrier is OK to start the thread synchronization *and* SLM fence
      p->BARRIER(src);
      p->curr.execWidth = 1;
      // Now we wait for the other threads
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->WAIT();
    p->pop();
    if (imageFence) {
      p->FLUSH_SAMPLERCACHE(fenceDst);
      p->MOV(fenceDst, fenceDst);
    }
  }

  void BxtContext::newSelection(void) {
    this->sel = GBE_NEW(SelectionBxt, *this);
  }

  void BxtContext::calculateFullU64MUL(GenRegister src0, GenRegister src1, GenRegister dst_h,
                                             GenRegister dst_l, GenRegister s0l_s1h, GenRegister s0h_s1l)
  {
    src0.type = src1.type = GEN_TYPE_UD;
    dst_h.type = dst_l.type = GEN_TYPE_UL;
    s0l_s1h.type = s0h_s1l.type = GEN_TYPE_UL;

    //GenRegister tmp;

    GenRegister s0l = unpacked_ud(src0);
    GenRegister s1l = unpacked_ud(src1);
    GenRegister s0h = unpacked_ud(s0l_s1h); //s0h only used before s0l_s1h, reuse s0l_s1h
    GenRegister s1h = unpacked_ud(dst_l); //s1h only used before dst_l, reuse dst_l

    p->MOV(s0h, GenRegister::offset(s0l, 0, 4));
    p->MOV(s1h, GenRegister::offset(s1l, 0, 4));

    /* High 32 bits X High 32 bits. */
    p->MUL(dst_h, s0h, s1h);
    /* High 32 bits X low 32 bits. */
    p->MUL(s0h_s1l, s0h, s1l);
    /* Low 32 bits X high 32 bits. */
    p->MUL(s0l_s1h, s0l, s1h);
    /* Low 32 bits X low 32 bits. */
    p->MUL(dst_l, s0l, s1l);

    /*  Because the max product of s0l*s1h is (2^N - 1) * (2^N - 1) = 2^2N + 1 - 2^(N+1), here N = 32
        The max of addding 2 32bits integer to it is
        2^2N + 1 - 2^(N+1) + 2*(2^N - 1) = 2^2N - 1
        which means the product s0h_s1l adds dst_l's high 32 bits and then adds s0l_s1h's low 32 bits will not
        overflow and have no carry.
        By this manner, we can avoid using acc register, which has a lot of restrictions. */

    GenRegister s0l_s1h_l = unpacked_ud(s0l_s1h);
    p->ADD(s0h_s1l, s0h_s1l, s0l_s1h_l);

    p->SHR(s0l_s1h, s0l_s1h, GenRegister::immud(32));
    GenRegister s0l_s1h_h = unpacked_ud(s0l_s1h);
    p->ADD(dst_h, dst_h, s0l_s1h_h);

    GenRegister dst_l_h = unpacked_ud(s0l_s1h);
    p->MOV(dst_l_h, unpacked_ud(dst_l, 1));
    p->ADD(s0h_s1l, s0h_s1l, dst_l_h);

    // No longer need s0l_s1h
    GenRegister tmp = s0l_s1h;

    p->SHL(tmp, s0h_s1l, GenRegister::immud(32));
    GenRegister tmp_unpacked = unpacked_ud(tmp, 1);
    p->MOV(unpacked_ud(dst_l, 1), tmp_unpacked);

    p->SHR(tmp, s0h_s1l, GenRegister::immud(32));
    p->ADD(dst_h, dst_h, tmp);
  }

  void BxtContext::emitI64MULInstruction(const SelectionInstruction &insn)
  {
    GenRegister src0 = ra->genReg(insn.src(0));
    GenRegister src1 = ra->genReg(insn.src(1));
    GenRegister dst = ra->genReg(insn.dst(0));
    GenRegister res = ra->genReg(insn.dst(1));

    src0.type = src1.type = GEN_TYPE_UD;
    dst.type = GEN_TYPE_UL;
    res.type = GEN_TYPE_UL;

    /* Low 32 bits X low 32 bits. */
    GenRegister s0l = unpacked_ud(src0);
    GenRegister s1l = unpacked_ud(src1);
    p->MUL(dst, s0l, s1l);

    /* Low 32 bits X high 32 bits. */
    GenRegister s1h = unpacked_ud(res);
    p->MOV(s1h, unpacked_ud(src1, 1));

    p->MUL(res, s0l, s1h);
    p->SHL(res, res, GenRegister::immud(32));
    p->ADD(dst, dst, res);

    /* High 32 bits X low 32 bits. */
    GenRegister s0h = unpacked_ud(res);
    p->MOV(s0h, unpacked_ud(src0, 1));

    p->MUL(res, s0h, s1l);
    p->SHL(res, res, GenRegister::immud(32));
    p->ADD(dst, dst, res);
  }

  void BxtContext::setA0Content(uint16_t new_a0[16], uint16_t max_offset, int sz) {
    if (sz == 0)
      sz = 16;
    GBE_ASSERT(sz%4 == 0);
    GBE_ASSERT(new_a0[0] >= 0 && new_a0[0] < 4096);

    p->push();
    p->curr.execWidth = 1;
    p->curr.predicate = GEN_PREDICATE_NONE;
    p->curr.noMask = 1;
    for (int i = 0; i < sz/2; i++) {
      p->MOV(GenRegister::retype(GenRegister::addr1(i*2), GEN_TYPE_UD),
             GenRegister::immud(new_a0[i*2 + 1] << 16 | new_a0[i*2]));
    }
    p->pop();
  }

  void BxtContext::emitStackPointer(void) {
    using namespace ir;

    // Only emit stack pointer computation if we use a stack
    if (kernel->getStackSize() == 0)
      return;

    // Check that everything is consistent in the kernel code
    const uint32_t perLaneSize = kernel->getStackSize();
    GBE_ASSERT(perLaneSize > 0);

    const GenRegister selStatckPtr = this->simdWidth == 8 ?
      GenRegister::ud8grf(ir::ocl::stackptr) :
      GenRegister::ud16grf(ir::ocl::stackptr);
    const GenRegister stackptr = ra->genReg(selStatckPtr);
    // borrow block ip as temporary register as we will
    // initialize block ip latter.
    const GenRegister tmpReg = GenRegister::retype(GenRegister::vec1(getBlockIP()), GEN_TYPE_UW);
    const GenRegister tmpReg_ud = GenRegister::retype(tmpReg, GEN_TYPE_UD);

    loadLaneID(stackptr);

    // We compute the per-lane stack pointer here
    // threadId * perThreadSize + laneId*perLaneSize or
    // (threadId * simdWidth + laneId)*perLaneSize
    // let private address start from zero
    //p->MOV(stackptr, GenRegister::immud(0));
    p->push();
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->AND(tmpReg, GenRegister::ud1grf(0,5), GenRegister::immuw(0x1ff)); //threadId
      p->MUL(tmpReg, tmpReg, GenRegister::immuw(this->simdWidth));  //threadId * simdWidth
      p->curr.execWidth = this->simdWidth;
      p->ADD(stackptr, GenRegister::unpacked_uw(stackptr), tmpReg);  //threadId * simdWidth + laneId, must < 64K
      p->curr.execWidth = 1;
      p->MOV(tmpReg_ud, GenRegister::immud(perLaneSize));
      p->curr.execWidth = this->simdWidth;
      p->MUL(stackptr, tmpReg_ud, GenRegister::unpacked_uw(stackptr)); // (threadId * simdWidth + laneId)*perLaneSize
      if (fn.getPointerFamily() == ir::FAMILY_QWORD) {
        const GenRegister selStatckPtr2 = this->simdWidth == 8 ?
          GenRegister::ul8grf(ir::ocl::stackptr) :
          GenRegister::ul16grf(ir::ocl::stackptr);
        GenRegister stackptr2 = ra->genReg(selStatckPtr2);
        GenRegister sp = GenRegister::unpacked_ud(stackptr2.nr, stackptr2.subnr);
        int simdWidth = p->curr.execWidth;
        if (simdWidth == 16) {
          // we need do second quarter first, because the dst type is QW,
          // while the src is DW. If we do first quater first, the 1st
          // quarter's dst would contain the 2nd quarter's src.
          p->curr.execWidth = 8;
          p->curr.quarterControl = GEN_COMPRESSION_Q2;
          p->MOV(GenRegister::Qn(sp, 1), GenRegister::Qn(stackptr,1));
          p->MOV(GenRegister::Qn(stackptr2, 1), GenRegister::Qn(sp,1));
        }
        p->curr.quarterControl = GEN_COMPRESSION_Q1;
        p->MOV(sp, stackptr);
        p->MOV(stackptr2, sp);
      }
    p->pop();
  }

  void KblContext::newSelection(void) {
    this->sel = GBE_NEW(SelectionKbl, *this);
  }

  void GlkContext::newSelection(void) {
    this->sel = GBE_NEW(SelectionGlk, *this);
  }

}

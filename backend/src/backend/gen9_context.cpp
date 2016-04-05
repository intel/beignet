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
      p->WAIT();
    p->pop();
    if (imageFence) {
      p->FLUSH_SAMPLERCACHE(fenceDst);
      p->MOV(fenceDst, fenceDst);
    }
  }
}

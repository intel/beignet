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
 * \file gen75_context.cpp
 */

#include "backend/gen75_context.hpp"
#include "backend/gen75_encoder.hpp"
#include "backend/gen_program.hpp"
#include "backend/gen_defs.hpp"
#include "backend/gen_encoder.hpp"
#include "backend/gen_insn_selection.hpp"
#include "backend/gen_insn_scheduling.hpp"
#include "backend/gen_reg_allocation.hpp"
#include "sys/cvar.hpp"
#include "ir/function.hpp"
#include "ir/value.hpp"
#include <cstring>

namespace gbe
{
  void Gen75Context::emitSLMOffset(void) {
    if(kernel->getUseSLM() == false)
      return;

    const GenRegister slm_index = GenRegister::ud1grf(0, 0);
    //the slm index is hold in r0.0 24-27 bit, in 4K unit, move it to sr0.1's 8-11 bits.
    p->push();
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      GenRegister sr0 = GenRegister::sr(0, 1);
      p->SHR(sr0, slm_index, GenRegister::immud(16));
    p->pop();
  }

  uint32_t Gen75Context::alignScratchSize(uint32_t size){
    if(size == 0)
      return 0;
    uint32_t i = 2048;
    while(i < size) i *= 2;
    return i;
  }

  void Gen75Context::emitStackPointer(void) {
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
    const GenRegister tmpReg_ud = GenRegister::retype(GenRegister::vec1(getBlockIP()), GEN_TYPE_UD);

    // We compute the per-lane stack pointer here
    // threadId * perThreadSize + laneId*perLaneSize or
    // (threadId * simdWidth + laneId)*perLaneSize
    p->push();
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      //p->AND(GenRegister::ud1grf(126,0), GenRegister::ud1grf(0,5), GenRegister::immud(0x1ff));
      p->AND(tmpReg, GenRegister::ud1grf(0,5), GenRegister::immud(0x7f));
      p->AND(stackptr, GenRegister::ud1grf(0,5), GenRegister::immud(0x180));
      p->SHR(stackptr, stackptr, GenRegister::immud(7));
      p->SHL(tmpReg, tmpReg, GenRegister::immud(2));
      p->ADD(tmpReg, tmpReg, stackptr); //threadId

      p->MUL(tmpReg, tmpReg, GenRegister::immuw(this->simdWidth));  //threadId * simdWidth
      p->curr.execWidth = this->simdWidth;
      loadLaneID(stackptr);
      p->ADD(stackptr, GenRegister::unpacked_uw(stackptr), tmpReg);  //threadId * simdWidth + laneId, must < 64K
      p->curr.execWidth = 1;
      p->MOV(tmpReg_ud, GenRegister::immud(perLaneSize));
      p->curr.execWidth = this->simdWidth;
      p->MUL(stackptr, tmpReg_ud, stackptr); // (threadId * simdWidth + laneId)*perLaneSize

    p->pop();
  }

  void Gen75Context::newSelection(void) {
    this->sel = GBE_NEW(Selection75, *this);
  }
}

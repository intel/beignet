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

    const GenRegister slm_offset = ra->genReg(GenRegister::ud1grf(ir::ocl::slmoffset));
    const GenRegister slm_index = GenRegister::ud1grf(0, 0);
    //the slm index is hold in r0.0 24-27 bit, in 4K unit, shift left 12 to get byte unit
    p->push();
      p->curr.execWidth = 1;
      p->curr.predicate = GEN_PREDICATE_NONE;
      p->SHR(slm_offset, slm_index, GenRegister::immud(12));
    p->pop();
  }

  void Gen75Context::allocSLMOffsetCurbe(void) {
    if(fn.getUseSLM())
      allocCurbeReg(ir::ocl::slmoffset, GBE_CURBE_SLM_OFFSET);
  }

  void Gen75Context::newSelection(void) {
    this->sel = GBE_NEW(Selection75, *this);
  }
}

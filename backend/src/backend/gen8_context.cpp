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
 * \file gen8_context.cpp
 */

#include "backend/gen8_context.hpp"
#include "backend/gen8_encoder.hpp"
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
  void Gen8Context::emitSLMOffset(void) {
    return;
  }

  uint32_t Gen8Context::alignScratchSize(uint32_t size){
    if(size == 0)
      return 0;
    uint32_t i = 1024;
    while(i < size) i *= 2;
    return i;
  }

  void Gen8Context::newSelection(void) {
    this->sel = GBE_NEW(Selection8, *this);
  }

  void Gen8Context::packLongVec(GenRegister unpacked, GenRegister packed, uint32_t simd)
  {
    GBE_ASSERT(packed.subnr == 0);
    GBE_ASSERT(unpacked.subnr == 0);

    unpacked = GenRegister::retype(unpacked, GEN_TYPE_UD);
    packed = GenRegister::retype(packed, GEN_TYPE_UD);

    if (simd == 16) {
      p->push();
      p->curr.execWidth = 8;
      p->MOV(GenRegister::h2(packed), unpacked);
      p->MOV(GenRegister::h2(GenRegister::offset(packed, 0, typeSize(GEN_TYPE_UD))),
             GenRegister::offset(unpacked, 2));
      p->curr.quarterControl = 1;
      p->MOV(GenRegister::h2(GenRegister::offset(packed, 2, 0)), GenRegister::offset(unpacked, 1));
      p->MOV(GenRegister::h2(GenRegister::offset(packed, 2, typeSize(GEN_TYPE_UD))),
             GenRegister::offset(unpacked, 3));
      p->pop();
    } else {
      GBE_ASSERT(simd == 8);
      p->MOV(GenRegister::h2(packed), unpacked);
      p->MOV(GenRegister::h2(GenRegister::offset(packed, 0, typeSize(GEN_TYPE_UD))),
             GenRegister::offset(unpacked, 1));
    }
  }

  void Gen8Context::unpackLongVec(GenRegister packed, GenRegister unpacked, uint32_t simd)
  {
    GBE_ASSERT(packed.subnr == 0);
    GBE_ASSERT(unpacked.subnr == 0);

    unpacked = GenRegister::retype(unpacked, GEN_TYPE_UD);
    packed = GenRegister::retype(packed, GEN_TYPE_UD);

    packed.vstride = GEN_VERTICAL_STRIDE_8;
    packed.width = GEN_WIDTH_4;

    p->push();
    p->curr.execWidth = 8;
    if (simd == 16) {
      p->MOV(unpacked, GenRegister::h2(packed));
      p->MOV(GenRegister::offset(unpacked, 2),
             GenRegister::h2(GenRegister::offset(packed, 0, typeSize(GEN_TYPE_UD))));

      p->curr.quarterControl = 1;
      p->MOV(GenRegister::offset(unpacked, 1), GenRegister::h2(GenRegister::offset(packed, 2)));
      p->MOV(GenRegister::offset(unpacked, 3),
             GenRegister::h2(GenRegister::offset(packed, 2, typeSize(GEN_TYPE_UD))));
    } else {
      GBE_ASSERT(simd == 8);
      p->MOV(unpacked, GenRegister::h2(packed));
      p->MOV(GenRegister::offset(unpacked, 1),
             GenRegister::h2(GenRegister::offset(packed, 0, typeSize(GEN_TYPE_UD))));
    }
    p->pop();
  }

}

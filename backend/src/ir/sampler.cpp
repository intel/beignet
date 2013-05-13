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
 * \file sampler.cpp
 *
 */
#include "sampler.hpp"
#include "context.hpp"
#include "ocl_common_defines.h"

namespace gbe {
namespace ir {

  const uint32_t SamplerSet::getIdx(const Register reg) const
  {
    auto it = regMap.find(reg);
    GBE_ASSERT(it != regMap.end());
    return it->second.slot;
  }

  void SamplerSet::appendReg(const Register reg, uint32_t key, Context *ctx) {
    struct SamplerRegSlot samplerSlot;
    samplerSlot.reg = reg;
    samplerSlot.slot = samplerMap.size();
    samplerMap.insert(std::make_pair(key, samplerSlot));
    regMap.insert(std::make_pair(samplerSlot.reg, samplerSlot));
  }

  Register SamplerSet::append(uint32_t samplerValue, Context *ctx)
  {
    auto it = samplerMap.find(samplerValue);
    if (it != samplerMap.end())
        return it->second.reg;
    // This register is just used as a key.
    Register reg = ctx->reg(FAMILY_DWORD);
    appendReg(reg, samplerValue, ctx);
    return reg;
  }

#define SAMPLER_ID(id) ((id << __CLK_SAMPLER_ARG_BASE) | __CLK_SAMPLER_ARG_KEY_BIT)
  void SamplerSet::append(Register samplerReg, Context *ctx)
  {
    ir::FunctionArgument *arg =  ctx->getFunction().getArg(samplerReg);
    GBE_ASSERT(arg != NULL);

    // XXX As LLVM 3.2/3.1 doesn't have a new data type for the sampler_t, we have to fix up the argument
    // type here. Once we switch to the LLVM and use the new data type sampler_t, we can remove this
    // work around.
    arg->type = ir::FunctionArgument::SAMPLER;
    int32_t id = ctx->getFunction().getArgID(arg);
    GBE_ASSERT(id < (1 << __CLK_SAMPLER_ARG_BITS));

    auto it = samplerMap.find(SAMPLER_ID(id));
    if (it != samplerMap.end()) {
      GBE_ASSERT(it->second.reg == samplerReg);
      return;
    }
    appendReg(samplerReg, SAMPLER_ID(id), ctx);
  }

} /* namespace ir */
} /* namespace gbe */

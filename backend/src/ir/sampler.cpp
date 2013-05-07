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

namespace gbe {
namespace ir {

  Register SamplerSet::append(uint32_t samplerValue, Context *ctx)
  {
    int i = 0;

    for(auto it = regMap.begin();
        it != regMap.end(); ++it, ++i)
    {
      if (it->first == samplerValue)
        return it->second;
    }
    Register reg = ctx->reg(FAMILY_DWORD);
    ctx->LOADI(ir::TYPE_S32, reg, ctx->newIntegerImmediate(i, ir::TYPE_S32));
    regMap.insert(std::make_pair(samplerValue, reg));
    return reg;
  }

} /* namespace ir */
} /* namespace gbe */

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
 * \file sampler.cpp
 *
 */
#include "sampler.hpp"
#include "context.hpp"
#include "ocl_common_defines.h"

namespace gbe {
namespace ir {

#ifdef GBE_COMPILER_AVAILABLE
  uint8_t SamplerSet::appendReg(uint32_t key, Context *ctx) {
    uint8_t samplerSlot = samplerMap.size();
    samplerMap.insert(std::make_pair(key, samplerSlot));
    return samplerSlot;
  }

  uint8_t SamplerSet::append(uint32_t samplerValue, Context *ctx)
  {
    auto it = samplerMap.find(samplerValue);
    if (it != samplerMap.end())
        return it->second;
    // This register is just used as a key.
    return appendReg(samplerValue, ctx);
  }

#define SAMPLER_ID(id) ((id << __CLK_SAMPLER_ARG_BASE) | __CLK_SAMPLER_ARG_KEY_BIT)
  uint8_t SamplerSet::append(Register samplerReg, Context *ctx)
  {
    ir::FunctionArgument *arg =  ctx->getFunction().getArg(samplerReg);
    GBE_ASSERT(arg != NULL);

    GBE_ASSERT(arg->type == ir::FunctionArgument::SAMPLER);
    int32_t id = ctx->getFunction().getArgID(arg);
    GBE_ASSERT(id < (1 << __CLK_SAMPLER_ARG_BITS));

    map<uint32_t, uint32_t>::iterator it = samplerMap.find(SAMPLER_ID(id));
    if (it != samplerMap.end()) {
      return it->second;
    }
    return appendReg(SAMPLER_ID(id), ctx);
  }
#endif

#define OUT_UPDATE_SZ(elt) SERIALIZE_OUT(elt, outs, ret_size)
#define IN_UPDATE_SZ(elt) DESERIALIZE_IN(elt, ins, total_size)

  /*! Implements the serialization. */
  uint32_t SamplerSet::serializeToBin(std::ostream& outs) {
    uint32_t ret_size = 0;
    uint32_t sz = 0;

    OUT_UPDATE_SZ(magic_begin);

    sz = samplerMap.size();
    OUT_UPDATE_SZ(sz);
    for (map<uint32_t, uint32_t>::iterator it = samplerMap.begin(); it != samplerMap.end(); ++it) {
      OUT_UPDATE_SZ(it->first);
      OUT_UPDATE_SZ(it->second);
    }

    OUT_UPDATE_SZ(magic_end);
    OUT_UPDATE_SZ(ret_size);

    return ret_size;
  }

  uint32_t SamplerSet::deserializeFromBin(std::istream& ins) {
    uint32_t total_size = 0;
    uint32_t magic;
    uint32_t sampler_map_sz = 0;

    IN_UPDATE_SZ(magic);
    if (magic != magic_begin)
      return 0;

    IN_UPDATE_SZ(sampler_map_sz);
    for (size_t i = 0; i < sampler_map_sz; i++) {
      uint32_t key;
      uint32_t slot;

      IN_UPDATE_SZ(key);
      IN_UPDATE_SZ(slot);
      samplerMap.insert(std::make_pair(key, slot));
    }

    IN_UPDATE_SZ(magic);
    if (magic != magic_end)
      return 0;

    uint32_t total_bytes;
    IN_UPDATE_SZ(total_bytes);
    if (total_bytes + sizeof(total_size) != total_size)
      return 0;

    return total_size;
  }

  void SamplerSet::printStatus(int indent, std::ostream& outs) {
    using namespace std;
    string spaces = indent_to_str(indent);
    string spaces_nl = indent_to_str(indent + 4);

    outs << spaces << "------------ Begin SamplerSet ------------" << "\n";

    outs << spaces_nl << "  SamplerSet Map: [index, sampler_reg, sampler_slot]\n";
    outs << spaces_nl << "     samplerMap size: " << samplerMap.size() << "\n";

    for (map<uint32_t, uint32_t>::iterator it = samplerMap.begin(); it != samplerMap.end(); ++it) {
      outs << spaces_nl <<  "     [" << it->first << ", "
           << it->second << "]\n";
    }

    outs << spaces << "------------- End SamplerSet -------------" << "\n";
  }

} /* namespace ir */
} /* namespace gbe */

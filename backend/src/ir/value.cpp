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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

/**
 * \file value.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "ir/value.hpp"
#include "ir/liveness.hpp"

namespace gbe {
namespace ir {

  GraphUseDef::GraphUseDef(const Liveness &liveness) {
    const Function &fn = liveness.getFunction();

    // First create the chains and insert them in their respective maps
    fn.foreachInstruction([this](const Instruction &insn) {
      // sources == value uses
      const uint32_t srcNum = insn.getSrcNum();
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        ValueUse *valueUse = this->newValueUse(insn, srcID);
        UDChain *udChain = this->newUDChain();
        udChain->first = valueUse;
        udGraph.insert(std::make_pair(*valueUse, udChain));
      }
      // destinations == value defs
      const uint32_t dstNum = insn.getDstNum();
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
        ValueDef *valueDef = this->newValueDef(insn, dstID);
        DUChain *duChain = this->newDUChain();
        duChain->first = valueDef;
        duGraph.insert(std::make_pair(*valueDef, duChain));
      }
    });

    // Function arguments are also value definitions
    const uint32_t inputNum = fn.inputNum();
    for (uint32_t inputID = 0; inputID < inputNum; ++inputID) {
      const FunctionInput &input = fn.getInput(inputID);
      ValueDef *valueDef = this->newValueDef(input);
      DUChain *duChain = this->newDUChain();
      duChain->first = valueDef;
      duGraph.insert(std::make_pair(*valueDef, duChain));
    }
  }

} /* namespace ir */
} /* namespace gbe */


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
 * \file lowering.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "ir/context.hpp"

namespace gbe {
namespace ir {

  /*! Small helper class to lower return instructions */
  class ContextReturn : public Context
  {
  public:
    /*! Initialize a context dedicated to return instruction lowering */
    ContextReturn(Unit &unit) : Context(unit) {
      this->usedLabels = GBE_NEW(vector<uint8_t>);
    }
    /*! Lower the return instruction to gotos for the given function */
    void lower(const std::string &functionName);
  };

  void ContextReturn::lower(const std::string &functionName) {
    if ((this->fn = unit.getFunction(functionName)) == NULL)
      return;

    // Append a new block at the end of the function with a return instruction:
    // the only one we are going to have
    this->bb = &this->fn->getBottomBlock();
    const LabelIndex index = this->label();
    this->LABEL(index);
    const BasicBlock *lastBlock = this->bb;
    this->RET();

    // Now traverse all instructions and replace all returns by GOTO index
    fn->foreachInstruction([&](Instruction &insn) {
      if (insn.getParent() == lastBlock) return; // This is the last block
      if (insn.getOpcode() != OP_RET) return;
      const Instruction bra = ir::BRA(index);
      bra.replace(&insn);
    });
  }

  void lowerReturn(Unit &unit, const std::string &functionName)
  {
    ContextReturn *ctx = GBE_NEW(ContextReturn, unit);
    ctx->lower(functionName);
    GBE_DELETE(ctx);
  }

} /* namespace ir */
} /* namespace gbe */


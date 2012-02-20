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
 * \file context.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "ir/context.hpp"
#include "ir/unit.hpp"

namespace gbe {
namespace ir {

  Context::Context(Unit &unit) : unit(unit), fn(NULL), bb(NULL) {}

  void Context::startFunction(const std::string &name) {
    if (fn != NULL) fnStack.push_back(fn);
    fn = unit.newFunction(name);
  }

  void Context::endFunction(void) {
    GBE_ASSERTM(fn != NULL, "No function to end");
    if (fnStack.size() != 0) {
      fn = fnStack.back();
      fnStack.pop_back();
    } else
      fn = NULL;
  }

  Register Context::reg(RegisterData::Family family) {
    GBE_ASSERTM(fn != NULL, "No function currently defined");
    return fn->file.append(family);
  }

  void Context::input(Register reg) {
    GBE_ASSERTM(fn != NULL, "No function currently defined");
    GBE_ASSERTM(reg < fn->file.regNum(), "Out-of-bound register");
    fn->input.push_back(reg);
  }

  void Context::output(Register reg) {
    GBE_ASSERTM(fn != NULL, "No function currently defined");
    GBE_ASSERTM(reg < fn->file.regNum(), "Out-of-bound register");
    fn->output.push_back(reg);
  }

  void Context::startBlock(void) {
    GBE_ASSERTM(fn != NULL, "No function currently defined");
    this->bb = GBE_NEW(BasicBlock, *fn);
    fn->blocks.push_back(bb);
  }

  void Context::endBlock(void) {
    this->bb = NULL;
  }

  void Context::append(const Instruction &insn)
  {
    GBE_ASSERTM(fn != NULL, "No function currently defined");

    // Start a new block if this is a label
    if (insn.isMemberOf<LabelInstruction>() == true) {
      this->endBlock();
      this->startBlock();
      const LabelIndex index = cast<LabelInstruction>(insn).getLabelIndex();
      GBE_ASSERTM(index < fn->labelNum(), "Out-of-bound label");
      GBE_ASSERTM(fn->labels[index] == NULL, "Label used in a previous block");
      fn->labels[index] = bb;
    }
    // We create a new label for a new block if the user did not do it
    else if (bb == NULL) {
      this->startBlock();
      const LabelIndex index = fn->newLabel();
      const Instruction insn = ir::LABEL(index);
      this->append(insn);
    }

    // Append the instruction in the stream
    Instruction *insnPtr = fn->newInstruction();
    *insnPtr = insn;
#ifndef NDEBUG
    std::string whyNot;
    GBE_ASSERTM(insn.wellFormed(*fn, whyNot), whyNot.c_str());
#endif /* NDEBUG */
    bb->append(*insnPtr);

    // Close the current block if this is a branch
    if (insn.isMemberOf<BranchInstruction>() == true)
      this->endBlock();
  }

} /* namespace ir */
} /* namespace gbe */


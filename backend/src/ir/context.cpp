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
    GBE_ASSERT(fn != NULL);
    if (fnStack.size() != 0) {
      fn = fnStack.back();
      fnStack.pop_back();
    } else
      fn = NULL;
  }

  Register Context::reg(RegisterData::Family family) {
    GBE_ASSERT(fn != NULL);
    return fn->file.append(family);
  }

  void Context::input(Register reg) {
    GBE_ASSERT(fn != NULL && reg < fn->file.regNum());
    fn->input.push_back(reg);
  }

  void Context::output(Register reg) {
    GBE_ASSERT(fn != NULL && reg < fn->file.regNum());
    fn->output.push_back(reg);
  }

  void Context::startBlock(void) {
    GBE_ASSERT(fn != NULL);
    this->bb = GBE_NEW(BasicBlock, *fn);
    fn->blocks.push_back(bb);
  }

  void Context::endBlock(void) {
    this->bb = NULL;
  }

  void Context::append(const Instruction &insn) {

    // Start a new block if this is a label
    if (insn.isMemberOf<LabelInstruction>() == true) {
      this->endBlock();
      this->startBlock();
    }

    // Append the instruction in the stream
    GBE_ASSERT(fn != NULL && bb != NULL);
    Instruction *insnPtr = fn->newInstruction();
    *insnPtr = insn;
    bb->append(*insnPtr);

    // Close the current block if this is a branch
    if (insn.isMemberOf<BranchInstruction>() == true)
      this->endBlock();
  }

} /* namespace ir */
} /* namespace gbe */


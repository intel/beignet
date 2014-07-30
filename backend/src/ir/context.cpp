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
#include "ir/lowering.hpp"

namespace gbe {
namespace ir {

  Context::Context(Unit &unit) :
    unit(unit), fn(NULL), bb(NULL), usedLabels(NULL) {}

  Context::~Context(void) {
    for (const auto &elem : fnStack) GBE_SAFE_DELETE(elem.usedLabels);
    GBE_SAFE_DELETE(usedLabels);
  }

  Function &Context::getFunction(void) {
    GBE_ASSERTM(fn != NULL, "No function currently defined");
    return *fn;
  }

  void Context::appendPushedConstant(Register reg, const PushLocation &pushed)
  {
    GBE_ASSERTM(fn != NULL, "No function currently defined");
    GBE_ASSERTM(fn->pushMap.contains(reg) == false, "Register already pushed");
    fn->pushMap.insert(std::make_pair(reg, pushed));
    fn->locationMap.insert(std::make_pair(pushed, reg));
  }

  void Context::startFunction(const std::string &name) {
    fnStack.push_back(StackElem(fn,bb,usedLabels));
    fn = unit.newFunction(name);
    usedLabels = GBE_NEW_NO_ARG(vector<uint8_t>);
    bb = NULL;
  }

  void Context::endFunction(void) {
    GBE_ASSERTM(fn != NULL, "No function to end");
    GBE_ASSERT(fnStack.size() != 0);
    GBE_ASSERT(usedLabels != NULL);

    // Empty function -> append a return
    if (fn->blockNum() == 0) this->RET();

    // Check first that all branch instructions point to valid labels
    GBE_ASSERT(usedLabels);
#if GBE_DEBUG
    for (auto usage : *usedLabels)
      GBE_ASSERTM(usage != LABEL_IS_POINTED, "A label is used and not defined");
#endif /* GBE_DEBUG */
    GBE_DELETE(usedLabels);

    // Remove all returns and insert one unique return block at the end of the
    // function
    lowerReturn(unit, fn->getName());
    // check if there is empty labels at first
    fn->checkEmptyLabels();
    // Properly order labels and compute the CFG, it's needed by FunctionArgumentLower
    fn->sortLabels();
    fn->computeCFG();

    // Spill function argument to the stack if required and identify which
    // function arguments can use constant push
    lowerFunctionArguments(unit, fn->getName());

    const StackElem elem = fnStack.back();
    fnStack.pop_back();
    fn = elem.fn;
    bb = elem.bb;
    usedLabels = elem.usedLabels;
  }

  Register Context::reg(RegisterFamily family, bool uniform) {
    GBE_ASSERTM(fn != NULL, "No function currently defined");
    return fn->newRegister(family, uniform);
  }

  LabelIndex Context::label(void) {
    GBE_ASSERTM(fn != NULL, "No function currently defined");
    const LabelIndex index = fn->newLabel();
    if (index >= usedLabels->size()) {
      usedLabels->resize(index + 1);
      (*usedLabels)[index] = 0;
    }
    return index;
  }

  void Context::input(const std::string &name, FunctionArgument::Type type, Register reg,
                      FunctionArgument::InfoFromLLVM& info, uint32_t elementSize, uint32_t align, unsigned char bti) {
    GBE_ASSERTM(fn != NULL, "No function currently defined");
    GBE_ASSERTM(reg < fn->file.regNum(), "Out-of-bound register");
    FunctionArgument *arg = GBE_NEW(FunctionArgument, type, reg, elementSize, name, align, info, bti);
    fn->args.push_back(arg);
  }

  void Context::output(Register reg) {
    GBE_ASSERTM(fn != NULL, "No function currently defined");
    GBE_ASSERTM(reg < fn->file.regNum(), "Out-of-bound register");
    fn->outputs.push_back(reg);
  }

  void Context::startBlock(void) {
    GBE_ASSERTM(fn != NULL, "No function currently defined");
    this->bb = GBE_NEW(BasicBlock, *fn);
    fn->blocks.push_back(bb);
  }

  void Context::endBlock(void) {
    this->bb = NULL;
  }

  void Context::append(const Instruction &insn) {
    GBE_ASSERTM(fn != NULL, "No function currently defined");

    // Start a new block if this is a label
    if (insn.isMemberOf<LabelInstruction>() == true) {
      this->endBlock();
      this->startBlock();
      const LabelIndex index = cast<LabelInstruction>(insn).getLabelIndex();
      GBE_ASSERTM(index < fn->labelNum(), "Out-of-bound label");
      GBE_ASSERTM(fn->labels[index] == NULL, "Label used in a previous block");
      fn->labels[index] = bb;

      // Now the label index is properly defined
      GBE_ASSERT(index < usedLabels->size());
      (*usedLabels)[index] |= LABEL_IS_DEFINED;
    }
    // We create a new label for a new block if the user did not do it
    else if (bb == NULL) {
      // this->startBlock();
      const LabelIndex index = this->label();
      const Instruction insn = ir::LABEL(index);
      this->append(insn);
    }

    // Append the instruction in the stream
    Instruction *insnPtr = fn->newInstruction(insn);
    bb->append(*insnPtr);
#if GBE_DEBUG
    std::string whyNot;
    if(getUnit().getValid())
      GBE_ASSERTM(insnPtr->wellFormed(whyNot), whyNot.c_str());
#endif /* GBE_DEBUG */

    // Close the current block if this is a branch
    if (insn.isMemberOf<BranchInstruction>() == true) {
      // We must book keep the fact that the label is used
      if (insn.getOpcode() == OP_BRA) {
        const BranchInstruction &branch = cast<BranchInstruction>(insn);
        const LabelIndex index = branch.getLabelIndex();
        GBE_ASSERT(index < usedLabels->size());
        (*usedLabels)[index] |= LABEL_IS_POINTED;
      }
      this->endBlock();
    }
  }

} /* namespace ir */
} /* namespace gbe */


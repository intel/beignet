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
#include "ir/value.hpp"
#include "ir/liveness.hpp"
#include "ir/argument_analysis.hpp"
#include "sys/set.hpp"

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

  void lowerReturn(Unit &unit, const std::string &functionName) {
    ContextReturn *ctx = GBE_NEW(ContextReturn, unit);
    ctx->lower(functionName);
    GBE_DELETE(ctx);
  }

  /*! Helper class to lower function arguments if required */
  class FunctionArgumentLowerer
  {
  public:
    /*! Build the helper structure */
    FunctionArgumentLowerer(Unit &unit);
    /*! Free everything we needed */
    ~FunctionArgumentLowerer(void);
    /*! Perform the function argument substitution if needed */
    void lower(const std::string &name);
    /*! Inspect and possibly the given function argument */
    void lower(FunctionInput &input);
    /*! Recursively look if there is a store in the given use */
    bool useStore(Register reg, set<const Instruction*> &visited);
    /*! Look if the pointer use only load with immediate offsets */
    bool matchLoadImm(Register reg, uint64_t &offset);
    Liveness *liveness; //!< To compute the function graph
    FunctionDAG *dag;   //!< Contains complete dependency information
    Unit &unit;         //!< The unit we process
    Function *fn;       //!< Function to patch
  };

  FunctionArgumentLowerer::FunctionArgumentLowerer(Unit &unit) :
    liveness(NULL), dag(NULL), unit(unit) {}
  FunctionArgumentLowerer::~FunctionArgumentLowerer(void) {
    GBE_SAFE_DELETE(dag);
    GBE_SAFE_DELETE(liveness);
  }

  void FunctionArgumentLowerer::lower(const std::string &functionName) {
    if ((this->fn = unit.getFunction(functionName)) == NULL)
      return;
    GBE_SAFE_DELETE(dag);
    GBE_SAFE_DELETE(liveness);
    this->liveness = GBE_NEW(ir::Liveness, *fn);
    this->dag = GBE_NEW(ir::FunctionDAG, *this->liveness);
    const uint32_t inputNum = fn->inputNum();
    for (uint32_t inputID = 0; inputID < inputNum; ++inputID) {
      FunctionInput &input = fn->getInput(inputID);
      if (input.type != FunctionInput::STRUCTURE) return;
      this->lower(input);
    }
  }

  bool FunctionArgumentLowerer::useStore(Register reg, set<const Instruction*> &visited) {
    const UseSet &useSet = dag->getUse(reg);
    for (const auto &use : useSet) {
      const Instruction *insn = use->getInstruction();
      const uint32_t srcID = use->getSrcID();
      const Opcode opcode = insn->getOpcode();
      if (visited.contains(insn)) continue;
      visited.insert(insn);
      if (opcode == OP_STORE && srcID == StoreInstruction::addressIndex)
        return true;
      if (insn->isMemberOf<UnaryInstruction>() == false &&
          insn->isMemberOf<BinaryInstruction>() == false &&
          insn->isMemberOf<TernaryInstruction>() == false)
        continue;
      else {
        const uint32_t dstNum = insn->getDstNum();
        for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
          const Register dst = insn->getDst(dstID);
          if (this->useStore(dst, visited) == true)
            return true;
        }
      }
    }
    return false;
  }

  bool FunctionArgumentLowerer::matchLoadImm(Register reg, uint64_t &offset) {
    const UseSet &useSet = dag->getUse(reg);
    offset = 0;
    for (const auto &use : useSet) {
      const Instruction *insn = use->getInstruction();
      const uint32_t srcID = use->getSrcID();
      const Opcode opcode = insn->getOpcode();
      // load dst ptr -> it is fine
      if (opcode == OP_LOAD) continue;

      // add dst ptr other -> we must inspect other to see if it comes from
      // LOADI
      if (opcode == OP_ADD) {
        const uint32_t otherID = srcID ^ 1;
        const DefSet &defSet = dag->getDef(insn, otherID);
        const uint32_t defNum = defSet.size();
        if (defNum == 0) continue; // undefined value
        if (defNum > 1) return false; // only *one* LOADI is allowed

        // Only accept one LOADI as definition
        const ValueDef *otherDef = *defSet.begin();
        const Instruction *otherInsn = otherDef->getInstruction();
        if (otherInsn->getOpcode() != OP_LOADI) return false;
        const LoadImmInstruction *loadImm = cast<LoadImmInstruction>(otherInsn);
        offset = loadImm->getImmediate().data.u64;
      }
    }
    return true;
  }

  void FunctionArgumentLowerer::lower(FunctionInput &input)
  {
    // Look at what happened at the input pointer to see what we need to do
    const Register reg = input.reg;

    // case 1 - we may store something to the structure argument. Right now we
    // abort but we will need to spill the structures into register (actually
    // all argument that may be also indirectly read (due to aliasing problems)
    set<const Instruction*> visited;
    GBE_ASSERTM(this->useStore(reg, visited) == false,
               "A store to a structure argument "
               "(i.e. not a char/short/int/float argument) has been found. "
               "This is not supported yet");

    // case 2 - we look for the patterns:
    // LOAD(ptr) or LOAD(ptr+imm)
    // if all patterns are like this, we know we can safely use the push
    // constant and we save the analysis result for the final backend code
    // generation
    uint64_t offset;
    if (this->matchLoadImm(reg, offset) == true) {


    }
    // case 3 - LOAD(ptr+runtime_value) is *not* supported yet
    else
      GBE_ASSERTM(false, "Only direct loads of structure arguments are "
                         "supported now.");
  }

  void lowerFunctionArguments(Unit &unit, const std::string &functionName) {
    FunctionArgumentLowerer lowerer(unit);
    lowerer.lower(functionName);
  }

} /* namespace ir */
} /* namespace gbe */


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
 * \file function.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "ir/function.hpp"
#include "ir/unit.hpp"
#include "sys/map.hpp"
#include "sys/intrusive_list.hpp"

namespace gbe {
namespace ir {

  Register PushLocation::getRegister(void) const {
    const Function::LocationMap &locationMap = fn.getLocationMap();
    GBE_ASSERT(locationMap.contains(*this) == true);
    return locationMap.find(*this)->second;
  }

  uint32_t BasicBlock::getInstructionNum(void) const {
    uint32_t insnNum = 0;
    this->foreach([&insnNum](const Instruction &insn) { insnNum++; });
    return insnNum;
  }

  Function::Function(const std::string &name, const Unit &unit, Profile profile) :
    name(name), unit(unit), profile(profile), simdWidth(0)
  {
    initProfile(*this);
  }

  Function::~Function(void) {
    for (auto block : blocks) GBE_DELETE(block);
    for (auto arg : args) GBE_DELETE(arg);
  }

  RegisterFamily Function::getPointerFamily(void) const {
    return unit.getPointerFamily();
  }

  void Function::sortLabels(void) {
    uint32_t last = 0;

    // Compute the new labels and patch the label instruction
    map<LabelIndex, LabelIndex> labelMap;
    foreachInstruction([&](Instruction &insn) {
      if (insn.getOpcode() != OP_LABEL) return;

      // Create the new label
      const Instruction newLabel = LABEL(LabelIndex(last));

      // Replace the previous label instruction
      LabelInstruction &label = cast<LabelInstruction>(insn);
      const LabelIndex index = label.getLabelIndex();
      labelMap.insert(std::make_pair(index, LabelIndex(last++)));
      newLabel.replace(&insn);
    });

    // Patch all branch instructions with the new labels
    foreachInstruction([&](Instruction &insn) {
      if (insn.getOpcode() != OP_BRA) return;

      // Get the current branch instruction
      BranchInstruction &bra = cast<BranchInstruction>(insn);
      const LabelIndex index = bra.getLabelIndex();
      const LabelIndex newIndex = labelMap.find(index)->second;

      // Insert the patched branch instruction
      if (bra.isPredicated() == true) {
        const Instruction newBra = BRA(newIndex, bra.getPredicateIndex());
        newBra.replace(&insn);
      } else {
        const Instruction newBra = BRA(newIndex);
        newBra.replace(&insn);
      }
    });

    // Reset the label to block mapping
    this->labels.resize(last);
    foreachBlock([&](BasicBlock &bb) {
      const Instruction *first = bb.getFirstInstruction();
      const LabelInstruction *label = cast<LabelInstruction>(first);
      const LabelIndex index = label->getLabelIndex();
      this->labels[index] = &bb;
    });
  }

  LabelIndex Function::newLabel(void) {
    GBE_ASSERTM(labels.size() < 0xffff,
                "Too many labels are defined (65536 only are supported)");
    const LabelIndex index(labels.size());
    labels.push_back(NULL);
    return index;
  }

  void Function::outImmediate(std::ostream &out, ImmediateIndex index) const {
    GBE_ASSERT(index < immediates.size());
    const Immediate imm = immediates[index];
    switch (imm.type) {
      case TYPE_BOOL: out << !!imm.data.u8; break;
      case TYPE_S8: out << imm.data.s8; break;
      case TYPE_U8: out << imm.data.u8; break;
      case TYPE_S16: out << imm.data.s16; break;
      case TYPE_U16: out << imm.data.u16; break;
      case TYPE_S32: out << imm.data.s32; break;
      case TYPE_U32: out << imm.data.u32; break;
      case TYPE_S64: out << imm.data.s64; break;
      case TYPE_U64: out << imm.data.u64; break;
      case TYPE_HALF: out << "half(" << imm.data.u16 << ")"; break;
      case TYPE_FLOAT: out << imm.data.f32; break;
      case TYPE_DOUBLE: out << imm.data.f64; break;
    }
  }

  uint32_t Function::getLargestBlockSize(void) const {
    uint32_t insnNum = 0;
    foreachBlock([&insnNum](const ir::BasicBlock &bb) {
      insnNum = std::max(insnNum, bb.getInstructionNum());
    });
    return insnNum;
  }

  uint32_t Function::getFirstSpecialReg(void) const {
    return this->profile == PROFILE_OCL ? 0u : ~0u;
  }

  uint32_t Function::getSpecialRegNum(void) const {
    return this->profile == PROFILE_OCL ? ocl::regNum : ~0u;
  }

  void Function::computeCFG(void) {
    // Clear possible previously computed CFG and compute the direct
    // predecessors and successors
    BasicBlock *prev = NULL;
    this->foreachBlock([this, &prev](BasicBlock &bb) {
      bb.successors.clear();
      bb.predecessors.clear();
      if (prev != NULL) {
        prev->nextBlock = &bb;
        bb.prevBlock = prev;
      }
      prev = &bb;
    });

    // Update it. Do not forget that a branch can also jump to the next block
    BasicBlock *jumpToNext = NULL;
    this->foreachBlock([this, &jumpToNext](BasicBlock &bb) {
      if (jumpToNext) {
        jumpToNext->successors.insert(&bb);
        bb.predecessors.insert(jumpToNext);
        jumpToNext = NULL;
      }
      if (bb.last == NULL) return;
      if (bb.last->isMemberOf<BranchInstruction>() == false) {
        jumpToNext = &bb;
        return;
      }
      const BranchInstruction &insn = cast<BranchInstruction>(*bb.last);
      if (insn.getOpcode() == OP_BRA) {
        const LabelIndex label = insn.getLabelIndex();
        BasicBlock *target = this->blocks[label];
        GBE_ASSERT(target != NULL);
        target->predecessors.insert(&bb);
        bb.successors.insert(target);
        if (insn.isPredicated() == true) jumpToNext = &bb;
      }
    });
  }

  std::ostream &operator<< (std::ostream &out, const Function &fn)
  {
    out << ".decl_function " << fn.getName() << std::endl;
    out << fn.getRegisterFile();
    out << "## " << fn.argNum() << " input register"
        << (fn.argNum() ? "s" : "") << " ##" << std::endl;
    for (uint32_t i = 0; i < fn.argNum(); ++i) {
      const FunctionArgument &input = fn.getArg(i);
      out << "decl_input.";
      switch (input.type) {
        case FunctionArgument::GLOBAL_POINTER: out << "global"; break;
        case FunctionArgument::LOCAL_POINTER: out << "local"; break;
        case FunctionArgument::CONSTANT_POINTER: out << "constant"; break;
        case FunctionArgument::VALUE: out << "value"; break;
        case FunctionArgument::STRUCTURE:
          out << "structure." << input.size;
        break;
        default: break;
      }
      out << " %" << input.reg << std::endl;
    }
    out << "## " << fn.outputNum() << " output register"
        << (fn.outputNum() ? "s" : "") << " ##" << std::endl;
    for (uint32_t i = 0; i < fn.outputNum(); ++i)
      out << "decl_output %" << fn.getOutput(i) << std::endl;
    out << "## " << fn.pushedNum() << " pushed register" << std::endl;
    const Function::PushMap &pushMap = fn.getPushMap();
    for (const auto &pushed : pushMap) {
      out << "decl_pushed %" << pushed.first
           << " @{" << pushed.second.argID << ","
           << pushed.second.offset << "}" << std::endl;
    }
    out << "## " << fn.blockNum() << " block"
        << (fn.blockNum() ? "s" : "") << " ##" << std::endl;
    fn.foreachBlock([&](const BasicBlock &bb) {
      bb.foreach([&out] (const Instruction &insn) {
        out << insn << std::endl;
      });
      out << std::endl;
    });
    out << ".end_function" << std::endl;
    return out;
  }

  BasicBlock::BasicBlock(Function &fn) : fn(fn) {
    this->first = this->last = NULL;
    this->nextBlock = this->prevBlock = NULL;
  }
  BasicBlock::~BasicBlock(void) {
    this->foreach([this] (Instruction &insn) {
     this->fn.deleteInstruction(&insn);
    });
  }

#define DECL_GET_PRED \
  if (predecessor != NULL) return predecessor; \
  if (stayInBlock == true) return NULL; \
  const BasicBlock *parent = this->getParent(); \
  const BasicBlock *prev = parent->getPrevBlock(); \
  while (prev) { \
    Instruction *last = prev->getLastInstruction(); \
    if (last) return last; \
    prev = prev->getPrevBlock(); \
  } \
  return NULL;

  Instruction *Instruction::getPredecessor(bool stayInBlock) {
    DECL_GET_PRED;
  }
  const Instruction *Instruction::getPredecessor(bool stayInBlock) const {
    DECL_GET_PRED;
  }

#undef DECL_GET_PRED

#define DECL_GET_SUCC \
  if (successor != NULL) return successor; \
  if (stayInBlock == true) return NULL; \
  const BasicBlock *parent = this->getParent(); \
  const BasicBlock *next = parent->getNextBlock(); \
  while (next) { \
    Instruction *first = next->getFirstInstruction(); \
    if (first) return first; \
    next = next->getNextBlock(); \
  } \
  return NULL;

  Instruction *Instruction::getSuccessor(bool stayInBlock) {
    DECL_GET_SUCC;
  }
  const Instruction *Instruction::getSuccessor(bool stayInBlock) const {
    DECL_GET_SUCC;
  }

#undef DECL_GET_SUCC

} /* namespace ir */
} /* namespace gbe */


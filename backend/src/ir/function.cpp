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
#include "sys/string.hpp"

namespace gbe {
namespace ir {

  Function::Function(const std::string &name, Profile profile) :
    name(name), profile(profile) { initProfile(*this); }

  Function::~Function(void) {
    for (auto it = blocks.begin(); it != blocks.end(); ++it)
      GBE_DELETE(*it);
    for (auto it = inputs.begin(); it != inputs.end(); ++it)
      GBE_DELETE(*it);
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

  uint32_t Function::getFirstSpecialReg(void) const {
    return this->profile == PROFILE_OCL ? 0u : ~0u;
  }

  uint32_t Function::getSpecialRegNum(void) const {
    return this->profile == PROFILE_OCL ? ocl::regNum : ~0u;
  }

  void Function::computeCFG(void) {
    // Clear possible previously computed CFG
    this->foreachBlock([this](BasicBlock &bb) {
      bb.successors.clear();
      bb.predecessors.clear();
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
    out << "## " << fn.inputNum() << " input register"
        << plural(fn.inputNum())  << " ##" << std::endl;
    for (uint32_t i = 0; i < fn.inputNum(); ++i) {
      const FunctionInput &input = fn.getInput(i);
      out << "decl_input.";
      switch (input.type) {
        case FunctionInput::GLOBAL_POINTER: out << "global"; break;
        case FunctionInput::LOCAL_POINTER: out << "local"; break;
        case FunctionInput::CONSTANT_POINTER: out << "constant"; break;
        case FunctionInput::VALUE: out << "value"; break;
        case FunctionInput::STRUCTURE:
          out << "structure." << input.elementSize;
        break;
        default: break;
      }
      out << " %" << input.reg << std::endl;
    }
    out << "## " << fn.outputNum() << " output register"
        << plural(fn.outputNum()) << " ##" << std::endl;
    for (uint32_t i = 0; i < fn.outputNum(); ++i)
      out << "decl_output %" << fn.getOutput(i) << std::endl;
    out << "## " << fn.blockNum() << " block"
        << plural(fn.blockNum()) << " ##" << std::endl;
    for (uint32_t i = 0; i < fn.blockNum(); ++i) {
      const BasicBlock &bb = fn.getBlock(i);
      bb.foreach([&out] (const Instruction &insn) {
        out << insn << std::endl;
      });
      out << std::endl;
    }
    out << ".end_function" << std::endl;
    return out;
  }

  BasicBlock::BasicBlock(Function &fn) : fn(fn) {
    this->first = this->last = NULL;
  }
  BasicBlock::~BasicBlock(void) {
    this->foreach([this] (Instruction &insn) {
     this->fn.deleteInstruction(&insn);
    });
  }

} /* namespace ir */
} /* namespace gbe */




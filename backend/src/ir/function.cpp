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

  Function::Function(const std::string &name) : name(name) {}

  Function::~Function(void) {
    for (auto it = blocks.begin(); it != blocks.end(); ++it)
      GBE_DELETE(*it);
  }

  LabelIndex Function::newLabel(void) {
    GBE_ASSERTM(labels.size() < 0xffff,
                "Too many labels are defined (65536 only are supported)");
    const LabelIndex index(labels.size());
    labels.push_back(NULL);
    return index;
  }

  std::ostream &Function::outImmediate(std::ostream &out, ImmediateIndex index) const {
    GBE_ASSERT(index < immediates.size());
    const Immediate imm = immediates[index];
    switch (imm.type) {
      case TYPE_BOOL: return out << !!imm.data.u8;
      case TYPE_S8: return out << imm.data.s8;
      case TYPE_U8: return out << imm.data.u8;
      case TYPE_S16: return out << imm.data.s16;
      case TYPE_U16: return out << imm.data.u16;
      case TYPE_S32: return out << imm.data.s32;
      case TYPE_U32: return out << imm.data.u32;
      case TYPE_S64: return out << imm.data.s64;
      case TYPE_U64: return out << imm.data.u64;
      case TYPE_HALF: return out << "half(" << imm.data.u16 << ")";
      case TYPE_FLOAT: return out << imm.data.f32;
      case TYPE_DOUBLE: return out << imm.data.f64;
    };
    return out;
  }

  std::ostream &operator<< (std::ostream &out, const Function &fn)
  {
    out << ".decl_function " << fn.getName() << std::endl << std::endl;
    out << fn.getRegisterFile();
    out << "## " << fn.inputNum() << " input register"
        << plural(fn.inputNum())  << " ##" << std::endl;
    for (uint32_t i = 0; i < fn.inputNum(); ++i)
      out << "decl_input %" << fn.getInput(i) << std::endl;
    out << "## " << fn.outputNum() << " output register"
        << plural(fn.outputNum()) << " ##" << std::endl;
    for (uint32_t i = 0; i < fn.outputNum(); ++i)
      out << "decl_output %" << fn.getOutput(i) << std::endl;
    out << "## " << fn.blockNum() << " block"
        << plural(fn.blockNum()) << " ##" << std::endl;
    for (uint32_t i = 0; i < fn.blockNum(); ++i) {
      const BasicBlock &bb = fn.getBlock(i);
      bb.map([&out, &fn] (const Instruction &insn) {
        out << insn.proxy(fn) << std::endl;
      });
      out << std::endl;
    }
    out << ".end_function" << std::endl;
    return out;
  }

  BasicBlock::BasicBlock(Function &fn) : fn(fn) {}
  BasicBlock::~BasicBlock(void) {
    for (auto it = instructions.begin(); it != instructions.end(); ++it)
      fn.deleteInstruction(*it);
  }

} /* namespace ir */
} /* namespace gbe */


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
 * \file liveness.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "ir/liveness.hpp"
#include <sstream>

namespace gbe {
namespace ir {

  Liveness::Liveness(Function &fn) : fn(fn) {
    // Initialize UEVar and VarKill for each block
    fn.apply([this](const BasicBlock &bb) { this->initBlock(bb); });
    // Now with iterative analysis, we compute liveout sets
    this->computeLiveOut();
  }

  Liveness::~Liveness(void) {
    for (auto it = liveness.begin(); it != liveness.end(); ++it)
      GBE_SAFE_DELETE(it->second);
  }

  void Liveness::initBlock(const BasicBlock &bb) {
    GBE_ASSERT(liveness.find(&bb) == liveness.end());
    BlockInfo *info = GBE_NEW(BlockInfo, bb);
    // Traverse all instructions to handle UEVar and VarKill
    bb.apply([this, info](const Instruction &insn) {
      this->initInstruction(*info, insn);
    });
    liveness[&bb] = info;
  }

  void Liveness::initInstruction(BlockInfo &info, const Instruction &insn) {
    const uint32_t srcNum = insn.getSrcNum();
    const uint32_t dstNum = insn.getDstNum();
    const Function &fn = info.bb.getParent();
    // First look for used before killed
    for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
      const Register reg = insn.getSrcIndex(fn, srcID);
      // Not killed -> it is really an upward use
      if (info.varKill.find(reg) == info.varKill.end())
        info.upwardUsed.insert(reg);
    }
    // A destination is a killed value
    for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
      const Register reg = insn.getDstIndex(fn, dstID);
      info.varKill.insert(reg);
    }
  }

  void Liveness::computeLiveOut(void) {
    // First insert the UEVar from the successors
    forEachSuccessor([](BlockInfo &info, const BlockInfo &succ) {
      const UEVar &ueVarSet = succ.upwardUsed;
      // Iterate over all the registers in the UEVar of our successor
      for (auto ueVar = ueVarSet.begin(); ueVar != ueVarSet.end(); ++ueVar)
        info.liveOut.insert(*ueVar);
    });
    // Now iterate on liveOut
    bool changed = true;
    while (changed) {
      changed = false;
      forEachSuccessor([&changed](BlockInfo &info, const BlockInfo &succ) {
        const UEVar &killSet = succ.varKill;
        const LiveOut &liveOut = succ.liveOut;
        auto end = killSet.end();
        // Iterate over all the registers in the UEVar of our successor
        for (auto living = liveOut.begin(); living != liveOut.end(); ++living) {
          if (killSet.find(*living) != end) continue;
          if (info.liveOut.find(*living) != info.liveOut.end()) continue;
          info.liveOut.insert(*living);
          changed = true;
        }
      });
    }
  }

  static const uint32_t prettyInsnStrSize = 48;
  static const uint32_t prettyRegStrSize = 5;

  enum RegisterUse
  {
    USE_NONE    = 0,
    USE_READ    = 1,
    USE_WRITTEN = 2
  };

  /*! "next" includes the provided instruction */
  static INLINE RegisterUse nextUse(const Instruction &insn, Register reg) {
    const Function &fn = insn.getParent()->getParent();
    const Instruction *curr = &insn;
    while (curr) {
      for (uint32_t srcID = 0; srcID < curr->getSrcNum(); ++srcID) {
        const Register src = curr->getSrcIndex(fn, srcID);
        if (src == reg) return USE_READ;
      }
      for (uint32_t dstID = 0; dstID < curr->getDstNum(); ++dstID) {
        const Register dst = curr->getDstIndex(fn, dstID);
        if (dst == reg) return USE_WRITTEN;
      }
      curr = curr->getSuccessor();
    }
    return USE_NONE;
  }
  /*! "previous" does not include the provided instruction */
  static INLINE RegisterUse previousUse(const Instruction &insn, Register reg) {
    return USE_NONE;
  }

  /*! Just print spaceNum spaces */
  static INLINE void printSpaces(std::ostream &out, uint32_t spaceNum) {
    for (uint32_t space = 0; space < spaceNum; ++space) out << " ";
  }
  /*! Print the "alive" string */
  static INLINE void printAlive(std::ostream &out) {
    static_assert(prettyRegStrSize == 5, "Bad register string size");
    out << " #   ";
  }
  /*! Print the "dead" string */
  static INLINE void printDead(std::ostream &out) {
    static_assert(prettyRegStrSize == 5, "Bad register string size");
    out << " .   ";
  }
  /*! Print the instruction liveness for each register */
  static void printInstruction(std::ostream &out,
                               const Liveness::BlockInfo &info,
                               const Instruction &insn)
  {
    const BasicBlock &bb = info.bb;
    const Function &fn = bb.getParent();

    // Print the instruction first
    {
      std::stringstream ss;
      ss << insn;
      std::string str = ss.str();
      str.resize(std::min((uint32_t)str.size(), prettyInsnStrSize));
      out << str;
      printSpaces(out, prettyInsnStrSize - str.size());
    }

    // Now print the liveness "." for dead and "#" for alive.
    {
      for (uint32_t regID = 0; regID < fn.regNum(); ++regID) {
        const Register reg(regID);
        // Non-killed and liveout == alive in the complete block
        if (info.inLiveOut(reg) == true && info.inVarKill(reg) == false)
          printAlive(out);
        // We must look for the last use of the instruction
        else if (info.inLiveOut(reg) == false) {

        } else
         printDead(out);
      }
    }
    out << std::endl;
  }
  /*! Print all the instruction liveness for the given block */
  static void printBlock(std::ostream &out, const Liveness::BlockInfo &info) {
    const BasicBlock &bb = info.bb;
    const Function &fn = bb.getParent();
    bb.apply([&out, &info](const Instruction &insn) {
      printInstruction(out, info, insn);
    });
    // At the end of block, we also output the variables actually alive at the
    // end of the block
    printSpaces(out, prettyInsnStrSize);
    for (uint32_t reg = 0; reg < fn.regNum(); ++reg) {
      if (info.inLiveOut(Register(reg)) == true)
        printAlive(out);
      else
        printDead(out);
    }

    out << std::endl; // let a blank line
  }

  std::ostream &operator<< (std::ostream &out, const Liveness &liveness) {
    const Function &fn = liveness.getFunction();
    const uint32_t regNum = fn.regNum();
    printSpaces(out, prettyInsnStrSize);

    // Print all the function registers
    for (uint32_t reg = 0; reg < regNum; ++reg) {
      std::stringstream ss;
      ss << "%" << reg;
      std::string str = ss.str();
      str.resize(std::min((uint32_t)str.size(), prettyRegStrSize));
      out << str;
      printSpaces(out, prettyRegStrSize - str.size());
    }
    out << std::endl << std::endl; // skip a line

    // Print liveness in each block
    fn.apply([&out, &liveness] (const BasicBlock &bb) {
      const Liveness::Info &info = liveness.getLiveness();
      auto it = info.find(&bb);
      GBE_ASSERT(it != info.end());
      printBlock(out, *it->second);
    });
    return out;
  }

} /* namespace ir */
} /* namespace gbe */


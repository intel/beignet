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
    fn.foreachBlock([this](const BasicBlock &bb) { this->initBlock(bb); });
    // Now with iterative analysis, we compute liveout sets
    this->computeLiveOut();
  }

  Liveness::~Liveness(void) {
    for (auto it = liveness.begin(); it != liveness.end(); ++it)
      GBE_SAFE_DELETE(it->second);
  }

  void Liveness::initBlock(const BasicBlock &bb) {
    GBE_ASSERT(liveness.contains(&bb) == false);
    BlockInfo *info = GBE_NEW(BlockInfo, bb);
    // Traverse all instructions to handle UEVar and VarKill
    bb.foreach([this, info](const Instruction &insn) {
      this->initInstruction(*info, insn);
    });
    liveness[&bb] = info;
  }

  void Liveness::initInstruction(BlockInfo &info, const Instruction &insn) {
    const uint32_t srcNum = insn.getSrcNum();
    const uint32_t dstNum = insn.getDstNum();
    // First look for used before killed
    for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
      const Register reg = insn.getSrc(srcID);
      // Not killed -> it is really an upward use
      if (info.varKill.contains(reg) == false)
        info.upwardUsed.insert(reg);
    }
    // A destination is a killed value
    for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
      const Register reg = insn.getDst(dstID);
      info.varKill.insert(reg);
    }
  }

  void Liveness::computeLiveOut(void) {
    // First insert the UEVar from the successors
    foreach<DF_SUCC>([](BlockInfo &info, const BlockInfo &succ) {
      const UEVar &ueVarSet = succ.upwardUsed;
      // Iterate over all the registers in the UEVar of our successor
      for (auto ueVar = ueVarSet.begin(); ueVar != ueVarSet.end(); ++ueVar)
        info.liveOut.insert(*ueVar);
    });
    // Now iterate on liveOut
    bool changed = true;
    while (changed) {
      changed = false;
      foreach<DF_SUCC>([&changed](BlockInfo &info, const BlockInfo &succ) {
        const UEVar &killSet = succ.varKill;
        const LiveOut &liveOut = succ.liveOut;
        // Iterate over all the registers in the UEVar of our successor
        for (auto living = liveOut.begin(); living != liveOut.end(); ++living) {
          if (killSet.contains(*living)) continue;
          if (info.liveOut.contains(*living)) continue;
          info.liveOut.insert(*living);
          changed = true;
        }
      });
    }
  }

  /*! To pretty print the livfeness info */
  static const uint32_t prettyInsnStrSize = 48;
  static const uint32_t prettyRegStrSize = 5;

  /*! Describe how the register is used */
  static const uint32_t USE_NONE    = 0;
  static const uint32_t USE_READ    = 1 << 0;
  static const uint32_t USE_WRITTEN = 1 << 1;

  enum UsePosition {
    POS_BEFORE = 0,
    POS_HERE = 1,
    POS_AFTER = 2
  };

  /*! Compute the use of a register in all direction in a block */
  template <UsePosition pos>
  static INLINE uint32_t usage(const Instruction &insn, Register reg) {
    const Instruction *curr = &insn;
    uint32_t use = USE_NONE;

    // Skip the current element if you are looking forward or backward
    if (curr && pos == POS_BEFORE)
      curr = curr->getPredecessor();
    else if (curr && pos == POS_AFTER)
      curr = curr->getSuccessor();
    while (curr) {
      for (uint32_t srcID = 0; srcID < curr->getSrcNum(); ++srcID) {
        const Register src = curr->getSrc(srcID);
        if (src == reg) {
          use |= USE_READ;
          break;
        }
      }
      for (uint32_t dstID = 0; dstID < curr->getDstNum(); ++dstID) {
        const Register dst = curr->getDst(dstID);
        if (dst == reg) {
          use |= USE_WRITTEN;
          break;
        }
      }
      if (use != USE_NONE)
        break;
      if (pos == POS_BEFORE)
        curr = curr->getPredecessor();
      else if (pos == POS_AFTER)
        curr = curr->getSuccessor();
      else
        curr = NULL;
    }
    return use;
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
        // Use in that instruction means alive
        if (usage<POS_HERE>(insn, reg) != USE_NONE) {
          printAlive(out);
          continue;
        }
        // Non-killed and liveout == alive in the complete block
        if (info.inLiveOut(reg) == true && info.inVarKill(reg) == false) {
          printAlive(out);
          continue;
        }
        // It is going to be read
        const uint32_t nextUsage = usage<POS_AFTER>(insn, reg);
        if ((nextUsage & USE_READ) != USE_NONE) {
          printAlive(out);
          continue;
        }
        // It is not written and alive at the end of the block
        if ((nextUsage & USE_WRITTEN) == USE_NONE && info.inLiveOut(reg) == true) {
          printAlive(out);
          continue;
        }
        printDead(out);
      }
    }
    out << std::endl;
  }
  /*! Print all the instruction liveness for the given block */
  static void printBlock(std::ostream &out, const Liveness::BlockInfo &info) {
    const BasicBlock &bb = info.bb;
    const Function &fn = bb.getParent();
    bb.foreach([&out, &info](const Instruction &insn) {
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
    fn.foreachBlock([&out, &liveness] (const BasicBlock &bb) {
      const Liveness::Info &info = liveness.getLivenessInfo();
      auto it = info.find(&bb);
      GBE_ASSERT(it != info.end());
      printBlock(out, *it->second);
    });
    return out;
  }

} /* namespace ir */
} /* namespace gbe */


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
    for (auto &pair : liveness) GBE_SAFE_DELETE(pair.second);
  }

  void Liveness::initBlock(const BasicBlock &bb) {
    GBE_ASSERT(liveness.contains(&bb) == false);
    BlockInfo *info = GBE_NEW(BlockInfo, bb);
    // Traverse all instructions to handle UEVar and VarKill
    const_cast<BasicBlock&>(bb).foreach([this, info](const Instruction &insn) {
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
      for (auto ueVar : ueVarSet) info.liveOut.insert(ueVar);
    });
    // Now iterate on liveOut
    bool changed = true;
    while (changed) {
      changed = false;
      foreach<DF_SUCC>([&changed](BlockInfo &info, const BlockInfo &succ) {
        const UEVar &killSet = succ.varKill;
        const LiveOut &liveOut = succ.liveOut;
        // Iterate over all the registers in the UEVar of our successor
        for (auto living : liveOut) {
          if (killSet.contains(living)) continue;
          if (info.liveOut.contains(living)) continue;
          info.liveOut.insert(living);
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
} /* namespace ir */
} /* namespace gbe */


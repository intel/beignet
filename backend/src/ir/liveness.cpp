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
    fn.foreachBlock([this](const BasicBlock &bb) {
      this->initBlock(bb);
      // If the bb has ret instruction, add it to the work list set.
      const Instruction *lastInsn = bb.getLastInstruction();
      const ir::Opcode op = lastInsn->getOpcode();
      if (op == OP_RET) {
        struct BlockInfo * info = liveness[&bb];
        workList.push_back(info);
        info->liveOut.insert(ocl::retVal);
      }
    });
    // Now with iterative analysis, we compute liveout and livein sets
    this->computeLiveInOut();

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

// Use simple backward data flow analysis to solve the liveness problem.
  void Liveness::computeLiveInOut(void) {
    do {
      struct BlockInfo *currInfo = workList.pop_front();
      for (auto currOutVar : currInfo->liveOut)
        if (!currInfo->varKill.contains(currOutVar))
          currInfo->upwardUsed.insert(currOutVar);
      bool isChanged = false;
      for (auto prev : currInfo->bb.getPredecessorSet()) {
        BlockInfo *prevInfo = liveness[prev];
        for (auto currInVar : currInfo->upwardUsed) {
          auto changed = prevInfo->liveOut.insert(currInVar);
          if (changed.second) isChanged = true;
        }
        if (isChanged ) workList.push_back(prevInfo);
      }
    } while (!workList.empty());

#if 0
    fn.foreachBlock([this](const BasicBlock &bb){
      printf("label %d:\n", bb.getLabelIndex());
      BlockInfo *info = liveness[&bb];
      auto &outVarSet = info->liveOut;
      auto &inVarSet = info->upwardUsed;
      printf("\tout Lives: ");
      for (auto outVar : outVarSet) {
        printf("%d ", outVar);
      }
      printf("\n\tin Lives: ");
      for (auto inVar : inVarSet) {
        printf("%d ", inVar);
      }
      printf("\n");
    });
#endif
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


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
      struct BlockInfo * info = liveness[&bb];
      if (op == OP_RET) {
        workSet.insert(info);
        info->liveOut.insert(ocl::retVal);
      } else if (op == OP_BRA) {
        // If this is a backward jump, put it to the extra work list.
        if (((BranchInstruction*)lastInsn)->getLabelIndex() < bb.getLabelIndex())
          extraWorkSet.insert(info);
      }
    });
    // Now with iterative analysis, we compute liveout and livein sets
    this->computeLiveInOut();
    for (auto it : extraWorkSet) {
      for (auto reg : it->liveOut) {
        it->extraLiveIn.insert(reg);
      }
    }
    this->computeExtraLiveInOut();
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
    while(!workSet.empty()) {
      auto currInfo = *workSet.begin();
      workSet.erase(currInfo);
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
        if (isChanged )
          workSet.insert(prevInfo);
      }
    };
#if 0
    fn.foreachBlock([this](const BasicBlock &bb){
      printf("label %d:\n", bb.getLabelIndex());
      BlockInfo *info = liveness[&bb];
      auto &outVarSet = info->liveOut;
      auto &inVarSet = info->upwardUsed;
      auto &extraInVarSet = info->extraLiveIn;
      auto &extraOutVarSet = info->extraLiveOut;
      printf("\n\tin Lives: ");
      for (auto inVar : inVarSet) {
        printf("%d ", inVar);
      }
      printf("\n");
      printf("\tout Lives: ");
      for (auto outVar : outVarSet) {
        printf("%d ", outVar);
      }
      printf("\n");

    });
#endif
   }

/*
  Consider the following scenario, %100's normal liveness will start from Ln-1's
  position. In normal analysis, the Ln-1 is not Ln's predecessor, thus the liveness
  of %100 will be passed to Ln and then will not be passed to L0.

  But considering we are running on a multilane with predication's vector machine.
  The unconditional BR in Ln-1 may be removed and it will enter Ln with a subset of
  the revert set of Ln-1's predication. For example when running Ln-1, the active lane
  is 0-7, then at Ln the active lane is 8-15. Then at the end of Ln, a subset of 8-15
  will jump to L0. If a register %10 is allocated the same GRF as %100, given the fact
  that their normal liveness doesn't overlapped, the a subset of 8-15 lanes will be
  modified. If the %10 and %100 are the same vector data type, then we are fine. But if
  %100 is a float vector, and the %10 is a bool or short vector, then we hit a bug here.

L0:
  ...
  %10 = 5
  ...
Ln-1:
  %100 = 2
  BR Ln+1

Ln:
  ...
  BR(%xxx) L0

Ln+1:
  %101 = %100 + 2;
  ...

  The solution to fix this issue is to build another liveness data. We will start with
  those BBs with backward jump. Then pass all the liveOut register as extra liveIn
  of current BB and then forward this extra liveIn to all the blocks. This is very similar
  to the normal liveness analysis just with reverse direction.
*/
  void Liveness::computeExtraLiveInOut(void) {
    while(!extraWorkSet.empty()) {
      struct BlockInfo *currInfo = *extraWorkSet.begin();
      extraWorkSet.erase(currInfo);
      for (auto currInVar : currInfo->extraLiveIn)
        currInfo->extraLiveOut.insert(currInVar);
      bool isChanged = false;
      for (auto succ : currInfo->bb.getSuccessorSet()) {
        BlockInfo *succInfo = liveness[succ];
        for (auto currOutVar : currInfo->extraLiveOut) {
          bool changed = false;
          if (!succInfo->upwardUsed.contains(currOutVar)) {
            auto it  = succInfo->extraLiveIn.insert(currOutVar);
            changed = it.second;
          }
          if (changed) isChanged = true;
        }
        if (isChanged)
          extraWorkSet.insert(succInfo);}
    };
#if 0
    fn.foreachBlock([this](const BasicBlock &bb){
      printf("label %d:\n", bb.getLabelIndex());
      BlockInfo *info = liveness[&bb];
      auto &outVarSet = info->liveOut;
      auto &inVarSet = info->upwardUsed;
      auto &extraInVarSet = info->extraLiveIn;
      auto &extraOutVarSet = info->extraLiveOut;
      printf("\n\tin Lives: ");
      for (auto inVar : inVarSet) {
        printf("%d ", inVar);
      }
      printf("\n\textra in Lives: ");
      for (auto inVar : extraInVarSet) {
        printf("%d ", inVar);
      }
      printf("\n");
      printf("\tout Lives: ");
      for (auto outVar : outVarSet) {
        printf("%d ", outVar);
      }
      printf("\n\textra out Lives: ");
      for (auto outVar : extraOutVarSet) {
        printf("%d ", outVar);
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


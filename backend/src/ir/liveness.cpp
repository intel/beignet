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
#include "ir/function.hpp"
#include "sys/map.hpp"
#include "sys/set.hpp"

namespace gbe {
namespace ir {

  /*! Compute liveness of each register */
  class LivenessInfo
  {
  public:
    LivenessInfo(Function &fn);
    ~LivenessInfo(void);
    /*! Set of variables used upwards in the block (before a definition) */
    typedef set<Register> UEVar;
    /*! Set of variables alive at the exit of the block */
    typedef set<Register> LiveOut;
    /*! Set of variables actually killed in each block */
    typedef set<Register> VarKill;
    /*! Per-block info */
    struct BlockInfo {
      BlockInfo(const BasicBlock &bb) : bb(bb) {}
      const BasicBlock &bb;
      UEVar upwardUsed;
      LiveOut liveOut;
      VarKill varKill;
    };
    /*! Gives for each block the variables alive at entry / exit */
    typedef map<const BasicBlock*, BlockInfo*> Liveness;
  private:
    /*! Store the liveness of all blocks */
    Liveness liveness;
    /*! Compute the liveness for this function */
    Function &fn;
    /*! Initialize UEVar and VarKill per block */
    void initBlock(const BasicBlock &bb);
    /*! Initialize UEVar and VarKill per instruction */
    void initInstruction(BlockInfo &info, const Instruction &insn);
    /*! Now really compute LiveOut based on UEVar and VarKill */
    void computeLiveOut(void);
    /*! Actually do something for each successor of *all* blocks */
    template <typename T>
    void forEachSuccessor(const T &functor) {
      // Iterate on all blocks
      for (auto it = liveness.begin(); it != liveness.end(); ++it) {
        BlockInfo &info = *it->second;
        const BasicBlock &bb = info.bb;
        const BlockSet set = bb.getSuccessorSet();
        // Iterate over all successors
        for (auto other = set.begin(); other != set.end(); ++other) {
          auto otherInfo = liveness.find(*other);
          GBE_ASSERT(otherInfo != liveness.end() && otherInfo->second != NULL);
          functor(info, *otherInfo->second);
        }
      }
    }
  };

  LivenessInfo::LivenessInfo(Function &fn) : fn(fn) {
    // Initialize UEVar and VarKill for each block
    fn.apply([this](const BasicBlock &bb) { this->initBlock(bb); });
    // Now with iterative analysis, we compute liveout sets
    this->computeLiveOut();
  }

  LivenessInfo::~LivenessInfo(void) {
    for (auto it = liveness.begin(); it != liveness.end(); ++it)
      GBE_SAFE_DELETE(it->second);
  }

  void LivenessInfo::initBlock(const BasicBlock &bb) {
    GBE_ASSERT(liveness.find(&bb) == liveness.end());
    BlockInfo *info = GBE_NEW(BlockInfo, bb);
    // Traverse all instructions to handle UEVar and VarKill
    bb.apply([this, info](const Instruction &insn) {
      this->initInstruction(*info, insn);
    });
    liveness[&bb] = info;
  }

  void LivenessInfo::initInstruction(BlockInfo &info, const Instruction &insn) {
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

  void LivenessInfo::computeLiveOut(void) {
    // First insert the UEVar from the successors
    forEachSuccessor([](BlockInfo &info, const BlockInfo &succ) {
      const UEVar &ueVarSet = succ.upwardUsed;
      // Iterate over all the registers in the UEVar of our successor
      for (auto ueVar = ueVarSet.begin(); ueVar != ueVarSet.end(); ++ueVar)
        info.liveOut.insert(*ueVar);
    });
    int counter = 0;
    // Now iterate on liveOut
    bool changed = true;
    while (changed) {
      changed = false;
      forEachSuccessor([&changed, &counter](BlockInfo &info, const BlockInfo &succ) {
        const UEVar &killSet = succ.varKill;
        const LiveOut &liveOut = succ.liveOut;
        auto end = killSet.end();
        // Iterate over all the registers in the UEVar of our successor
        for (auto living = liveOut.begin(); living != liveOut.end(); ++living) {
          counter++;
          if (killSet.find(*living) != end) continue;
          if (info.liveOut.find(*living) != info.liveOut.end()) continue;
          info.liveOut.insert(*living);
          changed = true;
        }
      });
    }
    std::cout << counter << std::endl;
  }

} /* namespace ir */
} /* namespace gbe */


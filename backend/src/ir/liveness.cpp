/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

  Liveness::Liveness(Function &fn, bool isInGenBackend) : fn(fn) {
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
      }
    });
    // Now with iterative analysis, we compute liveout and livein sets
    while (unvisitBlocks.size()) {
      if (workSet.size() == 0)
        workSet.insert(--unvisitBlocks.end(), unvisitBlocks.end());
      this->computeLiveInOut();
    }
    // extend register (def in loop, use out-of-loop) liveness to the whole loop
    set<Register> extentRegs;
    // Only in Gen backend we need to take care of extra live out analysis.
    if (isInGenBackend) {
      this->computeExtraLiveInOut(extentRegs);
      // analyze uniform values. The extentRegs contains all the values which is
      // defined in a loop and use out-of-loop which could not be a uniform. The reason
      // is that when it reenter the second time, it may active different lanes. So
      // reenter many times may cause it has different values in different lanes.
      this->analyzeUniform(&extentRegs);
    }
  }

  void Liveness::removeRegs(const set<Register> &removes) {
    for (auto &pair : liveness) {
      BlockInfo &info = *(pair.second);
      for (auto reg : removes) {
        if (info.liveOut.contains(reg))
          info.liveOut.erase(reg);
        if (info.upwardUsed.contains(reg))
          info.upwardUsed.erase(reg);
      }
    }
  }

  void Liveness::replaceRegs(const map<Register, Register> &replaceMap) {

    for (auto &pair : liveness) {
      BlockInfo &info = *pair.second;
      BasicBlock *bb = const_cast<BasicBlock *>(&info.bb);
      for (auto &pair : replaceMap) {
        Register from = pair.first;
        Register to = pair.second;
        if (info.liveOut.contains(from)) {
          info.liveOut.erase(from);
          info.liveOut.insert(to);
          // FIXME, a hack method to avoid the "to" register be treated as
          // uniform value.
          bb->definedPhiRegs.insert(to);
        }
        if (info.upwardUsed.contains(from)) {
          info.upwardUsed.erase(from);
          info.upwardUsed.insert(to);
        }
        if (info.varKill.contains(from)) {
          info.varKill.erase(from);
          info.varKill.insert(to);
        }
        if (bb->undefPhiRegs.contains(from)) {
          bb->undefPhiRegs.erase(from);
          bb->undefPhiRegs.insert(to);
        }
      }
    }
  }

  Liveness::~Liveness(void) {
    for (auto &pair : liveness) GBE_SAFE_DELETE(pair.second);
  }

  void Liveness::analyzeUniform(set<Register> *extentRegs) {
    fn.foreachBlock([this, extentRegs](const BasicBlock &bb) {
      const_cast<BasicBlock&>(bb).foreach([this, extentRegs](const Instruction &insn) {
        const uint32_t srcNum = insn.getSrcNum();
        const uint32_t dstNum = insn.getDstNum();
        bool uniform = true;

        //do not change dst uniform for simd id
        if (insn.getOpcode() == ir::OP_SIMD_ID)
          uniform = false;

        // do not change dst uniform for block read
        if ((insn.getOpcode() == ir::OP_LOAD && ir::cast<ir::LoadInstruction>(insn).isBlock()) ||
            insn.getOpcode() == ir::OP_MBREAD)
          uniform = false;

        for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
          const Register reg = insn.getSrc(srcID);
          if (!fn.isUniformRegister(reg))
            uniform = false;
        }

        // A destination is a killed value
        for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
          const Register reg = insn.getDst(dstID);
          int opCode = insn.getOpcode();
          // FIXME, ADDSAT and uniform vector should be supported.
          if (uniform &&
              fn.getRegisterFamily(reg) != ir::FAMILY_QWORD &&
              !insn.getParent()->definedPhiRegs.contains(reg) &&
              opCode != ir::OP_ATOMIC &&
              opCode != ir::OP_MUL_HI &&
              opCode != ir::OP_HADD &&
              opCode != ir::OP_RHADD &&
              opCode != ir::OP_READ_ARF &&
              opCode != ir::OP_ADDSAT &&
              opCode != ir::OP_IME &&
              (dstNum == 1 || insn.getOpcode() != ir::OP_LOAD) &&
              !extentRegs->contains(reg)
             )
            fn.setRegisterUniform(reg, true);
        }
      });
    });
  }

  void Liveness::initBlock(const BasicBlock &bb) {
    GBE_ASSERT(liveness.contains(&bb) == false);
    BlockInfo *info = GBE_NEW(BlockInfo, bb);
    // Traverse all instructions to handle UEVar and VarKill
    const_cast<BasicBlock&>(bb).foreach([this, info](const Instruction &insn) {
      this->initInstruction(*info, insn);
    });
    liveness[&bb] = info;
    unvisitBlocks.insert(info);
    if(!bb.liveout.empty())
      info->liveOut.insert(bb.liveout.begin(), bb.liveout.end());
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
      if (unvisitBlocks.find(currInfo) != unvisitBlocks.end())
        unvisitBlocks.erase(currInfo);
      for (auto currOutVar : currInfo->liveOut)
        if (!currInfo->varKill.contains(currOutVar))
          currInfo->upwardUsed.insert(currOutVar);
      bool isChanged = false;
      for (auto prev : currInfo->bb.getPredecessorSet()) {
        BlockInfo *prevInfo = liveness[prev];
        if (unvisitBlocks.find(currInfo) != unvisitBlocks.end())
          unvisitBlocks.erase(currInfo);
        for (auto currInVar : currInfo->upwardUsed) {
          if (!prevInfo->bb.undefPhiRegs.contains(currInVar)) {
            auto changed = prevInfo->liveOut.insert(currInVar);
            if (changed.second) isChanged = true;
          }
        }
        if (isChanged )
          workSet.insert(prevInfo);
      }
    };
   }
/*
  As we run in SIMD mode with prediction mask to indicate active lanes.
  If a vreg is defined in a loop, and there are som uses of the vreg out of the loop,
  the define point may be run several times under *different* prediction mask.
  For these kinds of vreg, we must extend the vreg liveness into the whole loop.
  If we don't do this, it's liveness is killed before the def point inside loop.
  If the vreg's corresponding physical reg is assigned to other vreg during the
  killed period, and the instructions before kill point were re-executed with different prediction,
  the inactive lanes of vreg maybe over-written. Then the out-of-loop use will got wrong data.
*/
  void Liveness::computeExtraLiveInOut(set<Register> &extentRegs) {
    const vector<Loop *> &loops = fn.getLoops();
    extentRegs.clear();
    if(loops.size() == 0) return;

    for (auto l : loops) {
      const BasicBlock &preheader = fn.getBlock(l->preheader);
      BlockInfo *preheaderInfo = liveness[&preheader];
      for (auto x : l->exits) {
        const BasicBlock &a = fn.getBlock(x.first);
        const BasicBlock &b = fn.getBlock(x.second);
        BlockInfo * exiting = liveness[&a];
        BlockInfo * exit = liveness[&b];
        std::vector<Register> toExtend;
        std::vector<Register> toExtendCand;

        if(b.getPredecessorSet().size() <= 1) {
          // the exits only have one predecessor
          for (auto p : exit->upwardUsed)
            toExtendCand.push_back(p);
        } else {
          // the exits have more than one predecessors
          std::set_intersection(exiting->liveOut.begin(),
                                exiting->liveOut.end(),
                                exit->upwardUsed.begin(),
                                exit->upwardUsed.end(),
                                std::back_inserter(toExtendCand));
        }
        // toExtendCand may contain some virtual register defined before loop,
        // which need to be excluded. Because what we need is registers defined
        // in the loop. Such kind of registers must be in live-out of the loop's
        // preheader. So we do the subtraction here.
        std::set_difference(toExtendCand.begin(),
                            toExtendCand.end(),
                            preheaderInfo->liveOut.begin(),
                            preheaderInfo->liveOut.end(),
                            std::back_inserter(toExtend));

        if (toExtend.size() == 0) continue;
        for(auto r : toExtend)
          extentRegs.insert(r);
        for (auto bb : l->bbs) {
          BlockInfo * bI = liveness[&fn.getBlock(bb)];
          for(auto r : toExtend) {
            if(!bI->upwardUsed.contains(r))
              bI->upwardUsed.insert(r);
            bI->liveOut.insert(r);
          }
        }
      }
    }
   }

  std::ostream &operator<< (std::ostream &out, const Liveness &live) {
    const Function &fn = live.getFunction();
    fn.foreachBlock([&] (const BasicBlock &bb) {
      out << std::endl;
      out << "Label $" << bb.getLabelIndex() << std::endl;
      const Liveness::BlockInfo &bbInfo = live.getBlockInfo(&bb);
      out << "liveIn:" << std::endl;
      for (auto &x: bbInfo.upwardUsed) {
        out << x << " ";
      }
      out << std::endl << "liveOut:" << std::endl;
      for (auto &x : bbInfo.liveOut)
        out << x << " ";
      out << std::endl << "varKill:" << std::endl;
      for (auto &x : bbInfo.varKill)
        out << x << " ";
      out << std::endl;
    });
    return out;
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


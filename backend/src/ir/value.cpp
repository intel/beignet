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
 * \file value.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "ir/value.hpp"
#include "ir/liveness.hpp"

namespace gbe {
namespace ir {

  /*! To build the chains (i.e. basically the DAG of values), we are going to
   *  iterate on liveout definitions: for each block and for each variable
   *  (ir::Register) alive at the end of the block (in Block::LiveOut), we are
   *  computing the set of all possible value definitions. Using these value
   *  definitions, we will finally transfer these sets to the successors to get
   *  the ud / du chains
   *
   *  LiveOutSet contains the set of definitions for each basic block
   */
  class LiveOutSet
  {
  public:
    LiveOutSet(Liveness &liveness, const FunctionDAG &dag);
    ~LiveOutSet(void);
    /*! One set per register */
    typedef set<const ValueDef*> RegDefSet;
    /*! We have one map of liveout register per block */
    typedef map<Register, RegDefSet*> BlockDefMap;
    /*! All the block definitions map in the functions */
    typedef map<const BasicBlock*, BlockDefMap*> FunctionDefMap;
    /*! Performs the double look-up to get the set of defs per register */
    RegDefSet &getDefSet(const BasicBlock *bb, const Register &reg);
    /*! Build a UD-chain as the union of the predecessor chains */
    void fillUDChain(UDChain &udChain, const BasicBlock &bb, const Register &reg);
    /*! Fast per register definition set allocation */
    DECL_POOL(RegDefSet, regDefSetPool);
    /*! Fast register sets allocation */
    DECL_POOL(BlockDefMap, blockDefMapPool);
    FunctionDefMap defMap;    //!< All per-block data
    Liveness &liveness;       //!< Contains LiveOut information
    const FunctionDAG &dag;   //!< Structure we are building
  private:
    /*! Initialize liveOut with the instruction destination values */
    void initializeInstructionDst(void);
    /*! Initialize liveOut with the function argument */
    void initializeFunctionInput(void);
    /*! Iterate to completely transfer the liveness and get the def sets */
    void iterateLiveOut(void);
  };

  LiveOutSet::LiveOutSet(Liveness &liveness, const FunctionDAG &dag) :
    liveness(liveness), dag(dag)
  {
    this->initializeInstructionDst();
    this->initializeFunctionInput();
    this->iterateLiveOut();
  }

  LiveOutSet::RegDefSet &LiveOutSet::getDefSet(const BasicBlock *bb,
                                               const Register &reg)
  {
    auto bbIt = defMap.find(bb);
    GBE_ASSERT(bbIt != defMap.end());
    auto defIt = bbIt->second->find(reg);
    GBE_ASSERT(defIt != bbIt->second->end() && defIt->second != NULL);
    return *defIt->second;
  }

  void LiveOutSet::fillUDChain(UDChain &udChain,
                               const BasicBlock &bb,
                               const Register &reg)
  {
  }

  void LiveOutSet::initializeInstructionDst(void) {
    const Function &fn = liveness.getFunction();

    // Iterate over each block and initialize the liveOut data
    fn.foreachBlock([&](const BasicBlock &bb) {
      GBE_ASSERT(defMap.find(&bb) == defMap.end());

      // Allocate a map of register definition
      auto blockDefMap = this->newBlockDefMap();
      defMap.insert(std::make_pair(&bb, blockDefMap));

      // We only consider liveout registers
      const auto &info = this->liveness.getBlockInfo(bb);
      const auto &liveOut = info.liveOut;
      for (auto it = liveOut.begin(); it != liveOut.end(); ++it) {
        GBE_ASSERT(blockDefMap->find(*it) == blockDefMap->end());
        auto regDefSet = this->newRegDefSet();
        blockDefMap->insert(std::make_pair(*it, regDefSet));
      }

      // Now traverse the blocks backwards and find the definition of each
      // liveOut register
      set<Register> defined; // Liveout registers for which we found a def
      bb.rforeach([&](const Instruction &insn) {
        const uint32_t dstNum = insn.getDstNum();
        for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
          const Register reg = insn.getDstIndex(fn, dstID);
          // We only take the most recent definition
          if (defined.contains(reg) == true) continue;
          // Not in LiveOut, so does not matter
          if (info.inLiveOut(reg) == false) continue;
          defined.insert(reg);
          // Insert the outgoing definition for this register
          auto regDefSet = blockDefMap->find(reg);
          const ValueDef *def = this->dag.getDefAddress(&insn, dstID);
          GBE_ASSERT(regDefSet != blockDefMap->end() && def != NULL);
          // May be NULL if there is no definition
          regDefSet->second->insert(def);
        }
      });
    });

    // The first block must also transfer the function arguments
    const BasicBlock &top = fn.getBlock(0);
    const auto &info = this->liveness.getBlockInfo(top);
    auto blockDefMapIt = defMap.find(&top);
    GBE_ASSERT(blockDefMapIt != defMap.end());
    auto blockDefMap = blockDefMapIt->second;
    const uint32_t inputNum = fn.inputNum();
    for (uint32_t inputID = 0; inputID < inputNum; ++inputID) {
      const FunctionInput &input = fn.getInput(inputID);
      const Register reg = input.reg;
      // Do not transfer dead values
      if (info.inLiveOut(reg) == false) continue;
      // If we overwrite it, do not transfer the initial value
      if (info.inVarKill(reg) == false) continue;
      const ValueDef *def = this->dag.getDefAddress(&input);
      GBE_ASSERT(blockDefMap->contains(reg) == false);
      auto regDefSet = this->newRegDefSet();
      regDefSet->insert(def);
      blockDefMap->insert(std::make_pair(reg, regDefSet));
    }
  }

  void LiveOutSet::initializeFunctionInput(void) {
    const Function &fn = liveness.getFunction();
    const uint32_t inputNum = fn.inputNum();

    // The first block must also transfer the function arguments
    const BasicBlock &top = fn.getBlock(0);
    const Liveness::BlockInfo &info = this->liveness.getBlockInfo(top);
    GBE_ASSERT(defMap.contains(&top) == true);
    auto blockDefMap = defMap.find(&top)->second;

    // Insert all the values that are not overwritten in the block and alive at
    // the end of it
    for (uint32_t inputID = 0; inputID < inputNum; ++inputID) {
      const FunctionInput &input = fn.getInput(inputID);
      const Register reg = input.reg;
      // Do not transfer dead values
      if (info.inLiveOut(reg) == false) continue;
      // If we overwrite it, do not transfer the initial value
      if (info.inVarKill(reg) == false) continue;
      const ValueDef *def = this->dag.getDefAddress(&input);
      GBE_ASSERT(blockDefMap->contains(reg) == false);
      auto regDefSet = this->newRegDefSet();
      regDefSet->insert(def);
      blockDefMap->insert(std::make_pair(reg, regDefSet));
    }
  }

  void LiveOutSet::iterateLiveOut(void) {
    bool changed = true;

    while (changed) {
      changed = false;

      // Compute the union of the current liveout definitions with the previous
      // ones. Do not take into account the killed values though
      liveness.foreach<DF_PRED>([&](Liveness::BlockInfo &curr,
                                    const Liveness::BlockInfo &pred)
      {
        const BasicBlock &bb = curr.bb;
        const BasicBlock &pbb = pred.bb;
        for (auto it = curr.liveOut.begin(); it != curr.liveOut.end(); ++it) {
          const Register reg = *it;
          if (pred.inLiveOut(reg) == false) continue;
          if (curr.inVarKill(reg) == true) continue;
          RegDefSet &currSet = this->getDefSet(&bb, reg);
          RegDefSet &predSet = this->getDefSet(&pbb, reg);

          // Transfer the values
          for (auto it = predSet.begin(); it != predSet.end(); ++it) {
            if (currSet.contains(*it)) continue;
            changed = true;
            currSet.insert(*it);
          }
        }
      });
    }
  }

  LiveOutSet::~LiveOutSet(void) {
    for (auto it = defMap.begin(); it != defMap.end(); ++it) {
      BlockDefMap *block = it->second;
      for (auto regSet = block->begin();regSet != block->end(); ++regSet)
        this->deleteRegDefSet(regSet->second);
      this->deleteBlockDefMap(block);
    }
  }

  FunctionDAG::FunctionDAG(Liveness &liveness) {
    const Function &fn = liveness.getFunction();

    // We first start with empty chains
    udEmpty = this->newUDChain();
    duEmpty = this->newDUChain();

    // First create the chains and insert them in their respective maps
    fn.foreachInstruction([this, udEmpty, duEmpty](const Instruction &insn) {
      // sources == value uses
      const uint32_t srcNum = insn.getSrcNum();
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        ValueUse *valueUse = this->newValueUse(&insn, srcID);
        useName.insert(std::make_pair(*valueUse, valueUse));
        udGraph.insert(std::make_pair(*valueUse, udEmpty));
      }
      // destinations == value defs
      const uint32_t dstNum = insn.getDstNum();
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
        ValueDef *valueDef = this->newValueDef(&insn, dstID);
        defName.insert(std::make_pair(*valueDef, valueDef));
        duGraph.insert(std::make_pair(*valueDef, duEmpty));
      }
    });

    // Function arguments are also value definitions
    const uint32_t inputNum = fn.inputNum();
    for (uint32_t inputID = 0; inputID < inputNum; ++inputID) {
      const FunctionInput &input = fn.getInput(inputID);
      ValueDef *valueDef = this->newValueDef(&input);
      defName.insert(std::make_pair(*valueDef, valueDef));
      duGraph.insert(std::make_pair(*valueDef, duEmpty));
    }

    // We create the liveOutSet to help us transfer the definitions
    const LiveOutSet liveOutSet(liveness, *this);
  }

/*! Helper to deallocate objects */
#define PTR_RELEASE(TYPE, VAR)                      \
  do {                                              \
    if (VAR && destroyed.contains(VAR) == false) {  \
      destroyed.insert(VAR);                        \
      delete##TYPE(VAR);                            \
    }                                               \
  } while (0)

  FunctionDAG::~FunctionDAG(void) {

    // We track the already destroyed pointers
    set<void*> destroyed;

    // Release the empty ud-chains and du-chains
    PTR_RELEASE(UDChain, udEmpty);
    PTR_RELEASE(DUChain, duEmpty);

    // We free all the ud-chains
    for (auto it = udGraph.begin(); it != udGraph.end(); ++it) {
      auto defs = it->second;
      for (auto def = defs->begin(); def != defs->end(); ++def)
        PTR_RELEASE(ValueDef, *def);
      PTR_RELEASE(UDChain, defs);
    }

    // We free all the du-chains
    for (auto it = duGraph.begin(); it != duGraph.end(); ++it) {
      auto uses = it->second;
      for (auto use = uses->begin(); use != uses->end(); ++use)
        PTR_RELEASE(ValueUse, *use);
      PTR_RELEASE(DUChain, uses);
    }
  }
#undef PTR_RELEASE

  const DUChain &FunctionDAG::getUse(const Instruction *insn, uint32_t dstID) const {
    const ValueDef def(insn, dstID);
    auto it = duGraph.find(def);
    GBE_ASSERT(it != duGraph.end());
    return *it->second;
  }

  const DUChain &FunctionDAG::getUse(const FunctionInput *input) const {
    const ValueDef def(input);
    auto it = duGraph.find(def);
    GBE_ASSERT(it != duGraph.end());
    return *it->second;
  }

  const UDChain &FunctionDAG::getDef(const Instruction *insn, uint32_t srcID) const {
    const ValueUse use(insn, srcID);
    auto it = udGraph.find(use);
    GBE_ASSERT(it != udGraph.end());
    return *it->second;
  }
  const ValueDef *FunctionDAG::getDefAddress(const Instruction *insn, uint32_t dstID) const {
    const ValueDef def(insn, dstID);
    auto it = defName.find(def);
    GBE_ASSERT(it != defName.end() && it->second != NULL);
    return it->second;
  }
  const ValueDef *FunctionDAG::getDefAddress(const FunctionInput *input) const {
    const ValueDef def(input);
    auto it = defName.find(def);
    GBE_ASSERT(it != defName.end() && it->second != NULL);
    return it->second;
  }
  const ValueUse *FunctionDAG::getUseAddress(const Instruction *insn, uint32_t srcID) const {
    const ValueUse use(insn, srcID);
    auto it = useName.find(use);
    GBE_ASSERT(it != useName.end() && it->second != NULL);
    return it->second;
  }
} /* namespace ir */
} /* namespace gbe */


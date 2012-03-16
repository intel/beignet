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
    LiveOutSet(const Liveness &liveness, const FunctionDAG &dag);
    ~LiveOutSet(void);
    /*! One set per register */
    typedef set<const ValueDef*> RegDefSet;
    /*! We have one map of liveout register per block */
    typedef map<Register, RegDefSet*> BlockDefMap;
    /*! All the block definitions map in the functions */
    typedef map<const BasicBlock*, BlockDefMap*> FunctionDefMap;
    FunctionDefMap defMap;    //!< All per-block data
    const Liveness &liveness; //!< Contains LiveOut information
    const FunctionDAG &dag;   //!< Structure we are building
    DECL_POOL(RegDefSet, regDefSetPool);
    DECL_POOL(BlockDefMap, blockDefMapPool);
  };

  LiveOutSet::LiveOutSet(const Liveness &liveness, const FunctionDAG &dag) :
    liveness(liveness), dag(dag)
  {
    const Function &fn = liveness.getFunction();

    // Iterate over each block
    fn.foreachBlock([&](const BasicBlock &bb) {
      GBE_ASSERT(defMap.find(&bb) == defMap.end());

      // Allocate a map of register definition
      auto blockDefMap = this->newBlockDefMap();
      defMap.insert(std::make_pair(&bb, blockDefMap));

      // We only consider liveout registers
      auto info = this->liveness.getBlockInfo(bb);
      auto liveOut = info.liveOut;
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
          if (defined.contains(reg) == false) continue;
          // Not in LiveOut, so does not matter
          if (info.inLiveOut(reg) == false) continue;
          // Insert the outgoing definition for this register
          auto regDefSet = blockDefMap->find(reg);
          const ValueDef *def = this->dag.getDefAddress(insn, dstID);
          GBE_ASSERT(regDefSet != blockDefMap->end() && def != NULL);
          regDefSet->second->insert(def);
        }
      });
    });
  }

  LiveOutSet::~LiveOutSet(void) {
    for (auto it = defMap.begin(); it != defMap.end(); ++it) {
      BlockDefMap *block = it->second;
      for (auto regSet = block->begin();regSet != block->end(); ++regSet)
        this->deleteRegDefSet(regSet->second);
      this->deleteBlockDefMap(block);
    }
  }

  FunctionDAG::FunctionDAG(const Liveness &liveness) {
    const Function &fn = liveness.getFunction();
    LiveOutSet p(liveness, *this);
    // We first start with empty chains
    udEmpty = this->newUDChain(); udEmpty->second = NULL;
    duEmpty = this->newDUChain(); duEmpty->second = NULL;

    // First create the chains and insert them in their respective maps
    fn.foreachInstruction([this, udEmpty, duEmpty](const Instruction &insn) {

      // sources == value uses
      const uint32_t srcNum = insn.getSrcNum();
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        ValueUse *valueUse = this->newValueUse(insn, srcID);
        udGraph.insert(std::make_pair(*valueUse, udEmpty));
      }
      // destinations == value defs
      const uint32_t dstNum = insn.getDstNum();
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
        ValueDef *valueDef = this->newValueDef(insn, dstID);
        duGraph.insert(std::make_pair(*valueDef, duEmpty));
      }
    });

    // Function arguments are also value definitions
    const uint32_t inputNum = fn.inputNum();
    for (uint32_t inputID = 0; inputID < inputNum; ++inputID) {
      const FunctionInput &input = fn.getInput(inputID);
      ValueDef *valueDef = this->newValueDef(input);
      duGraph.insert(std::make_pair(*valueDef, duEmpty));
    }
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
    PTR_RELEASE(ValueUse, udEmpty->second);
    PTR_RELEASE(ValueDef, duEmpty->second);
    PTR_RELEASE(UDChain, udEmpty);
    PTR_RELEASE(DUChain, duEmpty);

    // We free all the ud-chains
    for (auto it = udGraph.begin(); it != udGraph.end(); ++it) {
      auto udChain = it->second;
      auto defs = udChain->first;
      for (auto def = defs.begin(); def != defs.end(); ++def)
        PTR_RELEASE(ValueDef, *def);
      PTR_RELEASE(ValueUse, udChain->second);
      PTR_RELEASE(UDChain, udChain);
    }

    // We free all the du-chains
    for (auto it = duGraph.begin(); it != duGraph.end(); ++it) {
      auto duChain = it->second;
      auto uses = duChain->first;
      for (auto use = uses.begin(); use != uses.end(); ++use)
        PTR_RELEASE(ValueUse, *use);
      PTR_RELEASE(ValueDef, duChain->second);
      PTR_RELEASE(DUChain, duChain);
    }
  }
#undef PTR_RELEASE

  const DUChain &FunctionDAG::getDUChain(const Instruction &insn, uint32_t dstID) const {
    const ValueDef def(insn, dstID);
    auto it = duGraph.find(def);
    GBE_ASSERT(it != duGraph.end());
    return *it->second;
  }

  const DUChain &FunctionDAG::getDUChain(const FunctionInput &input) const {
    const ValueDef def(input);
    auto it = duGraph.find(def);
    GBE_ASSERT(it != duGraph.end());
    return *it->second;
  }

  const UDChain &FunctionDAG::getUDChain(const Instruction &insn, uint32_t srcID) const {
    const ValueUse use(insn, srcID);
    auto it = udGraph.find(use);
    GBE_ASSERT(it != udGraph.end());
    return *it->second;
  }
  const ValueUseSet &FunctionDAG::getUse(const Instruction &insn, uint32_t dstID) const {
    const DUChain &chain = this->getDUChain(insn, dstID);
    return chain.first;
  }
  const ValueUseSet &FunctionDAG::getUse(const FunctionInput &input) const {
    const DUChain &chain = this->getDUChain(input);
    return chain.first;
  }
  const ValueDefSet &FunctionDAG::getDef(const Instruction &insn, uint32_t srcID) const {
    const UDChain &chain = this->getUDChain(insn, srcID);
    return chain.first;
  }
  const ValueDef *FunctionDAG::getDefAddress(const Instruction &insn, uint32_t dstID) const {
    const DUChain &chain = this->getDUChain(insn, dstID);
    return chain.second;
  }
  const ValueDef *FunctionDAG::getDefAddress(const FunctionInput &input) const {
    const DUChain &chain = this->getDUChain(input);
    return chain.second;
  }
  const ValueUse *FunctionDAG::getUseAddress(const Instruction &insn, uint32_t srcID) const {
    const UDChain &chain = this->getUDChain(insn, srcID);
    return chain.second;
  }
} /* namespace ir */
} /* namespace gbe */


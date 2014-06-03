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

  /*! To build the chains (i.e. basically the graph of values), we are going to
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
    typedef set<ValueDef*> RegDefSet;
    /*! We have one map of liveout register per block */
    typedef map<Register, RegDefSet*> BlockDefMap;
    /*! All the block definitions map in the functions */
    typedef map<const BasicBlock*, BlockDefMap*> FunctionDefMap;
    /*! Performs the double look-up to get the set of defs per register */
    RegDefSet &getDefSet(const BasicBlock *bb, Register reg);
    /*! Build a UD-chain as the union of the predecessor chains */
    void makeDefSet(DefSet &udChain, const BasicBlock &bb, Register reg);
    /*! Fast per register definition set allocation */
    DECL_POOL(RegDefSet, regDefSetPool);
    /*! Fast register sets allocation */
    DECL_POOL(BlockDefMap, blockDefMapPool);
    FunctionDefMap defMap;    //!< All per-block data
    Liveness &liveness;       //!< Contains LiveOut information
    const FunctionDAG &dag;   //!< Structure we are building
  private:
    /*! Initialize liveOut with the instruction destination values */
    void initializeInstructionDef(void);
    /*! Initialize liveOut with the function argument, special and pushed
     *  registers
     */
    void initializeOtherDef(void);
    /*! Iterate to completely transfer the liveness and get the def sets */
    void iterateLiveOut(void);
    /*! Use custom allocators */
    GBE_CLASS(LiveOutSet);
  };

  /*! Debug print of the liveout set */
  std::ostream &operator<< (std::ostream &out, LiveOutSet &set);

  LiveOutSet::LiveOutSet(Liveness &liveness, const FunctionDAG &dag) :
    liveness(liveness), dag(dag)
  {
    this->initializeInstructionDef();
    this->initializeOtherDef();
    this->iterateLiveOut();
  }

  LiveOutSet::RegDefSet &LiveOutSet::getDefSet(const BasicBlock *bb, Register reg)
  {
    auto bbIt = defMap.find(bb);
    GBE_ASSERT(bbIt != defMap.end());
    auto defIt = bbIt->second->find(reg);
    GBE_ASSERT(defIt != bbIt->second->end() && defIt->second != NULL);
    return *defIt->second;
  }

  void LiveOutSet::makeDefSet(DefSet &udChain, const BasicBlock &bb, Register reg)
  {
    // Iterate over all the predecessors
    const auto &preds = bb.getPredecessorSet();
    for (const auto &pred : preds) {
      if (pred->undefPhiRegs.contains(reg))
        continue;
      RegDefSet &predDef = this->getDefSet(pred, reg);
      for (auto def : predDef) udChain.insert(def);
    }

    // If this is the top block we must take into account both function
    // arguments and special registers
    const Function &fn = bb.getParent();
    if (fn.isEntryBlock(bb) == false) return;

    // Is it a function input?
    const FunctionArgument *arg = fn.getArg(reg);
    const PushLocation *pushed = fn.getPushLocation(reg);

    // Is it a pushed register?
    if (pushed != NULL) {
      ValueDef *def = const_cast<ValueDef*>(dag.getDefAddress(pushed));
      udChain.insert(def);
    }
    // Is a function argument?
    else if (arg != NULL) {
      ValueDef *def = const_cast<ValueDef*>(dag.getDefAddress(arg));
      udChain.insert(def);
    }
    // Is it a special register?
    else if (fn.isSpecialReg(reg) == true) {
      ValueDef *def = const_cast<ValueDef*>(dag.getDefAddress(reg));
      udChain.insert(def);
    }
  }

  void LiveOutSet::initializeInstructionDef(void) {
    const Function &fn = liveness.getFunction();

    // Iterate over each block and initialize the liveOut data
    fn.foreachBlock([&](const BasicBlock &bb) {
      GBE_ASSERT(defMap.find(&bb) == defMap.end());

      // Allocate a map of register definitions
      auto blockDefMap = this->newBlockDefMap();
      defMap.insert(std::make_pair(&bb, blockDefMap));

      // We only consider liveout registers
      const auto &info = this->liveness.getBlockInfo(&bb);
      const auto &liveOut = info.liveOut;
      for (auto reg : liveOut) {
        GBE_ASSERT(blockDefMap->find(reg) == blockDefMap->end());
        auto regDefSet = this->newRegDefSet();
        blockDefMap->insert(std::make_pair(reg, regDefSet));
      }

      // Now traverse the blocks backwards and find the definition of each
      // liveOut register
      set<Register> defined;
      for (auto it = --bb.end(); it != bb.end(); --it) {
        const Instruction &insn = *it;
        const uint32_t dstNum = insn.getDstNum();
        for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
          const Register reg = insn.getDst(dstID);
          // We only take the most recent definition
          if (defined.contains(reg) == true) continue;
          // Not in LiveOut, so does not matter
          if (info.inLiveOut(reg) == false) continue;
          defined.insert(reg);
          // Insert the outgoing definition for this register
          auto regDefSet = blockDefMap->find(reg);
          ValueDef *def = const_cast<ValueDef*>(this->dag.getDefAddress(&insn, dstID));
          GBE_ASSERT(regDefSet != blockDefMap->end() && def != NULL);
          regDefSet->second->insert(def);
        }
      }
    });
  }

  void LiveOutSet::initializeOtherDef(void) {
    const Function &fn = liveness.getFunction();
    const uint32_t argNum = fn.argNum();

    // The first block must also transfer the function arguments
    const BasicBlock &top = fn.getTopBlock();
    const Liveness::BlockInfo &info = this->liveness.getBlockInfo(&top);
    GBE_ASSERT(defMap.contains(&top) == true);
    auto blockDefMap = defMap.find(&top)->second;

    // Insert all the values that are not overwritten in the block and alive at
    // the end of it
    for (uint32_t argID = 0; argID < argNum; ++argID) {
      const FunctionArgument &arg = fn.getArg(argID);
      const Register reg = arg.reg;
      // Do not transfer dead values
      if (info.inLiveOut(reg) == false) continue;
      // If we overwrite it, do not transfer the initial value
      if (info.inVarKill(reg) == true) continue;
      ValueDef *def = const_cast<ValueDef*>(this->dag.getDefAddress(&arg));
      auto it = blockDefMap->find(reg);
      GBE_ASSERT(it != blockDefMap->end());
      it->second->insert(def);
    }

    // Now transfer the special registers that are not over-written
    const uint32_t firstID = fn.getFirstSpecialReg();
    const uint32_t specialNum = fn.getSpecialRegNum();
    for (uint32_t regID = firstID; regID < firstID + specialNum; ++regID) {
      const Register reg(regID);
      // Do not transfer dead values
      if (info.inLiveOut(reg) == false) continue;
      // If we overwrite it, do not transfer the initial value
      if (info.inVarKill(reg) == true) continue;
      ValueDef *def = const_cast<ValueDef*>(this->dag.getDefAddress(reg));
      auto it = blockDefMap->find(reg);
      GBE_ASSERT(it != blockDefMap->end());
      it->second->insert(def);
    }

    // Finally do the same thing with pushed registers
    const Function::PushMap &pushMap = fn.getPushMap();
    for (const auto &pushed : pushMap) {
      const Register reg = pushed.first;
      // Do not transfer dead values
      if (info.inLiveOut(reg) == false) continue;
      // If we overwrite it, do not transfer the initial value
      if (info.inVarKill(reg) == true) continue;
      ValueDef *def = const_cast<ValueDef*>(this->dag.getDefAddress(&pushed.second));
      auto it = blockDefMap->find(reg);
      GBE_ASSERT(it != blockDefMap->end());
      it->second->insert(def);
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
        for (auto reg : curr.liveOut) {
          if (pred.inLiveOut(reg) == false) continue;
          if (curr.inVarKill(reg) == true) continue;
          RegDefSet &currSet = this->getDefSet(&bb, reg);
          RegDefSet &predSet = this->getDefSet(&pbb, reg);

          // Transfer the values
          for (auto def : predSet) {
            if (currSet.contains(def)) continue;
            changed = true;
            currSet.insert(def);
          }
        }
      });
    }
  }

  LiveOutSet::~LiveOutSet(void) {
    for (const auto pair : defMap) {
      BlockDefMap *block = pair.second;
      for (auto regSet : *block)
        this->deleteRegDefSet(regSet.second);
      this->deleteBlockDefMap(block);
    }
  }

  std::ostream &operator<< (std::ostream &out, LiveOutSet &set) {
    for (const auto &pair : set.defMap) {
      // To recognize the block, just print its instructions
      out << "Block:" << std::endl;
      for (const auto &insn : *pair.first) out << insn << std::endl;

      // Iterate over all alive registers to get their definitions
      const LiveOutSet::BlockDefMap *defMap = pair.second;
      if (defMap->size() > 0) out << "LiveSet:" << std::endl;
      for (const auto &pair : *defMap) {
        const Register reg = pair.first;
        const LiveOutSet::RegDefSet *set = pair.second;
        for (auto def : *set) {
          const ValueDef::Type type = def->getType();
          if (type == ValueDef::DEF_FN_ARG)
            out << "%" << reg << ": " << "function input" << std::endl;
          else if (type == ValueDef::DEF_FN_PUSHED)
            out << "%" << reg << ": " << "pushed register" << std::endl;
          else if (type == ValueDef::DEF_SPECIAL_REG)
            out << "%" << reg << ": " << "special register" << std::endl;
          else {
            const Instruction *insn = def->getInstruction();
            out << "%" << reg << ": " << insn << " " << *insn << std::endl;
          }
        }
      }
      out << std::endl;
    }
    return out;
  }

  FunctionDAG::FunctionDAG(Liveness &liveness) :
    fn(liveness.getFunction())
  {
    // We first start with empty chains
    udEmpty = this->newDefSet();
    duEmpty = this->newUseSet();

    // First create the chains and insert them in their respective maps
    fn.foreachInstruction([this](const Instruction &insn) {
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
    const uint32_t argNum = fn.argNum();
    for (uint32_t argID = 0; argID < argNum; ++argID) {
      const FunctionArgument &arg = fn.getArg(argID);
      ValueDef *valueDef = this->newValueDef(&arg);
      defName.insert(std::make_pair(*valueDef, valueDef));
      duGraph.insert(std::make_pair(*valueDef, duEmpty));
    }

    // Special registers are also definitions
    const uint32_t firstID = fn.getFirstSpecialReg();
    const uint32_t specialNum = fn.getSpecialRegNum();
    for (uint32_t regID = firstID; regID < firstID + specialNum; ++regID) {
      const Register reg(regID);
      ValueDef *valueDef = this->newValueDef(reg);
      defName.insert(std::make_pair(*valueDef, valueDef));
      duGraph.insert(std::make_pair(*valueDef, duEmpty));
    }

    // Pushed registers are also definitions
    const Function::PushMap &pushMap = fn.getPushMap();
    for (const auto &pushed : pushMap) {
      ValueDef *valueDef = this->newValueDef(&pushed.second);
      defName.insert(std::make_pair(*valueDef, valueDef));
      duGraph.insert(std::make_pair(*valueDef, duEmpty));
    }

    // We create the liveOutSet to help us transfer the definitions
    LiveOutSet liveOutSet(liveness, *this);

    // Build UD chains traversing the blocks top to bottom
    fn.foreachBlock([&](const BasicBlock &bb) {
      // Track the allocated chains to be able to reuse them
      map<Register, DefSet*> allocated;
      // Some chains may be not used (ie they are dead). We track them to be
      // able to deallocate them later
      set<DefSet*> unused;

      // For each instruction build the UD chains
      const_cast<BasicBlock&>(bb).foreach([&](const Instruction &insn) {
        // Instruction sources consumes definitions
        const uint32_t srcNum = insn.getSrcNum();
        for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
          const Register src = insn.getSrc(srcID);
          const ValueUse use(&insn, srcID);
          auto ud = udGraph.find(use);
          GBE_ASSERT(ud != udGraph.end());

          // We already allocate the ud chain for this register
          auto it = allocated.find(src);
          if (it != allocated.end()) {
            udGraph.erase(ud);
            udGraph.insert(std::make_pair(use, it->second));
            if (unused.contains(it->second))
              unused.erase(it->second);
          }
          // Create a new one from the predecessor chains (upward used value)
          else {
            DefSet *udChain = this->newDefSet();
            liveOutSet.makeDefSet(*udChain, bb, src);
            allocated.insert(std::make_pair(src, udChain));
            ud->second = udChain;
          }
        }

        // Instruction destinations create new chains
        const uint32_t dstNum = insn.getDstNum();
        for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
          const Register dst = insn.getDst(dstID);
          ValueDef *def = const_cast<ValueDef*>(this->getDefAddress(&insn, dstID));
          DefSet *udChain = this->newDefSet();
          udChain->insert(def);
          unused.insert(udChain);
          // Remove the previous definition if any
          if (allocated.contains(dst) == true)
            allocated.erase(dst);
          allocated.insert(std::make_pair(dst, udChain));
        }
      });

      // Deallocate unused chains
      for (auto set : unused) this->deleteDefSet(set);
    });

    // Build the DU chains from the UD ones
    fn.foreachInstruction([&](const Instruction &insn) {

      // For each value definition of each source, we push back this use
      const uint32_t srcNum = insn.getSrcNum();
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        ValueUse *use = const_cast<ValueUse*>(getUseAddress(&insn, srcID));

        // Find all definitions for this source
        const auto &defs = this->getDef(&insn, srcID);
        for (auto def : defs) {
          auto uses = duGraph.find(*def);
          UseSet *du = uses->second;
          GBE_ASSERT(uses != duGraph.end());
          if (du == duEmpty) {
            duGraph.erase(*def);
            du = this->newUseSet();
            duGraph.insert(std::make_pair(*def, du));
          }
          du->insert(use);
        }
      }
    });

    // Allocate the set of uses and defs per register
    const uint32_t regNum = fn.regNum();
    for (uint32_t regID = 0; regID < regNum; ++regID) {
      const Register reg(regID);
      UseSet *useSet = GBE_NEW_NO_ARG(UseSet);
      DefSet *defSet = GBE_NEW_NO_ARG(DefSet);
      regUse.insert(std::make_pair(reg, useSet));
      regDef.insert(std::make_pair(reg, defSet));
    }

    // Fill use sets (one per register)
    for (auto &useSet : duGraph) {
      for (auto use : *useSet.second) {
        const Register reg = use->getRegister();
        auto it = regUse.find(reg);
        GBE_ASSERT(it != regUse.end() && it->second != NULL);
        it->second->insert(use);
      }
    }

    // Fill def sets (one per register)
    for (auto &defSet : udGraph) {
      for (auto def : *defSet.second) {
        const Register reg = def->getRegister();
        auto it = regDef.find(reg);
        GBE_ASSERT(it != regDef.end() && it->second != NULL);
        it->second->insert(def);
      }
    }
  }

/*! Helper to deallocate objects */
#define PTR_RELEASE(TYPE, VAR) \
  do { \
    if (VAR && destroyed.contains(VAR) == false) { \
      destroyed.insert(VAR); \
      delete##TYPE(VAR); \
    } \
  } while (0)

  FunctionDAG::~FunctionDAG(void) {

    // We track the already destroyed pointers
    set<void*> destroyed;

    // Release the empty ud-chains and du-chains
    PTR_RELEASE(DefSet, udEmpty);
    PTR_RELEASE(UseSet, duEmpty);

    // We free all the ud-chains
    for (const auto &pair : udGraph) {
      auto defs = pair.second;
      if (destroyed.contains(defs)) continue;
      for (auto def : *defs) PTR_RELEASE(ValueDef, def);
      PTR_RELEASE(DefSet, defs);
    }

    // We free all the du-chains
    for (const auto &pair : duGraph) {
      auto uses = pair.second;
      if (destroyed.contains(uses)) continue;
      for (auto use : *uses) PTR_RELEASE(ValueUse, use);
      PTR_RELEASE(UseSet, uses);
    }

    // Release all the use and definition sets per register
    for (const auto &pair : regUse) GBE_SAFE_DELETE(pair.second);
    for (const auto &pair : regDef) GBE_SAFE_DELETE(pair.second);
  }
#undef PTR_RELEASE

  const UseSet &FunctionDAG::getUse(const ValueDef &def) const {
    auto it = duGraph.find(def);
    GBE_ASSERT(it != duGraph.end());
    return *it->second;
  }
  const UseSet &FunctionDAG::getUse(const Instruction *insn, uint32_t dstID) const {
    return this->getUse(ValueDef(insn, dstID));
  }
  const UseSet &FunctionDAG::getUse(const FunctionArgument *arg) const {
    return this->getUse(ValueDef(arg));
  }
  const UseSet &FunctionDAG::getUse(const Register &reg) const {
    return this->getUse(ValueDef(reg));
  }
  const DefSet &FunctionDAG::getDef(const ValueUse &use) const {
    auto it = udGraph.find(use);
    GBE_ASSERT(it != udGraph.end());
    return *it->second;
  }
  const DefSet &FunctionDAG::getDef(const Instruction *insn, uint32_t srcID) const {
    return this->getDef(ValueUse(insn, srcID));
  }
  const UseSet *FunctionDAG::getRegUse(const Register &reg) const {
    auto it = regUse.find(reg);
    GBE_ASSERT(it != regUse.end());
    return it->second;
  }
  const DefSet *FunctionDAG::getRegDef(const Register &reg) const {
    auto it = regDef.find(reg);
    GBE_ASSERT(it != regDef.end());
    return it->second;
  }

  const ValueDef *FunctionDAG::getDefAddress(const ValueDef &def) const {
    auto it = defName.find(def);
    GBE_ASSERT(it != defName.end() && it->second != NULL);
    return it->second;
  }
  const ValueDef *FunctionDAG::getDefAddress(const PushLocation *pushed) const {
    return this->getDefAddress(ValueDef(pushed));
  }
  const ValueDef *FunctionDAG::getDefAddress(const Instruction *insn, uint32_t dstID) const {
    return this->getDefAddress(ValueDef(insn, dstID));
  }
  const ValueDef *FunctionDAG::getDefAddress(const FunctionArgument *arg) const {
    return this->getDefAddress(ValueDef(arg));
  }
  const ValueDef *FunctionDAG::getDefAddress(const Register &reg) const {
    return this->getDefAddress(ValueDef(reg));
  }
  const ValueUse *FunctionDAG::getUseAddress(const Instruction *insn, uint32_t srcID) const {
    const ValueUse use(insn, srcID);
    auto it = useName.find(use);
    GBE_ASSERT(it != useName.end() && it->second != NULL);
    return it->second;
  }

  std::ostream &operator<< (std::ostream &out, const FunctionDAG &dag) {
    const Function &fn = dag.getFunction();

    // Print all uses for the definitions and all definitions for each uses
    fn.foreachInstruction([&](const Instruction &insn) {
      out << &insn << ": " << insn << std::endl;

      // Display the set of definition for each destination
      const uint32_t dstNum = insn.getDstNum();
      if (dstNum > 0) out << "USES:" << std::endl;
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
        const Register reg = insn.getDst(dstID);
        const auto &uses = dag.getUse(&insn, dstID);
        for (auto use : uses) {
          const Instruction *other = use->getInstruction();
          out << "  %" << reg << " " << other << ": " << *other << std::endl;
        }
      }

      // Display the set of definitions for each source
      const uint32_t srcNum = insn.getSrcNum();
      if (srcNum > 0) out << "DEFS:" << std::endl;
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        const Register reg = insn.getSrc(srcID);
        const auto &defs = dag.getDef(&insn, srcID);
        for (auto def : defs) {
          if (def->getType() == ValueDef::DEF_FN_PUSHED)
            out << "  %" << reg << " # pushed register" << std::endl;
          else if (def->getType() == ValueDef::DEF_FN_ARG)
            out << "  %" << reg << " # function argument" << std::endl;
          else if (def->getType() == ValueDef::DEF_SPECIAL_REG)
            out << "  %" << reg << " # special register" << std::endl;
          else {
            const Instruction *other = def->getInstruction();
            out << "  %" << reg << " " << other << ": " << *other << std::endl;
          }
        }
      }
      out << std::endl;
    });

    return out;
  }

} /* namespace ir */
} /* namespace gbe */


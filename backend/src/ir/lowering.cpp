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
 * \file lowering.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "ir/context.hpp"
#include "ir/value.hpp"
#include "ir/liveness.hpp"
#include "ir/constant_push.hpp"
#include "sys/set.hpp"

namespace gbe {
namespace ir {

  /*! Small helper class to lower return instructions */
  class ContextReturn : public Context
  {
  public:
    /*! Initialize a context dedicated to return instruction lowering */
    ContextReturn(Unit &unit) : Context(unit) {
      this->usedLabels = GBE_NEW(vector<uint8_t>);
    }
    /*! Lower the return instruction to gotos for the given function */
    void lower(const std::string &functionName);
  };

  void ContextReturn::lower(const std::string &functionName) {
    if ((this->fn = unit.getFunction(functionName)) == NULL)
      return;

    // Append a new block at the end of the function with a return instruction:
    // the only one we are going to have
    this->bb = &this->fn->getBottomBlock();
    const LabelIndex index = this->label();
    this->LABEL(index);
    const BasicBlock *lastBlock = this->bb;
    this->RET();

    // Now traverse all instructions and replace all returns by GOTO index
    fn->foreachInstruction([&](Instruction &insn) {
      if (insn.getParent() == lastBlock) return; // This is the last block
      if (insn.getOpcode() != OP_RET) return;
      const Instruction bra = ir::BRA(index);
      bra.replace(&insn);
    });
  }

  void lowerReturn(Unit &unit, const std::string &functionName) {
    ContextReturn *ctx = GBE_NEW(ContextReturn, unit);
    ctx->lower(functionName);
    GBE_DELETE(ctx);
  }

  /*! Characterizes how the argument is used (directly read, indirectly read,
   *  written)
   */
  enum ArgUse {
    ARG_DIRECT_READ = 0,
    ARG_INDIRECT_READ = 1,
    ARG_WRITTEN = 2
  };

  /*! Just to book keep the sequence of instructions that directly load an input
   *  argument
   */
  struct LoadAddImm {
    Instruction *load;    //!< Load from the argument
    Instruction *add;     //!< Can be NULL if we only have load(arg)
    Instruction *loadImm; //!< Can also be NULL
    uint64_t offset;      //!< Offset where to load in the structure
  };

  /*! List of direct loads */
  typedef vector<LoadAddImm> LoadAddImmSeq;

  /*! Helper class to lower function arguments if required */
  class FunctionArgumentLowerer
  {
  public:
    /*! Build the helper structure */
    FunctionArgumentLowerer(Unit &unit);
    /*! Free everything we needed */
    ~FunctionArgumentLowerer(void);
    /*! Perform all function arguments substitution if needed */
    void lower(const std::string &name);
    /*! Lower the given function argument accesses */
    void lower(FunctionArgument &arg);
    /*! Inspect the given function argument to see how it is used. If this is
     *  direct loads only, we also output the list of instructions used for each
     *  load
     */
    ArgUse getArgUse(FunctionArgument &arg);
    /*! Recursively look if there is a store in the given use */
    bool useStore(const ValueDef &def, set<const Instruction*> &visited);
    /*! Look if the pointer use only load with immediate offsets */
    bool matchLoadAddImm(const FunctionArgument &arg);
    Liveness *liveness; //!< To compute the function graph
    FunctionDAG *dag;   //!< Contains complete dependency information
    Unit &unit;         //!< The unit we process
    Function *fn;       //!< Function to patch
    LoadAddImmSeq seq;  //!< All the direct loads
  };

  FunctionArgumentLowerer::FunctionArgumentLowerer(Unit &unit) :
    liveness(NULL), dag(NULL), unit(unit) {}
  FunctionArgumentLowerer::~FunctionArgumentLowerer(void) {
    GBE_SAFE_DELETE(dag);
    GBE_SAFE_DELETE(liveness);
  }

  void FunctionArgumentLowerer::lower(const std::string &functionName) {
    if ((this->fn = unit.getFunction(functionName)) == NULL)
      return;
    GBE_SAFE_DELETE(dag);
    GBE_SAFE_DELETE(liveness);
    this->liveness = GBE_NEW(ir::Liveness, *fn);
    this->dag = GBE_NEW(ir::FunctionDAG, *this->liveness);
    const uint32_t argNum = fn->argNum();
    for (uint32_t argID = 0; argID < argNum; ++argID) {
      FunctionArgument &arg = fn->getInput(argID);
      if (arg.type != FunctionArgument::STRUCTURE) continue;
      this->lower(arg);
    }
  }

  bool FunctionArgumentLowerer::useStore(const ValueDef &def, set<const Instruction*> &visited)
  {
    const UseSet &useSet = dag->getUse(def);
    for (const auto &use : useSet) {
      const Instruction *insn = use->getInstruction();
      const uint32_t srcID = use->getSrcID();
      const Opcode opcode = insn->getOpcode();
      if (visited.contains(insn)) continue;
      visited.insert(insn);
      if (opcode == OP_STORE && srcID == StoreInstruction::addressIndex)
        return true;
      if (insn->isMemberOf<UnaryInstruction>() == false &&
          insn->isMemberOf<BinaryInstruction>() == false &&
          insn->isMemberOf<TernaryInstruction>() == false)
        continue;
      else {
        const uint32_t dstNum = insn->getDstNum();
        for (uint32_t dstID = 0; dstID < dstNum; ++dstID)
          if (this->useStore(ValueDef(insn, dstID), visited) == true)
            return true;
      }
    }
    return false;
  }

  INLINE uint64_t getOffsetFromImm(const Immediate &imm) {
    switch (imm.type) {
      case TYPE_DOUBLE:
      case TYPE_FLOAT:
      case TYPE_S64:
      case TYPE_U64:
      case TYPE_U32:
      case TYPE_U16:
      case TYPE_U8: return imm.data.u64;
      case TYPE_S32: return int64_t(imm.data.s32);
      case TYPE_S16: return int64_t(imm.data.s16);
      case TYPE_S8: return int64_t(imm.data.s8);
      case TYPE_BOOL:
      case TYPE_HALF: NOT_SUPPORTED; return 0;
    }
    return 0;
  }

  bool matchLoad(Instruction *insn,
                 Instruction *add,
                 Instruction *loadImm,
                 uint64_t offset,
                 LoadAddImm &loadAddImm)
  {
    const Opcode opcode = insn->getOpcode();

    if (opcode == OP_LOAD) {
      LoadInstruction *load = cast<LoadInstruction>(insn);
      if (load->getAddressSpace() != MEM_PRIVATE)
        return false;
      loadAddImm.load = insn;
      loadAddImm.add = add;
      loadAddImm.loadImm = loadImm;
      loadAddImm.offset = offset;
      return true;
    } else
      return false;
  }

  bool FunctionArgumentLowerer::matchLoadAddImm(const FunctionArgument &arg) {
    LoadAddImmSeq tmpSeq;

    // Inspect all uses of the function argument pointer
    const UseSet &useSet = dag->getUse(&arg);
    for (auto use : useSet) {
      Instruction *insn = const_cast<Instruction*>(use->getInstruction());
      const Opcode opcode = insn->getOpcode();

      // load dst arg
      LoadAddImm loadAddImm;
      if (matchLoad(insn, NULL, NULL, 0, loadAddImm)) {
        tmpSeq.push_back(loadAddImm);
        continue;
      }

      // add.ptr_type dst ptr other
      if (opcode != OP_ADD) return false;
      BinaryInstruction *add = cast<BinaryInstruction>(insn);
      const Type addType = add->getType();
      const RegisterFamily family = getFamily(addType);
      if (family != unit.getPointerFamily()) return false;
      if (addType == TYPE_FLOAT) return false;

      // step 1 -> check that the other source comes from a load immediate
      const uint32_t srcID = use->getSrcID();
      const uint32_t otherID = srcID ^ 1;
      const DefSet &defSet = dag->getDef(insn, otherID);
      const uint32_t defNum = defSet.size();
      if (defNum == 0 || defNum > 1) continue; // undefined or more than one def
      const ValueDef *otherDef = *defSet.begin();
      if (otherDef->getType() != ValueDef::DEF_INSN_DST) return false;
      Instruction *otherInsn = const_cast<Instruction*>(otherDef->getInstruction());
      if (otherInsn->getOpcode() != OP_LOADI) return false;
      LoadImmInstruction *loadImm = cast<LoadImmInstruction>(otherInsn);
      const Immediate imm = loadImm->getImmediate();
      const uint64_t offset = getOffsetFromImm(imm);

      // step 2 -> check that the results of the add are loads from private
      // memory
      const UseSet &addUseSet = dag->getUse(add, 0);
      for (auto addUse : addUseSet) {
        Instruction *insn = const_cast<Instruction*>(addUse->getInstruction());

        // We finally find something like load dst arg+imm
        LoadAddImm loadAddImm;
        if (matchLoad(insn, add, loadImm, offset, loadAddImm)) {
          tmpSeq.push_back(loadAddImm);
          continue;
        }
      }
    }

    // OK, the argument only need direct loads. We can now append all the
    // direct load definitions we found
    for (const auto &loadImmSeq : tmpSeq)
      seq.push_back(loadImmSeq);
    return true;
  }

  ArgUse FunctionArgumentLowerer::getArgUse(FunctionArgument &arg)
  {
    // case 1 - we may store something to the structure argument
    set<const Instruction*> visited;
    if (this->useStore(ValueDef(&arg), visited))
      return ARG_WRITTEN;

    // case 2 - we look for the patterns: LOAD(ptr) or LOAD(ptr+imm)
    if (this->matchLoadAddImm(arg))
      return ARG_DIRECT_READ;

    // case 3 - LOAD(ptr+runtime_value)
    return ARG_INDIRECT_READ;
  }

  void FunctionArgumentLowerer::lower(FunctionArgument &arg) {
    const ArgUse argUse = this->getArgUse(arg);
    GBE_ASSERTM(argUse != ARG_WRITTEN,
                "TODO A store to a structure argument "
                "(i.e. not a char/short/int/float argument) has been found. "
                "This is not supported yet");
    GBE_ASSERTM(argUse != ARG_INDIRECT_READ,
                "TODO Only direct loads of structure arguments are "
                "supported now");
  }

  void lowerFunctionArguments(Unit &unit, const std::string &functionName) {
    FunctionArgumentLowerer lowerer(unit);
    lowerer.lower(functionName);
  }

} /* namespace ir */
} /* namespace gbe */


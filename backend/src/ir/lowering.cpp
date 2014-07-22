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
 * \file lowering.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "ir/context.hpp"
#include "ir/value.hpp"
#include "ir/liveness.hpp"
#include "sys/set.hpp"

namespace gbe {
namespace ir {

  /*! Small helper class to lower return instructions */
  class ContextReturn : public Context
  {
  public:
    /*! Initialize a context dedicated to return instruction lowering */
    ContextReturn(Unit &unit) : Context(unit) {
      this->usedLabels = GBE_NEW_NO_ARG(vector<uint8_t>);
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
    ContextReturn ctx(unit);
    ctx.lower(functionName);
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
    uint32_t argID;       //!< Associated function argument
  };

  /*! List of direct loads */
  typedef vector<LoadAddImm> LoadAddImmSeq;

  /*! Helper class to lower function arguments if required */
  class FunctionArgumentLowerer : public Context
  {
  public:
    /*! Build the helper structure */
    FunctionArgumentLowerer(Unit &unit);
    /*! Free everything we needed */
    virtual ~FunctionArgumentLowerer(void);
    /*! Perform all function arguments substitution if needed */
    void lower(const std::string &name);
    /*! Lower the given function argument accesses */
    void lower(uint32_t argID);
    /*! Build the constant push for the function */
    void buildConstantPush(void);
    /*! Inspect the given function argument to see how it is used. If this is
     *  direct loads only, we also output the list of instructions used for each
     *  load
     */
    ArgUse getArgUse(uint32_t argID);
    /*! Recursively look if there is a store in the given use */
    bool useStore(const ValueDef &def, set<const Instruction*> &visited);
    /*! Look if the pointer use only load with immediate offsets */
    bool matchLoadAddImm(uint32_t argID);
    Liveness *liveness; //!< To compute the function graph
    FunctionDAG *dag;   //!< Contains complete dependency information
    LoadAddImmSeq seq;  //!< All the direct loads
  };

  INLINE uint64_t getOffsetFromImm(const Immediate &imm) {
    switch (imm.getType()) {
      // bit-cast these ones
      case TYPE_DOUBLE:
      case TYPE_FLOAT: NOT_SUPPORTED; return 0;
      case TYPE_S64:
      case TYPE_U64:
      case TYPE_U32:
      case TYPE_U16:
      case TYPE_U8:
      // sign extend these ones
      case TYPE_S32:
      case TYPE_S16:
      case TYPE_S8: return imm.getIntegerValue();
      case TYPE_BOOL:
      case TYPE_HALF: NOT_SUPPORTED; return 0;
      default:
        GBE_ASSERT(0 && "Unsupported imm type.\n");
    }
    return 0;
  }

  bool matchLoad(Instruction *insn,
                 Instruction *add,
                 Instruction *loadImm,
                 uint64_t offset,
                 uint32_t argID,
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
      loadAddImm.argID = argID;
      return true;
    } else
      return false;
  }


  FunctionArgumentLowerer::FunctionArgumentLowerer(Unit &unit) :
    Context(unit), liveness(NULL), dag(NULL) {}
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

    // Process all structure arguments and find all the direct loads we can
    // replace
    const uint32_t argNum = fn->argNum();
    for (uint32_t argID = 0; argID < argNum; ++argID) {
      FunctionArgument &arg = fn->getArg(argID);
      if (arg.type != FunctionArgument::STRUCTURE) continue;
      this->lower(argID);
    }

    // Build the constant push description and remove the instruction that
    // therefore become useless
    this->buildConstantPush();
  }

// Remove all the given instructions from the stream (if dead)
#define REMOVE_INSN(WHICH) \
  for (const auto &loadAddImm : seq) { \
    Instruction *WHICH = loadAddImm.WHICH; \
    if (WHICH == NULL) continue; \
    const UseSet &useSet = dag->getUse(WHICH, 0); \
    bool isDead = true; \
    for (auto use : useSet) { \
      if (dead.contains(use->getInstruction()) == false) { \
        isDead = false; \
        break; \
      } \
    } \
    if (isDead && !dead.contains(WHICH)) { \
      dead.insert(WHICH); \
      WHICH->remove(); \
    } \
  }

  void FunctionArgumentLowerer::buildConstantPush(void)
  {
    if (seq.size() == 0)
      return;

    // Track instructions we remove to recursively kill them properly
    set<const Instruction*> dead;

    // The argument location we already pushed (since the same argument location
    // can be used several times)
    set<PushLocation> inserted;
    for (const auto &loadAddImm : seq) {
      LoadInstruction *load = cast<LoadInstruction>(loadAddImm.load);
      const uint32_t valueNum = load->getValueNum();
      bool replaced = false;
      Instruction *ins_after = load; // the instruction to insert after.
      for (uint32_t valueID = 0; valueID < valueNum; ++valueID) {
        const Type type = load->getValueType();
        const RegisterFamily family = getFamily(type);
        const uint32_t size = getFamilySize(family);
        const uint32_t offset = loadAddImm.offset + valueID * size;
        const PushLocation argLocation(*fn, loadAddImm.argID, offset);
        Register pushed;
        const Register reg = load->getValue(valueID);
        if (offset != 0) {
          if(inserted.contains(argLocation)) {
            pushed = argLocation.getRegister();
          } else {
            // pushed register should be uniform register.
            pushed = fn->newRegister(family, true);
            this->appendPushedConstant(pushed, argLocation);
            inserted.insert(argLocation);
          }
        } else {
          pushed = fn->getArg(loadAddImm.argID).reg;
        }

        // TODO the MOV instruction can be most of the time avoided if the
        // register is never written. We must however support the register
        // replacement in the instruction interface to be able to patch all the
        // instruction that uses "reg"
        Instruction mov = ir::MOV(type, reg, pushed);
        mov.insert(ins_after, &ins_after);
        replaced = true;
      }

      if (replaced)
        dead.insert(load);
    }

    REMOVE_INSN(load)
    REMOVE_INSN(add)
    REMOVE_INSN(loadImm)
  }

#undef REMOVE_INSN

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
          insn->isMemberOf<BinaryInstruction>() == false)
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

  bool FunctionArgumentLowerer::matchLoadAddImm(uint32_t argID)
  {
    const FunctionArgument &arg = fn->getArg(argID);
    LoadAddImmSeq tmpSeq;

    // Inspect all uses of the function argument pointer
    const UseSet &useSet = dag->getUse(&arg);
    for (auto use : useSet) {
      Instruction *insn = const_cast<Instruction*>(use->getInstruction());
      const Opcode opcode = insn->getOpcode();

      // load dst arg
      LoadAddImm loadAddImm;
      if (matchLoad(insn, NULL, NULL, 0, argID, loadAddImm)) {
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
        if (matchLoad(insn, add, loadImm, offset, argID, loadAddImm)) {
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

  ArgUse FunctionArgumentLowerer::getArgUse(uint32_t argID)
  {
    FunctionArgument &arg = fn->getArg(argID);

    // case 1 - we may store something to the structure argument
    set<const Instruction*> visited;
    if (this->useStore(ValueDef(&arg), visited))
      return ARG_WRITTEN;

    // case 2 - we look for the patterns: LOAD(ptr) or LOAD(ptr+imm)
    if (this->matchLoadAddImm(argID))
      return ARG_DIRECT_READ;

    // case 3 - LOAD(ptr+runtime_value)
    return ARG_INDIRECT_READ;
  }

  void FunctionArgumentLowerer::lower(uint32_t argID) {
    IF_DEBUG(const ArgUse argUse = )this->getArgUse(argID);
#if GBE_DEBUG
    GBE_ASSERTM(argUse != ARG_WRITTEN,
                "TODO A store to a structure argument "
                "(i.e. not a char/short/int/float argument) has been found. "
                "This is not supported yet");
    GBE_ASSERTM(argUse != ARG_INDIRECT_READ,
                "TODO Only direct loads of structure arguments are "
                "supported now");
#endif /* GBE_DEBUG */
  }

  void lowerFunctionArguments(Unit &unit, const std::string &functionName) {
    FunctionArgumentLowerer lowerer(unit);
    lowerer.lower(functionName);
  }

} /* namespace ir */
} /* namespace gbe */


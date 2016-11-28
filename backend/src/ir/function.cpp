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
 * \file function.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "ir/function.hpp"
#include "ir/unit.hpp"
#include "sys/map.hpp"

namespace gbe {
namespace ir {

  ///////////////////////////////////////////////////////////////////////////
  // PushLocation
  ///////////////////////////////////////////////////////////////////////////

  Register PushLocation::getRegister(void) const {
    const Function::LocationMap &locationMap = fn.getLocationMap();
    GBE_ASSERT(locationMap.contains(*this) == true);
    return locationMap.find(*this)->second;
  }

  ///////////////////////////////////////////////////////////////////////////
  // Function
  ///////////////////////////////////////////////////////////////////////////

  Function::Function(const std::string &name, const Unit &unit, Profile profile) :
    name(name), unit(unit), profile(profile), simdWidth(0), useSLM(false), slmSize(0), stackSize(0),
    wgBroadcastSLM(-1), tidMapSLM(-1), useDeviceEnqueue(false)
  {
    initProfile(*this);
    samplerSet = GBE_NEW(SamplerSet);
    imageSet = GBE_NEW(ImageSet);
    printfSet = GBE_NEW(PrintfSet);
  }

  Function::~Function(void) {
    for (auto block : blocks) GBE_DELETE(block);
    for (auto loop : loops) GBE_DELETE(loop);
    for (auto arg : args) GBE_DELETE(arg);
  }

  RegisterFamily Function::getPointerFamily(void) const {
    return unit.getPointerFamily();
  }

  uint32_t Function::getOclVersion(void) const {
    return unit.getOclVersion();
  }

  void Function::addLoop(LabelIndex preheader,
                        int parent,
                        const vector<LabelIndex> &bbs,
                        const vector<std::pair<LabelIndex, LabelIndex>> &exits) {
    loops.push_back(GBE_NEW(Loop, preheader, parent, bbs, exits));
  }

  int Function::getLoopDepth(LabelIndex Block) const{
    if (loops.size() == 0) return 0;

    int LoopIndex = -1;
    int LoopDepth = 0;
    // get innermost loop
    for (int Idx = loops.size()-1; Idx >= 0; Idx--) {
      Loop *lp = loops[Idx];
      vector<LabelIndex> &Blocks = lp->bbs;
      bool Found = (std::find(Blocks.begin(), Blocks.end(), Block) != Blocks.end());
      if (Found) {
        LoopIndex = Idx;
        break;
      }
    }

    if (LoopIndex != -1) {
      int LoopId = LoopIndex;
      do {
        LoopId = loops[LoopId]->parent;
        LoopDepth++;
      } while(LoopId != -1);
    }

    return LoopDepth;
  }

  void Function::checkEmptyLabels(void) {
    // Empty label map, we map the removed label to the next label.
    map<LabelIndex, LabelIndex> labelMap;
    map<LabelIndex, LabelIndex> revLabelMap;
    foreachBlock([&](BasicBlock &BB) {
      Instruction * insn = BB.getLastInstruction();
      if (insn->getOpcode() == OP_LABEL) {
        GBE_ASSERTM(0, "Found empty block. ");
      }
    });
  }

  void Function::sortLabels(void) {
    uint32_t last = 0;

    // Compute the new labels and patch the label instruction
    map<LabelIndex, LabelIndex> labelMap;
    foreachInstruction([&](Instruction &insn) {
      if (insn.getOpcode() != OP_LABEL) return;

      // Create the new label
      const Instruction newLabel = LABEL(LabelIndex(last));

      // Replace the previous label instruction
      LabelInstruction &label = cast<LabelInstruction>(insn);
      const LabelIndex index = label.getLabelIndex();
      labelMap.insert(std::make_pair(index, LabelIndex(last++)));
      newLabel.replace(&insn);
    });

    // Patch all branch instructions with the new labels
    foreachInstruction([&](Instruction &insn) {
      if (insn.getOpcode() != OP_BRA) return;

      // Get the current branch instruction
      BranchInstruction &bra = cast<BranchInstruction>(insn);
      const LabelIndex index = bra.getLabelIndex();
      const LabelIndex newIndex = labelMap.find(index)->second;

      // Insert the patched branch instruction
      if (bra.isPredicated() == true) {
        const Instruction newBra = BRA(newIndex, bra.getPredicateIndex());
        newBra.replace(&insn);
      } else {
        const Instruction newBra = BRA(newIndex);
        newBra.replace(&insn);
      }
    });

    // fix labels for loops
    for (auto &x : loops) {
      for (auto &y : x->bbs)
        y = labelMap[y];

      x->preheader = labelMap[x->preheader];

      for (auto &z : x->exits) {
        z.first = labelMap[z.first];
        z.second = labelMap[z.second];
      }
    }

    // Reset the label to block mapping
    //this->labels.resize(last);
    foreachBlock([&](BasicBlock &bb) {
      const Instruction *first = bb.getFirstInstruction();
      const LabelInstruction *label = cast<LabelInstruction>(first);
      const LabelIndex index = label->getLabelIndex();
      this->labels[index] = &bb;
    });
  }

  LabelIndex Function::newLabel(void) {
    GBE_ASSERTM(labels.size() < 0xffffffffull,
                "Too many labels are defined (4G only are supported)");
    const LabelIndex index(labels.size());
    labels.push_back(NULL);
    return index;
  }

  void Function::outImmediate(std::ostream &out, ImmediateIndex index) const {
    GBE_ASSERT(index < immediates.size());
    const Immediate imm = immediates[index];
    switch (imm.getType()) {
      case TYPE_BOOL: out << !!imm.getIntegerValue(); break;
      case TYPE_S8:
      case TYPE_U8:
      case TYPE_S16:
      case TYPE_U16:
      case TYPE_S32:
      case TYPE_U32:
      case TYPE_S64: out << imm.getIntegerValue(); break;
      case TYPE_U64: out << (uint64_t)imm.getIntegerValue(); break;
      case TYPE_HALF: out << "half(" << (float)imm.getHalfValue() << ")"; break;
      case TYPE_FLOAT: out << imm.getFloatValue(); break;
      case TYPE_DOUBLE: out << imm.getDoubleValue(); break;
      default:
        GBE_ASSERT(0 && "unsupported imm type.\n");
    }
  }

  uint32_t Function::getLargestBlockSize(void) const {
    uint32_t insnNum = 0;
    foreachBlock([&insnNum](const ir::BasicBlock &bb) {
      insnNum = std::max(insnNum, uint32_t(bb.size()));
    });
    return insnNum;
  }

  uint32_t Function::getFirstSpecialReg(void) const {
    return this->profile == PROFILE_OCL ? 0u : ~0u;
  }

  uint32_t Function::getSpecialRegNum(void) const {
    return this->profile == PROFILE_OCL ? ocl::regNum : ~0u;
  }

  bool Function::isEntryBlock(const BasicBlock &bb) const {
    if (this->blockNum() == 0)
      return false;
    else
      return &bb == this->blocks[0];
  }

  BasicBlock &Function::getTopBlock(void) const {
    GBE_ASSERT(blockNum() > 0 && blocks[0] != NULL);
    return *blocks[0];
  }

  const BasicBlock &Function::getBottomBlock(void) const {
    const uint32_t n = blockNum();
    GBE_ASSERT(n > 0 && blocks[n-1] != NULL);
    return *blocks[n-1];
  }

  BasicBlock &Function::getBottomBlock(void) {
    const uint32_t n = blockNum();
    GBE_ASSERT(n > 0 && blocks[n-1] != NULL);
    return *blocks[n-1];
  }

  BasicBlock &Function::getBlock(LabelIndex label) const {
    GBE_ASSERT(label < labelNum() && labels[label] != NULL);
    return *labels[label];
  }

  const LabelInstruction *Function::getLabelInstruction(LabelIndex index) const {
    const BasicBlock *bb = this->labels[index];
    const Instruction *first = bb->getFirstInstruction();
    return cast<LabelInstruction>(first);
  }

  /*! Indicate if the given register is a special one (like localID in OCL) */
  bool Function::isSpecialReg(const Register &reg) const {
    const uint32_t ID = uint32_t(reg);
    const uint32_t firstID = this->getFirstSpecialReg();
    const uint32_t specialNum = this->getSpecialRegNum();
    return ID >= firstID && ID < firstID + specialNum;
  }
  Register Function::getSurfaceBaseReg(uint8_t bti) const {
    map<uint8_t, Register>::const_iterator iter = btiRegMap.find(bti);
    GBE_ASSERT(iter != btiRegMap.end());
    return iter->second;
  }

  void Function::appendSurface(uint8_t bti, Register reg) {
    btiRegMap.insert(std::make_pair(bti, reg));
  }

  void Function::computeCFG(void) {
    // Clear possible previously computed CFG and compute the direct
    // predecessors and successors
    BasicBlock *prev = NULL;
    this->foreachBlock([this, &prev](BasicBlock &bb) {
      bb.successors.clear();
      bb.predecessors.clear();
      if (prev != NULL) {
        prev->nextBlock = &bb;
        bb.prevBlock = prev;
      }
      prev = &bb;
    });

    // Update it. Do not forget that a branch can also jump to the next block
    BasicBlock *jumpToNext = NULL;
    this->foreachBlock([this, &jumpToNext](BasicBlock &bb) {
      if (jumpToNext) {
        jumpToNext->successors.insert(&bb);
        bb.predecessors.insert(jumpToNext);
        jumpToNext = NULL;
      }
      if (bb.size() == 0) return;
      Instruction *last = bb.getLastInstruction();
      if (last->isMemberOf<BranchInstruction>() == false || last->getOpcode() == OP_ENDIF || last->getOpcode() == OP_ELSE) {
        jumpToNext = &bb;
        return;
      }
      ir::BasicBlock::iterator it = --bb.end();
      uint32_t handledInsns = 0;
      while ((handledInsns < 2 && it != bb.end()) &&
             static_cast<ir::BranchInstruction *>(&*it)->getOpcode() == OP_BRA) {
        const BranchInstruction &insn = cast<BranchInstruction>(*it);
        if (insn.getOpcode() != OP_BRA)
          break;
        const LabelIndex label = insn.getLabelIndex();
        BasicBlock *target = this->blocks[label];
        GBE_ASSERT(target != NULL);
        target->predecessors.insert(&bb);
        bb.successors.insert(target);
        if (insn.isPredicated() == true) jumpToNext = &bb;
        // If we are going to handle the second bra, this bra must be a predicated bra
        GBE_ASSERT(handledInsns == 0 || insn.isPredicated() == true);
        --it;
        ++handledInsns;
      }
    });
  }

  void Function::outputCFG(void) {
    std::string fileName = getName() + std::string(".dot");
    ::FILE *fp = fopen(fileName.c_str(), "w");
    if (fp == NULL) return;

    printf("writing Gen IR CFG to %s\n", fileName.c_str());
    fprintf(fp, "digraph \"%s\" {\n", getName().c_str());
    this->foreachBlock([this, fp](BasicBlock &bb) {
      uint32_t lid = bb.getLabelIndex();
      fprintf(fp, "Node%d [shape=record, label=\"{%d}\"];\n", lid, lid);
      set<BasicBlock*> &succ = bb.successors;
      for (auto x : succ) {
        uint32_t next = x->getLabelIndex();
        fprintf(fp, "Node%d -> Node%d\n", lid, next);
      }
    });
    fprintf(fp, "}\n");
    fclose(fp);
  }


  std::ostream &operator<< (std::ostream &out, const Function &fn)
  {
    out << ".decl_function " << fn.getName() << std::endl;
    out << fn.getRegisterFile();
    out << "## " << fn.argNum() << " input register"
        << (fn.argNum() ? "s" : "") << " ##" << std::endl;
    for (uint32_t i = 0; i < fn.argNum(); ++i) {
      const FunctionArgument &input = fn.getArg(i);
      out << "decl_input.";
      switch (input.type) {
        case FunctionArgument::GLOBAL_POINTER: out << "global"; break;
        case FunctionArgument::LOCAL_POINTER: out << "local"; break;
        case FunctionArgument::CONSTANT_POINTER: out << "constant"; break;
        case FunctionArgument::VALUE: out << "value"; break;
        case FunctionArgument::STRUCTURE:
          out << "structure." << input.size;
        break;
        case FunctionArgument::IMAGE: out << "image"; break;
        case FunctionArgument::PIPE: out << "pipe"; break;
        default: break;
      }
      out << " %" << input.reg << " " << input.name << std::endl;
    }
    out << "## " << fn.outputNum() << " output register"
        << (fn.outputNum() ? "s" : "") << " ##" << std::endl;
    for (uint32_t i = 0; i < fn.outputNum(); ++i)
      out << "decl_output %" << fn.getOutput(i) << std::endl;
    out << "## " << fn.pushedNum() << " pushed register" << std::endl;
    const Function::PushMap &pushMap = fn.getPushMap();
    for (const auto &pushed : pushMap) {
      out << "decl_pushed %" << pushed.first
           << " @{" << pushed.second.argID << ","
           << pushed.second.offset << "}" << std::endl;
    }
    out << "## " << fn.blockNum() << " block"
        << (fn.blockNum() ? "s" : "") << " ##" << std::endl;
    fn.foreachBlock([&](const BasicBlock &bb) {
      const_cast<BasicBlock&>(bb).foreach([&out] (const Instruction &insn) {
        out << insn << std::endl;
      });
      out << std::endl;
    });
    out << ".end_function" << std::endl;
    return out;
  }

  ///////////////////////////////////////////////////////////////////////////
  // Basic Block
  ///////////////////////////////////////////////////////////////////////////

  BasicBlock::BasicBlock(Function &fn) : needEndif(true), needIf(true), endifLabel(0),
                                         matchingEndifLabel(0), matchingElseLabel(0),
                                         thisElseLabel(0), belongToStructure(false),
                                         isStructureExit(false), isLoopExit(false),
                                         hasExtraBra(false),
                                         matchingStructureEntry(NULL),
                                         fn(fn) {
    this->nextBlock = this->prevBlock = NULL;
  }

  BasicBlock::~BasicBlock(void) {
    this->foreach([this] (Instruction &insn) {
     this->fn.deleteInstruction(&insn);
    });
  }

  void BasicBlock::append(Instruction &insn) {
    insn.setParent(this);
    this->push_back(&insn);
  }

  void BasicBlock::insertAt(iterator pos, Instruction &insn) {
    insn.setParent(this);
    this->insert(pos, &insn);
  }

  Instruction *BasicBlock::getFirstInstruction(void) const {
    GBE_ASSERT(this->begin() != this->end());
    const Instruction &insn = *this->begin();
    return const_cast<Instruction*>(&insn);
  }

  Instruction *BasicBlock::getLastInstruction(void) const {
    GBE_ASSERT(this->begin() != this->end());
    const Instruction &insn = *(--this->end());
    return const_cast<Instruction*>(&insn);
  }

  LabelIndex BasicBlock::getLabelIndex(void) const {
    const Instruction *first = this->getFirstInstruction();
    const LabelInstruction *label = cast<LabelInstruction>(first);
    return label?label->getLabelIndex():LabelIndex(-1);
  }

} /* namespace ir */
} /* namespace gbe */


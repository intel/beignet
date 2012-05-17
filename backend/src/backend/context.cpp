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
 * \file context.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "backend/context.hpp"
#include "backend/program.hpp"
#include "backend/gen_eu.hpp"
#include "ir/unit.hpp"
#include "ir/function.hpp"
#include "ir/profile.hpp"
#include "ir/liveness.hpp"
#include "ir/value.hpp"
#include "sys/cvar.hpp"
#include <algorithm>

namespace gbe
{
  IVAR(OCL_SIMD_WIDTH, 8, 16, 32);

  Context::Context(const ir::Unit &unit, const std::string &name) :
    unit(unit), fn(*unit.getFunction(name)), name(name), liveness(NULL), dag(NULL)
  {
    GBE_ASSERT(unit.getPointerSize() == ir::POINTER_32_BITS);
    this->liveness = GBE_NEW(ir::Liveness, (ir::Function&) fn);
    this->dag = GBE_NEW(ir::FunctionDAG, *this->liveness);
    this->simdWidth = nextHighestPowerOf2(OCL_SIMD_WIDTH);
    // std::cout << *dag;
    //std::cout << *liveness;
  }
  Context::~Context(void) {
    GBE_SAFE_DELETE(this->dag);
    GBE_SAFE_DELETE(this->liveness);
  }

  Kernel *Context::compileKernel(void) {
    this->kernel = this->allocateKernel();
    this->kernel->simdWidth = this->simdWidth;
    this->buildPatchList();
    this->buildArgList();
    this->buildUsedLabels();
    this->buildJIPs();
    this->buildStack();
    this->emitCode();
    return this->kernel;
  }

  void Context::buildStack(void) {
    const auto &stackUse = dag->getUse(ir::ocl::stackptr);
    if (stackUse.size() == 0)  // no stack is used if stackptr is unused
      return;
    // Be sure that the stack pointer is set
    GBE_ASSERT(this->kernel->getCurbeOffset(GBE_CURBE_STACK_POINTER, 0) >= 0);
    this->kernel->stackSize = KB; // XXX compute that in a better way
  }

  void Context::buildPatchList(void) {
    const uint32_t ptrSize = unit.getPointerSize() == ir::POINTER_32_BITS ? 4u : 8u;
    kernel->curbeSize = 0u;

    // We insert the block IP mask first
    kernel->patches.push_back(PatchInfo(GBE_CURBE_BLOCK_IP, 0, kernel->curbeSize));
    kernel->curbeSize += this->simdWidth * sizeof(uint16_t);

    // Go over the arguments and find the related patch locations
    const uint32_t inputNum = fn.inputNum();
    for (uint32_t inputID = 0u; inputID < inputNum; ++inputID) {
      const ir::FunctionInput &input = fn.getInput(inputID);
      // This is a pointer -> 4 bytes to patch (do 64 bits later)
      if (input.type == ir::FunctionInput::GLOBAL_POINTER ||
          input.type == ir::FunctionInput::CONSTANT_POINTER) {
        const PatchInfo patch(GBE_CURBE_KERNEL_ARGUMENT, inputID, kernel->curbeSize);
        kernel->patches.push_back(patch);
        kernel->curbeSize += ptrSize;
      }
    }

    // Already inserted registers go here
    set<ir::Register> specialRegs;

    // Then the local IDs (not scalar, so we align them properly)
    kernel->curbeSize = ALIGN(kernel->curbeSize, GEN_REG_SIZE);
    if (this->simdWidth == 16 || this->simdWidth == 32)
      if ((kernel->curbeSize + GEN_REG_SIZE) % (2*GEN_REG_SIZE) != 0)
        kernel->curbeSize += GEN_REG_SIZE;
    const size_t localIDSize = sizeof(uint32_t) * this->simdWidth;
    const PatchInfo lid0(GBE_CURBE_LOCAL_ID_X, 0, kernel->curbeSize);
    kernel->curbeSize += localIDSize;
    const PatchInfo lid1(GBE_CURBE_LOCAL_ID_Y, 0, kernel->curbeSize);
    kernel->curbeSize += localIDSize;
    const PatchInfo lid2(GBE_CURBE_LOCAL_ID_Z, 0, kernel->curbeSize);
    kernel->curbeSize += localIDSize;
    kernel->patches.push_back(lid0);
    kernel->patches.push_back(lid1);
    kernel->patches.push_back(lid2);
    specialRegs.insert(ir::ocl::lid0);
    specialRegs.insert(ir::ocl::lid1);
    specialRegs.insert(ir::ocl::lid2);

    // Go over all the instructions and find the special register value we need
    // to push
#define INSERT_REG(SPECIAL_REG, PATCH, WIDTH) \
  if (reg == ir::ocl::SPECIAL_REG) { \
    if (specialRegs.find(reg) != specialRegs.end()) continue; \
    kernel->curbeSize = ALIGN(kernel->curbeSize, ptrSize * WIDTH); \
    const PatchInfo patch(GBE_CURBE_##PATCH, 0, kernel->curbeSize); \
    kernel->patches.push_back(patch); \
    kernel->curbeSize += ptrSize * WIDTH; \
  } else
    fn.foreachInstruction([&](const ir::Instruction &insn) {
      const uint32_t srcNum = insn.getSrcNum();
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        const ir::Register reg = insn.getSrc(srcID);
        if (fn.isSpecialReg(reg) == false) continue;
        if (specialRegs.contains(reg) == true) continue;
        INSERT_REG(lsize0, LOCAL_SIZE_X, 1)
        INSERT_REG(lsize1, LOCAL_SIZE_Y, 1)
        INSERT_REG(lsize2, LOCAL_SIZE_Z, 1)
        INSERT_REG(gsize0, GLOBAL_SIZE_X, 1)
        INSERT_REG(gsize1, GLOBAL_SIZE_Y, 1)
        INSERT_REG(gsize2, GLOBAL_SIZE_Z, 1)
        INSERT_REG(goffset0, GLOBAL_OFFSET_X, 1)
        INSERT_REG(goffset1, GLOBAL_OFFSET_Y, 1)
        INSERT_REG(goffset2, GLOBAL_OFFSET_Z, 1)
        INSERT_REG(numgroup0, GROUP_NUM_X, 1)
        INSERT_REG(numgroup1, GROUP_NUM_Y, 1)
        INSERT_REG(numgroup2, GROUP_NUM_Z, 1)
        INSERT_REG(stackptr, STACK_POINTER, this->simdWidth);
        specialRegs.insert(reg);
      }
    });
#undef INSERT_REG

    // Insert the stack buffer if used
    if (kernel->getCurbeOffset(GBE_CURBE_STACK_POINTER, 0) >= 0) {
      kernel->curbeSize = ALIGN(kernel->curbeSize, ptrSize);
      const PatchInfo patch(GBE_CURBE_EXTRA_ARGUMENT, GBE_STACK_BUFFER, kernel->curbeSize);
      kernel->patches.push_back(patch);
      kernel->curbeSize += ptrSize;
    }

    // After this point the vector is immutable. Sorting it will make
    // research faster
    std::sort(kernel->patches.begin(), kernel->patches.end());

    // Align it on 128 bytes properly
    kernel->curbeSize = ALIGN(kernel->curbeSize, GEN_REG_SIZE);
  }

  void Context::buildArgList(void) {
    kernel->argNum = fn.inputNum();
    kernel->args = GBE_NEW_ARRAY(KernelArgument, kernel->argNum);
    for (uint32_t inputID = 0; inputID < kernel->argNum; ++inputID) {
      const auto &input = fn.getInput(inputID);
      switch (input.type) {
        case ir::FunctionInput::VALUE:
        case ir::FunctionInput::STRUCTURE:
          kernel->args[inputID].type = GBE_ARG_VALUE;
          kernel->args[inputID].size = input.elementSize;
          break;
        case ir::FunctionInput::GLOBAL_POINTER:
          kernel->args[inputID].type = GBE_ARG_GLOBAL_PTR;
          kernel->args[inputID].size = sizeof(void*);
          break;
        case ir::FunctionInput::CONSTANT_POINTER:
          kernel->args[inputID].type = GBE_ARG_CONSTANT_PTR;
          kernel->args[inputID].size = sizeof(void*);
          break;
        case ir::FunctionInput::LOCAL_POINTER:
          kernel->args[inputID].type = GBE_ARG_LOCAL_PTR;
          kernel->args[inputID].size = sizeof(void*);
          break;
        case ir::FunctionInput::IMAGE:
          kernel->args[inputID].type = GBE_ARG_IMAGE;
          kernel->args[inputID].size = sizeof(void*);
          break;
      }
    }
  }

  void Context::buildUsedLabels(void) {
    usedLabels.clear();
    fn.foreachInstruction([this](const ir::Instruction &insn) {
      using namespace ir;
      if (insn.getOpcode() != OP_BRA) return;
      const LabelIndex index = cast<BranchInstruction>(insn).getLabelIndex();
      usedLabels.insert(index);
    });
  }

  // The idea is that foward branches can by-pass the target of previous
  // forward branches. Since we run in SIMD mode, we must be sure that we are
  // not skipping some computations. The idea is therefore to put JOIN points at
  // the head of each block and to restrict the distance where to jump when
  // taking a forward branch. We traverse the blocks top to bottom and use a
  // O(n^2) stupid algorithm to track down which branches we can by-pass
  void Context::buildJIPs(void) {
    using namespace ir;

    // Linearly store the branch target for each block and its own label
    const LabelIndex noTarget(fn.labelNum());
    vector<std::pair<LabelIndex, LabelIndex>> braTargets;
    int32_t curr = 0, blockNum = fn.blockNum();
    braTargets.resize(blockNum);

    // If some blocks are unused we mark them as such by setting their own label
    // as "invalid" (== noTarget)
    for (auto &bb : braTargets) bb = std::make_pair(noTarget, noTarget);

    fn.foreachBlock([&](const BasicBlock &bb) {
      const LabelIndex ownLabel = bb.getLabelIndex();
      const Instruction *last = bb.getLastInstruction();
      if (last->getOpcode() != OP_BRA)
        braTargets[curr++] = std::make_pair(ownLabel, noTarget);
      else {
        const BranchInstruction *bra = cast<BranchInstruction>(last);
        braTargets[curr++] = std::make_pair(ownLabel, bra->getLabelIndex());
      }
    });

    // For each block, we also figure out if the JOIN point (at the label
    // instruction location) needs a branch to bypass useless computations
    vector<LabelIndex> joinTargets;
    joinTargets.resize(fn.labelNum());
    for (auto &bb : joinTargets) bb = noTarget;

    // We store here the labels bypassed by the current branch
    vector<LabelIndex> bypassedLabels;
    bypassedLabels.resize(blockNum);

    // Now retraverse the blocks and figure out all JIPs
    for (int32_t blockID = 0; blockID < blockNum; ++blockID) {
      const LabelIndex ownLabel = braTargets[blockID].first;
      const LabelIndex target = braTargets[blockID].second;
      const BasicBlock &bb = fn.getBlock(ownLabel);
      const Instruction *insn = bb.getLastInstruction();
      if (ownLabel == noTarget) continue; // unused block
      if (target == noTarget) continue; // no branch at all
      GBE_ASSERT(insn->isMemberOf<BranchInstruction>() == true);
      if (target <= ownLabel) { // bwd branch: we always jump
        JIPs.insert(std::make_pair(insn, LabelIndex(target)));
        continue;
      }

      // Traverse all previous blocks and see if we bypass their target
      uint32_t bypassedNum = 0;
      uint32_t JIP = target;
      for (int32_t prevID = blockID-1; prevID >= 0; --prevID) {
        const LabelIndex prevTarget = braTargets[prevID].second;
        if (prevTarget == noTarget) continue; // no branch
        if (prevTarget >= target) continue; // complete bypass
        if (prevTarget <= ownLabel) continue; // branch falls before
        bypassedLabels[bypassedNum++] = prevTarget;
        JIP = min(uint32_t(JIP), uint32_t(prevTarget));
      }

      // We now have the (possibly) updated JIP for the branch
      JIPs.insert(std::make_pair(insn, LabelIndex(JIP)));

      // No bypassed targets
      if (bypassedNum == 0) continue;

      // When we have several bypassed targets, we must simply sort them and
      // chain them such target_n points to target_{n+1}
      bypassedLabels[bypassedNum++] = target;
      std::sort(&bypassedLabels[0], &bypassedLabels[bypassedNum]);

      // Bypassed labels have a JIP now. However, we will only insert the
      // instructions later since *several* branches can bypass the same label.
      // For that reason, we must consider the *minimum* JIP
      for (uint32_t bypassedID = 0; bypassedID < bypassedNum-1; ++bypassedID) {
        const LabelIndex curr = bypassedLabels[bypassedID];
        const LabelIndex next = bypassedLabels[bypassedID+1];
        joinTargets[curr] = min(joinTargets[curr], next);
      }
    }

    // Now we also processed all JOIN points (i.e. each label). We can insert
    // the label instructions that have a JIP
    for (uint32_t label = 0; label < fn.labelNum(); ++label) {
      const LabelIndex target = joinTargets[label];
      if (target == noTarget) continue;
      const Instruction *insn = fn.getLabelInstruction(LabelIndex(label));
      JIPs.insert(std::make_pair(insn, target));
    }
  }

  bool Context::isScalarReg(const ir::Register &reg) const {
    GBE_ASSERT(fn.getProfile() == ir::Profile::PROFILE_OCL);
    if (fn.getInput(reg) != NULL) return true;
    if (reg == ir::ocl::groupid0  ||
        reg == ir::ocl::groupid1  ||
        reg == ir::ocl::groupid2  ||
        reg == ir::ocl::numgroup0 ||
        reg == ir::ocl::numgroup1 ||
        reg == ir::ocl::numgroup2 ||
        reg == ir::ocl::lsize0    ||
        reg == ir::ocl::lsize1    ||
        reg == ir::ocl::lsize2    ||
        reg == ir::ocl::gsize0    ||
        reg == ir::ocl::gsize1    ||
        reg == ir::ocl::gsize2    ||
        reg == ir::ocl::goffset0  ||
        reg == ir::ocl::goffset1  ||
        reg == ir::ocl::goffset2)
      return true;
    return false;
  }

} /* namespace gbe */


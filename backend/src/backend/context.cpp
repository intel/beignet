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
#include "backend/gen_encoder.hpp"
#include "ir/unit.hpp"
#include "ir/function.hpp"
#include "ir/profile.hpp"
#include "ir/liveness.hpp"
#include "ir/value.hpp"
#include "sys/cvar.hpp"
#include <algorithm>

namespace gbe
{
  /*! Structure that keeps track of allocation in the register file. This is
   *  actually needed by Context (and not only by GenContext) because both
   *  simulator and hardware have to deal with constant pushing which uses the
   *  register file
   *
   *  Since Gen is pretty flexible, we just maintain a free list for the
   *  register file (as a classical allocator) and coalesce blocks when required
   */
  class RegisterFilePartitioner
  {
  public:
    RegisterFilePartitioner(void);
    ~RegisterFilePartitioner(void);

    /*! Allocate some memory in the register file. Return 0 if out-of-memory. By
     *  the way, zero is not a valid offset since r0 is always preallocated by
     *  the hardware. Note that we always use the left most block when
     *  allocating, so it makes sense for constant pushing
     */
    int16_t allocate(int16_t size, int16_t alignment);

    /*! Free the given register file piece */
    void deallocate(int16_t offset);

  private:
    /*! May need to make that run-time in the future */
    static const int16_t RegisterFileSize = 4*KB;

    /*! Double chained list of free spaces */
    struct Block {
      Block(int16_t offset, int16_t size) :
        prev(NULL), next(NULL), offset(offset), size(size) {}
      Block *prev, *next; //!< Previous and next free blocks
      int16_t offset;        //!< Where the free block starts
      int16_t size;          //!< Size of the free block
    };

    /*! Try to coalesce two blocks (left and right). They must be in that order.
     *  If the colascing was done, the left block is deleted
     */
    void coalesce(Block *left, Block *right);
    /*! Head of the free list */
    Block *head;
    /*! Handle free list element allocation */
    DECL_POOL(Block, blockPool);
    /*! Track allocated memory blocks <offset, size> */
    map<int16_t, int16_t> allocatedBlocks;
    /*! Use custom allocators */
    GBE_CLASS(RegisterFilePartitioner);
  };

  RegisterFilePartitioner::RegisterFilePartitioner(void) {
    // r0 is always set by the HW and used at the end by EOT
    const int16_t offset = GEN_REG_SIZE;
    const int16_t size = RegisterFileSize  - offset;
    head = this->newBlock(offset, size);
  }

  RegisterFilePartitioner::~RegisterFilePartitioner(void) { 
    while (this->head) {
      Block *next = this->head->next;
      this->deleteBlock(this->head);
      this->head = next;
    }
  }

  int16_t RegisterFilePartitioner::allocate(int16_t size, int16_t alignment)
  {
    // Make it simple and just use the first block we find
    Block *list = head;
    while (list) {
      const int16_t aligned = ALIGN(list->offset, alignment);
      const int16_t spaceOnLeft = aligned - list->offset;
      const int16_t spaceOnRight = list->size - size - spaceOnLeft;

      // Not enough space in this block
      if (spaceOnRight < 0) {
        list = list->next;
        continue;
      }
      // Cool we can use this block
      else {
        Block *left = list->prev;
        Block *right = list->next;

        // If we left a hole on the left, create a new block
        if (spaceOnLeft) {
          Block *newBlock = this->newBlock(list->offset, spaceOnLeft);
          if (left) {
            left->next = newBlock;
            newBlock->prev = left;
          }
          if (right) {
            newBlock->next = right;
            right->prev = newBlock;
          }
          left = newBlock;
        }

        // If we left a hole on the right, create a new block as well
        if (spaceOnRight) {
          Block *newBlock = this->newBlock(aligned + size, spaceOnRight);
          if (left) {
            left->next = newBlock;
            newBlock->prev = left;
          }
          if (right) {
            right->prev = newBlock;
            newBlock->next = right;
          }
          right = newBlock;
        }

        // Chain both successors and predecessors when the entire block was
        // allocated
        if (spaceOnLeft == 0 && spaceOnRight == 0) {
          if (left) left->next = right;
          if (right) right->prev = left;
        }

        // Update the head of the free blocks
        if (list == head) {
          if (left)
            head = left;
          else if (right)
            head = right;
          else
            head = NULL;
        }

        // Free the block and check the consistency
        this->deleteBlock(list);
        if (head && head->next) GBE_ASSERT(head->next->prev == head);

        // Track the allocation to retrieve the size later
        allocatedBlocks.insert(std::make_pair(aligned, size));

        // We have a valid offset now
        return aligned;
      }
    }
    return 0;
  }

  void RegisterFilePartitioner::deallocate(int16_t offset)
  {
    // Retrieve the size in the allocation map
    auto it = allocatedBlocks.find(offset);
    GBE_ASSERT(it != allocatedBlocks.end());
    const int16_t size = it->second;

    // Find the two blocks where to insert the new block
    Block *list = head, *prev = NULL;
    while (list != NULL) {
      if (list->offset > offset)
        break;
      prev = list;
      list = list->next;
    }

    // Create the block and insert it
    Block *newBlock = this->newBlock(offset, size);
    if (prev) {
      GBE_ASSERT(prev->offset + prev->size <= offset);
      prev->next = newBlock;
      newBlock->prev = prev;
    }
    if (list) {
      GBE_ASSERT(offset + size <= list->offset);
      list->prev = newBlock;
      newBlock->next = list;
    }

    // There were no block anymore
    if (prev == NULL && list == NULL)
      this->head = newBlock;
    else {
      // Coalesce the blocks if possible
      this->coalesce(prev, newBlock);
      this->coalesce(newBlock, list);
    }

    // Do not track this allocation anymore
    allocatedBlocks.erase(it);
  }

  void RegisterFilePartitioner::coalesce(Block *left, Block *right) {
    if (left == NULL || right == NULL) return;
    GBE_ASSERT(left->offset < right->offset);
    GBE_ASSERT(left->next == right);
    GBE_ASSERT(right->prev == left);
    if (left->offset + left->size == right->offset) {
      right->offset = left->offset;
      right->size += left->size;
      if (left->prev) left->prev->next = right;
      right->prev = left->prev;
      if (left == this->head)
        this->head = right;
      this->deleteBlock(left);
    }
  }

  ///////////////////////////////////////////////////////////////////////////
  // Generic Context (shared by the simulator and the HW context)
  ///////////////////////////////////////////////////////////////////////////
  IVAR(OCL_SIMD_WIDTH, 8, 16, 32);

  Context::Context(const ir::Unit &unit, const std::string &name) :
    unit(unit), fn(*unit.getFunction(name)), name(name), liveness(NULL), dag(NULL)
  {
    GBE_ASSERT(unit.getPointerSize() == ir::POINTER_32_BITS);
    this->liveness = GBE_NEW(ir::Liveness, (ir::Function&) fn);
    this->dag = GBE_NEW(ir::FunctionDAG, *this->liveness);
    this->partitioner = GBE_NEW(RegisterFilePartitioner);
    this->simdWidth = nextHighestPowerOf2(OCL_SIMD_WIDTH);
  }
  Context::~Context(void) {
    GBE_SAFE_DELETE(this->partitioner);
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

  int16_t Context::allocate(int16_t size, int16_t alignment) {
    return partitioner->allocate(size, alignment);
  }

  void Context::deallocate(int16_t offset) { partitioner->deallocate(offset); }

  void Context::buildStack(void) {
    const auto &stackUse = dag->getUse(ir::ocl::stackptr);
    if (stackUse.size() == 0)  // no stack is used if stackptr is unused
      return;
    // Be sure that the stack pointer is set
    GBE_ASSERT(this->kernel->getCurbeOffset(GBE_CURBE_STACK_POINTER, 0) >= 0);
    this->kernel->stackSize = 1*KB; // XXX compute that in a better way
  }

  void Context::newCurbeEntry(gbe_curbe_type value,
                              uint32_t subValue,
                              uint32_t size,
                              uint32_t alignment)
  {
    alignment = alignment == 0 ? size : alignment;
    const uint32_t offset = partitioner->allocate(size, alignment);
    GBE_ASSERT(offset >= GEN_REG_SIZE);
    kernel->patches.push_back(PatchInfo(value, subValue, offset - GEN_REG_SIZE));
    kernel->curbeSize = max(kernel->curbeSize, offset + size - GEN_REG_SIZE);
  }

  void Context::buildPatchList(void) {
    const uint32_t ptrSize = unit.getPointerSize() == ir::POINTER_32_BITS ? 4u : 8u;
    kernel->curbeSize = 0u;

    // We insert the block IP mask first
    this->newCurbeEntry(GBE_CURBE_BLOCK_IP, 0, this->simdWidth*sizeof(uint16_t));

    // Go over the arguments and find the related patch locations
    const uint32_t argNum = fn.argNum();
    for (uint32_t argID = 0u; argID < argNum; ++argID) {
      const ir::FunctionArgument &arg = fn.getArg(argID);
      // For pointers and values, we have nothing to do. We just push the values
      if (arg.type == ir::FunctionArgument::GLOBAL_POINTER ||
          arg.type == ir::FunctionArgument::CONSTANT_POINTER ||
          arg.type == ir::FunctionArgument::VALUE ||
          arg.type == ir::FunctionArgument::STRUCTURE)
        this->newCurbeEntry(GBE_CURBE_KERNEL_ARGUMENT, argID, arg.size, ptrSize);
    }

    // Already inserted registers go here
    set<ir::Register> specialRegs;

    const size_t localIDSize = sizeof(uint32_t) * this->simdWidth;
    this->newCurbeEntry(GBE_CURBE_LOCAL_ID_X, 0, localIDSize);
    this->newCurbeEntry(GBE_CURBE_LOCAL_ID_Y, 0, localIDSize);
    this->newCurbeEntry(GBE_CURBE_LOCAL_ID_Z, 0, localIDSize);
    specialRegs.insert(ir::ocl::lid0);
    specialRegs.insert(ir::ocl::lid1);
    specialRegs.insert(ir::ocl::lid2);

    // Go over all the instructions and find the special register we need
    // to push
#define INSERT_REG(SPECIAL_REG, PATCH, WIDTH) \
  if (reg == ir::ocl::SPECIAL_REG) { \
    if (specialRegs.find(reg) != specialRegs.end()) continue; \
    this->newCurbeEntry(GBE_CURBE_##PATCH, 0, ptrSize * WIDTH); \
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
    if (kernel->getCurbeOffset(GBE_CURBE_STACK_POINTER, 0) >= 0)
      this->newCurbeEntry(GBE_CURBE_EXTRA_ARGUMENT, GBE_STACK_BUFFER, ptrSize);

    // After this point the vector is immutable. Sorting it will make
    // research faster
    std::sort(kernel->patches.begin(), kernel->patches.end());

    // Align it on 32 bytes properly
    kernel->curbeSize = ALIGN(kernel->curbeSize, GEN_REG_SIZE);
  }

  void Context::buildArgList(void) {
    kernel->argNum = fn.argNum();
    kernel->args = GBE_NEW_ARRAY(KernelArgument, kernel->argNum);
    for (uint32_t argID = 0; argID < kernel->argNum; ++argID) {
      const auto &arg = fn.getArg(argID);
      switch (arg.type) {
        case ir::FunctionArgument::VALUE:
        case ir::FunctionArgument::STRUCTURE:
          kernel->args[argID].type = GBE_ARG_VALUE;
          kernel->args[argID].size = arg.size;
          break;
        case ir::FunctionArgument::GLOBAL_POINTER:
          kernel->args[argID].type = GBE_ARG_GLOBAL_PTR;
          kernel->args[argID].size = sizeof(void*);
          break;
        case ir::FunctionArgument::CONSTANT_POINTER:
          kernel->args[argID].type = GBE_ARG_CONSTANT_PTR;
          kernel->args[argID].size = sizeof(void*);
          break;
        case ir::FunctionArgument::LOCAL_POINTER:
          kernel->args[argID].type = GBE_ARG_LOCAL_PTR;
          kernel->args[argID].size = sizeof(void*);
          break;
        case ir::FunctionArgument::IMAGE:
          kernel->args[argID].type = GBE_ARG_IMAGE;
          kernel->args[argID].size = sizeof(void*);
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
    if (fn.getArg(reg) != NULL) return true;
    if (fn.getPushLocation(reg) != NULL) return true;
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


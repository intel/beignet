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
#include "ir/image.hpp"
#include "sys/cvar.hpp"
#include <algorithm>

namespace gbe
{
  class SimpleAllocator
  {
  public:
    SimpleAllocator(int16_t startOffset, int16_t size, bool _assertFail);
    ~SimpleAllocator(void);

    /*! Allocate some memory from the pool.
     */
    int16_t allocate(int16_t size, int16_t alignment, bool bFwd=false);

    /*! Free the given register file piece */
    void deallocate(int16_t offset);

    /*! Spilt a block into 2 blocks */
    void splitBlock(int16_t offset, int16_t subOffset);

  protected:
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
    /*! the maximum offset */
    int16_t maxOffset;
    /*! whether trigger an assertion on allocation failure */
    bool assertFail;
    /*! Head and tail of the free list */
    Block *head;
    Block *tail;
    /*! Handle free list element allocation */
    DECL_POOL(Block, blockPool);
    /*! Track allocated memory blocks <offset, size> */
    map<int16_t, int16_t> allocatedBlocks;
    /*! Use custom allocators */
    GBE_CLASS(SimpleAllocator);
  };

  /*! Structure that keeps track of allocation in the register file. This is
   *  actually needed by Context (and not only by GenContext) because both
   *  simulator and hardware have to deal with constant pushing which uses the
   *  register file
   *
   *  Since Gen is pretty flexible, we just reuse the Simpleallocator
   */

  class RegisterAllocator: public SimpleAllocator {
  public:
    RegisterAllocator(int16_t offset, int16_t size): SimpleAllocator(offset, size, false) {}

    GBE_CLASS(RegisterAllocator);
  };

  /*!
   * an allocator for scratch memory allocation. Scratch memory are used for register spilling.
   * You can query how much scratch memory needed through getMaxScatchMemUsed().
   */

  class ScratchAllocator: public SimpleAllocator {
  public:
    ScratchAllocator(int16_t size): SimpleAllocator(0, size, true) {}
    int16_t getMaxScatchMemUsed() { return maxOffset; }

    GBE_CLASS(ScratchAllocator);
  };

  SimpleAllocator::SimpleAllocator(int16_t startOffset,
                                   int16_t size,
                                   bool _assertFail)
                                  : maxOffset(0),
                                  assertFail(_assertFail){
    tail = head = this->newBlock(startOffset, size);
  }

  SimpleAllocator::~SimpleAllocator(void) {
    while (this->head) {
      Block *next = this->head->next;
      this->deleteBlock(this->head);
      this->head = next;
    }
  }

  int16_t SimpleAllocator::allocate(int16_t size, int16_t alignment, bool bFwd)
  {
    // Make it simple and just use the first block we find
    Block *list = bFwd ? head : tail;
    while (list) {
      int16_t aligned;
      int16_t spaceOnLeft;
      int16_t spaceOnRight;
      if(bFwd) {
        aligned = ALIGN(list->offset, alignment);
        spaceOnLeft = aligned - list->offset;
        spaceOnRight = list->size - size - spaceOnLeft;

      // Not enough space in this block
        if (spaceOnRight < 0) {
          list = list->next;
          continue;
        }
      } else {
        int16_t unaligned = list->offset + list->size - size - (alignment-1);
        if(unaligned < 0) {
          list = list->prev;
          continue;
        }
        aligned = ALIGN(unaligned, alignment);   //alloc from block's tail
        spaceOnLeft = aligned - list->offset;
        spaceOnRight = list->size - size - spaceOnLeft;

        // Not enough space in this block
        if (spaceOnLeft < 0) {
          list = list->prev;
          continue;
        }
      }

      // Cool we can use this block
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

      // Update the tail of the free blocks
      if (list == tail) {
        if (right)
          tail = right;
        else if (left)
          tail = left;
        else
          tail = NULL;
      }
      // Free the block and check the consistency
      this->deleteBlock(list);
      if (head && head->next) GBE_ASSERT(head->next->prev == head);
      if (tail && tail->prev) GBE_ASSERT(tail->prev->next == tail);

      // Track the allocation to retrieve the size later
      allocatedBlocks.insert(std::make_pair(aligned, size));
      // update max offset
      if(aligned + size > maxOffset) maxOffset = aligned + size;
      // We have a valid offset now
      return aligned;
    }
    GBE_ASSERT( !assertFail );
    return 0;
  }

  void SimpleAllocator::deallocate(int16_t offset)
  {
    // Retrieve the size in the allocation map
    auto it = allocatedBlocks.find(offset);
    GBE_ASSERT(it != allocatedBlocks.end());
    const int16_t size = it->second;

    // Find the two blocks where to insert the new block
    Block *list = tail, *next = NULL;
    while (list != NULL) {
      if (list->offset < offset)
        break;
      next = list;
      list = list->prev;
    }

    // Create the block and insert it
    Block *newBlock = this->newBlock(offset, size);
    if (list) {
      GBE_ASSERT(list->offset + list->size <= offset);
      list->next = newBlock;
      newBlock->prev = list;
    } else
      this->head = newBlock;  // list is NULL means newBlock should be the head.

    if (next) {
      GBE_ASSERT(offset + size <= next->offset);
      next->prev = newBlock;
      newBlock->next = next;
    } else
      this->tail = newBlock;  // next is NULL means newBlock should be the tail.

    if (list != NULL || next != NULL)
    {
      // Coalesce the blocks if possible
      this->coalesce(list, newBlock);
      this->coalesce(newBlock, next);
    }

    // Do not track this allocation anymore
    allocatedBlocks.erase(it);
  }

  void SimpleAllocator::coalesce(Block *left, Block *right) {
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

  void SimpleAllocator::splitBlock(int16_t offset, int16_t subOffset) {
    // Retrieve the size in the allocation map
    auto it = allocatedBlocks.find(offset);
    GBE_ASSERT(it != allocatedBlocks.end());

    while(subOffset > it->second) {
      subOffset -= it->second;
      offset += it->second;
      it = allocatedBlocks.find(offset);
      GBE_ASSERT(it != allocatedBlocks.end());
    }

    if(subOffset == 0)
      return;
    int16_t size = it->second;
    allocatedBlocks.erase(it);
    // Track the allocation to retrieve the size later
    allocatedBlocks.insert(std::make_pair(offset, subOffset));
    allocatedBlocks.insert(std::make_pair(offset + subOffset, size - subOffset));
  }

  ///////////////////////////////////////////////////////////////////////////
  // Generic Context (shared by the simulator and the HW context)
  ///////////////////////////////////////////////////////////////////////////
  IVAR(OCL_SIMD_WIDTH, 8, 15, 16);

  Context::Context(const ir::Unit &unit, const std::string &name) :
    unit(unit), fn(*unit.getFunction(name)), name(name), liveness(NULL), dag(NULL)
  {
    GBE_ASSERT(unit.getPointerSize() == ir::POINTER_32_BITS);
    this->liveness = GBE_NEW(ir::Liveness, const_cast<ir::Function&>(fn));
    this->dag = GBE_NEW(ir::FunctionDAG, *this->liveness);
    // r0 (GEN_REG_SIZE) is always set by the HW and used at the end by EOT
    this->registerAllocator = NULL; //GBE_NEW(RegisterAllocator, GEN_REG_SIZE, 4*KB - GEN_REG_SIZE);
    this->scratchAllocator = NULL; //GBE_NEW(ScratchAllocator, 12*KB);
  }

  Context::~Context(void) {
    GBE_SAFE_DELETE(this->registerAllocator);
    GBE_SAFE_DELETE(this->scratchAllocator);
    GBE_SAFE_DELETE(this->dag);
    GBE_SAFE_DELETE(this->liveness);
  }

  void Context::startNewCG(uint32_t simdWidth) {
    if (simdWidth == 0 || OCL_SIMD_WIDTH != 15)
      this->simdWidth = nextHighestPowerOf2(OCL_SIMD_WIDTH);
    else
      this->simdWidth = simdWidth;
    GBE_SAFE_DELETE(this->registerAllocator);
    GBE_SAFE_DELETE(this->scratchAllocator);
    GBE_ASSERT(dag != NULL && liveness != NULL);
    this->registerAllocator = GBE_NEW(RegisterAllocator, GEN_REG_SIZE, 4*KB - GEN_REG_SIZE);
    this->scratchAllocator = GBE_NEW(ScratchAllocator, this->getScratchSize());
    this->curbeRegs.clear();
    this->JIPs.clear();
  }

  Kernel *Context::compileKernel(void) {
    this->kernel = this->allocateKernel();
    this->kernel->simdWidth = this->simdWidth;
    this->buildArgList();
    if (usedLabels.size() == 0)
      this->buildUsedLabels();
    if (JIPs.size() == 0)
      this->buildJIPs();
    this->buildStack();
    this->handleSLM();
    if (this->emitCode() == false) {
      GBE_DELETE(this->kernel);
      this->kernel = NULL;
    }
    if(this->kernel != NULL) {
      this->kernel->scratchSize = this->alignScratchSize(scratchAllocator->getMaxScatchMemUsed());
      this->kernel->ctx = this;
    }
    return this->kernel;
  }

  int16_t Context::allocate(int16_t size, int16_t alignment) {
    return registerAllocator->allocate(size, alignment);
  }

  void Context::deallocate(int16_t offset) { registerAllocator->deallocate(offset); }

  void Context::splitBlock(int16_t offset, int16_t subOffset) {
    registerAllocator->splitBlock(offset, subOffset);
  }

  // FIXME TODO as we optimize scratch memory usage using the register interval.
  // we need to add some dependency in post_reg_alloc scheduler, to keep scratch
  // memory that are reused still keep the order

  int32_t Context::allocateScratchMem(uint32_t size) {
    return scratchAllocator->allocate(size, 32, true);
  }
  void Context::deallocateScratchMem(int32_t offset) {
    scratchAllocator->deallocate(offset);
  }

  void Context::buildStack(void) {
    const auto &stackUse = dag->getUse(ir::ocl::stackptr);
    if (stackUse.size() == 0)  // no stack is used if stackptr is unused
      return;
    // Be sure that the stack pointer is set
    // GBE_ASSERT(this->kernel->getCurbeOffset(GBE_CURBE_STACK_POINTER, 0) >= 0);
    uint32_t stackSize = 1*KB;
    while (stackSize < fn.getStackSize()) {
      stackSize <<= 1;
      GBE_ASSERT(stackSize <= 64*KB);
    }
    this->kernel->stackSize = stackSize;
  }

  uint32_t Context::newCurbeEntry(gbe_curbe_type value,
                              uint32_t subValue,
                              uint32_t size,
                              uint32_t alignment)
  {
    alignment = alignment == 0 ? size : alignment;
    const uint32_t offset = registerAllocator->allocate(size, alignment, 1);
    GBE_ASSERT(offset >= GEN_REG_SIZE);
    kernel->patches.push_back(PatchInfo(value, subValue, offset - GEN_REG_SIZE));
    kernel->curbeSize = std::max(kernel->curbeSize, offset + size - GEN_REG_SIZE);
    return offset;
  }

  uint32_t Context::getImageInfoCurbeOffset(ir::ImageInfoKey key, size_t size)
  {
    int32_t offset = fn.getImageSet()->getInfoOffset(key);
    if (offset >= 0)
      return offset + GEN_REG_SIZE;
    newCurbeEntry(GBE_CURBE_IMAGE_INFO, key.data, size, 4);
    std::sort(kernel->patches.begin(), kernel->patches.end());

    offset = kernel->getCurbeOffset(GBE_CURBE_IMAGE_INFO, key.data);
    GBE_ASSERT(offset >= 0); // XXX do we need to spill it out to bo?
    fn.getImageSet()->appendInfo(key, offset);
    return offset + GEN_REG_SIZE;
  }

  void Context::insertCurbeReg(ir::Register reg, uint32_t offset) {
    curbeRegs.insert(std::make_pair(reg, offset));
  }
  ir::Register Context::getSurfaceBaseReg(unsigned char bti) {
    return fn.getSurfaceBaseReg(bti);
  }

  void Context::buildArgList(void) {
    kernel->argNum = fn.argNum();
    if (kernel->argNum)
      kernel->args = GBE_NEW_ARRAY_NO_ARG(KernelArgument, kernel->argNum);
    else
      kernel->args = NULL;
    for (uint32_t argID = 0; argID < kernel->argNum; ++argID) {
      const auto &arg = fn.getArg(argID);

      kernel->args[argID].align = arg.align;
      kernel->args[argID].info = arg.info;
      switch (arg.type) {
        case ir::FunctionArgument::VALUE:
        case ir::FunctionArgument::STRUCTURE:
          kernel->args[argID].type = GBE_ARG_VALUE;
          kernel->args[argID].size = arg.size;
          break;
        case ir::FunctionArgument::GLOBAL_POINTER:
          kernel->args[argID].type = GBE_ARG_GLOBAL_PTR;
          kernel->args[argID].size = sizeof(void*);
          kernel->args[argID].bti = arg.bti;
          break;
        case ir::FunctionArgument::CONSTANT_POINTER:
          kernel->args[argID].type = GBE_ARG_CONSTANT_PTR;
          kernel->args[argID].size = sizeof(void*);
          break;
        case ir::FunctionArgument::LOCAL_POINTER:
          kernel->args[argID].type = GBE_ARG_LOCAL_PTR;
          kernel->args[argID].size = 0;
          break;
        case ir::FunctionArgument::IMAGE:
          kernel->args[argID].type = GBE_ARG_IMAGE;
          kernel->args[argID].size = sizeof(void*);
          break;
        case ir::FunctionArgument::SAMPLER:
          kernel->args[argID].type = GBE_ARG_SAMPLER;
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

  /* Because of the structural analysis, control flow of blocks inside a structure
   * is manipulated by if, else and endif. so these blocks don't need jips. so here
   * treats all the blocks belong to the same structure as a whole.
   */
  void Context::buildJIPs(void) {
    using namespace ir;
    // Linearly store the branch target for each block and its own label
    const LabelIndex noTarget(fn.labelNum());
    vector<std::pair<LabelIndex, LabelIndex>> braTargets;
    int32_t curr = 0;
    // If some blocks are unused we mark them as such by setting their own label
    // as "invalid" (== noTarget)
    int blockCount = 0;
    // because some blocks maybe belong to the same structure, so the number of
    // blocks we are dealing with may be less than the number of basic blocks.
    // here calculate the actual block number we would handle.
    fn.foreachBlock([&](const BasicBlock &bb)
    {
      if(bb.belongToStructure && bb.isStructureExit)
        blockCount++;
      else if(!bb.belongToStructure)
        blockCount++;
    });
    braTargets.resize(blockCount);

    LabelIndex structureExitLabel;
    LabelIndex structureEntryLabel;
    bool flag;
    set<uint32_t> pos;
    map<uint32_t, LabelIndex> exitMap;
    map<uint32_t, LabelIndex> entryMap;
    for (auto &bb : braTargets) bb = std::make_pair(noTarget, noTarget);
    fn.foreachBlock([&](const BasicBlock &bb) {
      LabelIndex ownLabel;
      Instruction *last;
      flag = false;
      // bb belongs to a structure and it's not the structure's exit, just simply insert
      // the target of bra to JIPs.
      if(bb.belongToStructure && !bb.isStructureExit)
      {
        last = bb.getLastInstruction();
        if(last->getOpcode() == OP_BRA)
        {
          BranchInstruction *bra = cast<BranchInstruction>(last);
          JIPs.insert(std::make_pair(bra, bra->getLabelIndex()));
        }
        return;
      }
      else
      {
        // bb belongs to a structure and it's the strucutre's exit, we treat this bb
        // as the structure it belongs to, use the label of structure's entry as this
        // structure's label and last instruction of structure's exit as this structure's
        // last instruction.
        if(bb.belongToStructure && bb.isStructureExit)
        {
          ownLabel = (bb.matchingStructureEntry)->getLabelIndex();
          last = bb.getLastInstruction();
          structureExitLabel = bb.getLabelIndex();
          structureEntryLabel = ownLabel;
          flag = true;
        }
        // bb belongs to no structure.
        else
        {
          ownLabel = bb.getLabelIndex();
          last = bb.getLastInstruction();
        }

        if (last->getOpcode() != OP_BRA)
        {
          braTargets[curr++] = std::make_pair(ownLabel, noTarget);
          if(flag)
          {
            pos.insert(curr-1);
            exitMap[curr-1] = structureExitLabel;
            entryMap[curr-1] = structureEntryLabel;
          }
        }
        else {
          const BranchInstruction *bra = cast<BranchInstruction>(last);
          braTargets[curr++] = std::make_pair(ownLabel, bra->getLabelIndex());
          if(flag)
          {
            exitMap[curr-1] = structureExitLabel;
            entryMap[curr-1] = structureEntryLabel;
            pos.insert(curr-1);
          }
        }
      }
    });
    // Backward jumps are special. We must insert the label of the next block
    // when we hit the "DO" i.e. the target label of the backward branch (as in
    // do { } while) . So, we store the bwd jumps per targets
    // XXX does not use custom allocator
    std::multimap<LabelIndex, LabelIndex> bwdTargets;
    for (int32_t blockID = 0; blockID <curr; ++blockID) {
      const LabelIndex ownLabel = braTargets[blockID].first;
      const LabelIndex target = braTargets[blockID].second;
      if (ownLabel == noTarget) continue; // unused block
      if (target == noTarget) continue; // no branch
      if (target <= ownLabel) { // This is a backward jump
        // Last block is just "RET". So, it cannot be the last block
        GBE_ASSERT(blockID < curr - 1);
        const LabelIndex fallThrough = braTargets[blockID+1].first;
        bwdTargets.insert(std::make_pair(target, fallThrough));
      }
    }

    // Stores the current forward targets
    set<LabelIndex> fwdTargets;
    // Now retraverse the blocks and figure out all JIPs
    for (int32_t blockID = 0; blockID <curr; ++blockID) {

      const LabelIndex ownLabel = braTargets[blockID].first;
      const LabelIndex target = braTargets[blockID].second;
      LabelIndex tmp;
      if(pos.find(blockID)!=pos.end())
        tmp = exitMap[blockID];
      else
        tmp = ownLabel;
      BasicBlock &bb = fn.getBlock(tmp);
      Instruction *label = bb.getFirstInstruction();
      if(pos.find(blockID)!=pos.end())
        label = fn.getBlock(entryMap[blockID]).getFirstInstruction();
      const Instruction *bra = bb.getLastInstruction();
      // Expires the branches that point to us (if any)
      auto it = fwdTargets.find(ownLabel);
      if (it != fwdTargets.end()) fwdTargets.erase(it);

      // Insert the fall through of the bwd branches that point to us if any
      auto ii = bwdTargets.equal_range(ownLabel);
      for (auto it = ii.first; it != ii.second; ++it)
        fwdTargets.insert(it->second);

      // If there is an outstanding forward branch, compute a JIP for the label
      auto lower = fwdTargets.lower_bound(LabelIndex(0));
      GBE_ASSERT(label->isMemberOf<LabelInstruction>() == true);
      if (lower != fwdTargets.end())
        JIPs.insert(std::make_pair(label, *lower));

      // Handle special cases and backward branches first
      if (ownLabel == noTarget) continue; // unused block
      if (target == noTarget) continue; // no branch at all
      GBE_ASSERT(bra->isMemberOf<BranchInstruction>() == true);
      if (target <= ownLabel) { // bwd branch: we always jump
        JIPs.insert(std::make_pair(bra, LabelIndex(target)));
        continue;
      }

      // This is a forward jump, register it and get the JIP
      fwdTargets.insert(target);
      auto jip = fwdTargets.lower_bound(LabelIndex(0));
      JIPs.insert(std::make_pair(bra, *jip));
    }

  }

  void Context::handleSLM(void) {
    const bool useSLM = fn.getUseSLM();
    kernel->useSLM = useSLM;
    kernel->slmSize = fn.getSLMSize();
  }

} /* namespace gbe */


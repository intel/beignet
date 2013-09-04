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
#include "ir/image.hpp"
#include "ir/sampler.hpp"
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
    int16_t allocate(int16_t size, int16_t alignment, bool bFwd=false);

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
    /*! Head and tail of the free list */
    Block *head;
    Block *tail;
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
    tail = head = this->newBlock(offset, size);
  }

  RegisterFilePartitioner::~RegisterFilePartitioner(void) {
    while (this->head) {
      Block *next = this->head->next;
      this->deleteBlock(this->head);
      this->head = next;
    }
  }

  int16_t RegisterFilePartitioner::allocate(int16_t size, int16_t alignment, bool bFwd)
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
      // We have a valid offset now
      return aligned;
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

  static int
  alignScratchSize(int size){
    int i = 0;

    for(; i < size; i+=1024)
      ;

    return i;
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
    this->partitioner = GBE_NEW_NO_ARG(RegisterFilePartitioner);
    if (fn.getSimdWidth() == 0 || OCL_SIMD_WIDTH != 15)
      this->simdWidth = nextHighestPowerOf2(OCL_SIMD_WIDTH);
    else
      this->simdWidth = fn.getSimdWidth();
    this->scratchOffset = 0;
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
    this->handleSLM();
    if (this->emitCode() == false) {
      GBE_DELETE(this->kernel);
      this->kernel = NULL;
    }
    if(this->kernel != NULL)
      this->kernel->scratchSize = alignScratchSize(this->scratchOffset);
    if(this->kernel != NULL)
      this->kernel->ctx = this;
    return this->kernel;
  }

  int16_t Context::allocate(int16_t size, int16_t alignment) {
    return partitioner->allocate(size, alignment);
  }

  void Context::deallocate(int16_t offset) { partitioner->deallocate(offset); }

  int32_t Context::allocConstBuf(uint32_t argID) {
     GBE_ASSERT(kernel->args[argID].type == GBE_ARG_CONSTANT_PTR);

    //free previous
    int32_t offset = kernel->getCurbeOffset(GBE_CURBE_EXTRA_ARGUMENT, argID+GBE_CONSTANT_BUFFER);
    if(offset >= 0)
        deallocate(offset+GEN_REG_SIZE);

    if(kernel->args[argID].bufSize > 0) {
      //use 32 alignment here as GEN_REG_SIZE, need dynamic by type?
      newCurbeEntry(GBE_CURBE_EXTRA_ARGUMENT, GBE_CONSTANT_BUFFER+argID, kernel->args[argID].bufSize, 32);
    }

    std::sort(kernel->patches.begin(), kernel->patches.end());
    offset = kernel->getCurbeOffset(GBE_CURBE_EXTRA_ARGUMENT, argID+GBE_CONSTANT_BUFFER);
    GBE_ASSERT(offset>=0);

    kernel->curbeSize = ALIGN(kernel->curbeSize, GEN_REG_SIZE);
    return offset + GEN_REG_SIZE;
  }

  uint32_t Context::allocateScratchMem(uint32_t size) {
    uint32_t offset = scratchOffset;
    scratchOffset += size;
    return offset;
  }

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
    const uint32_t offset = partitioner->allocate(size, alignment, 1);
    GBE_ASSERT(offset >= GEN_REG_SIZE);
    kernel->patches.push_back(PatchInfo(value, subValue, offset - GEN_REG_SIZE));
    kernel->curbeSize = std::max(kernel->curbeSize, offset + size - GEN_REG_SIZE);
  }

  uint32_t Context::getImageInfoCurbeOffset(ir::ImageInfoKey key, size_t size)
  {
    int32_t offset = fn.getImageSet()->getInfoOffset(key);
    if (offset >= 0)
      return offset;
    newCurbeEntry(GBE_CURBE_IMAGE_INFO, key.data, size, 4);
    std::sort(kernel->patches.begin(), kernel->patches.end());

    offset = kernel->getCurbeOffset(GBE_CURBE_IMAGE_INFO, key.data);
    GBE_ASSERT(offset >= 0); // XXX do we need to spill it out to bo?
    fn.getImageSet()->appendInfo(key, offset);
    return offset;
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
          arg.type == ir::FunctionArgument::LOCAL_POINTER ||
          arg.type == ir::FunctionArgument::CONSTANT_POINTER ||
          arg.type == ir::FunctionArgument::VALUE ||
          arg.type == ir::FunctionArgument::STRUCTURE ||
          arg.type == ir::FunctionArgument::IMAGE ||
          arg.type == ir::FunctionArgument::SAMPLER)
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

    bool useStackPtr = false;
    fn.foreachInstruction([&](const ir::Instruction &insn) {
      const uint32_t srcNum = insn.getSrcNum();
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        const ir::Register reg = insn.getSrc(srcID);
        if (fn.isSpecialReg(reg) == false) continue;
        if (specialRegs.contains(reg) == true) continue;
        if (reg == ir::ocl::stackptr) useStackPtr = true;
        INSERT_REG(lsize0, LOCAL_SIZE_X, 1)
        INSERT_REG(lsize1, LOCAL_SIZE_Y, 1)
        INSERT_REG(lsize2, LOCAL_SIZE_Z, 1)
        INSERT_REG(gsize0, GLOBAL_SIZE_X, 1)
        INSERT_REG(gsize1, GLOBAL_SIZE_Y, 1)
        INSERT_REG(gsize2, GLOBAL_SIZE_Z, 1)
        INSERT_REG(goffset0, GLOBAL_OFFSET_X, 1)
        INSERT_REG(goffset1, GLOBAL_OFFSET_Y, 1)
        INSERT_REG(goffset2, GLOBAL_OFFSET_Z, 1)
        INSERT_REG(workdim, WORK_DIM, 1)
        INSERT_REG(numgroup0, GROUP_NUM_X, 1)
        INSERT_REG(numgroup1, GROUP_NUM_Y, 1)
        INSERT_REG(numgroup2, GROUP_NUM_Z, 1)
        INSERT_REG(stackptr, STACK_POINTER, this->simdWidth)
        do {} while (0);
        specialRegs.insert(reg);
      }
    });
#undef INSERT_REG

    // Insert the number of threads
    this->newCurbeEntry(GBE_CURBE_THREAD_NUM, 0, sizeof(uint32_t));

    // Insert the stack buffer if used
    if (useStackPtr)
      this->newCurbeEntry(GBE_CURBE_EXTRA_ARGUMENT, GBE_STACK_BUFFER, ptrSize);

    // After this point the vector is immutable. Sorting it will make
    // research faster
    std::sort(kernel->patches.begin(), kernel->patches.end());

    // Align it on 32 bytes properly
    kernel->curbeSize = ALIGN(kernel->curbeSize, GEN_REG_SIZE);
  }

  void Context::buildArgList(void) {
    kernel->argNum = fn.argNum();
    if (kernel->argNum)
      kernel->args = GBE_NEW_ARRAY_NO_ARG(KernelArgument, kernel->argNum);
    else
      kernel->args = NULL;
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

    // Backward jumps are special. We must insert the label of the next block
    // when we hit the "DO" i.e. the target label of the backward branch (as in
    // do { } while) . So, we store the bwd jumps per targets
    // XXX does not use custom allocator
    std::multimap<LabelIndex, LabelIndex> bwdTargets;
    for (int32_t blockID = 0; blockID < blockNum; ++blockID) {
      const LabelIndex ownLabel = braTargets[blockID].first;
      const LabelIndex target = braTargets[blockID].second;
      if (ownLabel == noTarget) continue; // unused block
      if (target == noTarget) continue; // no branch
      if (target <= ownLabel) { // This is a backward jump
        // Last block is just "RET". So, it cannot be the last block
        GBE_ASSERT(blockID < blockNum - 1);
        const LabelIndex fallThrough = braTargets[blockID+1].first;
        bwdTargets.insert(std::make_pair(target, fallThrough));
      }
    }

    // Stores the current forward targets
    set<LabelIndex> fwdTargets;

    // Now retraverse the blocks and figure out all JIPs
    for (int32_t blockID = 0; blockID < blockNum; ++blockID) {
      const LabelIndex ownLabel = braTargets[blockID].first;
      const LabelIndex target = braTargets[blockID].second;
      const BasicBlock &bb = fn.getBlock(ownLabel);
      const Instruction *label = bb.getFirstInstruction();
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
  }

  bool Context::isScalarReg(const ir::Register &reg) const {
    GBE_ASSERT(fn.getProfile() == ir::Profile::PROFILE_OCL);
    if (fn.getArg(reg) != NULL) return true;
    if (fn.getPushLocation(reg) != NULL) return true;
    if (reg == ir::ocl::groupid0  ||
        reg == ir::ocl::groupid1  ||
        reg == ir::ocl::groupid2  ||
        reg == ir::ocl::barrierid ||
        reg == ir::ocl::threadn   ||
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
        reg == ir::ocl::goffset2  ||
        reg == ir::ocl::workdim)
      return true;
    return false;
  }

} /* namespace gbe */


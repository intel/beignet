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
 * \file gen_insn_scheduling.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "backend/gen_insn_selection.hpp"
#include "sys/cvar.hpp"

namespace gbe
{
  // Helper structure to schedule the basic blocks
  struct SelectionScheduler;

  // Node for the schedule DAG
  struct ScheduleNode;

  /*! We need to chain together the node we point */
  struct ScheduleNodeList {
    INLINE ScheduleNodeList(ScheduleNode *node) : node(node), next(NULL), prev(NULL) {}
    /*! Pointed node */
    ScheduleNode *node;
    /*! Next and previous dependent node in the list */
    ScheduleNodeList *next, *prev;
  };

  /*! Node of the DAG */
  struct ScheduleNode {
    INLINE ScheduleNode(SelectionInstruction &insn) :
      children(NULL), insn(insn), refNum(0), retiredCycle(0) {}
    bool dependsOn(ScheduleNode *node) const {
      GBE_ASSERT(node != NULL);
      ScheduleNodeList *dep = node->children;
      while (dep) {
        if (dep->node == this)
          return true;
        else
          dep = dep->next;
      }
      return false;
    }
    /*! Children that depends on us */
    ScheduleNodeList *children;
    /*! Instruction after code selection */
    SelectionInstruction &insn;
    /*! Number of nodes that point to us (i.e. nodes we depend on) */
    uint32_t refNum;
    /*! Cycle when the instruction is retired */
    uint32_t retiredCycle;
  };

  /*! To track loads and stores */
  enum GenMemory : uint8_t {
    GLOBAL_MEMORY = 0,
    LOCAL_MEMORY,
    MAX_MEM_SYSTEM
  };

  /*! Helper structure to handle dependencies while scheduling. Takes into
   *  account virtual and physical registers and memory sub-systems
   */
  struct DependencyTracker : public NonCopyable
  {
    DependencyTracker(const Selection &selection, SelectionScheduler &scheduler);
    /*! Reset it before scheduling a new block */
    void clear(void);
    /*! Get an index in the node array for the given register */
    uint32_t getIndex(GenRegister reg) const;
    /*! Get an index in the node array for the given memory system */
    uint32_t getIndex(uint32_t bti) const;
    /*! Add a new dependency "node0 depends on node1" */
    void addDependency(ScheduleNode *node0, ScheduleNode *node1);
    /*! Add a new dependency "node0 depends on node located at index" */
    void addDependency(ScheduleNode *node0, uint32_t index);
    /*! Add a new dependency "node located at index depends on node0" */
    void addDependency(uint32_t index, ScheduleNode *node0);
    /*! No dependency for null registers and immediate */
    INLINE bool ignoreDependency(GenRegister reg) const {
      if (reg.file == GEN_IMMEDIATE_VALUE)
        return true;
      else if (reg.file == GEN_ARCHITECTURE_REGISTER_FILE) {
        if ((reg.nr & 0xf0) == GEN_ARF_NULL)
          return true;
      }
      return false;
    }
    /*! Add a new dependency "node0 depends on node set for register reg" */
    INLINE void addDependency(ScheduleNode *node0, GenRegister reg) {
      if (this->ignoreDependency(reg) == false) {
        const uint32_t index = this->getIndex(reg);
        this->addDependency(node0, index);
      }
    }
    /*! Add a new dependency "node set for register reg depends on node0" */
    INLINE void addDependency(GenRegister reg, ScheduleNode *node0) {
      if (this->ignoreDependency(reg) == false) {
        const uint32_t index = this->getIndex(reg);
        this->addDependency(index, node0);
      }
    }
    /*! Make the node located at insnID a barrier */
    void makeBarrier(int32_t insnID, int32_t insnNum);
    /*! Update all the writes (memory, predicates, registers) */
    void updateWrites(ScheduleNode *node);
    /*! Maximum number of *physical* flag registers */
    static const uint32_t MAX_FLAG_REGISTER = 8u;
    /*! Maximum number of *physical* accumulators registers */
    static const uint32_t MAX_ACC_REGISTER = 1u;
    /*! Owns the tracker */
    SelectionScheduler &scheduler;
    /*! Stores the last node that wrote to a register / memory ... */
    vector<ScheduleNode*> nodes;
    /*! Stores the nodes per instruction */
    vector<ScheduleNode*> insnNodes;
    /*! Number of virtual register in the selection */
    uint32_t virtualNum;
  };

  /*! Perform the instruction scheduling */
  struct SelectionScheduler : public NonCopyable
  {
    /*! Init the book keeping structures */
    SelectionScheduler(GenContext &ctx, Selection &selection);
    /*! Make all lists empty */
    void clearLists(void);
    /*! Return the number of instructions to schedule in the DAG */
    int32_t buildDAG(SelectionBlock &bb);
    /*! Schedule the DAG */
    void scheduleDAG(SelectionBlock &bb, int32_t insnNum);
    /*! When an instruction is done, update the dependencies and remove it */
    void retire(ScheduleNodeList *node);
    /*! Insert the instruction in the ready list (it can be scheduled) */
    void makeReady(ScheduleNodeList *node);
    /*! Schedule the instruction (i.e. insert it in the new insn stream) */
    void schedule(ScheduleNodeList *node, uint32_t cycle);
    /*! Make ScheduleNodeList allocation faster */
    DECL_POOL(ScheduleNodeList, listPool);
    /*! Make ScheduleNode allocation faster */
    DECL_POOL(ScheduleNode, nodePool);
    /*! Scheduled instructions go here */
    SelectionInstruction *scheduled;
    /*! Ready list is instructions that can be scheduled */
    ScheduleNodeList *readyTail, *readyHead;
    /*! Active list is instructions that are executing */
    ScheduleNodeList *active;
    /*! Handle complete compilation */
    GenContext &ctx;
    /*! Code to schedule */
    Selection &selection;
    /*! To help tracking dependencies */
    DependencyTracker tracker;
  };

  DependencyTracker::DependencyTracker(const Selection &selection, SelectionScheduler &scheduler) :
    scheduler(scheduler)
  {
    this->virtualNum = selection.getRegNum();
    nodes.resize(virtualNum + MAX_FLAG_REGISTER + MAX_ACC_REGISTER + MAX_MEM_SYSTEM);
    insnNodes.resize(selection.getLargestBlockSize());
  }

  void DependencyTracker::clear(void) { for (auto &x : nodes) x = NULL; }

  void DependencyTracker::addDependency(ScheduleNode *node0, ScheduleNode *node1) {
    if (node0 != NULL && node1 != NULL && node0 != node1 && node0->dependsOn(node1) == false) {
      ScheduleNodeList *dep = scheduler.newScheduleNodeList(node0);
      node0->refNum++;
      dep->next = node1->children;
      node1->children = dep;
      if (dep->next)
        dep->next->prev = dep;
    }
  }

  void DependencyTracker::addDependency(ScheduleNode *node, uint32_t index) {
    this->addDependency(node, this->nodes[index]);
  }

  void DependencyTracker::addDependency(uint32_t index, ScheduleNode *node) {
    this->addDependency(this->nodes[index], node);
  }

  void DependencyTracker::makeBarrier(int32_t barrierID, int32_t insnNum) {
    ScheduleNode *barrier = this->insnNodes[barrierID];

    // The barrier depends on all nodes before it
    for (int32_t insnID = 0; insnID < barrierID; ++insnID)
      this->addDependency(barrier, this->insnNodes[insnID]);

    // All nodes after barriers depend on the barrier
    for (int32_t insnID = barrierID + 1; insnID < insnNum; ++insnID)
      this->addDependency(this->insnNodes[insnID], barrier);
  }

  static GenRegister getFlag(const SelectionInstruction &insn) {
    if (insn.state.physicalFlag) {
      const uint32_t nr = insn.state.flag;
      const uint32_t subnr = insn.state.subFlag;
      return GenRegister::flag(nr, subnr);
    } else
      return GenRegister::uw1grf(ir::Register(insn.state.flagIndex));
  }

  uint32_t DependencyTracker::getIndex(GenRegister reg) const {
    if (reg.physical) {
      GBE_ASSERT (reg.file == GEN_ARCHITECTURE_REGISTER_FILE);
      const uint32_t file = reg.nr & 0xf0;
      const uint32_t nr = reg.nr & 0x0f;
      if (file == GEN_ARF_FLAG) {
        const uint32_t subnr = reg.subnr / sizeof(uint16_t);
        GBE_ASSERT(nr < MAX_FLAG_REGISTER && (subnr == 0 || subnr == 1));
        return virtualNum + 2*nr + subnr;
      } else if (file == GEN_ARF_ACCUMULATOR) {
        GBE_ASSERT(nr < MAX_ACC_REGISTER);
        return virtualNum + MAX_FLAG_REGISTER + nr;
      } else {
        NOT_SUPPORTED;
        return 0;
      }
    } else
      return reg.value.reg;
  }

  uint32_t DependencyTracker::getIndex(uint32_t bti) const {
    const uint32_t memDelta = virtualNum + MAX_FLAG_REGISTER + MAX_ACC_REGISTER;
    return bti == 0xfe ? memDelta + LOCAL_MEMORY : memDelta + GLOBAL_MEMORY;
  }

  void DependencyTracker::updateWrites(ScheduleNode *node) {
    const SelectionInstruction &insn = node->insn;

    // Track writes in registers
    for (uint32_t dstID = 0; dstID < insn.dstNum; ++dstID) {
      const GenRegister dst = insn.dst(dstID);
      if (this->ignoreDependency(dst) == false) {
        const uint32_t index = this->getIndex(dst);
        this->nodes[index] = node;
      }
    }

    // Track writes in predicates
    if (insn.opcode == SEL_OP_CMP) {
      const uint32_t index = this->getIndex(getFlag(insn));
      this->nodes[index] = node;
    }

    // Track writes in accumulators
    if (insn.state.accWrEnable) {
      const uint32_t index = this->getIndex(GenRegister::acc());
      this->nodes[index] = node;
    }

    // Track writes in memory
    if (insn.isWrite()) {
      const uint32_t index = this->getIndex(insn.extra.function);
      this->nodes[index] = node;
    }
  }

  /*! Kind-of roughly estimated latency. Nothing real here */
  static uint32_t getLatencyGen7(const SelectionInstruction &insn) {
#define DECL_GEN7_SCHEDULE(FAMILY, LATENCY, SIMD16, SIMD8)\
    const uint32_t FAMILY##InstructionLatency = LATENCY;
#include "gen_insn_gen7_schedule_info.hxx"
#undef DECL_GEN7_SCHEDULE

    switch (insn.opcode) {
#define DECL_SELECTION_IR(OP, FAMILY) case SEL_OP_##OP: return FAMILY##Latency;
#include "backend/gen_insn_selection.hxx"
#undef DECL_SELECTION_IR
    };
    return 0;
  }

  /*! Throughput in cycles for SIMD8 or SIMD16 */
  static uint32_t getThroughputGen7(const SelectionInstruction &insn, bool isSIMD8) {
#define DECL_GEN7_SCHEDULE(FAMILY, LATENCY, SIMD16, SIMD8)\
    const uint32_t FAMILY##InstructionThroughput = isSIMD8 ? SIMD8 : SIMD16;
#include "gen_insn_gen7_schedule_info.hxx"
#undef DECL_GEN7_SCHEDULE

    switch (insn.opcode) {
#define DECL_SELECTION_IR(OP, FAMILY) case SEL_OP_##OP: return FAMILY##Throughput;
#include "backend/gen_insn_selection.hxx"
#undef DECL_SELECTION_IR
    };
    return 0;
  }

  SelectionScheduler::SelectionScheduler(GenContext &ctx, Selection &selection) :
    listPool(nextHighestPowerOf2(selection.getLargestBlockSize())),
    ctx(ctx), selection(selection), tracker(selection, *this)
  {
    this->clearLists();
  }

  void SelectionScheduler::clearLists(void) {
    this->scheduled = NULL;
    this->readyTail = NULL;
    this->readyHead = NULL;
    this->active = NULL;
  }

  int32_t SelectionScheduler::buildDAG(SelectionBlock &bb) {
    nodePool.rewind();
    listPool.rewind();
    tracker.clear();
    this->clearLists();

    // Track write-after-write and read-after-write dependencies
    int32_t insnNum = 0;
    bb.foreach([&](SelectionInstruction &insn) {
      // Create a new node for this instruction
      ScheduleNode *node = this->newScheduleNode(insn);
      tracker.insnNodes[insnNum++] = node;

      // read-after-write in registers
      for (uint32_t srcID = 0; srcID < insn.srcNum; ++srcID)
        tracker.addDependency(node, insn.src(srcID));

      // read-after-write for predicate
      if (insn.state.predicate != GEN_PREDICATE_NONE)
        tracker.addDependency(node, getFlag(insn));

      // read-after-write in memory
      if (insn.isRead()) {
        const uint32_t index = tracker.getIndex(insn.extra.function);
        tracker.addDependency(node, index);
      }

      // write-after-write in registers
      for (uint32_t dstID = 0; dstID < insn.dstNum; ++dstID)
        tracker.addDependency(node, insn.dst(dstID));

      // write-after-write for predicate
      if (insn.opcode == SEL_OP_CMP)
        tracker.addDependency(node, getFlag(insn));

      // write-after-write for accumulators
      if (insn.state.accWrEnable)
        tracker.addDependency(node, GenRegister::acc());

      // write-after-write in memory
      if (insn.isWrite()) {
        const uint32_t index = tracker.getIndex(insn.extra.function);
        tracker.addDependency(node, index);
      }

      // Track all writes done by the instruction
      tracker.updateWrites(node);
    });

    // Track write-after-read dependencies
    tracker.clear();
    for (int32_t insnID = insnNum-1; insnID >= 0; --insnID) {
      ScheduleNode *node = tracker.insnNodes[insnID];
      const SelectionInstruction &insn = node->insn;

      // write-after-read in registers
      for (uint32_t srcID = 0; srcID < insn.srcNum; ++srcID)
        tracker.addDependency(insn.src(srcID), node);

      // write-after-read for predicate
      if (insn.state.predicate != GEN_PREDICATE_NONE)
        tracker.addDependency(getFlag(insn), node);

      // write-after-read in memory
      if (insn.isRead()) {
        const uint32_t index = tracker.getIndex(insn.extra.function);
        tracker.addDependency(index, node);
      }

      // Track all writes done by the instruction
      tracker.updateWrites(node);
    }

    // Make labels and branches non-schedulable (i.e. they act as barriers)
    for (int32_t insnID = 0; insnID < insnNum; ++insnID) {
      ScheduleNode *node = tracker.insnNodes[insnID];
      if (node->insn.isBranch() || node->insn.isLabel() || node->insn.opcode == SEL_OP_EOT)
        tracker.makeBarrier(insnID, insnNum);
    }

    // Build the initial ready list
    for (int32_t insnID = 0; insnID < insnNum; ++insnID) {
      ScheduleNode *node = tracker.insnNodes[insnID];
      if (node->refNum == 0) {
        ScheduleNodeList *nodeList = this->newScheduleNodeList(node);
        this->makeReady(nodeList);
      }
    }

    return insnNum;
  }

  void SelectionScheduler::makeReady(ScheduleNodeList *list) {
    // Remove it from the current list
    GBE_ASSERT(list);
    if (list->prev) list->prev->next = list->next;
    if (list->next) list->next->prev = list->prev;

    // Insert it in the ready list
    list->prev = this->readyHead;
    list->next = NULL;
    if (this->readyHead != NULL) this->readyHead->next = list;
    if (this->readyTail == NULL) this->readyTail = list;
    this->readyHead = list;
  }

  void SelectionScheduler::retire(ScheduleNodeList *list) {
    // Update list
    if (list->prev) list->prev->next = list->next;
    if (list->next) list->next->prev = list->prev;

    // Update the active list
    if (list == this->active) this->active = list->next;

    // Update children reference counters and possibly make them active
    ScheduleNodeList *children = list->node->children;
    while (children) {
      ScheduleNodeList *next = children->next;
      if (--children->node->refNum == 0)
        this->makeReady(children);
      children = next;
    }
  }

  void SelectionScheduler::schedule(ScheduleNodeList *list, uint32_t cycle) {
    // Update list
    if (list->prev) list->prev->next = list->next;
    if (list->next) list->next->prev = list->prev;

    // Update the ready list
    if (list == this->readyHead) this->readyHead = list->prev;
    if (list == this->readyTail) this->readyTail = list->next;

    // Insert it in the active list
    list->next = this->active;
    list->prev = NULL;
    if (this->active) this->active->prev = list;
    this->active = list;

    // To know when the instruction retires
    list->node->retiredCycle = cycle + getLatencyGen7(list->node->insn);
  }

  void SelectionScheduler::scheduleDAG(SelectionBlock &bb, int32_t insnNum) {
    uint32_t cycle = 0;
    const bool isSIMD8 = this->ctx.getSimdWidth() == 8;
    while (insnNum) {

      // Retire all the instructions that finished
      ScheduleNodeList *toRetire = this->active;
      while (toRetire) {
        ScheduleNodeList *next = toRetire->next;
        if (toRetire->node->retiredCycle <= cycle)
          this->retire(toRetire);
        toRetire = next;
      }

      // Try to schedule something
      ScheduleNodeList *toSchedule = this->readyTail;
      if (toSchedule) {
        cycle += getThroughputGen7(toSchedule->node->insn, isSIMD8);
        this->schedule(toSchedule, cycle);
        toSchedule->node->insn.next = toSchedule->node->insn.prev = NULL;
        bb.append(&toSchedule->node->insn);
        insnNum--;
      } else 
        cycle++;
    }
  }

  BVAR(OCL_SCHEDULE_INSN, true);

  void schedulePreRegAllocation(GenContext &ctx, Selection &selection) {
    if (OCL_SCHEDULE_INSN) {
      SelectionScheduler scheduler(ctx, selection);
      selection.foreach([&](SelectionBlock &bb) {
        const int32_t insnNum = scheduler.buildDAG(bb);
        bb.insnHead = bb.insnTail = NULL;
        scheduler.scheduleDAG(bb, insnNum);
      });
    }
  }

} /* namespace gbe */


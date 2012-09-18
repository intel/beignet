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

namespace gbe
{
  /*! Chain selection instruction together */
  struct SelectionInstructionList {
    INLINE SelectionInstructionList(SelectionInstruction &insn) :
      next(NULL), insn(insn) {}
    /*! We chain the selection instruction together */
    SelectionInstructionList *next;
    /*! Instruction after code selection */
    SelectionInstruction &insn;
  };

  /*! Perform the instruction scheduling */
  struct SelectionScheduler
  {
    /*! Init the book keeping structures */
    SelectionScheduler(Selection &selection);
    /*! Reschedule the instructions */
    void schedule(void);
    /*! Make their allocation faster */
    DECL_POOL(SelectionInstructionList, listPool);
    /*! Scheduled instructions go here */
    SelectionInstruction *scheduled;
    /*! Ready list is instructions that can be scheduled */
    SelectionInstructionList *ready;
    /*! Active list is instructions that are executing */
    SelectionInstructionList *active;
  };
#if 0
  /*! Kind-of roughly estimated latency. Nothing real here */
  static uint32_t Gen7GetLatency(const SelectionInstruction &insn) {
    const uint32_t LabelInstructionLatency = 0;
    const uint32_t UnaryInstructionLatency = 20;
    const uint32_t BinaryInstructionLatency = 20;
    const uint32_t TernaryInstructionLatency = 20;
    const uint32_t CompareInstructionLatency = 20;
    const uint32_t JumpInstructionLatency = 20;
    const uint32_t EotInstructionLatency = 20;
    const uint32_t NoOpInstructionLatency  = 4;
    const uint32_t WaitInstructionLatency  = 20;
    const uint32_t MathInstructionLatency = 20;
    const uint32_t UntypedReadInstructionLatency  = 80;
    const uint32_t UntypedWriteInstructionLatency  = 80;
    const uint32_t ByteGatherInstructionLatency = 80;
    const uint32_t ByteScatterInstructionLatency = 80;
    const uint32_t RGatherInstructionLatency = 30;
    const uint32_t RegionInstructionLatency = 20;
    const uint32_t OBReadInstructionLatency = 80;
    const uint32_t OBWriteInstructionLatency = 80;
    switch (insn.opcode) {
#define DECL_SELECTION_IR(OP, FAMILY) case SEL_OP_##OP: return FAMILY##Latency;
#include "backend/gen_insn_selection.hxx"
#undef DECL_SELECTION_IR
    };
    return 0;
  }

  /*! Throughput in cycles for SIMD8 or SIMD16 */
  static uint32_t Gen7GetThroughput(const SelectionInstruction &insn, bool isSIMD8) {
    const uint32_t LabelInstructionThroughput = 0;
    const uint32_t UnaryInstructionThroughput = isSIMD8 ? 2 : 4;
    const uint32_t BinaryInstructionThroughput = isSIMD8 ? 2 : 4;
    const uint32_t TernaryInstructionThroughput = isSIMD8 ? 2 : 4;
    const uint32_t CompareInstructionThroughput = isSIMD8 ? 2 : 4;
    const uint32_t JumpInstructionThroughput = 1;
    const uint32_t EotInstructionThroughput = 1;
    const uint32_t NoOpInstructionThroughput = isSIMD8 ? 2 : 4;
    const uint32_t WaitInstructionThroughput = 1;
    const uint32_t MathInstructionThroughput = isSIMD8 ? 2 : 4;
    const uint32_t UntypedReadInstructionThroughput  = 1;
    const uint32_t UntypedWriteInstructionThroughput  = 1;
    const uint32_t ByteGatherInstructionThroughput = 1;
    const uint32_t ByteScatterInstructionThroughput = 1;
    const uint32_t RGatherInstructionThroughput = 1;
    const uint32_t RegionInstructionThroughput = isSIMD8 ? 2 : 4;
    const uint32_t OBReadInstructionThroughput = 1;
    const uint32_t OBWriteInstructionThroughput = 1;
    switch (insn.opcode) {
#define DECL_SELECTION_IR(OP, FAMILY) case SEL_OP_##OP: return FAMILY##Throughput;
#include "backend/gen_insn_selection.hxx"
#undef DECL_SELECTION_IR
    };
    return 0;
  }
#endif
  SelectionScheduler::SelectionScheduler(Selection &selection) :
    listPool(nextHighestPowerOf2(selection.getLargestBlockSize())),
    scheduled(NULL), ready(NULL), active(NULL)
  {}

  void SelectionScheduler::schedule(void) {

  }

  void schedulePreRegAllocation(Selection &selection) {
    SelectionScheduler scheduler(selection);
    scheduler.schedule();
  }

} /* namespace gbe */


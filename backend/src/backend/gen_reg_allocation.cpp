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
 * \file gen_reg_allocation.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */
#include "ir/profile.hpp"
#include "ir/function.hpp"
#include "backend/gen_insn_selection.hpp"
#include "backend/gen_reg_allocation.hpp"
#include "backend/gen_register.hpp"
#include "backend/program.hpp"
#include "sys/exception.hpp"
#include "sys/cvar.hpp"
#include <algorithm>
#include <climits>
#include <iostream>
#include <iomanip>


namespace gbe
{
  /////////////////////////////////////////////////////////////////////////////
  // Register allocator internal implementation
  /////////////////////////////////////////////////////////////////////////////

  /*! Provides the location of a register in a vector */
  typedef std::pair<SelectionVector*, uint32_t> VectorLocation;
  /*! Interval as used in linear scan allocator. Basically, stores the first and
   *  the last instruction where the register is alive
   */
  struct GenRegInterval {
    INLINE GenRegInterval(ir::Register reg) :
      reg(reg), minID(INT_MAX), maxID(-INT_MAX) {}
    ir::Register reg;     //!< (virtual) register of the interval
    int32_t minID, maxID; //!< Starting and ending points
  };

  typedef struct GenRegIntervalKey {
    GenRegIntervalKey(uint16_t reg, int32_t maxID) {
      key = ((uint64_t)maxID << 16) | reg;
    }
    const ir::Register getReg() const {
      return (ir::Register)(key & 0xFFFF);
    }
    int32_t getMaxID() const {
      return key >> 16;
    }
    uint64_t key;
  } GenRegIntervalKey;

  struct spillCmp {
    bool operator () (const GenRegIntervalKey &lhs, const GenRegIntervalKey &rhs) const
    { return lhs.key > rhs.key; }
  };

  typedef set <GenRegIntervalKey, spillCmp> SpillSet;

  class SpillCandidateSet : public SpillSet
  {
  public:
    std::set<GenRegIntervalKey, spillCmp>::iterator find(GenRegInterval interval) {
      GenRegIntervalKey key(interval.reg, interval.maxID);
      return SpillSet::find(key);
    }
    void insert(GenRegInterval interval) {
      GenRegIntervalKey key(interval.reg, interval.maxID);
      SpillSet::insert(key);
    }
    void erase(GenRegInterval interval) {
      GenRegIntervalKey key(interval.reg, interval.maxID);
      SpillSet::erase(key);
    }
  };

  /*! Implements the register allocation */
  class GenRegAllocator::Opaque
  {
  public:
    /*! Initialize the register allocator */
    Opaque(GenContext &ctx);
    /*! Release all taken resources */
    ~Opaque(void);
    /*! Perform the register allocation. Return true if success */
    bool allocate(Selection &selection);
    /*! Return the Gen register from the selection register */
    GenRegister genReg(const GenRegister &reg);
    /*! Output the register allocation */
    void outputAllocation(void);
    INLINE void getRegAttrib(ir::Register reg, uint32_t &regSize, ir::RegisterFamily *regFamily = NULL) const {
      // Note that byte vector registers use two bytes per byte (and can be
      // interleaved)
      static const size_t familyVectorSize[] = {2,2,2,4,8};
      static const size_t familyScalarSize[] = {2,2,2,4,8};
      using namespace ir;
      const bool isScalar = ctx.sel->isScalarReg(reg);
      const RegisterData regData = ctx.sel->getRegisterData(reg);
      const RegisterFamily family = regData.family;
      const uint32_t typeSize = isScalar ? familyScalarSize[family] : familyVectorSize[family];
      regSize = isScalar ? typeSize : ctx.getSimdWidth() * typeSize;
      if (regFamily != NULL)
        *regFamily = family;
    }
  private:
    /*! Expire one GRF interval. Return true if one was successfully expired */
    bool expireGRF(const GenRegInterval &limit);
    /*! Expire a flag register. Return true if one was successfully expired */
    bool expireFlag(const GenRegInterval &limit);
    /*! Allocate the virtual boolean (== flags) registers */
    void allocateFlags(Selection &selection);
    /*! validated flags which contains valid value in the physical flag register */
    set<uint16_t> validatedFlags;
    /*! validated temp flag register which indicate the flag 0,1 contains which virtual flag register. */
    uint16_t validTempFlagReg;
    /*! validate flag for the current flag user instruction */
    void validateFlag(Selection &selection, SelectionInstruction &insn);
    /*! Allocate the GRF registers */
    bool allocateGRFs(Selection &selection);
    /*! Create gen registers for all preallocated curbe registers. */
    void allocatePayloadRegs(void);
    /*! Create a Gen register from a register set in the payload */
    void allocatePayloadReg(ir::Register, uint32_t offset, uint32_t subOffset = 0);
    /*! Create the intervals for each register */
    /*! Allocate the vectors detected in the instruction selection pass */
    void allocateVector(Selection &selection);
    /*! Allocate the given interval. Return true if success */
    bool createGenReg(const GenRegInterval &interval);
    /*! Indicate if the registers are already allocated in vectors */
    bool isAllocated(const SelectionVector *vector) const;
    /*! Reallocate registers if needed to make the registers in the vector
     *  contigous in memory
     */
    void coalesce(Selection &selection, SelectionVector *vector);
    /*! The context owns the register allocator */
    GenContext &ctx;
    /*! Map virtual registers to offset in the (physical) register file */
    map<ir::Register, uint32_t> RA;
    /*! Map offset to virtual registers. */
    map<uint32_t, ir::Register> offsetReg;
    /*! Provides the position of each register in a vector */
    map<ir::Register, VectorLocation> vectorMap;
    /*! All vectors used in the selection */
    vector<SelectionVector*> vectors;
    /*! The set of booleans that will go to GRF (cannot be kept into flags) */
    set<ir::Register> grfBooleans;
    /*! The set of booleans which be held in flags, don't need to allocate grf */
    set<ir::Register> flagBooleans;
    /*! All the register intervals */
    vector<GenRegInterval> intervals;
    /*! All the boolean register intervals on the corresponding BB*/
    typedef map<ir::Register, GenRegInterval> RegIntervalMap;
    set<SelectionBlock *> flag0ReservedBlocks;
    map<SelectionBlock *, RegIntervalMap *> boolIntervalsMap;
    /*! Intervals sorting based on starting point positions */
    vector<GenRegInterval*> starting;
    /*! Intervals sorting based on ending point positions */
    vector<GenRegInterval*> ending;
    /*! registers that are spilled */
    SpilledRegs spilledRegs;
    /*! register which could be spilled.*/
    SpillCandidateSet spillCandidate;
    /* reserved registers for register spill/reload */
    uint32_t reservedReg;
    /*! Current vector to expire */
    uint32_t expiringID;
    INLINE void insertNewReg(ir::Register reg, uint32_t grfOffset, bool isVector = false);
    INLINE bool expireReg(ir::Register reg);
    INLINE bool spillAtInterval(GenRegInterval interval, int size, uint32_t alignment);
    INLINE uint32_t allocateReg(GenRegInterval interval, uint32_t size, uint32_t alignment);
    INLINE bool spillReg(GenRegInterval interval, bool isAllocated = false);
    INLINE bool spillReg(ir::Register reg, bool isAllocated = false);
    INLINE bool vectorCanSpill(SelectionVector *vector);
    INLINE void allocateScratchForSpilled();

    /*! replace specified source/dst register with temporary register and update interval */
    INLINE ir::Register replaceReg(Selection &sel, SelectionInstruction *insn,
                                   uint32_t regID, bool isSrc,
                                   ir::Type type = ir::TYPE_FLOAT, bool needMov = true) {
      ir::Register reg;
      if (isSrc) {
        reg = sel.replaceSrc(insn, regID, type, needMov);
        intervals.push_back(reg);
        intervals[reg].minID = insn->ID - 1;
        intervals[reg].maxID = insn->ID;
      } else {
        reg = sel.replaceDst(insn, regID, type, needMov);
        intervals.push_back(reg);
        intervals[reg].minID = insn->ID;
        intervals[reg].maxID = insn->ID + 1;
      }
      return reg;
    }
    /*! Use custom allocator */
    GBE_CLASS(Opaque);
  };


  GenRegAllocator::Opaque::Opaque(GenContext &ctx) : ctx(ctx) {}
  GenRegAllocator::Opaque::~Opaque(void) {}

  void GenRegAllocator::Opaque::allocatePayloadReg(ir::Register reg,
                                                   uint32_t offset,
                                                   uint32_t subOffset)
  {
    using namespace ir;
    assert(offset >= GEN_REG_SIZE);
    offset += subOffset;
    RA.insert(std::make_pair(reg, offset));
    GBE_ASSERT(reg != ocl::blockip || (offset % GEN_REG_SIZE == 0));
    this->intervals[reg].minID = 0;
    this->intervals[reg].maxID = 0;
  }

  INLINE void GenRegAllocator::Opaque::allocatePayloadRegs(void) {
    using namespace ir;
    for(auto &it : this->ctx.curbeRegs)
      allocatePayloadReg(it.first, it.second);

    // Allocate all pushed registers (i.e. structure kernel arguments)
    const Function &fn = ctx.getFunction();
    GBE_ASSERT(fn.getProfile() == PROFILE_OCL);
    const Function::PushMap &pushMap = fn.getPushMap();
    for (auto rit = pushMap.rbegin(); rit != pushMap.rend(); ++rit) {
      const uint32_t argID = rit->second.argID;
      const FunctionArgument arg = fn.getArg(argID);

      const uint32_t subOffset = rit->second.offset;
      const Register reg = rit->second.getRegister();
      auto it = this->ctx.curbeRegs.find(arg.reg);
      assert(it != ctx.curbeRegs.end());
      allocatePayloadReg(reg, it->second, subOffset);
      ctx.splitBlock(it->second, subOffset);
    }
  }

  bool GenRegAllocator::Opaque::createGenReg(const GenRegInterval &interval) {
    using namespace ir;
    const ir::Register reg = interval.reg;
    if (RA.contains(reg) == true)
      return true; // already allocated
    uint32_t regSize;
    ir::RegisterFamily family;
    getRegAttrib(reg, regSize, &family);
    uint32_t grfOffset = allocateReg(interval, regSize, regSize);
    if (grfOffset == 0) {
      return false;
    }
    insertNewReg(reg, grfOffset);
    return true;
  }

  bool GenRegAllocator::Opaque::isAllocated(const SelectionVector *vector) const {
    const ir::Register first = vector->reg[0].reg();
    const auto it = vectorMap.find(first);

    // If the first register is not allocated we are done
    if (it == vectorMap.end())
      return false;

    // If there are more left registers than in the found vector, there are
    // still registers to allocate
    const SelectionVector *other = it->second.first;
    const uint32_t otherFirst = it->second.second;
    const uint32_t leftNum = other->regNum - otherFirst;
    if (leftNum < vector->regNum)
      return false;

    // Now check that all the registers in the already allocated vector match
    // the current vector
    for (uint32_t regID = 1; regID < vector->regNum; ++regID) {
       const ir::Register from = vector->reg[regID].reg();
       const ir::Register to = other->reg[regID + otherFirst].reg();
       if (from != to)
         return false;
    }
    return true;
  }

  void GenRegAllocator::Opaque::coalesce(Selection &selection, SelectionVector *vector) {
    for (uint32_t regID = 0; regID < vector->regNum; ++regID) {
      const ir::Register reg = vector->reg[regID].reg();
      const auto it = this->vectorMap.find(reg);
      // case 1: the register is not already in a vector, so it can stay in this
      // vector. Note that local IDs are *non-scalar* special registers but will
      // require a MOV anyway since pre-allocated in the CURBE
      // If an element has very long interval, we don't want to put it into a
      // vector as it will add more pressure to the register allocation.
      if (it == vectorMap.end() &&
          ctx.sel->isScalarReg(reg) == false &&
          ctx.isSpecialReg(reg) == false &&
          (intervals[reg].maxID - intervals[reg].minID) < 2048)
      {
        const VectorLocation location = std::make_pair(vector, regID);
        this->vectorMap.insert(std::make_pair(reg, location));
      }
      // case 2: the register is already in another vector, so we need to move
      // it to a temporary register.
      // TODO: we can do better than that if we analyze the liveness of the
      // already allocated registers in the vector.  If there is no inteference
      // and the order is maintained, we can reuse the previous vector and avoid
      // the MOVs
      else {
        ir::Register tmp;
        ir::Type type = getIRType(vector->reg[regID].type);
        tmp = this->replaceReg(selection, vector->insn, regID, vector->isSrc, type);
        const VectorLocation location = std::make_pair(vector, regID);
        this->vectorMap.insert(std::make_pair(tmp, location));
      }
    }
  }

  /*! Will sort vector in decreasing order */
  inline bool cmp(const SelectionVector *v0, const SelectionVector *v1) {
    return v0->regNum > v1->regNum;
  }

  void GenRegAllocator::Opaque::allocateVector(Selection &selection) {
    const uint32_t vectorNum = selection.getVectorNum();
    this->vectors.resize(vectorNum);

    // First we find and store all vectors
    uint32_t vectorID = 0;
    for (auto &block : *selection.blockList)
      for (auto &v : block.vectorList)
        this->vectors[vectorID++] = &v;
    GBE_ASSERT(vectorID == vectorNum);

    // Heuristic (really simple...): sort them by the number of registers they
    // contain
    std::sort(this->vectors.begin(), this->vectors.end(), cmp);

    // Insert MOVs when this is required
    for (vectorID = 0; vectorID < vectorNum; ++vectorID) {
      SelectionVector *vector = this->vectors[vectorID];
      if (this->isAllocated(vector))
        continue;
      this->coalesce(selection, vector);
    }
  }

  template <bool sortStartingPoint>
  inline bool cmp(const GenRegInterval *i0, const GenRegInterval *i1) {
    if (sortStartingPoint) {
      if (i0->minID == i1->minID)
        return (i0->maxID < i1->maxID);
      return i0->minID < i1->minID;
    } else {
      if (i0->maxID == i1->maxID)
        return (i0->minID < i1->minID);
      return i0->maxID < i1->maxID;
    }
  }

  bool GenRegAllocator::Opaque::expireGRF(const GenRegInterval &limit) {
    bool ret = false;
    while (this->expiringID != ending.size()) {
      const GenRegInterval *toExpire = this->ending[this->expiringID];
      const ir::Register reg = toExpire->reg;

      // Dead code produced by the insn selection -> we skip it
      if (toExpire->minID > toExpire->maxID) {
        this->expiringID++;
        continue;
      }

      //ignore register that already spilled
      if(spilledRegs.find(reg) != spilledRegs.end()) {
        this->expiringID++;
        continue;
      }

      if (toExpire->maxID >= limit.minID)
        break;

      if (expireReg(reg))
        ret = true;
      this->expiringID++;
    }

    // We were not able to expire anything
    return ret;
  }


  #define IS_IMPLICITLY_MOD_FLAG(insn) (insn.state.modFlag == 1 &&      \
                                         (insn.opcode == SEL_OP_MOV ||  \
                                          insn.opcode == SEL_OP_AND  || \
                                          insn.opcode == SEL_OP_OR  ||  \
                                          insn.opcode == SEL_OP_XOR))

  #define IS_SCALAR_FLAG(insn) selection.isScalarReg(ir::Register(insn.state.flagIndex))
  #define GET_FLAG_REG(insn) GenRegister::uwxgrf(IS_SCALAR_FLAG(insn) ? 1 : 8,\
                                                 ir::Register(insn.state.flagIndex));
  #define IS_TEMP_FLAG(insn) (insn.state.flag == 0 && insn.state.subFlag == 1)
  // Flag is a virtual flag, this function is to validate the virtual flag
  // to a physical flag. It is used to validate both temporary flag and the
  // non-temporary flag registers.
  // We track the last temporary validate register, if it's the same as
  // current, we can avoid the revalidation.
  void GenRegAllocator::Opaque::validateFlag(Selection &selection,
                                             SelectionInstruction &insn) {
    GBE_ASSERT(insn.state.physicalFlag == 1);
    if (!IS_TEMP_FLAG(insn) && validatedFlags.find(insn.state.flagIndex) != validatedFlags.end())
      return;
    else if (IS_TEMP_FLAG(insn) && validTempFlagReg == insn.state.flagIndex)
      return;
    SelectionInstruction *cmp0 = selection.create(SEL_OP_CMP, 1, 2);
    cmp0->state = GenInstructionState(ctx.getSimdWidth());
    cmp0->state.flag = insn.state.flag;
    cmp0->state.subFlag = insn.state.subFlag;
    if (IS_SCALAR_FLAG(insn))
      cmp0->state.noMask = 1;
    cmp0->src(0) = GET_FLAG_REG(insn);
    cmp0->src(1) = GenRegister::immuw(0);
    cmp0->dst(0) = GenRegister::retype(GenRegister::null(), GEN_TYPE_UW);
    cmp0->extra.function = GEN_CONDITIONAL_NEQ;
    insn.prepend(*cmp0);
    if (!IS_TEMP_FLAG(insn))
      validatedFlags.insert(insn.state.flagIndex);
    else {
      if (insn.state.modFlag == 0)
        validTempFlagReg = insn.state.flagIndex;
      else
        validTempFlagReg = 0;
    }
  }

  
  void GenRegAllocator::Opaque::allocateFlags(Selection &selection) {
    // Previously, we have a global flag allocation implemntation.
    // After some analysis, I found the global flag allocation is not
    // the best solution here.
    // As for the cross block reference of bool value, we have to
    // combine it with current emask. There is no obvious advantage to
    // allocate deadicate physical flag register for those cross block usage.
    // We just need to allocate physical flag within each BB. We need to handle
    // the following cases:
    //
    // 1. The bool's liveness never beyond this BB. And the bool is only used as
    //    a dst register or a pred register. This bool value could be
    //    allocated in physical flag only if there is enough physical flag.
    //    We already identified those bool at the instruction select stage, and
    //    put them in the flagBooleans set.
    // 2. The bool is defined in another BB and used in this BB, then we need
    //    to prepend an instruction at the position where we use it.
    // 3. The bool is defined in this BB but is also used as some instruction's
    //    source registers rather than the pred register. We have to keep the normal
    //    grf (UW8/UW16) register for this bool. For some CMP instruction, we need to
    //    append a SEL instruction convert the flag to the grf register.
    // 4. Even for the spilling flag, if there is only one spilling flag, we will also
    //    try to reuse the temporary flag register latter. This requires all the
    //    instructions should got it flag at the instruction selection stage. And should
    //    not use the flag physical number directly at the gen_context stage. Otherwise,
    //    may break the algorithm here.
    // We will track all the validated bool value and to avoid any redundant
    // validation for the same flag. But if there is no enough physical flag,
    // we have to spill the previous allocated physical flag. And the spilling
    // policy is to spill the allocate flag which live to the last time end point.

    // we have three flags we use for booleans f0.0 , f1.0 and f1.1
    for (auto &block : *selection.blockList) {
      // Store the registers allocated in the map
      map<ir::Register, uint32_t> allocatedFlags;
      map<const GenRegInterval*, uint32_t> allocatedFlagIntervals;

      const uint32_t flagNum = flag0ReservedBlocks.contains(&block) ?  2 : 3;
      uint32_t freeFlags[] = {2, 3, 0};
      uint32_t freeNum = flagNum;
      if (boolIntervalsMap.find(&block) == boolIntervalsMap.end())
        continue;
      const auto boolsMap = boolIntervalsMap[&block];
      vector<const GenRegInterval*> flagStarting;
      vector<const GenRegInterval*> flagEnding;
      GBE_ASSERT(boolsMap->size() > 0);
      uint32_t regNum = boolsMap->size();
      flagStarting.resize(regNum);
      flagEnding.resize(regNum);
      uint32_t id = 0;
      for (auto &interval : *boolsMap) {
        flagStarting[id] = flagEnding[id] = &interval.second;
        id++;
      }
      std::sort(flagStarting.begin(), flagStarting.end(), cmp<true>);
      std::sort(flagEnding.begin(), flagEnding.end(), cmp<false>);

      uint32_t endID = 0; // interval to expire
      for (uint32_t startID = 0; startID < regNum; ++startID) {
        const GenRegInterval *interval = flagStarting[startID];
        const ir::Register reg = interval->reg;
        GBE_ASSERT(ctx.sel->getRegisterFamily(reg) == ir::FAMILY_BOOL);
        if (freeNum != 0) {
          allocatedFlags.insert(std::make_pair(reg, freeFlags[--freeNum]));
          allocatedFlagIntervals.insert(std::make_pair(interval, freeFlags[freeNum]));
        } else {
        // Try to expire one register
        while (endID != flagEnding.size()) {
          const GenRegInterval *toExpire = flagEnding[endID];
          // Dead code produced by the insn selection -> we skip it
          if (toExpire->minID > toExpire->maxID) {
            endID++;
            continue;
          }
          // We cannot expire this interval and the next ones
          if (toExpire->maxID >= interval->minID)
            break;
          // We reuse a flag from a previous interval (the oldest one)
          auto it = allocatedFlags.find(toExpire->reg);
          if (it == allocatedFlags.end()) {
            endID++;
            continue;
          }
          freeFlags[freeNum++] = it->second;
          endID++;
          break;
        }
        if (freeNum != 0) {
          allocatedFlags.insert(std::make_pair(reg, freeFlags[--freeNum]));
          allocatedFlagIntervals.insert(std::make_pair(interval, freeFlags[freeNum]));
        }
        else {
          // FIXME we may sort the allocated flags before do the spilling in the furture.
          int32_t spill = -1;
          const GenRegInterval *spillInterval = NULL;
          int32_t maxID = 0;
          for (auto &it : allocatedFlagIntervals) {
            if (it.first->maxID <= interval->minID)
              continue;
            if (it.first->maxID > maxID && it.second != 0) {
              maxID = it.first->maxID;
              spill = it.second;
              spillInterval = it.first;
            }
          }
          if (spill != -1) {
            allocatedFlags.insert(std::make_pair(reg, spill));
            allocatedFlagIntervals.insert(std::make_pair(interval, spill));
            allocatedFlags.erase(spillInterval->reg);
            allocatedFlagIntervals.erase(spillInterval);
            // We spill this flag booleans register, so erase it from the flag boolean set.
            if (flagBooleans.contains(spillInterval->reg))
              flagBooleans.erase(spillInterval->reg);
          } else {
            GBE_ASSERT(0);
          }
        }
        }
      }
      delete boolsMap;

      // Now, we traverse all the selection instructions and we patch them to make
      // them use flag registers
      validTempFlagReg = 0;
      validatedFlags.clear();
      for (auto &insn : block.insnList) {
        // Patch the predicate now. Note that only compares actually modify it (it
        // is called a "conditional modifier"). The other instructions just read
        // it
        if (insn.state.physicalFlag == 0) {
          auto it = allocatedFlags.find(ir::Register(insn.state.flagIndex));
          if (it != allocatedFlags.end()) {
            insn.state.physicalFlag = 1;
            insn.state.flag = it->second / 2;
            insn.state.subFlag = it->second & 1;

            // modFlag is for the LOADI/MOV/AND/OR/XOR instructions which will modify a
            // flag register. We set the condition for them to save one instruction if possible.
            if (IS_IMPLICITLY_MOD_FLAG(insn)) {
              // If this is a modFlag on a scalar bool, we need to remove it
              // from the allocated flags map. Then latter, the user could
              // validate the flag from the scalar value correctly.
              // The reason is we can not predicate the active channel when we
              // need to use this flag.
              if (IS_SCALAR_FLAG(insn)) {
                allocatedFlags.erase(ir::Register(insn.state.flagIndex));
                continue;
              }
              insn.extra.function = GEN_CONDITIONAL_NEQ;
            }
            // If this is an external bool, we need to validate it if it is not validated yet.
            if ((insn.state.externFlag &&
                 insn.state.predicate != GEN_PREDICATE_NONE))
              validateFlag(selection, insn);
          } else {
            insn.state.physicalFlag = 1;
            insn.state.flag = 0;
            insn.state.subFlag = 1;

            // If this is for MOV/AND/OR/... we don't need to waste an extra instruction
            // to generate the flag here, just continue to next instruction. And the validTempFlagReg
            // will not be destroyed.
            if (IS_IMPLICITLY_MOD_FLAG(insn))
              continue;
            // This bool doesn't have a deadicated flag, we use temporary flag here.
            // each time we need to validate it from the grf register.
            if (insn.state.predicate != GEN_PREDICATE_NONE)
              validateFlag(selection, insn);
          }
          // This is a CMP for a pure flag booleans, we don't need to write result to
          // the grf. And latter, we will not allocate grf for it.
          if (insn.opcode == SEL_OP_CMP &&
              (flagBooleans.contains(insn.dst(0).reg()) ||
               GenRegister::isNull(insn.dst(0)))) {
            // set a temporary register to avoid switch in this block.
            bool isSrc = false;
            bool needMov = false;
            this->replaceReg(selection, &insn, 0, isSrc, ir::TYPE_FLOAT, needMov);
          }
          // If the instruction requires to generate (CMP for long/int/float..)
          // the flag value to the register, and it's not a pure flag boolean,
          // we need to use SEL instruction to generate the flag value to the UW8
          // register.
          if (insn.state.flagGen == 1 &&
              !flagBooleans.contains((ir::Register)(insn.state.flagIndex))) {
            SelectionInstruction *sel0 = selection.create(SEL_OP_SEL, 1, 2);
            uint32_t simdWidth;
            simdWidth = IS_SCALAR_FLAG(insn) ? 1 : ctx.getSimdWidth();

            sel0->state = GenInstructionState(simdWidth);
            if (IS_SCALAR_FLAG(insn))
              sel0->state.noMask = 1;
            sel0->state.flag = insn.state.flag;
            sel0->state.subFlag = insn.state.subFlag;
            sel0->state.predicate = GEN_PREDICATE_NORMAL;
            sel0->src(0) = GenRegister::uw1grf(ir::ocl::one);
            sel0->src(1) = GenRegister::uw1grf(ir::ocl::zero);
            sel0->dst(0) = GET_FLAG_REG(insn);
            insn.append(*sel0);
            // We use the zero one after the liveness analysis, we have to update
            // the liveness data manually here.
            GenRegInterval &interval0 = intervals[ir::ocl::zero];
            GenRegInterval &interval1 = intervals[ir::ocl::one];
            interval0.minID = std::min(interval0.minID, (int32_t)insn.ID);
            interval0.maxID = std::max(interval0.maxID, (int32_t)insn.ID);
            interval1.minID = std::min(interval1.minID, (int32_t)insn.ID);
            interval1.maxID = std::max(interval1.maxID, (int32_t)insn.ID);
          }
        } else {
          // If the instruction use the temporary flag register manually,
          // we should invalidate the temp flag reg here.
          if (insn.state.flag == 0 && insn.state.subFlag == 1)
            validTempFlagReg = 0;
        }
      }
    }
  }

  IVAR(OCL_SIMD16_SPILL_THRESHOLD, 0, 16, 256);
  bool GenRegAllocator::Opaque::allocateGRFs(Selection &selection) {
    // Perform the linear scan allocator
    ctx.errCode = REGISTER_ALLOCATION_FAIL;
    const uint32_t regNum = ctx.sel->getRegNum();
    for (uint32_t startID = 0; startID < regNum; ++startID) {
      const GenRegInterval &interval = *this->starting[startID];
      const ir::Register reg = interval.reg;
      if (interval.maxID == -INT_MAX)
        continue; // Unused register
      if (RA.contains(reg))
        continue; // already allocated

      if (flagBooleans.contains(reg))
        continue;

      // Case 1: the register belongs to a vector, allocate all the registers in
      // one piece
      auto it = vectorMap.find(reg);
      if (it != vectorMap.end()) {
        const SelectionVector *vector = it->second.first;
        // all the reg in the SelectionVector are spilled
        if(spilledRegs.find(vector->reg[0].reg())
           != spilledRegs.end())
          continue;

        uint32_t alignment;
        uint32_t size = 0;
        for (uint32_t regID = 0; regID < vector->regNum; ++regID) {
          getRegAttrib(vector->reg[regID].reg(), alignment, NULL);
          size += alignment;
        }
        // FIXME this is workaround for scheduling limitation, which requires 2*GEN_REG_SIZE under SIMD16.
        const uint32_t maxAlignment = ctx.getSimdWidth()/8*GEN_REG_SIZE;
        const uint32_t grfOffset = allocateReg(interval, size, maxAlignment);
        if(grfOffset == 0) {
          for(int i = vector->regNum-1; i >= 0; i--) {
            if (!spillReg(vector->reg[i].reg()))
              return false;
          }
          continue;
        }
        uint32_t subOffset = 0;
        for (uint32_t regID = 0; regID < vector->regNum; ++regID) {
          const ir::Register reg = vector->reg[regID].reg();
          GBE_ASSERT(RA.contains(reg) == false);
          getRegAttrib(reg, alignment, NULL);
          // check all sub registers aligned correctly
          GBE_ASSERT((grfOffset + subOffset) % alignment == 0 || (grfOffset + subOffset) % GEN_REG_SIZE == 0);
          insertNewReg(reg, grfOffset + subOffset, true);
          ctx.splitBlock(grfOffset, subOffset);  //splitBlock will not split if regID == 0
          subOffset += alignment;
        }
      }
      // Case 2: This is a regular scalar register, allocate it alone
      else if (this->createGenReg(interval) == false) {
        if (!spillReg(interval))
          return false;
      }
    }
    if (!spilledRegs.empty()) {
      GBE_ASSERT(reservedReg != 0);
      if (ctx.getSimdWidth() == 16) {
        if (spilledRegs.size() > (unsigned int)OCL_SIMD16_SPILL_THRESHOLD) {
          ctx.errCode = REGISTER_SPILL_EXCEED_THRESHOLD;
          return false;
        }
      }
      allocateScratchForSpilled();
      bool success = selection.spillRegs(spilledRegs, reservedReg);
      if (!success) {
        ctx.errCode = REGISTER_SPILL_FAIL;
        return false;
      }
    }
    ctx.errCode = NO_ERROR;
    return true;
  }

  INLINE void GenRegAllocator::Opaque::allocateScratchForSpilled()
  {
    const uint32_t regNum = spilledRegs.size();
    this->starting.resize(regNum);
    this->ending.resize(regNum);
    uint32_t regID = 0;
    for(auto it = spilledRegs.begin(); it != spilledRegs.end(); ++it) {
      this->starting[regID] = this->ending[regID] = &intervals[it->first];
      regID++;
    }
    std::sort(this->starting.begin(), this->starting.end(), cmp<true>);
    std::sort(this->ending.begin(), this->ending.end(), cmp<false>);
    int toExpire = 0;
    for(uint32_t i = 0; i < regNum; i++) {
      const GenRegInterval * cur = starting[i];
      const GenRegInterval * exp = ending[toExpire];
      if (exp->maxID < cur->minID) {
        auto it = spilledRegs.find(exp->reg);
        GBE_ASSERT(it != spilledRegs.end());
        if(it->second.addr != -1) {
          ctx.deallocateScratchMem(it->second.addr);
        }
        toExpire++;
      }
      auto it = spilledRegs.find(cur->reg);
      GBE_ASSERT(it != spilledRegs.end());
      if(cur->minID == cur->maxID) {
        it->second.addr = -1;
        continue;
      }

      ir::RegisterFamily family = ctx.sel->getRegisterFamily(cur->reg);
      it->second.addr = ctx.allocateScratchMem(getFamilySize(family)
                                             * ctx.getSimdWidth());
      }
  }

  INLINE bool GenRegAllocator::Opaque::expireReg(ir::Register reg)
  {
    auto it = RA.find(reg);
    if (flagBooleans.contains(reg))
      return false;
    GBE_ASSERT(it != RA.end());
    // offset less than 32 means it is not managed by our reg allocator.
    if (it->second < 32)
      return false;

    ctx.deallocate(it->second);
    if (reservedReg != 0
        && (spillCandidate.find(intervals[reg]) != spillCandidate.end())) {
        spillCandidate.erase(intervals[reg]);
        /* offset --> reg map should keep updated. */
        offsetReg.erase(it->second);
    }

    return true;
  }

  // insert a new register with allocated offset,
  // put it to the RA map and the spill map if it could be spilled.
  INLINE void GenRegAllocator::Opaque::insertNewReg(ir::Register reg, uint32_t grfOffset, bool isVector)
  {
     RA.insert(std::make_pair(reg, grfOffset));

     if (reservedReg != 0) {

       uint32_t regSize;
       ir::RegisterFamily family;
       getRegAttrib(reg, regSize, &family);
       // At simd16 mode, we may introduce some simd8 registers in te instruction selection stage.
       // To spill those simd8 temporary registers will introduce unecessary complexity. We just simply
       // avoid to spill those temporary registers here.
       if (ctx.getSimdWidth() == 16 && reg.value() >= ctx.getFunction().getRegisterFile().regNum())
         return;

       if ((regSize == ctx.getSimdWidth()/8 * GEN_REG_SIZE && family == ir::FAMILY_DWORD)
          || (regSize == 2 * ctx.getSimdWidth()/8 * GEN_REG_SIZE && family == ir::FAMILY_QWORD)) {
         GBE_ASSERT(offsetReg.find(grfOffset) == offsetReg.end());
         offsetReg.insert(std::make_pair(grfOffset, reg));
         spillCandidate.insert(intervals[reg]);
       }
     }
  }

  INLINE bool GenRegAllocator::Opaque::spillReg(ir::Register reg,
                                                bool isAllocated) {
    return spillReg(intervals[reg], isAllocated);
  }

  INLINE bool GenRegAllocator::Opaque::spillReg(GenRegInterval interval,
                                                bool isAllocated) {
    if (reservedReg == 0)
      return false;

    if (interval.reg.value() >= ctx.getFunction().getRegisterFile().regNum() &&
        ctx.getSimdWidth() == 16)
      return false;

    ir::RegisterFamily family = ctx.sel->getRegisterFamily(interval.reg);
    // we currently only support DWORD/QWORD spill
    if(family != ir::FAMILY_DWORD && family != ir::FAMILY_QWORD)
      return false;

    SpillRegTag spillTag;
    spillTag.isTmpReg = interval.maxID == interval.minID;
    spillTag.addr = -1;

    if (isAllocated) {
      // If this register is allocated, we need to expire it and erase it
      // from the RA map.
      bool success = expireReg(interval.reg);
      GBE_ASSERT(success);
      if(!success) return success;
      RA.erase(interval.reg);
    }
    spilledRegs.insert(std::make_pair(interval.reg, spillTag));
    return true;
  }

  // Check whethere a vector which is allocated can be spilled out
  // If a partial of a vector is expired, the vector will be unspillable, currently.
  // FIXME we may need to fix those unspillable vector in the furture.
  INLINE bool GenRegAllocator::Opaque::vectorCanSpill(SelectionVector *vector) {
    for(uint32_t id = 0; id < vector->regNum; id++)
      if (spillCandidate.find(intervals[(ir::Register)(vector->reg[id].value.reg)])
          == spillCandidate.end())
        return false;
    return true;
  }

  INLINE bool GenRegAllocator::Opaque::spillAtInterval(GenRegInterval interval,
                                                       int size,
                                                       uint32_t alignment) {
    if (reservedReg == 0)
      return false;
    auto it = spillCandidate.begin();
    // If there is no spill candidate or current register is spillable and current register's
    // endpoint is after all the spillCandidate register's endpoint we return false. The
    // caller will spill current register.
    // At simd16 mode, we will always try to spill here rather than return to the caller.
    // The reason is that the caller may have a vector to allocate, and some element may be
    // temporary registers which could not be spilled.
    if (it == spillCandidate.end()
        || (ctx.getSimdWidth() == 8 && (it->getMaxID() <= interval.maxID
            && alignment == ctx.getSimdWidth()/8 * GEN_REG_SIZE)))
      return false;

    ir::Register reg = it->getReg();
    set<ir::Register> spillSet;
    int32_t savedSize = size;
    while(size > 0) {
      auto vectorIt = vectorMap.find(reg);
      bool isVector = vectorIt != vectorMap.end();
      bool needRestart = false;
      ir::RegisterFamily family = ctx.sel->getRegisterFamily(reg);
      if (isVector
          && (vectorCanSpill(vectorIt->second.first))) {
        const SelectionVector *vector = vectorIt->second.first;
        for (uint32_t id = 0; id < vector->regNum; id++) {
          GBE_ASSERT(spilledRegs.find(vector->reg[id].reg())
                     == spilledRegs.end());
          spillSet.insert(vector->reg[id].reg());
          reg = vector->reg[id].reg();
          family = ctx.sel->getRegisterFamily(reg);
          size -= family == ir::FAMILY_QWORD ? 2 * GEN_REG_SIZE * ctx.getSimdWidth()/8
                                             : GEN_REG_SIZE * ctx.getSimdWidth()/8;
        }
      } else if (!isVector) {
        spillSet.insert(reg);
        size -= family == ir::FAMILY_QWORD ? 2 * GEN_REG_SIZE * ctx.getSimdWidth()/8
                                           : GEN_REG_SIZE * ctx.getSimdWidth()/8;
      } else
        needRestart = true; // is a vector which could not be spilled.

      if (size <= 0)
        break;
      if (!needRestart) {
        uint32_t offset = RA.find(reg)->second;
        uint32_t nextOffset = (family == ir::FAMILY_QWORD) ? (offset + 2 * GEN_REG_SIZE * ctx.getSimdWidth() / 8)
                                                           : (offset + GEN_REG_SIZE * ctx.getSimdWidth() / 8);
        auto nextRegIt = offsetReg.find(nextOffset);
        if (nextRegIt != offsetReg.end())
          reg = nextRegIt->second;
        else
          needRestart = true;
      }

      if (needRestart) {
#if 0
        // FIXME, we should enable this code block in the future.
        // If the spill set is not zero and we need a restart, we can
        // simply return to try to allocate the registers at first.
        // As some vectors which have expired elements may be marked as
        // unspillable vector.
        if (spillSet.size() > 0)
          break;
#endif
        it++;
        // next register is not in spill candidate.
        // let's move to next candidate and start over.
        if (it == spillCandidate.end())
          return false;
        reg = it->getReg();
        size = savedSize;
        spillSet.clear();
      }
    }

    for(auto spillreg : spillSet)
      spillReg(spillreg, true);
    return true;
  }

  INLINE uint32_t GenRegAllocator::Opaque::allocateReg(GenRegInterval interval,
                                                       uint32_t size,
                                                       uint32_t alignment) {
    uint32_t grfOffset;
    // Doing expireGRF too freqently will cause the post register allocation
    // scheduling very hard. As it will cause a very high register conflict rate.
    // The tradeoff here is to reduce the freqency here. And if we are under spilling
    // then no need to reduce that freqency as the register pressure is the most
    // important factor.
    if (ctx.regSpillTick % 12 == 0 || ctx.reservedSpillRegs != 0)
      this->expireGRF(interval);
    ctx.regSpillTick++;
    // For some scalar byte register, it may be used as a destination register
    // and the source is a scalar Dword. If that is the case, the byte register
    // must get 4byte alignment register offset.
    alignment = (alignment + 3) & ~3;
    while ((grfOffset = ctx.allocate(size, alignment)) == 0) {
      const bool success = this->expireGRF(interval);
      if (success == false) {
        if (spillAtInterval(interval, size, alignment) == false)
          return 0;
      }
    }
    return grfOffset;
  }

  INLINE bool GenRegAllocator::Opaque::allocate(Selection &selection) {
    using namespace ir;
    if (ctx.reservedSpillRegs != 0) {
      reservedReg = ctx.allocate(ctx.reservedSpillRegs * GEN_REG_SIZE, GEN_REG_SIZE);
      reservedReg /= GEN_REG_SIZE;
    } else {
      reservedReg = 0;
    }
    // schedulePreRegAllocation(ctx, selection);

    // Now start the linear scan allocation
    for (uint32_t regID = 0; regID < ctx.sel->getRegNum(); ++regID)
      this->intervals.push_back(ir::Register(regID));

    // Allocate the special registers (only those which are actually used)
    this->allocatePayloadRegs();

    // Group and barrier IDs are always allocated by the hardware in r0
    RA.insert(std::make_pair(ocl::groupid0,  1*sizeof(float))); // r0.1
    RA.insert(std::make_pair(ocl::groupid1,  6*sizeof(float))); // r0.6
    RA.insert(std::make_pair(ocl::groupid2,  7*sizeof(float))); // r0.7
    RA.insert(std::make_pair(ocl::barrierid, 2*sizeof(float))); // r0.2

    // block IP used to handle the mask in SW is always allocated

    // Compute the intervals
    int32_t insnID = 0;
    for (auto &block : *selection.blockList) {
      int32_t lastID = insnID;
      int32_t firstID = insnID;
      // Update the intervals of each used register. Note that we do not
      // register allocate R0, so we skip all sub-registers in r0
      RegIntervalMap *boolsMap = new RegIntervalMap;
      if (block.isLargeBlock)
        flag0ReservedBlocks.insert(&block);
      for (auto &insn : block.insnList) {
        const uint32_t srcNum = insn.srcNum, dstNum = insn.dstNum;
        insn.ID  = insnID;
        for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
          const GenRegister &selReg = insn.src(srcID);
          const ir::Register reg = selReg.reg();
          if (selReg.file != GEN_GENERAL_REGISTER_FILE ||
              reg == ir::ocl::barrierid ||
              reg == ir::ocl::groupid0  ||
              reg == ir::ocl::groupid1  ||
              reg == ir::ocl::groupid2)
            continue;
          this->intervals[reg].minID = std::min(this->intervals[reg].minID, insnID);
          this->intervals[reg].maxID = std::max(this->intervals[reg].maxID, insnID);
        }
        for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
          const GenRegister &selReg = insn.dst(dstID);
          const ir::Register reg = selReg.reg();
          if (selReg.file != GEN_GENERAL_REGISTER_FILE ||
              reg == ir::ocl::barrierid ||
              reg == ir::ocl::groupid0 ||
              reg == ir::ocl::groupid1 ||
              reg == ir::ocl::groupid2)
            continue;
          this->intervals[reg].minID = std::min(this->intervals[reg].minID, insnID);
          this->intervals[reg].maxID = std::max(this->intervals[reg].maxID, insnID);
        }

        // OK, a flag is used as a predicate or a conditional modifier
        if (insn.state.physicalFlag == 0) {
          const ir::Register reg = ir::Register(insn.state.flagIndex);
          this->intervals[reg].minID = std::min(this->intervals[reg].minID, insnID);
          this->intervals[reg].maxID = std::max(this->intervals[reg].maxID, insnID);
          // Check whether this is a pure flag booleans candidate.
          if (insn.state.grfFlag == 0)
            flagBooleans.insert(reg);
          GBE_ASSERT(ctx.sel->getRegisterFamily(reg) == ir::FAMILY_BOOL);
          // update the bool register's per-BB's interval data
          if (boolsMap->find(reg) == boolsMap->end()) {
            GenRegInterval boolInterval(reg);
            boolsMap->insert(std::make_pair(reg, boolInterval));
          }
          boolsMap->find(reg)->second.minID = std::min(boolsMap->find(reg)->second.minID, insnID);
          boolsMap->find(reg)->second.maxID = std::max(boolsMap->find(reg)->second.maxID, insnID);
          if (&insn == block.insnList.back() &&
              insn.opcode == SEL_OP_JMPI &&
              insn.state.predicate != GEN_PREDICATE_NONE) {
            // If this is the last instruction and is a predicated JMPI.
            // We must extent its liveness before any other instrution.
            // As we need to allocate f0 to it, and need to keep the f0
            // unchanged during the block. The root cause is this instruction
            // is out-of the if/endif region, so we have to borrow the f0
            // to get correct bits for all channels.
            boolsMap->find(reg)->second.minID = 0;
            if (flag0ReservedBlocks.contains(&block))
              flag0ReservedBlocks.erase(&block);
          }
        } else {
          // Make sure that instruction selection stage didn't use physiacl flags incorrectly.
          GBE_ASSERT ((insn.opcode == SEL_OP_LABEL ||
                       insn.opcode == SEL_OP_IF ||
                       insn.opcode == SEL_OP_JMPI ||
                       insn.state.predicate == GEN_PREDICATE_NONE ||
                       (block.hasBarrier && insn.opcode == SEL_OP_MOV) ||
                       (insn.state.flag == 0 && insn.state.subFlag == 1) ||
                       (block.removeSimpleIfEndif && insn.state.flag == 0 && insn.state.subFlag == 0) ));
        }
        lastID = insnID;
        insnID++;
      }

      // All registers alive at the begining of the block must update their intervals.
      const ir::BasicBlock *bb = block.bb;
      for (auto reg : ctx.getLiveIn(bb))
        this->intervals[reg].minID = std::min(this->intervals[reg].minID, firstID);

      // All registers alive at the end of the block must have their intervals
      // updated as well
      for (auto reg : ctx.getLiveOut(bb))
        this->intervals[reg].maxID = std::max(this->intervals[reg].maxID, lastID);

      if (boolsMap->size() > 0)
        boolIntervalsMap.insert(std::make_pair(&block, boolsMap));
      else
        delete boolsMap;
    }

    this->intervals[ocl::retVal].minID = INT_MAX;
    this->intervals[ocl::retVal].maxID = -INT_MAX;

    // Allocate all the vectors first since they need to be contiguous
    this->allocateVector(selection);

    // First we try to put all booleans registers into flags
    this->allocateFlags(selection);

    // Sort both intervals in starting point and ending point increasing orders
    const uint32_t regNum = ctx.sel->getRegNum();
    this->starting.resize(regNum);
    this->ending.resize(regNum);
    for (uint32_t regID = 0; regID < regNum; ++regID)
      this->starting[regID] = this->ending[regID] = &intervals[regID];
    std::sort(this->starting.begin(), this->starting.end(), cmp<true>);
    std::sort(this->ending.begin(), this->ending.end(), cmp<false>);

    // Remove the registers that were not allocated
    this->expiringID = 0;
    while (this->expiringID < regNum) {
      const GenRegInterval *interval = ending[this->expiringID];
      if (interval->maxID == -INT_MAX)
        this->expiringID++;
      else
        break;
    }

    // Allocate all the GRFs now (regular register and boolean that are not in
    // flag registers)
    return this->allocateGRFs(selection);
  }

  INLINE void GenRegAllocator::Opaque::outputAllocation(void) {
    using namespace std;
    cout << "## register allocation ##" << endl;
    for(auto &i : RA) {
        ir::Register vReg = (ir::Register)i.first;
        ir::RegisterFamily family;
        uint32_t regSize;
        getRegAttrib(vReg, regSize, &family);
        int offst = (int)i.second;// / sizeof(float);
        int reg = offst / 32;
        int subreg = (offst % 32) / regSize;
        cout << "%" << setiosflags(ios::left) << setw(8) << vReg
             << "g" << setiosflags(ios::left) << setw(3) << reg << "."
             << setiosflags(ios::left) << setw(3) << subreg << ir::getFamilyName(family)
             << "  " << setw(-3) << regSize  << "B\t"
             << "[  " << setw(8) << this->intervals[(uint)vReg].minID
             << " -> " << setw(8) << this->intervals[(uint)vReg].maxID
             << "]" << endl;
    }
    if (!spilledRegs.empty())
      cout << "## spilled registers: " << spilledRegs.size() << endl;
    for(auto it = spilledRegs.begin(); it != spilledRegs.end(); it++) {
      ir::Register vReg = it->first;
      ir::RegisterFamily family;
      uint32_t regSize;
      getRegAttrib(vReg, regSize, &family);
      cout << "%" << setiosflags(ios::left) << setw(8) << vReg
           << "@" << setw(8) << it->second.addr
           << "  " << ir::getFamilyName(family)
           <<  "  " << setw(-3) << regSize << "B\t"
           << "[  " << setw(8) << this->intervals[(uint)vReg].minID
           << " -> " << setw(8) << this->intervals[(uint)vReg].maxID
           << "]" << endl;
    }
    cout << endl;
  }

  INLINE GenRegister setGenReg(const GenRegister &src, uint32_t grfOffset) {
    GenRegister dst;
    dst = src;
    dst.physical = 1;
    dst.nr = grfOffset / GEN_REG_SIZE;
    dst.subnr = grfOffset % GEN_REG_SIZE;
    return dst;
  }

  INLINE GenRegister GenRegAllocator::Opaque::genReg(const GenRegister &reg) {
    if (reg.file == GEN_GENERAL_REGISTER_FILE) {
      if(reg.physical == 1) {
        return reg;
      }
      GBE_ASSERT(RA.contains(reg.reg()) != false);
      const uint32_t grfOffset = RA.find(reg.reg())->second;
      const uint32_t suboffset = reg.subphysical ? reg.subnr : 0;
      const GenRegister dst = setGenReg(reg, grfOffset + suboffset);
      if (reg.quarter != 0)
        return GenRegister::Qn(dst, reg.quarter);
      else
        return dst;
    }
    else
      return reg;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Register allocator public implementation
  /////////////////////////////////////////////////////////////////////////////

  GenRegAllocator::GenRegAllocator(GenContext &ctx) {
    this->opaque = GBE_NEW(GenRegAllocator::Opaque, ctx);
  }

  GenRegAllocator::~GenRegAllocator(void) {
    GBE_DELETE(this->opaque);
  }

  bool GenRegAllocator::allocate(Selection &selection) {
    return this->opaque->allocate(selection);
  }

  GenRegister GenRegAllocator::genReg(const GenRegister &reg) {
    return this->opaque->genReg(reg);
  }

  void GenRegAllocator::outputAllocation(void) {
    this->opaque->outputAllocation();
  }

  uint32_t GenRegAllocator::getRegSize(ir::Register reg) {
     uint32_t regSize; 
     this->opaque->getRegAttrib(reg, regSize); 
     return regSize;
  }

} /* namespace gbe */


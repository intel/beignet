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
 * \file gen_reg_allocation.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "ir/profile.hpp"
#include "ir/function.hpp"
#include "backend/gen_insn_selection.hpp"
#include "backend/gen_reg_allocation.hpp"
#include "backend/program.hpp"
#include <algorithm>
#include <climits>

namespace gbe
{
  // Note that byte vector registers use two bytes per byte (and can be
  // interleaved)
  static const size_t familyVectorSize[] = {2,2,2,4,8};
  static const size_t familyScalarSize[] = {2,1,2,4,8};

  /*! IR-to-Gen type conversion */
  INLINE uint32_t getGenType(ir::Type type) {
    using namespace ir;
    switch (type) {
      case TYPE_BOOL: return GEN_TYPE_UW;
      case TYPE_S8: return GEN_TYPE_B;
      case TYPE_U8: return GEN_TYPE_UB;
      case TYPE_S16: return GEN_TYPE_W;
      case TYPE_U16: return GEN_TYPE_UW;
      case TYPE_S32: return GEN_TYPE_D;
      case TYPE_U32: return GEN_TYPE_UD;
      case TYPE_FLOAT: return GEN_TYPE_F;
      default: NOT_SUPPORTED; return GEN_TYPE_F;
    }
  }

#define LINEAR_SCAN 1

  /*! Interval as used in linear scan allocator. Basically, stores the first and
   *  the last instruction where the register is alive
   */
  struct GenRegInterval {
    INLINE GenRegInterval(ir::Register reg) :
      reg(reg), minID(INT_MAX), maxID(-INT_MAX) {}
    ir::Register reg;     //!< (virtual) register of the interval
    int32_t minID, maxID; //!< Starting and ending points
  };

  GenRegAllocator::GenRegAllocator(GenContext &ctx) : ctx(ctx) {}
  GenRegAllocator::~GenRegAllocator(void) {}

#define INSERT_REG(SIMD16, SIMD8, SIMD1) \
  if (ctx.sel->isScalarOrBool(reg) == true) \
    RA.insert(std::make_pair(reg, GenReg::SIMD1(nr, subnr))); \
  else if (simdWidth == 8) \
    RA.insert(std::make_pair(reg, GenReg::SIMD8(nr, subnr))); \
  else if (simdWidth == 16) \
    RA.insert(std::make_pair(reg, GenReg::SIMD16(nr, subnr))); \
  else \
    NOT_SUPPORTED;

  void GenRegAllocator::allocatePayloadReg(gbe_curbe_type value,
                                           ir::Register reg,
                                           uint32_t subValue,
                                           uint32_t subOffset)
  {
    using namespace ir;
    const Kernel *kernel = ctx.getKernel();
    const Function &fn = ctx.getFunction();
    const uint32_t simdWidth = ctx.getSimdWidth();
    const int32_t curbeOffset = kernel->getCurbeOffset(value, subValue);
    if (curbeOffset >= 0) {
      const uint32_t offset = curbeOffset + subOffset;
      const ir::RegisterData data = fn.getRegisterData(reg);
      const ir::RegisterFamily family = data.family;
      const bool isScalar = ctx.isScalarOrBool(reg);
      const uint32_t typeSize = isScalar ? familyScalarSize[family] : familyVectorSize[family];
      const uint32_t nr = (offset + GEN_REG_SIZE) / GEN_REG_SIZE;
      const uint32_t subnr = ((offset + GEN_REG_SIZE) % GEN_REG_SIZE) / typeSize;
      switch (family) {
        case FAMILY_BOOL: INSERT_REG(uw1grf, uw1grf, uw1grf); break;
        case FAMILY_WORD: INSERT_REG(uw16grf, uw8grf, uw1grf); break;
        case FAMILY_BYTE: INSERT_REG(ub16grf, ub8grf, ub1grf); break;
        case FAMILY_DWORD: INSERT_REG(f16grf, f8grf, f1grf); break;
        default: NOT_SUPPORTED;
      }
#if LINEAR_SCAN
      this->intervals[reg].minID = 0;
#endif
    }
  }

  void GenRegAllocator::createGenReg(ir::Register reg) {
    using namespace ir;
    const uint32_t simdWidth = ctx.getSimdWidth();
    if (RA.contains(reg) == true) return; // already allocated
    GBE_ASSERT(ctx.isScalarReg(reg) == false);
    const bool isScalar = ctx.sel->isScalarOrBool(reg);
    const RegisterData regData = ctx.sel->getRegisterData(reg);
    const RegisterFamily family = regData.family;
    const uint32_t typeSize = isScalar ? familyScalarSize[family] : familyVectorSize[family];
    const uint32_t regSize = simdWidth*typeSize;
    const uint32_t grfOffset = ctx.allocate(regSize, regSize);
    if (grfOffset != 0) {
      const uint32_t nr = grfOffset / GEN_REG_SIZE;
      const uint32_t subnr = (grfOffset % GEN_REG_SIZE) / typeSize;
      switch (family) {
        case FAMILY_BOOL: INSERT_REG(uw1grf, uw1grf, uw1grf); break;
        case FAMILY_WORD: INSERT_REG(uw16grf, uw8grf, uw1grf); break;
        case FAMILY_BYTE: INSERT_REG(ub16grf, ub8grf, ub1grf); break;
        case FAMILY_DWORD: INSERT_REG(f16grf, f8grf, f1grf); break;
        default: NOT_SUPPORTED;
      }
    } else
      GBE_ASSERTM(false, "Unable to register allocate");
  }

#undef INSERT_REG

  bool GenRegAllocator::isAllocated(const SelectionVector *vector) const {
    const ir::Register first = vector->reg[0].reg;
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
       const ir::Register from = vector->reg[regID].reg;
       const ir::Register to = other->reg[regID + otherFirst].reg;
       if (from != to)
         return false;
    }
    return true;
  }

  void GenRegAllocator::coalesce(Selection &selection, SelectionVector *vector) {
    for (uint32_t regID = 0; regID < vector->regNum; ++regID) {
      const ir::Register reg = vector->reg[regID].reg;
      const auto it = this->vectorMap.find(reg);
      // case 1: the register is not already in a vector, so it can stay in this
      // vector. Note that local IDs are *non-scalar* special registers but will
      // require a MOV anyway since pre-allocated in the CURBE
      if (it == vectorMap.end() &&
          ctx.sel->isScalarOrBool(reg) == false &&
          ctx.isSpecialReg(reg) == false)
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
        if (vector->isSrc)
          tmp = selection.replaceSrc(vector->insn, regID);
        else
          tmp = selection.replaceDst(vector->insn, regID);
        const VectorLocation location = std::make_pair(vector, regID);
        this->vectorMap.insert(std::make_pair(tmp, location));
      }
    }
  }

  /*! Will sort vector in decreasing order */
  INLINE bool cmp(const SelectionVector *v0, const SelectionVector *v1) {
    return v0->regNum > v1->regNum;
  }

  void GenRegAllocator::allocateVector(Selection &selection) {
    const uint32_t vectorNum = selection.getVectorNum();
    this->vectors.resize(vectorNum);

    // First we find and store all vectors
    uint32_t vectorID = 0;
    selection.foreach([&](SelectionBlock &block) {
      SelectionVector *v = block.vector;
      while (v) {
        GBE_ASSERT(vectorID < vectorNum);
        this->vectors[vectorID++] = v;
        v = v->next;
      }
    });
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

#if LINEAR_SCAN == 0
    // Allocate all the vector registers
    for (vectorID = 0; vectorID < vectorNum; ++vectorID) {
      const SelectionVector *vector = this->vectors[vectorID];
      const ir::Register first = vector->reg[0].reg;

      // Since there is no interference, if the first register is allocated,
      // this means that this vector is a sub-vector of a previous one, and
      // therefore all the registers are allocated
      if (RA.contains(first) == true) {
#if GBE_DEBUG
        for (uint32_t regID = 1; regID < vector->regNum; ++regID)
          GBE_ASSERT(RA.contains(vector->reg[regID].reg));
#endif /* GBE_DEBUG */
        continue;
      }

      //  Allocate all the vector registers consecutively
      const uint32_t simdWidth = ctx.getSimdWidth();
      const uint32_t alignment = simdWidth * sizeof(uint32_t);
      const uint32_t size = vector->regNum * alignment;
      uint32_t grfOffset = ctx.allocate(size, alignment);
      GBE_ASSERTM(grfOffset != 0, "Unable to register allocate");
      for (uint32_t regID = 0; regID < vector->regNum; ++regID, grfOffset += alignment) {
        const ir::Register reg = vector->reg[regID].reg;
        const uint32_t nr = grfOffset / GEN_REG_SIZE;
        const uint32_t subnr = (grfOffset % GEN_REG_SIZE) / sizeof(uint32_t);
        GBE_ASSERT(RA.contains(reg) == false);
        if (simdWidth == 16)
          RA.insert(std::make_pair(reg, GenReg::f16grf(nr, subnr)));
        else if (simdWidth == 8)
          RA.insert(std::make_pair(reg, GenReg::f8grf(nr, subnr)));
        else
          NOT_SUPPORTED;
      }
    }
#endif
  }

#if LINEAR_SCAN
  template <bool sortStartingPoint>
  INLINE bool cmp(const GenRegInterval *i0, const GenRegInterval *i1) {
    return sortStartingPoint ? i0->minID < i1->minID : i0->maxID < i1->maxID;
  }
#endif

  void GenRegAllocator::allocate(Selection &selection) {
    using namespace ir;
    const Kernel *kernel = ctx.getKernel();
    const Function &fn = ctx.getFunction();
    const uint32_t simdWidth = ctx.getSimdWidth();
    GBE_ASSERT(fn.getProfile() == PROFILE_OCL);

#if LINEAR_SCAN
    // Allocate all the vectors first since they need to be contiguous
    this->allocateVector(selection);

    // Now start the linear scan allocation
    for (uint32_t regID = 0; regID < ctx.sel->regNum(); ++regID) {
      this->intervals.push_back(ir::Register(regID));
    }
#endif

    // Allocate the special registers (only those which are actually used)
    allocatePayloadReg(GBE_CURBE_LOCAL_ID_X, ocl::lid0);
    allocatePayloadReg(GBE_CURBE_LOCAL_ID_Y, ocl::lid1);
    allocatePayloadReg(GBE_CURBE_LOCAL_ID_Z, ocl::lid2);
    allocatePayloadReg(GBE_CURBE_LOCAL_SIZE_X, ocl::lsize0);
    allocatePayloadReg(GBE_CURBE_LOCAL_SIZE_Y, ocl::lsize1);
    allocatePayloadReg(GBE_CURBE_LOCAL_SIZE_Z, ocl::lsize2);
    allocatePayloadReg(GBE_CURBE_GLOBAL_SIZE_X, ocl::gsize0);
    allocatePayloadReg(GBE_CURBE_GLOBAL_SIZE_Y, ocl::gsize1);
    allocatePayloadReg(GBE_CURBE_GLOBAL_SIZE_Z, ocl::gsize2);
    allocatePayloadReg(GBE_CURBE_GLOBAL_OFFSET_X, ocl::goffset0);
    allocatePayloadReg(GBE_CURBE_GLOBAL_OFFSET_Y, ocl::goffset1);
    allocatePayloadReg(GBE_CURBE_GLOBAL_OFFSET_Z, ocl::goffset2);
    allocatePayloadReg(GBE_CURBE_GROUP_NUM_X, ocl::numgroup0);
    allocatePayloadReg(GBE_CURBE_GROUP_NUM_Y, ocl::numgroup1);
    allocatePayloadReg(GBE_CURBE_GROUP_NUM_Z, ocl::numgroup2);
    allocatePayloadReg(GBE_CURBE_STACK_POINTER, ocl::stackptr);

    // Group IDs are always allocated by the hardware in r0
    RA.insert(std::make_pair(ocl::groupid0, GenReg::f1grf(0, 1)));
    RA.insert(std::make_pair(ocl::groupid1, GenReg::f1grf(0, 6)));
    RA.insert(std::make_pair(ocl::groupid2, GenReg::f1grf(0, 7)));

    // block IP used to handle the mask in SW is always allocated
    int32_t blockIPOffset = GEN_REG_SIZE + kernel->getCurbeOffset(GBE_CURBE_BLOCK_IP,0);
    GBE_ASSERT(blockIPOffset >= 0 && blockIPOffset % GEN_REG_SIZE == 0);
    blockIPOffset /= GEN_REG_SIZE;
    if (simdWidth == 8)
      RA.insert(std::make_pair(ocl::blockip, GenReg::uw8grf(blockIPOffset, 0)));
    else if (simdWidth == 16)
      RA.insert(std::make_pair(ocl::blockip, GenReg::uw16grf(blockIPOffset, 0)));
    else
      NOT_SUPPORTED;
#if LINEAR_SCAN
    this->intervals[ocl::blockip].minID = 0;
#endif

    // Allocate all (non-structure) argument parameters
    const uint32_t argNum = fn.argNum();
    for (uint32_t argID = 0; argID < argNum; ++argID) {
      const FunctionArgument &arg = fn.getArg(argID);
      GBE_ASSERT(arg.type == FunctionArgument::GLOBAL_POINTER ||
                 arg.type == FunctionArgument::CONSTANT_POINTER ||
                 arg.type == FunctionArgument::VALUE ||
                 arg.type == FunctionArgument::STRUCTURE);
      allocatePayloadReg(GBE_CURBE_KERNEL_ARGUMENT, arg.reg, argID);
    }

    // Allocate all pushed registers (i.e. structure kernel arguments)
    const Function::PushMap &pushMap = fn.getPushMap();
    for (const auto &pushed : pushMap) {
      const uint32_t argID = pushed.second.argID;
      const uint32_t subOffset = pushed.second.offset;
      const Register reg = pushed.second.getRegister();
      allocatePayloadReg(GBE_CURBE_KERNEL_ARGUMENT, reg, argID, subOffset);
    }
#if LINEAR_SCAN == 0
    // First we build the set of all used registers
    set<Register> usedRegs;
    selection.foreachInstruction([&](const SelectionInstruction &insn) {
      const uint32_t srcNum = insn.srcNum, dstNum = insn.dstNum;
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        const SelectionReg reg = insn.src[srcID];
        if (reg.file == GEN_GENERAL_REGISTER_FILE)
          usedRegs.insert(reg.reg);
      }
      for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
        const SelectionReg reg = insn.dst[dstID];
        if (reg.file == GEN_GENERAL_REGISTER_FILE)
          usedRegs.insert(reg.reg);
      }
    });
#else
    int32_t insnID = 0;
    selection.foreach([&](const SelectionBlock &block) {
      int32_t lastID = insnID;

      // Update the intervals of each used register
      block.foreach([&](const SelectionInstruction &insn) {
        const uint32_t srcNum = insn.srcNum, dstNum = insn.dstNum;
        for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
          const SelectionReg &selReg = insn.src[srcID];
          const ir::Register reg = selReg.reg;
          if (selReg.file != GEN_GENERAL_REGISTER_FILE)
            return;
          this->intervals[reg].minID = min(this->intervals[reg].minID, insnID);
          this->intervals[reg].maxID = max(this->intervals[reg].maxID, insnID);
        }
        for (uint32_t dstID = 0; dstID < dstNum; ++dstID) {
          const SelectionReg &selReg = insn.dst[dstID];
          const ir::Register reg = selReg.reg;
          if (selReg.file != GEN_GENERAL_REGISTER_FILE)
            return;
          this->intervals[reg].minID = min(this->intervals[reg].minID, insnID);
          this->intervals[reg].maxID = max(this->intervals[reg].maxID, insnID);
        }
        lastID = insnID;
        insnID++;
      });

      // All registers alive at the end of the block must have their intervals
      // updated as well
      const ir::BasicBlock *bb = block.bb;
      const ir::Liveness::LiveOut &liveOut = ctx.getLiveOut(bb);
      for (auto reg : liveOut) {
        this->intervals[reg].minID = min(this->intervals[reg].minID, lastID);
        this->intervals[reg].maxID = max(this->intervals[reg].maxID, lastID);
      }
    });
#endif

#if LINEAR_SCAN == 0
    // Allocate all the vectors first since they need to be contiguous
    this->allocateVector(selection);
    // Allocate all used registers. Just crash when we run out-of-registers
    for (auto reg : usedRegs) this->createGenReg(reg);
#else
    // Extend the liveness of the registers that belong to vectors. Actually,
    // this is way too brutal, we should instead maintain a list of allocated
    // intervals to handle vector registers independently while doing the linear
    // scan (or anything else)
    for (auto vector : this->vectors) {
      const uint32_t regNum = vector->regNum;
      const ir::Register first = vector->reg[0].reg;
      int32_t minID = this->intervals[first].minID;
      int32_t maxID = this->intervals[first].maxID;
      for (uint32_t regID = 1; regID < regNum; ++regID) {
        const ir::Register reg = vector->reg[regID].reg;
        minID = min(minID, this->intervals[reg].minID);
        maxID = max(maxID, this->intervals[reg].maxID);
      }
      for (uint32_t regID = 0; regID < regNum; ++regID) {
        const ir::Register reg = vector->reg[regID].reg;
        this->intervals[reg].minID = minID;
        this->intervals[reg].maxID = maxID;
      }
    }

    // Sort both intervals in starting point and ending point increasing orders
    gbe::vector<GenRegInterval*> starting, ending;
    uint32_t regNum = ctx.sel->regNum();
    starting.resize(regNum);
    ending.resize(regNum);
    for (uint32_t regID = 0; regID < regNum; ++regID)
      starting[regID] = ending[regID] = &intervals[regID];
    std::sort(starting.begin(), starting.end(), cmp<true>);
    std::sort(ending.begin(), ending.end(), cmp<false>);

    // Perform the linear scan allocator
    // uint32_t endID = 0;
    for (uint32_t startID = 0; startID < regNum; ++startID) {
      const GenRegInterval &interval = *starting[startID];
      const ir::Register reg = interval.reg;
      if (interval.minID == INT_MAX)
        break; // Since this is sorted, all the others will be like this
      if (RA.contains(reg))
        continue; // already allocated

      // Case 1: the register belongs to a vector, allocate all the registers in
      // one piece
      auto it = vectorMap.find(reg);
      if (it != vectorMap.end()) {
        const SelectionVector *vector = it->second.first;
        const uint32_t simdWidth = ctx.getSimdWidth();
        const uint32_t alignment = simdWidth * sizeof(uint32_t);
        const uint32_t size = vector->regNum * alignment;
        uint32_t grfOffset = ctx.allocate(size, alignment);
        GBE_ASSERTM(grfOffset != 0, "Unable to register allocate");
        for (uint32_t regID = 0; regID < vector->regNum; ++regID, grfOffset += alignment) {
          const ir::Register reg = vector->reg[regID].reg;
          const uint32_t nr = grfOffset / GEN_REG_SIZE;
          const uint32_t subnr = (grfOffset % GEN_REG_SIZE) / sizeof(uint32_t);
          GBE_ASSERT(RA.contains(reg) == false);
          if (simdWidth == 16)
            RA.insert(std::make_pair(reg, GenReg::f16grf(nr, subnr)));
          else if (simdWidth == 8)
            RA.insert(std::make_pair(reg, GenReg::f8grf(nr, subnr)));
          else
            NOT_SUPPORTED;
        }
      }
      // Case 2: This is a regular scalar register, allocate it alone
      else 
        this->createGenReg(reg);
    }

#endif
  }

  INLINE void setGenReg(GenReg &dst, const SelectionReg &src) {
    dst.type = src.type;
    dst.file = src.file;
    dst.negation = src.negation;
    dst.absolute = src.absolute;
    dst.vstride = src.vstride;
    dst.width = src.width;
    dst.hstride = src.hstride;
    dst.address_mode = GEN_ADDRESS_DIRECT;
    dst.dw1.ud = src.immediate.ud;
  }

  GenReg GenRegAllocator::genReg(const SelectionReg &reg) {
    // Right now, only GRF are allocated ...
    if (reg.file == GEN_GENERAL_REGISTER_FILE) {
      GBE_ASSERT(RA.contains(reg.reg) != false);
      GenReg dst = RA.find(reg.reg)->second;
      setGenReg(dst, reg);
      if (reg.quarter != 0)
        dst = GenReg::Qn(dst, reg.quarter+1);
      return dst;
    }
    // Other registers are already physical registers
    else {
      GenReg dst;
      setGenReg(dst, reg);
      dst.nr = reg.nr;
      dst.subnr = reg.subnr;
      return dst;
    }
  }

  GenReg GenRegAllocator::genReg(ir::Register reg, ir::Type type) {
    const uint32_t genType = getGenType(type);
    auto it = RA.find(reg);
    GBE_ASSERT(it != RA.end());
    return GenReg::retype(it->second, genType);
  }

  GenReg GenRegAllocator::genRegQn(ir::Register reg, uint32_t quarter, ir::Type type) {
    GBE_ASSERT(quarter == 2 || quarter == 3 || quarter == 4);
    GenReg genReg = this->genReg(reg, type);
    if (ctx.isScalarReg(reg) == true)
      return genReg;
    else
      return GenReg::Qn(genReg, quarter);
  }

} /* namespace gbe */


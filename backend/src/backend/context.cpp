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
  }
  Context::~Context(void) {
    GBE_SAFE_DELETE(this->dag);
    GBE_SAFE_DELETE(this->liveness);
  }

  Kernel *Context::compileKernel(void) {
    this->kernel = this->allocateKernel();
    this->buildPatchList();
    this->buildArgList();
    this->buildUsedLabels();
    this->emitCode();
    return this->kernel;
  }

  void Context::buildPatchList(void) {
    const uint32_t ptrSize = unit.getPointerSize() == ir::POINTER_32_BITS ? 4u : 8u;
    kernel->curbeSize = 0u;

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

    // Go over all the instructions and find the special register value we need
    // to push
#define INSERT_REG(SPECIAL_REG, PATCH)                              \
  if (reg == ir::ocl::SPECIAL_REG) {                                \
    if (specialRegs.find(reg) != specialRegs.end()) continue;       \
    const PatchInfo patch(GBE_CURBE_##PATCH, 0, kernel->curbeSize); \
    kernel->patches.push_back(patch);                               \
    kernel->curbeSize += ptrSize;                                   \
  } else
    set<ir::Register> specialRegs; // already inserted registers
    fn.foreachInstruction([&](const ir::Instruction &insn) {
      // Special registers are immutable. So only check sources
      const uint32_t srcNum = insn.getSrcNum();
      for (uint32_t srcID = 0; srcID < srcNum; ++srcID) {
        const ir::Register reg = insn.getSrc(srcID);
        if (fn.isSpecialReg(reg) == false) continue;
        if (specialRegs.contains(reg) == true) continue;
        INSERT_REG(lsize0, LOCAL_SIZE_X)
        INSERT_REG(lsize1, LOCAL_SIZE_Y)
        INSERT_REG(lsize2, LOCAL_SIZE_Z)
        INSERT_REG(gsize0, GLOBAL_SIZE_X)
        INSERT_REG(gsize1, GLOBAL_SIZE_Y)
        INSERT_REG(gsize2, GLOBAL_SIZE_Z)
        INSERT_REG(goffset0, GLOBAL_OFFSET_X)
        INSERT_REG(goffset1, GLOBAL_OFFSET_Y)
        INSERT_REG(goffset2, GLOBAL_OFFSET_Z)
        INSERT_REG(numgroup0, GROUP_NUM_X)
        INSERT_REG(numgroup1, GROUP_NUM_Y)
        INSERT_REG(numgroup2, GROUP_NUM_Z);
        specialRegs.insert(reg);
      }
    });

    kernel->curbeSize = ALIGN(kernel->curbeSize, GEN_REG_SIZE);
    if (this->simdWidth == 16)
      if ((kernel->curbeSize + GEN_REG_SIZE) % (2*GEN_REG_SIZE) != 0)
        kernel->curbeSize += GEN_REG_SIZE;

    // Local IDs always go at the end of the curbe
    const size_t localIDSize = sizeof(uint32_t) * this->simdWidth;
    const PatchInfo lid0(GBE_CURBE_LOCAL_ID_X, 0, kernel->curbeSize+0*localIDSize);
    const PatchInfo lid1(GBE_CURBE_LOCAL_ID_Y, 0, kernel->curbeSize+1*localIDSize);
    const PatchInfo lid2(GBE_CURBE_LOCAL_ID_Z, 0, kernel->curbeSize+2*localIDSize);
    kernel->patches.push_back(lid0);
    kernel->patches.push_back(lid1);
    kernel->patches.push_back(lid2);

    // After this point the vector is immutable. Sorting it will make
    // research faster
    std::sort(kernel->patches.begin(), kernel->patches.end());
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

  bool Context::isScalarReg(const ir::Register &reg) const {
    GBE_ASSERT(fn.getProfile() == ir::Profile::PROFILE_OCL);
    if (fn.getInput(reg) != NULL)
      return true;
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


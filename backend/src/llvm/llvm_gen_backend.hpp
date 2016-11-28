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
 * \file llvm_gen_backend.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * Pass generation functions
 */
#ifndef __GBE_LLVM_GEN_BACKEND_HPP__
#define __GBE_LLVM_GEN_BACKEND_HPP__

#include <cxxabi.h>
#include "llvm/Config/llvm-config.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Instructions.h"
#include "sys/platform.hpp"
#include "sys/map.hpp"
#include <algorithm>

// LLVM Type
namespace llvm {
  class Type;
  /* Imported from pNaCl */
  llvm::Instruction *PhiSafeInsertPt(llvm::Use *U);

  void PhiSafeReplaceUses(llvm::Use *U, llvm::Value *NewVal);

  FunctionPass *createExpandConstantExprPass();
  FunctionPass *createExpandLargeIntegersPass();
  FunctionPass *createPromoteIntegersPass();
  FunctionPass *createStripAttributesPass();
  // Copy debug information from Original to New, and return New.
  template <typename T> T *CopyDebug(T *New, llvm::Instruction *Original) {
    New->setDebugLoc(Original->getDebugLoc());
   return New;
  }
}

namespace gbe
{
  // Final target of the Gen backend
  namespace ir { class Unit; }

  /*! All intrinsic Gen functions */
  enum OCLInstrinsic {
#define DECL_LLVM_GEN_FUNCTION(ID, NAME) GEN_OCL_##ID,
#include "llvm_gen_ocl_function.hxx"
  GEN_OCL_NOT_FOUND,   
#undef DECL_LLVM_GEN_FUNCTION
  };

  /*! Build the hash map for OCL functions on Gen */
  struct OCLIntrinsicMap {
    /*! Build the intrinsic map */
    OCLIntrinsicMap(void) {
#define DECL_LLVM_GEN_FUNCTION(ID, NAME) \
  map.insert(std::make_pair(#NAME, GEN_OCL_##ID));
#include "llvm_gen_ocl_function.hxx"
#undef DECL_LLVM_GEN_FUNCTION
    }
    /*! Sort intrinsics with their names */
    gbe::map<std::string, OCLInstrinsic> map;
    OCLInstrinsic find(const std::string symbol) const {
      auto it = map.find(symbol);

      if (it == map.end()) {
        int status;
        char *realName = abi::__cxa_demangle(symbol.c_str(), NULL, NULL, &status);
        if (status == 0) {
          std::string realFnName(realName), stripName;
          stripName = realFnName.substr(0, realFnName.find("("));
          it = map.find(stripName);
        }
        free(realName);
      }
      // FIXME, should create a complete error reporting mechanism
      // when found error in beignet managed passes including Gen pass.
      if (it == map.end()) {
        std::cerr << "Unresolved symbol: " << symbol << std::endl;
        std::cerr << "Aborting..." << std::endl;
        return GEN_OCL_NOT_FOUND; 
      }
      return it->second;
    }
  };

  /*! Sort the OCL Gen instrinsic functions (built on pre-main) */
  static const OCLIntrinsicMap intrinsicMap;

  /*! Pad the offset */
  int32_t getPadding(int32_t offset, int32_t align);

  /*! Get the type alignment in bytes */
  uint32_t getAlignmentByte(const ir::Unit &unit, llvm::Type* Ty);

  /*! Get the type size in bits */
  uint32_t getTypeBitSize(const ir::Unit &unit, llvm::Type* Ty);

  /*! Get the type size in bytes */
  uint32_t getTypeByteSize(const ir::Unit &unit, llvm::Type* Ty);

  /*! Get GEP constant offset for the specified operand.*/
  int32_t getGEPConstOffset(const ir::Unit &unit, llvm::CompositeType *CompTy, int32_t TypeIndex);

  /*! whether this is a kernel function */
  bool isKernelFunction(const llvm::Function &f);

  /*! Create a Gen-IR unit */
  llvm::FunctionPass *createGenPass(ir::Unit &unit);

  /*! Remove the GEP instructions */
  llvm::BasicBlockPass *createRemoveGEPPass(const ir::Unit &unit);

  /*! Merge load/store if possible */
  llvm::BasicBlockPass *createLoadStoreOptimizationPass();

  /*! Scalarize all vector op instructions */
  llvm::FunctionPass* createScalarizePass();
  /*! Remove/add NoDuplicate function attribute for barrier functions. */
  llvm::ModulePass* createBarrierNodupPass(bool);

  /*! Convert the Intrinsic call to gen function */
  llvm::BasicBlockPass *createIntrinsicLoweringPass();

  /*! Passer the printf function call. */
  llvm::FunctionPass* createPrintfParserPass(ir::Unit &unit);

  /*! Insert the time stamp for profiling. */
  llvm::FunctionPass* createProfilingInserterPass(int profilingType, ir::Unit &unit);

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 5
  /* customized loop unrolling pass. */
  llvm::LoopPass *createCustomLoopUnrollPass();
#endif
  llvm::FunctionPass* createSamplerFixPass();

  /*! Add all the function call of ocl to our bitcode. */
  llvm::Module* runBitCodeLinker(llvm::Module *mod, bool strictMath, ir::Unit &unit);

  /*! Get the moudule's opencl version form meta data. */
  uint32_t getModuleOclVersion(const llvm::Module *M);

  void collectDeviceEnqueueInfo(llvm::Module *mod, ir::Unit &unit);

  void* getPrintfInfo(llvm::CallInst* inst);
} /* namespace gbe */

#endif /* __GBE_LLVM_GEN_BACKEND_HPP__ */


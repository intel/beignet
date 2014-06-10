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
 * \file llvm_gen_backend.hpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * Pass generation functions
 */
#ifndef __GBE_LLVM_GEN_BACKEND_HPP__
#define __GBE_LLVM_GEN_BACKEND_HPP__

#include "llvm/Pass.h"
#include "sys/platform.hpp"
#include "sys/map.hpp"
#include "sys/hash_map.hpp"
#include <algorithm>

// LLVM Type
namespace llvm { class Type; }

namespace gbe
{
  // Final target of the Gen backend
  namespace ir { class Unit; }

  /*! All intrinsic Gen functions */
  enum OCLInstrinsic {
#define DECL_LLVM_GEN_FUNCTION(ID, NAME) GEN_OCL_##ID,
#include "llvm_gen_ocl_function.hxx"
#undef DECL_LLVM_GEN_FUNCTION
  };

  /*! Build the hash map for OCL functions on Gen */
  struct OCLIntrinsicMap {
    /*! Build the intrinsic hash map */
    OCLIntrinsicMap(void) {
#define DECL_LLVM_GEN_FUNCTION(ID, NAME) \
  map.insert(std::make_pair(#NAME, GEN_OCL_##ID));
#include "llvm_gen_ocl_function.hxx"
#undef DECL_LLVM_GEN_FUNCTION
    }
    /*! Sort intrinsics with their names */
    hash_map<std::string, OCLInstrinsic> map;
  };

  /*! Sort the OCL Gen instrinsic functions (built on pre-main) */
  static const OCLIntrinsicMap instrinsicMap;

  /*! Pad the offset */
  uint32_t getPadding(uint32_t offset, uint32_t align);

  /*! Get the type alignment in bytes */
  uint32_t getAlignmentByte(const ir::Unit &unit, llvm::Type* Ty);

  /*! Get the type size in bits */
  uint32_t getTypeBitSize(const ir::Unit &unit, llvm::Type* Ty);

  /*! Get the type size in bytes */
  uint32_t getTypeByteSize(const ir::Unit &unit, llvm::Type* Ty);

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
  llvm::FunctionPass* createPrintfParserPass();

  void* getPrintfInfo(llvm::CallInst* inst);
} /* namespace gbe */

#endif /* __GBE_LLVM_GEN_BACKEND_HPP__ */


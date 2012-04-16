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
 * \file llvm_to_gen.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Assembly/PrintModulePass.h"

#include "llvm/llvm_gen_backend.hpp"
#include "llvm/llvm_to_gen.hpp"
#include "sys/cvar.hpp"
#include "sys/platform.hpp"

namespace gbe
{
  BVAR(OCL_OUTPUT_LLVM, false);
  BVAR(OCL_OUTPUT_LLVM_BEFORE_EXTRA_PASS, false);

  bool llvmToGen(ir::Unit &unit, const char *fileName)
  {
    using namespace llvm;
    // Get the global LLVM context
    llvm::LLVMContext& c = llvm::getGlobalContext();
    std::string errInfo;
    llvm::raw_fd_ostream o("-", errInfo);

    // Get the module from its file
    SMDiagnostic Err;
    std::auto_ptr<Module> M;
    M.reset(ParseIRFile(fileName, Err, c));
    if (M.get() == 0) return false;
    Module &mod = *M.get();

    llvm::PassManager passes;

    // Print the code before further optimizations
    if (OCL_OUTPUT_LLVM_BEFORE_EXTRA_PASS)
      passes.add(createPrintModulePass(&o));
    passes.add(createScalarReplAggregatesPass()); // Break up allocas
    passes.add(createRemoveGEPPass(unit));
    passes.add(createConstantPropagationPass());
    passes.add(createDeadInstEliminationPass());  // Remove simplified instructions
    passes.add(createLowerSwitchPass());
    passes.add(createPromoteMemoryToRegisterPass());
    passes.add(createGVNPass());                  // Remove redundancies
    passes.add(createGenPass(unit));

    // Print the code extra optimization passes
    if (OCL_OUTPUT_LLVM)
      passes.add(createPrintModulePass(&o));
    passes.run(mod);
    return true;
  }
} /* namespace gbe */


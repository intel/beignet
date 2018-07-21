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
 * \file llvm_to_gen.cpp
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "llvm_includes.hpp"

#include "llvm/llvm_gen_backend.hpp"
#include "llvm/llvm_to_gen.hpp"
#include <llvm/IR/DiagnosticInfo.h>
#include <llvm/IR/DiagnosticPrinter.h>
#include "sys/cvar.hpp"
#include "sys/platform.hpp"
#include "ir/unit.hpp"
#include "ir/function.hpp"
#include "ir/structurizer.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory>

namespace gbe
{
  BVAR(OCL_OUTPUT_CFG, false);
  BVAR(OCL_OUTPUT_CFG_ONLY, false);
  BVAR(OCL_OUTPUT_CFG_GEN_IR, false);
  using namespace llvm;

#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
  #define TARGETLIBRARY  TargetLibraryInfoImpl
#else
  #define TARGETLIBRARY  TargetLibraryInfo
#endif

  void runFuntionPass(Module &mod, TARGETLIBRARY *libraryInfo, const DataLayout &DL)
  {
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
    legacy::FunctionPassManager FPM(&mod);
#else
    FunctionPassManager FPM(&mod);
#endif

#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
#elif LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 36
    FPM.add(new DataLayoutPass());
#elif LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR == 35
    FPM.add(new DataLayoutPass(DL));
#else
    FPM.add(new DataLayout(DL));
#endif

#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 35
    FPM.add(createVerifierPass(true));
#else
    FPM.add(createVerifierPass());
#endif
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
    FPM.add(new TargetLibraryInfoWrapperPass(*libraryInfo));
#else
    FPM.add(new TargetLibraryInfo(*libraryInfo));
#endif
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 38
    FPM.add(createTypeBasedAAWrapperPass());
    FPM.add(createBasicAAWrapperPass());
#else
    FPM.add(createTypeBasedAliasAnalysisPass());
    FPM.add(createBasicAliasAnalysisPass());
#endif
    FPM.add(createCFGSimplificationPass());
    FPM.add(createSROAPass());
    FPM.add(createEarlyCSEPass());
    FPM.add(createLowerExpectIntrinsicPass());

    FPM.doInitialization();
    for (Module::iterator I = mod.begin(),
           E = mod.end(); I != E; ++I)
      if (!I->isDeclaration())
        FPM.run(*I);
    FPM.doFinalization();
  }

  void runModulePass(Module &mod, TARGETLIBRARY *libraryInfo, const DataLayout &DL, int optLevel, bool strictMath)
  {
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
    legacy::PassManager MPM;
#else
    PassManager MPM;
#endif

#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
#elif LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 36
    MPM.add(new DataLayoutPass());
#elif LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR == 35
    MPM.add(new DataLayoutPass(DL));
#else
    MPM.add(new DataLayout(DL));
#endif

#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
    MPM.add(new TargetLibraryInfoWrapperPass(*libraryInfo));
#else
    MPM.add(new TargetLibraryInfo(*libraryInfo));
#endif
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 38
    MPM.add(createTypeBasedAAWrapperPass());
    MPM.add(createBasicAAWrapperPass());
#else
    MPM.add(createTypeBasedAliasAnalysisPass());
    MPM.add(createBasicAliasAnalysisPass());
#endif
    MPM.add(createIntrinsicLoweringPass());
    MPM.add(createBarrierNodupPass(false));   // remove noduplicate fnAttr before inlining.
    MPM.add(createFunctionInliningPass(20000));
    MPM.add(createBarrierNodupPass(true));    // restore noduplicate fnAttr after inlining.
    MPM.add(createStripAttributesPass(false));     // Strip unsupported attributes and calling conventions.
    MPM.add(createSamplerFixPass());
    MPM.add(createGlobalOptimizerPass());     // Optimize out global vars

    MPM.add(createIPSCCPPass());              // IP SCCP
    MPM.add(createDeadArgEliminationPass());  // Dead argument elimination

    MPM.add(createInstructionCombiningPass());// Clean up after IPCP & DAE
    MPM.add(createCFGSimplificationPass());   // Clean up after IPCP & DAE
    MPM.add(createPruneEHPass());             // Remove dead EH info
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 39
    MPM.add(createPostOrderFunctionAttrsLegacyPass());
#elif LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 38
    MPM.add(createPostOrderFunctionAttrsPass());       // Set readonly/readnone attrs
#else
    MPM.add(createFunctionAttrsPass());       // Set readonly/readnone attrs
#endif

    //MPM.add(createScalarReplAggregatesPass(64, true, -1, -1, 64))
    if(optLevel > 0)
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 38
      MPM.add(createSROAPass());
#else
      MPM.add(createSROAPass(/*RequiresDomTree*/ false));
#endif
    MPM.add(createEarlyCSEPass());              // Catch trivial redundancies
    MPM.add(createJumpThreadingPass());         // Thread jumps.
    MPM.add(createCorrelatedValuePropagationPass()); // Propagate conditionals
    MPM.add(createCFGSimplificationPass());     // Merge & remove BBs
    MPM.add(createInstructionCombiningPass());  // Combine silly seq's

    MPM.add(createTailCallEliminationPass());   // Eliminate tail calls
    MPM.add(createCFGSimplificationPass());     // Merge & remove BBs
    MPM.add(createReassociatePass());           // Reassociate expressions
    MPM.add(createLoopRotatePass());            // Rotate Loop
    MPM.add(createLICMPass());                  // Hoist loop invariants
    MPM.add(createLoopUnswitchPass(true));
    MPM.add(createInstructionCombiningPass());
    MPM.add(createIndVarSimplifyPass());        // Canonicalize indvars
    MPM.add(createLoopIdiomPass());             // Recognize idioms like memset.
    MPM.add(createLoopDeletionPass());          // Delete dead loops
    MPM.add(createLoopUnrollPass(640)); //1024, 32, 1024, 512)); //Unroll loops
    if(optLevel > 0) {
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 38
      MPM.add(createSROAPass());
#else
      MPM.add(createSROAPass(/*RequiresDomTree*/ false));
#endif
      MPM.add(createGVNPass());                 // Remove redundancies
    }
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 35
    // FIXME Workaround: we find that CustomLoopUnroll may increase register pressure greatly,
    // and it may even make som cl kernel cannot compile because of limited scratch memory for spill.
    // As we observe this under strict math. So we disable CustomLoopUnroll if strict math is enabled.
    if (!strictMath) {
#if !defined(__ANDROID__)
      MPM.add(createCustomLoopUnrollPass()); //1024, 32, 1024, 512)); //Unroll loops
#endif
      MPM.add(createLoopUnrollPass()); //1024, 32, 1024, 512)); //Unroll loops
      if(optLevel > 0) {
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 38
        MPM.add(createSROAPass());
#else
        MPM.add(createSROAPass(/*RequiresDomTree*/ false));
#endif
        MPM.add(createGVNPass());                 // Remove redundancies
      }
    }
#endif
    MPM.add(createMemCpyOptPass());             // Remove memcpy / form memset
    MPM.add(createSCCPPass());                  // Constant prop with SCCP

    // Run instcombine after redundancy elimination to exploit opportunities
    // opened up by them.
    MPM.add(createInstructionCombiningPass());
    MPM.add(createJumpThreadingPass());         // Thread jumps
    MPM.add(createCorrelatedValuePropagationPass());
    MPM.add(createDeadStoreEliminationPass());  // Delete dead stores
    MPM.add(createAggressiveDCEPass());         // Delete dead instructions
    MPM.add(createCFGSimplificationPass()); // Merge & remove BBs
    MPM.add(createInstructionCombiningPass());  // Clean up after everything.
    MPM.add(createStripDeadPrototypesPass()); // Get rid of dead prototypes
    if(optLevel > 0) {
      MPM.add(createGlobalDCEPass());         // Remove dead fns and globals.
      MPM.add(createConstantMergePass());     // Merge dup global constants
    }

    MPM.run(mod);
  }


#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
#define OUTPUT_BITCODE(STAGE, MOD)  do {         \
  legacy::PassManager passes__;           \
   if (OCL_OUTPUT_LLVM_##STAGE) {                \
     passes__.add(createPrintModulePass(*o));    \
     passes__.run(MOD);                          \
   }                                             \
 }while(0)
#elif LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 35
#define OUTPUT_BITCODE(STAGE, MOD)  do {         \
   PassManager passes__;           \
   if (OCL_OUTPUT_LLVM_##STAGE) {                \
     passes__.add(createPrintModulePass(*o));    \
     passes__.run(MOD);                          \
   }                                             \
 }while(0)
#else
#define OUTPUT_BITCODE(STAGE, MOD)  do {         \
   PassManager passes__;           \
   if (OCL_OUTPUT_LLVM_##STAGE) {                \
     passes__.add(createPrintModulePass(&*o));   \
     passes__.run(MOD);                          \
   }                                             \
 }while(0)
#endif

  BVAR(OCL_OUTPUT_LLVM_BEFORE_LINK, false);
  BVAR(OCL_OUTPUT_LLVM_AFTER_LINK, false);
  BVAR(OCL_OUTPUT_LLVM_AFTER_GEN, false);

  class gbeDiagnosticContext
  {
  public:
    gbeDiagnosticContext() : _str(""), messages(_str), printer(messages), _has_errors(false) {}
    void process(const llvm::DiagnosticInfo &diagnostic)
    {
      if (diagnostic.getSeverity() != DS_Remark) { // avoid noise from function inlining remarks
        diagnostic.print(printer);
      }
      if (diagnostic.getSeverity() == DS_Error) {
        _has_errors = true;
      }
    }
    std::string str(){return messages.str();}
    bool has_errors(){return _has_errors;}
  private:
    std::string _str;
    llvm::raw_string_ostream messages;
    llvm::DiagnosticPrinterRawOStream printer;
    bool _has_errors;
  };
  
  void gbeDiagnosticHandler(const llvm::DiagnosticInfo &diagnostic, void *context)
  {
    gbeDiagnosticContext *dc = reinterpret_cast<gbeDiagnosticContext*>(context);
    dc->process(diagnostic);
  }

  bool llvmToGen(ir::Unit &unit, const void* module,
                 int optLevel, bool strictMath, int profiling, std::string &errors)
  {
    std::string errInfo;
    std::unique_ptr<llvm::raw_fd_ostream> o = NULL;
    if (OCL_OUTPUT_LLVM_BEFORE_LINK || OCL_OUTPUT_LLVM_AFTER_LINK || OCL_OUTPUT_LLVM_AFTER_GEN)
      o = std::unique_ptr<llvm::raw_fd_ostream>(new llvm::raw_fd_ostream(fileno(stdout), false));

    Module* cl_mod = NULL;
    if (module) {
      cl_mod = reinterpret_cast<Module*>(const_cast<void*>(module));
    }

    if (!cl_mod) return false;

    OUTPUT_BITCODE(BEFORE_LINK, (*cl_mod));
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
    legacy::PassManager passes__;
#else
    PassManager passes__;
#endif
    //run ExpandConstantExprPass before collectDeviceEnqueueInfo
    //to simplify the analyze of block.
    passes__.add(createExpandConstantExprPass());    // constant prop may generate ConstantExpr
    passes__.run(*cl_mod);
    /* Must call before materialize when link */
    collectDeviceEnqueueInfo(cl_mod, unit);

    std::unique_ptr<Module> M;

    /* Before do any thing, we first filter in all CL functions in bitcode. */
    /* Also set unit's pointer size in runBitCodeLinker */
    M.reset(runBitCodeLinker(cl_mod, strictMath, unit));

    if (M.get() == 0)
      return true;

    Module &mod = *M.get();
    DataLayout DL(&mod);
    
    gbeDiagnosticContext dc;
#if LLVM_VERSION_MAJOR >= 6
    mod.getContext().setDiagnosticHandlerCallBack(&gbeDiagnosticHandler,&dc);
#else
    mod.getContext().setDiagnosticHandler(&gbeDiagnosticHandler,&dc);
#endif

#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
    mod.setDataLayout(DL);
#endif
    Triple TargetTriple(mod.getTargetTriple());
    TARGETLIBRARY *libraryInfo = new TARGETLIBRARY(TargetTriple);
    libraryInfo->disableAllFunctions();

    OUTPUT_BITCODE(AFTER_LINK, mod);

    runFuntionPass(mod, libraryInfo, DL);
    runModulePass(mod, libraryInfo, DL, optLevel, strictMath);
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
    legacy::PassManager passes;
#else
    PassManager passes;
#endif
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
#elif LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 36
    passes.add(new DataLayoutPass());
#elif LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR == 35
    passes.add(new DataLayoutPass(DL));
#else
    passes.add(new DataLayout(DL));
#endif
    // Print the code before further optimizations
    passes.add(createIntrinsicLoweringPass());
    passes.add(createStripAttributesPass(true));     // Strip unsupported attributes and calling conventions.
    passes.add(createFunctionInliningPass(20000));
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 37
    passes.add(createSROAPass());
#else
    passes.add(createScalarReplAggregatesPass(64, true, -1, -1, 64));
#endif
    passes.add(createLoadStoreOptimizationPass());
    passes.add(createConstantPropagationPass());
    passes.add(createPromoteMemoryToRegisterPass());
    if(optLevel > 0)
      passes.add(createGVNPass());                 // Remove redundancies
    passes.add(createPrintfParserPass(unit));
    passes.add(createExpandConstantExprPass());    // expand ConstantExpr
    passes.add(createScalarizePass());             // Expand all vector ops
    passes.add(createExpandLargeIntegersPass());   // legalize large integer operation
    passes.add(createInstructionCombiningPass());  // legalize will generate some silly instructions
    passes.add(createConstantPropagationPass());   // propagate constant after scalarize/legalize
    passes.add(createExpandConstantExprPass());    // constant prop may generate ConstantExpr
    passes.add(createPromoteIntegersPass());       // align integer size to power of two
    passes.add(createRemoveGEPPass(unit));         // Constant prop may generate gep
    passes.add(createDeadInstEliminationPass());   // Remove simplified instructions
    passes.add(createCFGSimplificationPass());     // Merge & remove BBs
    passes.add(createLowerSwitchPass());           // simplify cfg will generate switch-case instruction
    if (profiling) {
      passes.add(createProfilingInserterPass(profiling, unit));     // insert the time stamp for profiling.
    }
    passes.add(createScalarizePass());             // Expand all vector ops

    if(OCL_OUTPUT_CFG)
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40
      passes.add(createCFGPrinterLegacyPassPass());
#else
      passes.add(createCFGPrinterPass());
#endif
    if(OCL_OUTPUT_CFG_ONLY)
#if LLVM_VERSION_MAJOR * 10 + LLVM_VERSION_MINOR >= 40
      passes.add(createCFGOnlyPrinterLegacyPassPass());
#else
      passes.add(createCFGOnlyPrinterPass());
#endif
    passes.add(createGenPass(unit));
    passes.run(mod);
    errors = dc.str();
    if(dc.has_errors()){
      unit.setValid(false);
      delete libraryInfo;
      return true;
    }

    // Print the code extra optimization passes
    OUTPUT_BITCODE(AFTER_GEN, mod);

    const ir::Unit::FunctionSet& fs = unit.getFunctionSet();
    ir::Unit::FunctionSet::const_iterator iter = fs.begin();
    while(iter != fs.end())
    {
      ir::CFGStructurizer *structurizer = new ir::CFGStructurizer(iter->second);
      structurizer->StructurizeBlocks();
      delete structurizer;
      if (OCL_OUTPUT_CFG_GEN_IR)
        iter->second->outputCFG();
      iter++;
    }


    delete libraryInfo;
    return true;
  }
} /* namespace gbe */

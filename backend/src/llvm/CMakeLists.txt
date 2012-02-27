include (${LLVM_DIR}/AddLLVM.cmake)
include (${LLVM_DIR}/AddLLVMDefinitions.cmake)
include (${LLVM_DIR}/HandleLLVMOptions.cmake)
include (${LLVM_DIR}/LLVMProcessSources.cmake)
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(../)
add_llvm_target(GenBackend llvm_to_gen.cpp llvm_gen_backend.cpp)
add_llvm_library_dependencies(LLVMGenBackend
  LLVMAnalysis
  LLVMCodeGen
  LLVMCore
  LLVMMC
  LLVMScalarOpts
  LLVMSupport
  LLVMTarget
  LLVMTransformUtils)


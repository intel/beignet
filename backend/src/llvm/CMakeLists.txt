include (${LLVM_DIR}/AddLLVM.cmake)
include (${LLVM_DIR}/AddLLVMDefinitions.cmake)
include (${LLVM_DIR}/HandleLLVMOptions.cmake)
include (${LLVM_DIR}/LLVMProcessSources.cmake)
include_directories(${LLVM_INCLUDE_DIRS})
add_llvm_target(GenBackend GenBackend.cpp)

add_llvm_library_dependencies(LLVMGenBackend
  LLVMAnalysis
  LLVMGenBackendInfo
  LLVMCodeGen
  LLVMCore
  LLVMMC
  LLVMScalarOpts
  LLVMSupport
  LLVMTarget
  LLVMTransformUtils)

add_subdirectory(TargetInfo)


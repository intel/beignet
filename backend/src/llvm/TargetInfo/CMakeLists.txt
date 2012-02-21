include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/.. )

add_llvm_library(LLVMGenBackendInfo
  GenBackendTargetInfo.cpp
  )

add_llvm_library_dependencies(LLVMGenBackendInfo
  LLVMMC
  LLVMSupport
  LLVMTarget)

include (${LLVM_DIR}/AddLLVM.cmake)
include (${LLVM_DIR}/AddLLVMDefinitions.cmake)
include (${LLVM_DIR}/HandleLLVMOptions.cmake)
include (${LLVM_DIR}/LLVMProcessSources.cmake)
include_directories (${LLVM_INCLUDE_DIRS})
set (LLVM_LINK_COMPONENTS ${LLVM_TARGETS_TO_BUILD} bitreader asmparser)
add_llvm_library(llc llc.cpp)
link_directories (${LLVM_LIBRARY_DIRS})


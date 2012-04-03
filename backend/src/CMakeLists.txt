add_subdirectory(llvm)

if (GBE_USE_BLOB)
  set (GBE_SRC blob.cpp)
else (GBE_USE_BLOB)
  set (GBE_SRC
    sys/vector.hpp
    sys/hash_map.hpp
    sys/map.hpp
    sys/set.hpp
    sys/exception.hpp
    sys/assert.cpp
    sys/assert.hpp
    sys/string.cpp
    sys/string.hpp
    sys/alloc.cpp
    sys/alloc.hpp
    sys/sysinfo.cpp
    sys/sysinfo.hpp
    sys/mutex.cpp
    sys/mutex.hpp
    sys/condition.cpp
    sys/condition.hpp
    sys/platform.cpp
    sys/platform.hpp
    gbe_program.cpp
    gbe_program.h
    ir/context.cpp
    ir/context.hpp
    ir/profile.cpp
    ir/profile.hpp
    ir/type.cpp
    ir/type.hpp
    ir/unit.cpp
    ir/unit.hpp
    ir/constant.cpp
    ir/constant.hpp
    ir/instruction.cpp
    ir/instruction.hpp
    ir/liveness.cpp
    ir/register.cpp
    ir/register.hpp
    ir/function.cpp
    ir/function.hpp
    ir/value.cpp
    ir/value.hpp
    gen/program.cpp
    gen/program.hpp
    gen/program.h
    gen/brw_disasm.c
    gen/brw_eu_emit.c
    gen/brw_eu.c
    sim/program.cpp
    sim/program.h)

  if (GBE_COMPILE_UTESTS)
    set (GBE_SRC
      ${GBE_SRC}
      utest/utest_llvm.cpp
      utest/utest_test_utest.cpp
      utest/utest_context.cpp
      utest/utest.cpp
      utest/utest.hpp)
  endif (GBE_COMPILE_UTESTS)
endif (GBE_USE_BLOB)

include_directories (.)
link_directories (${LLVM_LIBRARY_DIRS})
add_library (gbe SHARED ${GBE_SRC})
include (${LLVM_DIR}/AddLLVMDefinitions.cmake)
target_link_libraries (gbe
  LLVMGenBackend
  LLVMTransformUtils
  LLVMCore
  LLVMTarget
  LLVMAnalysis
  LLVMCodeGen
  LLVMScalarOpts
  LLVMSelectionDAG
  LLVMAsmPrinter
  LLVMMCParser
  LLVMAsmParser
  LLVMBitReader)

if (GBE_COMPILE_UTESTS)
  set (TESTER_SRC utest/tester.cpp)
  add_executable (tester utest/tester.cpp)
  target_link_libraries (tester gbe)
endif (GBE_COMPILE_UTESTS)

install (TARGETS gbe LIBRARY DESTINATION lib)
install (FILES gbe_program.h DESTINATION include/gen)


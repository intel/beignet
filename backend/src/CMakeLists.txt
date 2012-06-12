add_subdirectory(llvm)

macro (stringify TO_STRINGIFY_PATH TO_STRINGIFY_FILES)
foreach (to_stringify_file ${TO_STRINGIFY_FILES})
  set (input_file ${TO_STRINGIFY_PATH}/${to_stringify_file}.h)
  set (output_file ${TO_STRINGIFY_PATH}/${to_stringify_file}_str.cpp)
  set (string_header "\\\"string\\\"")
  add_custom_command(
    OUTPUT ${output_file}
    COMMAND rm -rf ${output_file}
    COMMAND echo "\\\#include ${string_header}" >> ${output_file}
    COMMAND echo "namespace gbe {" >> ${output_file}
    COMMAND echo "std::string ${to_stringify_file}_str = " >> ${output_file}
    # Yeah!!! welcome to back slash hell
    COMMAND cat ${input_file} |sed 's/\\\\/\\\\\\\\/g' | sed 's/\\\"/\\\\\\\"/g' | awk '{ printf \(\"\\"%s\\\\n\\"\\n\", $$0\) }' >> ${output_file}
    COMMAND echo "\;" >> ${output_file}
    COMMAND echo "}" >> ${output_file}
    COMMAND echo "" >> ${output_file}
    MAIN_DEPENDENCY ${input_file})
endforeach (to_stringify_file)
endmacro (stringify)

set (TO_STRINGIFY_FILES simulator sim_vector)
stringify ("${GBE_SOURCE_DIR}/src/backend/sim/" "${TO_STRINGIFY_FILES}")
set (TO_STRINGIFY_FILES ocl_stdlib)
stringify ("${GBE_SOURCE_DIR}/src/" "${TO_STRINGIFY_FILES}")

if (GBE_USE_BLOB)
  set (GBE_SRC blob.cpp)
else (GBE_USE_BLOB)
  set (GBE_SRC
    ocl_stdlib.h
    ocl_stdlib_str.cpp
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
    sys/cvar.cpp
    sys/cvar.hpp
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
    ir/lowering.cpp
    ir/lowering.hpp
    backend/context.cpp
    backend/context.hpp
    backend/program.cpp
    backend/program.hpp
    backend/program.h
    backend/sim_context.cpp
    backend/sim_context.hpp
    backend/sim_program.cpp
    backend/sim_program.hpp
    backend/sim_program.h
    backend/sim/simulator_str.cpp
    backend/sim/sim_vector_str.cpp
    backend/sim/sim_vector.h
    backend/gen_insn_selection.cpp
    backend/gen_insn_selection.hpp
    backend/gen_reg_allocation.cpp
    backend/gen_reg_allocation.hpp
    backend/gen_context.cpp
    backend/gen_context.hpp
    backend/gen_program.cpp
    backend/gen_program.hpp
    backend/gen_program.h
    backend/gen_defs.hpp
    backend/gen_encoder.hpp
    backend/gen_encoder.cpp)

  if (GBE_COMPILE_UTESTS)
    set (GBE_SRC
      ${GBE_SRC}
      utest/utest_vector.cpp
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
install (FILES backend/program.h backend/sim/simulator.h DESTINATION include/gen)


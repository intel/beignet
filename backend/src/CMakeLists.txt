set (OCL_BITCODE_BIN "${BEIGNET_INSTALL_DIR}beignet.bc")
set (OCL_HEADER_DIR "${BEIGNET_INSTALL_DIR}/include")
set (OCL_PCH_OBJECT "${BEIGNET_INSTALL_DIR}beignet.pch")
set (GBE_OBJECT_DIR "${BEIGNET_INSTALL_DIR}/libgbe.so")
set (INTERP_OBJECT_DIR "${BEIGNET_INSTALL_DIR}/libgbeinterp.so")

configure_file (
    "GBEConfig.h.in"
    "GBEConfig.h"
)

add_subdirectory(libocl)
set (LOCAL_GBE_OBJECT_DIR "${CMAKE_CURRENT_BINARY_DIR}/libgbe.so" PARENT_SCOPE)
set (LOCAL_INTERP_OBJECT_DIR "${CMAKE_CURRENT_BINARY_DIR}/libgbeinterp.so" PARENT_SCOPE)
set (LOCAL_OCL_BITCODE_BIN "${OCL_OBJECT_DIR}/beignet.bc" PARENT_SCOPE)
set (LOCAL_OCL_HEADER_DIR "${OCL_OBJECT_DIR}/include/" PARENT_SCOPE)
set (LOCAL_OCL_PCH_OBJECT "${OCL_OBJECT_DIR}/beignet.local.pch" PARENT_SCOPE)

add_dependencies(beignet_bitcode libocl)

set (GBE_SRC
    ${ocl_blob_file}
    sys/vector.hpp
    sys/hash_map.hpp
    sys/map.hpp
    sys/set.hpp
    sys/intrusive_list.hpp
    sys/intrusive_list.cpp
    sys/exception.hpp
    sys/assert.cpp
    sys/assert.hpp
    sys/alloc.cpp
    sys/alloc.hpp
    sys/mutex.cpp
    sys/mutex.hpp
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
    ir/sampler.cpp
    ir/sampler.hpp
    ir/image.cpp
    ir/image.hpp
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
    ir/printf.cpp
    ir/printf.hpp
    ir/structural_analysis.cpp
    ir/structural_analysis.hpp
    ir/immediate.hpp
    ir/immediate.cpp
    backend/context.cpp
    backend/context.hpp
    backend/program.cpp
    backend/program.hpp
    backend/program.h
    llvm/llvm_bitcode_link.cpp
    llvm/llvm_gen_backend.cpp
    llvm/llvm_passes.cpp
    llvm/llvm_scalarize.cpp
    llvm/llvm_legalize.cpp
    llvm/llvm_intrinsic_lowering.cpp
    llvm/llvm_barrier_nodup.cpp
    llvm/llvm_printf_parser.cpp
    llvm/llvm_to_gen.cpp
    llvm/llvm_loadstore_optimization.cpp
    llvm/llvm_gen_backend.hpp
    llvm/llvm_gen_ocl_function.hxx
    llvm/llvm_unroll.cpp
    llvm/llvm_to_gen.hpp
    backend/gen/gen_mesa_disasm.c
    backend/gen_insn_selection.cpp
    backend/gen_insn_selection.hpp
    backend/gen_insn_scheduling.cpp
    backend/gen_insn_scheduling.hpp
    backend/gen_reg_allocation.cpp
    backend/gen_reg_allocation.hpp
    backend/gen_context.cpp
    backend/gen_context.cpp
    backend/gen75_context.hpp
    backend/gen75_context.cpp
    backend/gen8_context.hpp
    backend/gen8_context.cpp
    backend/gen_program.cpp
    backend/gen_program.hpp
    backend/gen_program.h
    backend/gen7_instruction.hpp
    backend/gen8_instruction.hpp
    backend/gen_defs.hpp
    backend/gen_insn_compact.cpp
    backend/gen_encoder.hpp
    backend/gen_encoder.cpp
    backend/gen7_encoder.hpp
    backend/gen7_encoder.cpp
    backend/gen75_encoder.hpp
    backend/gen75_encoder.cpp
    backend/gen8_encoder.hpp
    backend/gen8_encoder.cpp
    )


include_directories (.)
link_directories (${LLVM_LIBRARY_DIRS} ${DRM_LIBDIR})
include_directories(${LLVM_INCLUDE_DIRS})
add_library (gbe SHARED ${GBE_SRC})


target_link_libraries(
                      gbe
                      ${DRM_INTEL_LIBRARIES}
                      ${DRM_LIBRARIES}
                      ${CLANG_LIBRARIES}
                      ${LLVM_MODULE_LIBS}
                      ${LLVM_SYSTEM_LIBS}
                      ${CMAKE_THREAD_LIBS_INIT}
                      ${CMAKE_DL_LIBS})

add_library(gbeinterp SHARED gbe_bin_interpreter.cpp)

add_dependencies(gbe beignet_bitcode)

if (LLVM_VERSION_NODOT VERSION_EQUAL 34)
  find_library(TERMINFO NAMES tinfo ncurses)
  if (${TERMINFO} STREQUAL TERMINFO-NOTFOUND)
    message(FATAL_ERROR "no libtinfo or libncurses is found in system")
  else (${TERMINFO} STREQUAL TERMINFO-NOTFOUND)
    target_link_libraries(gbe ${TERMINFO})
    message(STATUS "use ${TERMINFO} as terminal control library")
  endif (${TERMINFO} STREQUAL TERMINFO-NOTFOUND)
endif(LLVM_VERSION_NODOT VERSION_EQUAL 34)

link_directories (${LLVM_LIBRARY_DIR} ${DRM_LIBDIR})
ADD_EXECUTABLE(gbe_bin_generater gbe_bin_generater.cpp)
TARGET_LINK_LIBRARIES(gbe_bin_generater gbe)

install (TARGETS gbe LIBRARY DESTINATION ${BEIGNET_INSTALL_DIR})
install (TARGETS gbeinterp LIBRARY DESTINATION ${BEIGNET_INSTALL_DIR})
install (FILES ${OCL_OBJECT_DIR}/beignet.bc DESTINATION ${BEIGNET_INSTALL_DIR})
install (FILES ${OCL_OBJECT_DIR}/beignet.pch DESTINATION ${BEIGNET_INSTALL_DIR})
install (FILES ${OCL_HEADER_FILES} DESTINATION ${BEIGNET_INSTALL_DIR}/include)

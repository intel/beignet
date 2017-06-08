set (OCL_BITCODE_BIN "${BEIGNET_INSTALL_DIR}/beignet.bc")
set (OCL_HEADER_DIR "${BEIGNET_INSTALL_DIR}/include")
set (OCL_PCH_OBJECT "${BEIGNET_INSTALL_DIR}/beignet.pch")
set (GBE_OBJECT_DIR "${BEIGNET_INSTALL_DIR}/libgbe.so")
set (INTERP_OBJECT_DIR "${BEIGNET_INSTALL_DIR}/libgbeinterp.so")
if (ENABLE_OPENCL_20)
set (OCL_BITCODE_BIN_20 "${BEIGNET_INSTALL_DIR}/beignet_20.bc")
set (OCL_PCH_OBJECT_20 "${BEIGNET_INSTALL_DIR}/beignet_20.pch")
endif (ENABLE_OPENCL_20)

configure_file (
    "GBEConfig.h.in"
    "GBEConfig.h"
)

#do not involve libocl if the standalone compiler is given,
if (NOT (USE_STANDALONE_GBE_COMPILER STREQUAL "true"))
add_subdirectory(libocl)
endif ()

set (LOCAL_GBE_OBJECT_DIR "${CMAKE_CURRENT_BINARY_DIR}/libgbe.so" PARENT_SCOPE)
set (LOCAL_INTERP_OBJECT_DIR "${CMAKE_CURRENT_BINARY_DIR}/libgbeinterp.so" PARENT_SCOPE)
set (LOCAL_OCL_BITCODE_BIN "${OCL_OBJECT_DIR}/beignet.bc" PARENT_SCOPE)
set (LOCAL_OCL_HEADER_DIR "${OCL_OBJECT_DIR}/include/" PARENT_SCOPE)
set (LOCAL_OCL_PCH_OBJECT "${OCL_OBJECT_DIR}/beignet.local.pch" PARENT_SCOPE)
if (ENABLE_OPENCL_20)
set (LOCAL_OCL_BITCODE_BIN_20 "${OCL_OBJECT_DIR}/beignet_20.bc" PARENT_SCOPE)
set (LOCAL_OCL_PCH_OBJECT_20 "${OCL_OBJECT_DIR}/beignet_20.local.pch" PARENT_SCOPE)
endif (ENABLE_OPENCL_20)

set (GBE_SRC
    ${ocl_blob_file}
    sys/vector.hpp
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
    ir/half.cpp
    ir/half.hpp
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
    ir/constopt.cpp
    ir/constopt.hpp
    ir/profiling.cpp
    ir/profiling.hpp
    ir/printf.cpp
    ir/printf.hpp
    ir/immediate.hpp
    ir/immediate.cpp
    ir/structurizer.hpp
    ir/structurizer.cpp
    ir/reloc.hpp
    ir/reloc.cpp
    backend/context.cpp
    backend/context.hpp
    backend/program.cpp
    backend/program.hpp
    backend/program.h
    llvm/llvm_sampler_fix.cpp
    llvm/llvm_bitcode_link.cpp
    llvm/llvm_gen_backend.cpp
    llvm/llvm_passes.cpp
    llvm/llvm_scalarize.cpp
    llvm/llvm_intrinsic_lowering.cpp
    llvm/llvm_barrier_nodup.cpp
    llvm/llvm_printf_parser.cpp
    llvm/llvm_profiling.cpp
    llvm/ExpandConstantExpr.cpp
    llvm/ExpandUtils.cpp
    llvm/PromoteIntegers.cpp
    llvm/ExpandLargeIntegers.cpp
    llvm/llvm_device_enqueue.cpp
    llvm/StripAttributes.cpp
    llvm/llvm_to_gen.cpp
    llvm/llvm_loadstore_optimization.cpp
    llvm/llvm_gen_backend.hpp
    llvm/llvm_gen_ocl_function.hxx
    llvm/llvm_unroll.cpp
    llvm/llvm_to_gen.hpp
    backend/gen/gen_mesa_disasm.c
    backend/gen_insn_selection.cpp
    backend/gen_insn_selection.hpp
    backend/gen_insn_selection_optimize.cpp
    backend/gen_insn_selection_if_opt.cpp
    backend/gen_insn_scheduling.cpp
    backend/gen_insn_scheduling.hpp
    backend/gen_insn_selection_output.cpp
    backend/gen_insn_selection_output.hpp
    backend/gen_reg_allocation.cpp
    backend/gen_reg_allocation.hpp
    backend/gen_context.cpp
    backend/gen_context.cpp
    backend/gen75_context.hpp
    backend/gen75_context.cpp
    backend/gen8_context.hpp
    backend/gen8_context.cpp
    backend/gen9_context.hpp
    backend/gen9_context.cpp
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
    backend/gen9_encoder.hpp
    backend/gen9_encoder.cpp
    )

set (GBE_LINK_LIBRARIES
    ${DRM_INTEL_LIBRARIES}
    ${DRM_LIBRARIES}
    ${CLANG_LIBRARIES}
    ${LLVM_MODULE_LIBS}
    ${LLVM_SYSTEM_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
    )

include_directories (.)
link_directories (${LLVM_LIBRARY_DIRS} ${DRM_LIBDIR})
include_directories(${LLVM_INCLUDE_DIRS})

#do not build libgbe.so if the standalone compiler is given
if (NOT (USE_STANDALONE_GBE_COMPILER STREQUAL "true"))
add_library (gbe SHARED ${GBE_SRC})
target_link_libraries(gbe ${GBE_LINK_LIBRARIES})
add_dependencies(gbe beignet_bitcode)
endif (NOT (USE_STANDALONE_GBE_COMPILER STREQUAL "true"))

add_library(gbeinterp SHARED gbe_bin_interpreter.cpp)

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

#do not build nor install if the standalone compiler is given
if (NOT (USE_STANDALONE_GBE_COMPILER STREQUAL "true"))
if (BUILD_STANDALONE_GBE_COMPILER STREQUAL "true")
macro(remove_cxx_flag flag)
  string(REPLACE "${flag}" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endmacro()

remove_cxx_flag("-Wl,-E")
ADD_EXECUTABLE(gbe_bin_generater gbe_bin_generater.cpp ${GBE_SRC})
set_target_properties(gbe_bin_generater PROPERTIES LINK_FLAGS "-static")
TARGET_LINK_LIBRARIES(gbe_bin_generater ${GBE_LINK_LIBRARIES})

ADD_CUSTOM_TARGET(gbecompiler.tgz ALL
    COMMAND tar zcf ${OCL_OBJECT_DIR}/gbecompiler.tgz gbe_bin_generater -C ${OCL_OBJECT_DIR} beignet.bc -C ${OCL_OBJECT_DIR} beignet.pch -C ${OCL_OBJECT_DIR} include
    DEPENDS gbe_bin_generater beignet_bitcode
    )

else ()
ADD_EXECUTABLE(gbe_bin_generater gbe_bin_generater.cpp)
TARGET_LINK_LIBRARIES(gbe_bin_generater gbe)
endif ()

install (TARGETS gbe LIBRARY DESTINATION ${BEIGNET_INSTALL_DIR})
install (FILES ${OCL_OBJECT_DIR}/beignet.bc DESTINATION ${BEIGNET_INSTALL_DIR})
install (FILES ${OCL_OBJECT_DIR}/beignet.pch DESTINATION ${BEIGNET_INSTALL_DIR})
if (ENABLE_OPENCL_20)
install (FILES ${OCL_OBJECT_DIR}/beignet_20.bc DESTINATION ${BEIGNET_INSTALL_DIR})
install (FILES ${OCL_OBJECT_DIR}/beignet_20.pch DESTINATION ${BEIGNET_INSTALL_DIR})
endif (ENABLE_OPENCL_20)
install (FILES ${OCL_HEADER_FILES} DESTINATION ${BEIGNET_INSTALL_DIR}/include)
endif (NOT (USE_STANDALONE_GBE_COMPILER STREQUAL "true"))

install (TARGETS gbeinterp LIBRARY DESTINATION ${BEIGNET_INSTALL_DIR})

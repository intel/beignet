set (ocl_vector_spec_file ${GBE_SOURCE_DIR}/src/builtin_vector_proto.def)
set (ocl_vector_file ${GBE_SOURCE_DIR}/src/ocl_vector.h)
set (ocl_as_file ${GBE_SOURCE_DIR}/src/ocl_as.h)
set (ocl_convert_file ${GBE_SOURCE_DIR}/src/ocl_convert.h)
set (ocl_stdlib_tmpl_file ${GBE_SOURCE_DIR}/src/ocl_stdlib.tmpl.h)
set (ocl_common_header_file ${GBE_SOURCE_DIR}/src/ocl_common_defines.h)
set (ocl_blob_file ${CMAKE_CURRENT_BINARY_DIR}${BEIGNET_INSTALL_DIR}ocl_stdlib.h)
set (ocl_blob_cpp_file ${GBE_SOURCE_DIR}/src/ocl_stdlib_str.cpp)
set (ocl_gen_blob_cmd ${GBE_SOURCE_DIR}/src/update_blob_ocl_header.py)
set (ocl_gen_vector_cmd ${GBE_SOURCE_DIR}/src/gen_builtin_vector.py)

set (string_header "\\\"string\\\"")
add_custom_command(
    OUTPUT ${ocl_blob_cpp_file}
    COMMAND rm -rf ${ocl_blob_cpp_file}
    COMMAND echo "\\\#include ${string_header}" >> ${ocl_blob_cpp_file}
    COMMAND echo "namespace gbe {" >> ${ocl_blob_cpp_file}
    COMMAND echo "std::string ocl_stdlib_str = " >> ${ocl_blob_cpp_file}
    # Yeah!!! welcome to back slash hell
    COMMAND cat ${ocl_blob_file} |sed 's/\\\\/\\\\\\\\/g' | sed 's/\\\"/\\\\\\\"/g' | awk '{ printf \(\"\\"%s\\\\n\\"\\n\", $$0\) }' >> ${ocl_blob_cpp_file}
    COMMAND echo "\;" >> ${ocl_blob_cpp_file}
    COMMAND echo "}" >> ${ocl_blob_cpp_file}
    COMMAND echo "" >> ${ocl_blob_cpp_file}
    DEPENDS ${ocl_blob_file})

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "ocl_vector.h;${ocl_blob_file}")

add_custom_command(
  OUTPUT ${ocl_vector_file}
  COMMAND ${PYTHON_EXECUTABLE} ${ocl_gen_vector_cmd} ${ocl_vector_spec_file} ${ocl_vector_file}
  DEPENDS ${ocl_gen_vector_cmd} ${ocl_vector_spec_file}
  )

add_custom_command(
  OUTPUT ${ocl_blob_file}
  COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/${BEIGNET_INSTALL_DIR}
  COMMAND ${PYTHON_EXECUTABLE} ${ocl_gen_blob_cmd} ${ocl_stdlib_tmpl_file} ${ocl_blob_file}
  DEPENDS ${ocl_gen_blob_cmd} ${ocl_stdlib_tmpl_file} ${ocl_common_header_file} ${ocl_vector_file} ${ocl_as_file} ${ocl_convert_file}
  )

set (pch_object ${ocl_blob_file}.pch)
set (local_pch_object ${ocl_blob_file}.local.pch)
# generate pch object
if (LLVM_VERSION_NODOT VERSION_GREATER 32)
    set (clang_cmd -cc1 -x cl -triple spir -ffp-contract=off -cl-kernel-arg-info)
else (LLVM_VERSION_NODOT VERSION_GREATER 32)
    if (LLVM_VERSION_NODOT VERSION_GREATER 31)
        set (clang_cmd -cc1 -x cl -triple nvptx -ffp-contract=off)
    else (LLVM_VERSION_NODOT VERSION_GREATER 31)
        set (clang_cmd -cc1 -x cl -triple ptx32)
    endif (LLVM_VERSION_NODOT VERSION_GREATER 31)
endif (LLVM_VERSION_NODOT VERSION_GREATER 32)
set (clang_cmd ${clang_cmd} -cl-std=CL1.2 -fno-builtin -DGEN7_SAMPLER_CLAMP_BORDER_WORKAROUND)

add_custom_command(
     OUTPUT ${pch_object}
     COMMAND rm -f ${pch_object}
     COMMAND ${LLVM_INSTALL_DIR}clang ${clang_cmd} --relocatable-pch -emit-pch -isysroot ${CMAKE_CURRENT_BINARY_DIR} ${ocl_blob_file} -o ${pch_object}
     COMMAND ${LLVM_INSTALL_DIR}clang ${clang_cmd} -emit-pch ${ocl_blob_file} -o ${local_pch_object}
     DEPENDS ${ocl_blob_file}
     )

add_custom_target(pch_object
                  DEPENDS ${pch_object})

macro(ll_add_library ll_lib ll_sources)
  foreach (ll ${${ll_sources}})
  add_custom_command(
       OUTPUT  ${ll}.bc
       COMMAND rm -f ${ll}.bc
       COMMAND ${LLVM_INSTALL_DIR}llvm-as -o ${ll}.bc ${GBE_SOURCE_DIR}/src/${ll}
       DEPENDS ${ll}
       )
  set (ll_objects ${ll_objects} ${ll}.bc)
  endforeach (ll ${ll_sources})
  add_custom_command(
       OUTPUT ${ll_lib}
       COMMAND ${LLVM_INSTALL_DIR}llvm-link -o ${ll_lib} ${ll_objects}
       DEPENDS ${ll_objects}
       )
  add_custom_target(${ll_lib}
                    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${ll_lib})
endmacro(ll_add_library)

if (GBE_USE_BLOB)
  set (GBE_SRC
       blob.cpp
       backend/gen/gen_mesa_disasm.c)
else (GBE_USE_BLOB)
  set (GBE_SRC
    ${ocl_blob_file}
    ocl_stdlib_str.cpp  # this file is auto-generated.
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
    ir/immediate.hpp
    ir/immediate.cpp
    backend/context.cpp
    backend/context.hpp
    backend/program.cpp
    backend/program.hpp
    backend/program.h
    llvm/llvm_gen_backend.cpp
    llvm/llvm_passes.cpp
    llvm/llvm_scalarize.cpp
    llvm/llvm_intrinsic_lowering.cpp
    llvm/llvm_barrier_nodup.cpp
    llvm/llvm_printf_parser.cpp
    llvm/llvm_to_gen.cpp
    llvm/llvm_loadstore_optimization.cpp
    llvm/llvm_gen_backend.hpp
    llvm/llvm_gen_ocl_function.hxx
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
    backend/gen_program.cpp
    backend/gen_program.hpp
    backend/gen_program.h
    backend/gen_defs.hpp
    backend/gen_insn_compact.cpp
    backend/gen_encoder.hpp
    backend/gen_encoder.cpp
    backend/gen75_encoder.hpp
    backend/gen75_encoder.cpp
    )

endif (GBE_USE_BLOB)

include_directories (.)
link_directories (${LLVM_LIBRARY_DIRS} ${DRM_LIBDIR})
include_directories(${LLVM_INCLUDE_DIRS})
add_library (gbe SHARED ${GBE_SRC})

# for pre compiled module library.
set (pcm_lib "beignet.bc")
set (pcm_sources ocl_barrier.ll ocl_memset.ll ocl_memcpy.ll)
ll_add_library (${pcm_lib} pcm_sources)

ADD_DEPENDENCIES (gbe pch_object ${pcm_lib})
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
#install (FILES backend/program.h DESTINATION include/gen)
install (FILES ${ocl_blob_file} DESTINATION ${BEIGNET_INSTALL_DIR})
install (FILES ${pch_object} DESTINATION ${BEIGNET_INSTALL_DIR})
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${pcm_lib} DESTINATION ${BEIGNET_INSTALL_DIR})
# When build beignet itself, we need to export the local precompiled header file and precompiled module
# file to libcl and utests.
set (LOCAL_PCH_OBJECT_DIR "${local_pch_object}:${BEIGNET_INSTALL_DIR}/ocl_stdlib.h.pch" PARENT_SCOPE)
set (LOCAL_PCM_OBJECT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${pcm_lib}:${BEIGNET_INSTALL_DIR}/${pcm_lib}" PARENT_SCOPE)
set (LOCAL_GBE_OBJECT_DIR "${CMAKE_CURRENT_BINARY_DIR}/libgbe.so" PARENT_SCOPE)
set (LOCAL_INTERP_OBJECT_DIR "${CMAKE_CURRENT_BINARY_DIR}/libgbeinterp.so" PARENT_SCOPE)

set (PCH_OBJECT_DIR "${BEIGNET_INSTALL_DIR}/ocl_stdlib.h.pch")
set (PCM_OBJECT_DIR "${BEIGNET_INSTALL_DIR}/${pcm_lib}")
set (GBE_OBJECT_DIR "${BEIGNET_INSTALL_DIR}/libgbe.so")
set (INTERP_OBJECT_DIR "${BEIGNET_INSTALL_DIR}/libgbeinterp.so")
configure_file (
  "GBEConfig.h.in"
  "GBEConfig.h"
)

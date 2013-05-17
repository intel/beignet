#add_subdirectory(llvm)

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

set (TO_STRINGIFY_FILES ocl_stdlib ocl_common_defines)
stringify ("${GBE_SOURCE_DIR}/src/" "${TO_STRINGIFY_FILES}")

if (GBE_USE_BLOB)
  set (GBE_SRC
       blob.cpp
       backend/gen/gen_mesa_disasm.c)
else (GBE_USE_BLOB)
  set (GBE_SRC
    ocl_stdlib.h
    ocl_stdlib_str.cpp  # this file is auto-generated.
    ocl_stdlib_str.cpp
    ocl_common_defines.h
    ocl_common_defines_str.cpp # this file is auto-generated.
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
    backend/context.cpp
    backend/context.hpp
    backend/program.cpp
    backend/program.hpp
    backend/program.h
    llvm/llvm_gen_backend.cpp
    llvm/llvm_passes.cpp
    llvm/llvm_scalarize.cpp
    llvm/llvm_to_gen.cpp
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
    backend/gen_context.hpp
    backend/gen_program.cpp
    backend/gen_program.hpp
    backend/gen_program.h
    backend/gen_defs.hpp
    backend/gen_encoder.hpp
    backend/gen_encoder.cpp)

endif (GBE_USE_BLOB)

include_directories (.)
link_directories (${LLVM_LIBRARY_DIRS})
include_directories(${LLVM_INCLUDE_DIRS})
add_library (gbe SHARED ${GBE_SRC})

target_link_libraries(
                      gbe
                      ${DRM_INTEL_LIBRARY}
                      ${DRM_LIBRARY}
                      ${OPENGL_LIBRARIES}
                      ${LLVM_MODULE_LIBS}
                      ${CMAKE_THREAD_LIBS_INIT}
                      ${CMAKE_DL_LIBS})

install (TARGETS gbe LIBRARY DESTINATION lib)
install (FILES backend/program.h DESTINATION include/gen)


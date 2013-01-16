INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../include)

link_directories (${LLVM_LIBRARY_DIR})
ADD_LIBRARY(utests SHARED
  cl_create_kernel.cpp
  utest_error.c
compiler_shader_toy.cpp
  compiler_mandelbrot.cpp
  compiler_mandelbrot_alternate.cpp
  compiler_box_blur_float.cpp
  compiler_box_blur.cpp
  compiler_insert_to_constant.cpp
  compiler_argument_structure.cpp
  compiler_array0.cpp
  compiler_array.cpp
  compiler_array1.cpp
  compiler_array2.cpp
  compiler_array3.cpp
  compiler_byte_scatter.cpp
  compiler_copy_buffer.cpp
  compiler_copy_image.cpp
  compiler_copy_buffer_row.cpp
  compiler_function_argument0.cpp
  compiler_function_argument1.cpp
  compiler_function_argument.cpp
  compiler_if_else.cpp
  compiler_lower_return0.cpp
  compiler_lower_return1.cpp
  compiler_lower_return2.cpp
  compiler_multiple_kernels.cpp
  compiler_short_scatter.cpp
  compiler_sub_bytes.cpp
  compiler_sub_shorts.cpp
  compiler_uint2_copy.cpp
  compiler_uint3_copy.cpp
  compiler_uint8_copy.cpp
  compiler_uint16_copy.cpp
  compiler_uint3_unaligned_copy.cpp
  compiler_unstructured_branch0.cpp
  compiler_unstructured_branch1.cpp
  compiler_unstructured_branch2.cpp
  compiler_unstructured_branch3.cpp
  compiler_write_only_bytes.cpp
  compiler_write_only.cpp
  compiler_write_only_shorts.cpp
  compiler_switch.cpp
  compiler_math.cpp
  compiler_insn_selection_min.cpp
  compiler_insn_selection_max.cpp
  compiler_insn_selection_masked_min_max.cpp
  compiler_local_memory.cpp
  compiler_local_memory_two_ptr.cpp
  compiler_local_memory_barrier.cpp
  compiler_local_memory_barrier_wg64.cpp
  utest_assert.cpp
  utest.cpp
  utest_file_map.cpp
  utest_helper.cpp)

TARGET_LINK_LIBRARIES(utests cl m ${OPENGL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})


ADD_EXECUTABLE(utest_run utest_run.cpp)
TARGET_LINK_LIBRARIES(utest_run utests)

ADD_EXECUTABLE(flat_address_space runtime_flat_address_space.cpp)
TARGET_LINK_LIBRARIES(flat_address_space utests)


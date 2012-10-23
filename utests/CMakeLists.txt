INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../include)

link_directories (${LLVM_LIBRARY_DIRS})
ADD_LIBRARY(utests SHARED
  utest_error.c
  compiler_mandelbrot.cpp
  compiler_mandelbrot_alternate.cpp
  compiler_box_blur_float.cpp
  compiler_box_blur.cpp
  compiler_argument_structure.cpp
  compiler_array0.cpp
  compiler_array.cpp
  compiler_array1.cpp
  compiler_array2.cpp
  compiler_array3.cpp
  compiler_byte_scatter.cpp
  compiler_copy_buffer.cpp
  compiler_copy_buffer_row.cpp
  compiler_function_argument0.cpp
  compiler_function_argument1.cpp
  compiler_function_argument.cpp
  compiler_gather_register_file.cpp
  compiler_gather_register_file0.cpp
  compiler_gather_register_file1.cpp
  compiler_if_else.cpp
  compiler_lower_return0.cpp
  compiler_lower_return1.cpp
  compiler_lower_return2.cpp
  compiler_obread.cpp
  compiler_obwrite.cpp
  compiler_region.cpp
  compiler_region0.cpp
  compiler_region1.cpp
  compiler_short_scatter.cpp
  compiler_sub_bytes.cpp
  compiler_sub_shorts.cpp
  compiler_uint2_copy.cpp
  compiler_uint3_copy.cpp
  compiler_uint3_unaligned_copy.cpp
  compiler_unstructured_branch0.cpp
  compiler_unstructured_branch1.cpp
  compiler_unstructured_branch2.cpp
  compiler_unstructured_branch3.cpp
  compiler_vote_all.cpp
  compiler_vote_any.cpp
  compiler_write_only_bytes.cpp
  compiler_write_only.cpp
  compiler_write_only_shorts.cpp
  compiler_switch.cpp
  compiler_math.cpp
  utest_assert.cpp
  utest.cpp
  utest_file_map.cpp
  utest_helper.cpp)

TARGET_LINK_LIBRARIES(utests cl m)

ADD_EXECUTABLE(utest_run utest_run.cpp)
TARGET_LINK_LIBRARIES(run utests)

ADD_EXECUTABLE(flat_address_space runtime_flat_address_space.cpp)
TARGET_LINK_LIBRARIES(flat_address_space utests)


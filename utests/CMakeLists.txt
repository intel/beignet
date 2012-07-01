INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../include)

ADD_LIBRARY(utests SHARED
            utest_error.c
            utest_helper.cpp
            utest_file_map.cpp
            utest_assert.cpp
            utest.cpp
            app_mandelbrot.cpp
            compiler_write_only.cpp
            compiler_write_only_shorts.cpp
            compiler_write_only_bytes.cpp
            compiler_sub_bytes.cpp
            compiler_sub_shorts.cpp
            compiler_copy_buffer.cpp
            compiler_copy_buffer_row.cpp
            compiler_byte_scatter.cpp
            compiler_short_scatter.cpp
            compiler_uint2_copy.cpp
            compiler_uint3_copy.cpp
            compiler_uint3_unaligned_copy.cpp
            compiler_if_else.cpp
            compiler_unstructured_branch0.cpp
            compiler_unstructured_branch1.cpp
            compiler_unstructured_branch2.cpp
            compiler_unstructured_branch3.cpp
            compiler_lower_return0.cpp
            compiler_lower_return1.cpp
            compiler_lower_return2.cpp
# unsupported indirect load of kernel arguments
# compiler_argument_structure_indirect.cpp
            compiler_array.cpp
            compiler_array0.cpp
            compiler_function_argument.cpp
            compiler_function_argument0.cpp
            compiler_function_argument1.cpp
            compiler_argument_structure.cpp)
#compiler_local_slm.cpp)
TARGET_LINK_LIBRARIES(utests cl m)

ADD_EXECUTABLE(run utest_run.cpp)
TARGET_LINK_LIBRARIES(run utests)

ADD_EXECUTABLE(flat_address_space runtime_flat_address_space.cpp)
TARGET_LINK_LIBRARIES(flat_address_space utests)


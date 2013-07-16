INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../include)

link_directories (${LLVM_LIBRARY_DIR})
set (utests_sources
  cl_create_kernel.cpp
  utest_error.c
  compiler_basic_arithmetic.cpp
  compiler_displacement_map_element.cpp
  compiler_shader_toy.cpp
  compiler_mandelbrot.cpp
  compiler_mandelbrot_alternate.cpp
  compiler_box_blur_float.cpp
  compiler_box_blur_image.cpp
  compiler_box_blur.cpp
  compiler_insert_to_constant.cpp
  compiler_argument_structure.cpp
  compiler_arith_shift_right.cpp
  compiler_array0.cpp
  compiler_array.cpp
  compiler_array1.cpp
  compiler_array2.cpp
  compiler_array3.cpp
  compiler_byte_scatter.cpp
  compiler_ceil.cpp
  compiler_clz_short.cpp
  compiler_clz_int.cpp
  compiler_convert_uchar_sat.cpp
  compiler_copy_buffer.cpp
  compiler_copy_image.cpp
  compiler_copy_image_3d.cpp
  compiler_copy_buffer_row.cpp
  compiler_degrees.cpp
  compiler_step.cpp
  compiler_fabs.cpp
  compiler_abs.cpp
  compiler_abs_diff.cpp
  compiler_fill_image.cpp
  compiler_fill_image0.cpp
  compiler_fill_image_3d.cpp
  compiler_fill_image_3d_2.cpp
  compiler_function_argument0.cpp
  compiler_function_argument1.cpp
  compiler_function_argument.cpp
  compiler_function_constant0.cpp
  compiler_function_constant1.cpp
  compiler_function_constant.cpp
  compiler_global_constant.cpp
  compiler_global_constant_2.cpp
  compiler_group_size.cpp
  compiler_hadd.cpp
  compiler_if_else.cpp
  compiler_integer_division.cpp
  compiler_integer_remainder.cpp
  compiler_lower_return0.cpp
  compiler_lower_return1.cpp
  compiler_lower_return2.cpp
  compiler_mad_hi.cpp
  compiler_mul_hi.cpp
  compiler_mad24.cpp
  compiler_mul24.cpp
  compiler_multiple_kernels.cpp
  compiler_radians.cpp
  compiler_rhadd.cpp
  compiler_rotate.cpp
  compiler_saturate.cpp
  compiler_saturate_sub.cpp
  compiler_shift_right.cpp
  compiler_short_scatter.cpp
  compiler_smoothstep.cpp
  compiler_uint2_copy.cpp
  compiler_uint3_copy.cpp
  compiler_uint8_copy.cpp
  compiler_uint16_copy.cpp
  compiler_uint3_unaligned_copy.cpp
  compiler_upsample_int.cpp
  compiler_unstructured_branch0.cpp
  compiler_unstructured_branch1.cpp
  compiler_unstructured_branch2.cpp
  compiler_unstructured_branch3.cpp
  compiler_write_only_bytes.cpp
  compiler_write_only.cpp
  compiler_write_only_shorts.cpp
  compiler_switch.cpp
  compiler_math.cpp
  compiler_atomic_functions.cpp
  compiler_insn_selection_min.cpp
  compiler_insn_selection_max.cpp
  compiler_insn_selection_masked_min_max.cpp
  compiler_global_memory_barrier.cpp
  compiler_local_memory_two_ptr.cpp
  compiler_local_memory_barrier.cpp
  compiler_local_memory_barrier_wg64.cpp
  compiler_local_memory_barrier_2.cpp
  compiler_movforphi_undef.cpp
  compiler_volatile.cpp
  compiler_copy_image1.cpp
  compiler_get_image_info.cpp
  compiler_vect_compare.cpp
  compiler_vector_load_store.cpp
  compiler_cl_finish.cpp
  get_cl_info.cpp
  builtin_bitselect.cpp
  builtin_frexp.cpp
  builtin_mad_sat.cpp
  builtin_sign.cpp
  buildin_work_dim.cpp
  builtin_global_size.cpp
  runtime_createcontext.cpp
  runtime_null_kernel_arg.cpp
  compiler_double.cpp
  compiler_double_2.cpp
  compiler_double_3.cpp
  utest_assert.cpp
  utest.cpp
  utest_file_map.cpp
  utest_helper.cpp)

if (EGL_FOUND)
SET(utests_sources ${utests_sources} compiler_fill_gl_image.cpp)
SET(CMAKE_CXX_FLAGS "-DHAS_EGL ${CMAKE_CXX_FLAGS}")
SET(CMAKE_C_FLAGS "-DHAS_EGL ${CMAKE_C_FLAGS}")
endif (EGL_FOUND)

ADD_LIBRARY(utests SHARED ${utests_sources})

TARGET_LINK_LIBRARIES(utests cl m ${OPENGL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

ADD_EXECUTABLE(utest_run utest_run.cpp)
TARGET_LINK_LIBRARIES(utest_run utests)

ADD_EXECUTABLE(flat_address_space runtime_flat_address_space.cpp)
TARGET_LINK_LIBRARIES(flat_address_space utests)


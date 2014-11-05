INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../include)

##### Math Function Part:
EXEC_PROGRAM(mkdir ${CMAKE_CURRENT_SOURCE_DIR} ARGS generated -p)
EXEC_PROGRAM(${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} ARGS utest_math_gen.py OUTPUT_VARIABLE GEN_MATH_STRING)
string(REGEX REPLACE " " ";" ADDMATHFUNC ${GEN_MATH_STRING})

string(REGEX REPLACE "generated/([^\ ]*)\\.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/../kernels/\\1.cl" KERNEL_MATH_LIST ${GEN_MATH_STRING})
string(REGEX REPLACE " " ";" KERNEL_MATH_LIST ${KERNEL_MATH_LIST})
string(REGEX REPLACE "generated/([^\ ]*)\\.cpp" "\\1.cl" KERNEL_GITIGNORE_LIST ${GEN_MATH_STRING})
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "generated;${KERNEL_MATH_LIST}")

configure_file (
  "setenv.sh.in"
  "setenv.sh"
  )

#XXX only need GL if required
link_directories (${LLVM_LIBRARY_DIR} ${OPENGL_LIBDIR} ${DRM_LIBDIR})
set (utests_sources
  utest_error.c
  compiler_basic_arithmetic.cpp
  compiler_displacement_map_element.cpp
  compiler_mandelbrot.cpp
  compiler_mandelbrot_alternate.cpp
  compiler_box_blur_float.cpp
  compiler_box_blur_image.cpp
  compiler_box_blur.cpp
  compiler_insert_to_constant.cpp
  compiler_argument_structure.cpp
  compiler_arith_shift_right.cpp
  compiler_mixed_pointer.cpp
  compiler_array0.cpp
  compiler_array.cpp
  compiler_array1.cpp
  compiler_array2.cpp
  compiler_array3.cpp
  compiler_byte_scatter.cpp
  compiler_ceil.cpp
  compiler_clz_short.cpp
  compiler_clz_int.cpp
  compiler_popcount.cpp
  compiler_convert_uchar_sat.cpp
  compiler_copy_buffer.cpp
  compiler_copy_image.cpp
  compiler_copy_image_1d.cpp
  compiler_copy_image_3d.cpp
  compiler_copy_buffer_row.cpp
  compiler_degrees.cpp
  compiler_step.cpp
  compiler_fabs.cpp
  compiler_abs.cpp
  compiler_abs_diff.cpp
  compiler_fill_image.cpp
  compiler_fill_image0.cpp
  compiler_fill_image_1d.cpp
  compiler_fill_image_3d.cpp
  compiler_fill_image_3d_2.cpp
  compiler_function_argument0.cpp
  compiler_function_argument1.cpp
  compiler_function_argument2.cpp
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
  compiler_insert_vector.cpp
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
  compiler_upsample_long.cpp
  compiler_unstructured_branch0.cpp
  compiler_unstructured_branch1.cpp
  compiler_unstructured_branch2.cpp
  compiler_unstructured_branch3.cpp
  compiler_write_only_bytes.cpp
  compiler_write_only.cpp
  compiler_write_only_shorts.cpp
  compiler_switch.cpp
  compiler_overflow.cpp
  compiler_bswap.cpp
  compiler_math.cpp
  compiler_atomic_functions.cpp
  compiler_async_copy.cpp
  compiler_async_stride_copy.cpp
  compiler_insn_selection_min.cpp
  compiler_insn_selection_max.cpp
  compiler_insn_selection_masked_min_max.cpp
  compiler_load_bool_imm.cpp
  compiler_global_memory_barrier.cpp
  compiler_local_memory_two_ptr.cpp
  compiler_local_memory_barrier.cpp
  compiler_local_memory_barrier_wg64.cpp
  compiler_local_memory_barrier_2.cpp
  compiler_local_slm.cpp
  compiler_movforphi_undef.cpp
  compiler_volatile.cpp
  compiler_copy_image1.cpp
  compiler_get_image_info.cpp
  compiler_get_image_info_array.cpp
  compiler_vect_compare.cpp
  compiler_vector_load_store.cpp
  compiler_vector_inc.cpp
  compiler_cl_finish.cpp
  get_cl_info.cpp
  builtin_atan2.cpp
  builtin_bitselect.cpp
  builtin_frexp.cpp
  builtin_mad_sat.cpp
  builtin_modf.cpp
  builtin_nextafter.cpp
  builtin_remquo.cpp
  builtin_shuffle.cpp
  builtin_shuffle2.cpp
  builtin_sign.cpp
  builtin_lgamma.cpp
  builtin_lgamma_r.cpp
  builtin_tgamma.cpp
  buildin_work_dim.cpp
  builtin_global_size.cpp
  builtin_local_size.cpp
  builtin_global_id.cpp
  builtin_num_groups.cpp
  builtin_local_id.cpp
  builtin_acos_asin.cpp
  builtin_pow.cpp
  builtin_exp.cpp
  builtin_convert_sat.cpp
  sub_buffer.cpp
  runtime_createcontext.cpp
  runtime_set_kernel_arg.cpp
  runtime_null_kernel_arg.cpp
  runtime_event.cpp
  runtime_barrier_list.cpp
  runtime_marker_list.cpp
  runtime_compile_link.cpp
  compiler_long.cpp
  compiler_long_2.cpp
  compiler_long_convert.cpp
  compiler_long_shl.cpp
  compiler_long_shr.cpp
  compiler_long_asr.cpp
  compiler_long_mult.cpp
  compiler_long_cmp.cpp
  compiler_function_argument3.cpp
  compiler_function_qualifiers.cpp
  compiler_bool_cross_basic_block.cpp
  compiler_private_data_overflow.cpp
  compiler_getelementptr_bitcast.cpp
  compiler_simd_any.cpp
  compiler_simd_all.cpp
  compiler_time_stamp.cpp
  compiler_double_precision.cpp
  load_program_from_bin_file.cpp
  load_program_from_gen_bin.cpp
  get_arg_info.cpp
  profiling_exec.cpp
  enqueue_copy_buf.cpp
  enqueue_copy_buf_unaligned.cpp
  test_printf.cpp
  enqueue_fill_buf.cpp
  enqueue_built_in_kernels.cpp
  builtin_kernel_max_global_size.cpp
  image_1D_buffer.cpp
  compare_image_2d_and_1d_array.cpp
  compiler_fill_image_1d_array.cpp
  compiler_fill_image_2d_array.cpp
  compiler_constant_expr.cpp
  compiler_assignment_operation_in_if.cpp
  vload_bench.cpp
  runtime_use_host_ptr_buffer.cpp
  utest_assert.cpp
  utest.cpp
  utest_file_map.cpp
  utest_helper.cpp)

SET (kernel_bin ${CMAKE_CURRENT_SOURCE_DIR}/../kernels/compiler_ceil)

if(GEN_PCI_ID)
  ADD_CUSTOM_COMMAND(
  OUTPUT ${kernel_bin}.bin
  COMMAND ${GBE_BIN_GENERATER} ${kernel_bin}.cl -o${kernel_bin}.bin -t${GEN_PCI_ID}
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../backend/src/gbe_bin_generater ${kernel_bin}.cl)
else(GEN_PCI_ID)
  ADD_CUSTOM_COMMAND(
  OUTPUT ${kernel_bin}.bin
  COMMAND ${GBE_BIN_GENERATER} ${kernel_bin}.cl -o${kernel_bin}.bin
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../backend/src/gbe_bin_generater ${kernel_bin}.cl)
endif(GEN_PCI_ID)

if (DRM_INTEL_USERPTR)
SET(CMAKE_CXX_FLAGS "-DHAS_USERPTR ${CMAKE_CXX_FLAGS}")
SET(CMAKE_C_FLAGS "-DHAS_USERPTR ${CMAKE_C_FLAGS}")
endif (DRM_INTEL_USERPTR)

ADD_CUSTOM_TARGET(kernel_bin.bin
    DEPENDS ${kernel_bin}.bin)

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/generated
    COMMAND mkdir ${CMAKE_CURRENT_SOURCE_DIR}/generated -p
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/utest_math_gen.py > /dev/null 2>&1
    COMMAND echo ${KERNEL_GITIGNORE_LIST} |sed 's/ /\\n/g' > ${CMAKE_CURRENT_SOURCE_DIR}/../kernels/.gitignore
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
add_custom_target(utest_generator
    DEPENDS generated
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

if (EGL_FOUND AND MESA_SOURCE_FOUND)
SET(utests_sources ${utests_sources} compiler_fill_gl_image.cpp)
SET(CMAKE_CXX_FLAGS "-DHAS_EGL ${CMAKE_CXX_FLAGS} ${DEF_OCL_PCH_PCM_PATH}")
SET(CMAKE_C_FLAGS "-DHAS_EGL ${CMAKE_C_FLAGS} ${DEF_OCL_PCH_PCM_PATH}")
SET(UTESTS_REQUIRED_EGL_LIB ${EGL_LIBRARIES})
else()
SET(UTESTS_REQUIRED_EGL_LIB "")
endif()

if (COMPILER STREQUAL "CLANG")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-tautological-compare")
endif ()

ADD_LIBRARY(utests SHARED ${ADDMATHFUNC} ${utests_sources})

TARGET_LINK_LIBRARIES(utests cl m ${OPENGL_LIBRARIES} ${UTESTS_REQUIRED_EGL_LIB} ${CMAKE_THREAD_LIBS_INIT})

ADD_EXECUTABLE(utest_run utest_run.cpp)
TARGET_LINK_LIBRARIES(utest_run utests)
ADD_DEPENDENCIES (utest_run kernel_bin.bin)
ADD_DEPENDENCIES (utests utest_generator)

ADD_EXECUTABLE(flat_address_space runtime_flat_address_space.cpp)
TARGET_LINK_LIBRARIES(flat_address_space utests)

MY_LOCAL_PATH := $(call my-dir)
LOCAL_PATH:= $(MY_LOCAL_PATH)
include $(CLEAR_VARS)

include $(LOCAL_PATH)/../Android.common.mk

SUBDIR_C_INCLUDES := $(TOP_C_INCLUDE) $(LOCAL_PATH)/../include
SUBDIR_CPPFLAGS := $(TOP_CPPFLAGS)
SUBDIR_CPPFLAGS += -fexceptions -std=c++11
SUBDIR_LOCAL_CFLAGS := $(TOP_CFLAGS)
LOCAL_LDFLAGS := -Wl,-Bsymbolic

LOCAL_SRC_FILES:= \
  utest_error.c \
  utest_assert.cpp \
  utest.cpp \
  utest_file_map.cpp \
  utest_helper.cpp \
  compiler_basic_arithmetic.cpp \
  compiler_displacement_map_element.cpp \
  compiler_mandelbrot.cpp \
  compiler_mandelbrot_alternate.cpp \
  compiler_box_blur_float.cpp \
  compiler_box_blur_image.cpp \
  compiler_box_blur.cpp \
  compiler_insert_to_constant.cpp \
  compiler_argument_structure.cpp \
  compiler_argument_structure_indirect.cpp \
  compiler_argument_structure_select.cpp \
  compiler_arith_shift_right.cpp \
  compiler_mixed_pointer.cpp \
  compiler_array0.cpp \
  compiler_array.cpp \
  compiler_array1.cpp \
  compiler_array2.cpp \
  compiler_array3.cpp \
  compiler_array4.cpp \
  compiler_byte_scatter.cpp \
  compiler_ceil.cpp \
  compiler_popcount.cpp \
  compiler_convert_uchar_sat.cpp \
  compiler_copy_buffer.cpp \
  compiler_copy_image.cpp \
  compiler_copy_image_1d.cpp \
  compiler_copy_image_3d.cpp \
  compiler_copy_buffer_row.cpp \
  compiler_degrees.cpp \
  compiler_step.cpp \
  compiler_fabs.cpp \
  compiler_abs.cpp \
  compiler_abs_diff.cpp \
  compiler_fill_image.cpp \
  compiler_fill_image0.cpp \
  compiler_fill_image_1d.cpp \
  compiler_fill_image_3d.cpp \
  compiler_fill_image_3d_2.cpp \
  compiler_function_argument0.cpp \
  compiler_function_argument1.cpp \
  compiler_function_argument2.cpp \
  compiler_function_argument.cpp \
  compiler_function_constant0.cpp \
  compiler_function_constant1.cpp \
  compiler_function_constant.cpp \
  compiler_global_constant.cpp \
  compiler_global_constant_2.cpp \
  compiler_group_size.cpp \
  compiler_hadd.cpp \
  compiler_if_else.cpp \
  compiler_integer_division.cpp \
  compiler_integer_remainder.cpp \
  compiler_insert_vector.cpp \
  compiler_lower_return0.cpp \
  compiler_lower_return1.cpp \
  compiler_lower_return2.cpp \
  compiler_mad_hi.cpp \
  compiler_mul_hi.cpp \
  compiler_mad24.cpp \
  compiler_mul24.cpp \
  compiler_multiple_kernels.cpp \
  compiler_radians.cpp \
  compiler_rhadd.cpp \
  compiler_rotate.cpp \
  compiler_saturate.cpp \
  compiler_saturate_sub.cpp \
  compiler_shift_right.cpp \
  compiler_short_scatter.cpp \
  compiler_smoothstep.cpp \
  compiler_uint2_copy.cpp \
  compiler_uint3_copy.cpp \
  compiler_uint8_copy.cpp \
  compiler_uint16_copy.cpp \
  compiler_uint3_unaligned_copy.cpp \
  compiler_upsample_int.cpp \
  compiler_upsample_long.cpp \
  compiler_unstructured_branch0.cpp \
  compiler_unstructured_branch1.cpp \
  compiler_unstructured_branch2.cpp \
  compiler_unstructured_branch3.cpp \
  compiler_write_only_bytes.cpp \
  compiler_write_only.cpp \
  compiler_write_only_shorts.cpp \
  compiler_switch.cpp \
  compiler_bswap.cpp \
  compiler_clz.cpp \
  compiler_math.cpp \
  compiler_atomic_functions.cpp \
  compiler_async_copy.cpp \
  compiler_async_stride_copy.cpp \
  compiler_insn_selection_min.cpp \
  compiler_insn_selection_max.cpp \
  compiler_insn_selection_masked_min_max.cpp \
  compiler_load_bool_imm.cpp \
  compiler_global_memory_barrier.cpp \
  compiler_local_memory_two_ptr.cpp \
  compiler_local_memory_barrier.cpp \
  compiler_local_memory_barrier_wg64.cpp \
  compiler_local_memory_barrier_2.cpp \
  compiler_local_slm.cpp \
  compiler_movforphi_undef.cpp \
  compiler_volatile.cpp \
  compiler_copy_image1.cpp \
  compiler_get_image_info.cpp \
  compiler_get_image_info_array.cpp \
  compiler_vect_compare.cpp \
  compiler_vector_load_store.cpp \
  compiler_vector_inc.cpp \
  compiler_cl_finish.cpp \
  get_cl_info.cpp \
  builtin_atan2.cpp \
  builtin_bitselect.cpp \
  builtin_frexp.cpp \
  builtin_mad_sat.cpp \
  builtin_modf.cpp \
  builtin_nextafter.cpp \
  builtin_remquo.cpp \
  builtin_shuffle.cpp \
  builtin_shuffle2.cpp \
  builtin_sign.cpp \
  builtin_lgamma.cpp \
  builtin_lgamma_r.cpp \
  builtin_tgamma.cpp \
  buildin_work_dim.cpp \
  builtin_global_size.cpp \
  builtin_local_size.cpp \
  builtin_global_id.cpp \
  builtin_num_groups.cpp \
  builtin_local_id.cpp \
  builtin_acos_asin.cpp \
  builtin_pow.cpp \
  builtin_convert_sat.cpp \
  sub_buffer.cpp \
  runtime_createcontext.cpp \
  runtime_set_kernel_arg.cpp \
  runtime_null_kernel_arg.cpp \
  runtime_event.cpp \
  runtime_barrier_list.cpp \
  runtime_marker_list.cpp \
  runtime_compile_link.cpp \
  compiler_long.cpp \
  compiler_long_2.cpp \
  compiler_long_not.cpp \
  compiler_long_hi_sat.cpp \
  compiler_long_div.cpp \
  compiler_long_convert.cpp \
  compiler_long_shl.cpp \
  compiler_long_shr.cpp \
  compiler_long_asr.cpp \
  compiler_long_mult.cpp \
  compiler_long_cmp.cpp \
  compiler_long_bitcast.cpp \
  compiler_half.cpp \
  compiler_function_argument3.cpp \
  compiler_function_qualifiers.cpp \
  compiler_bool_cross_basic_block.cpp \
  compiler_private_const.cpp \
  compiler_private_data_overflow.cpp \
  compiler_getelementptr_bitcast.cpp \
  compiler_time_stamp.cpp \
  compiler_double_precision.cpp \
  load_program_from_gen_bin.cpp \
  load_program_from_spir.cpp \
  get_arg_info.cpp \
  profiling_exec.cpp \
  enqueue_copy_buf.cpp \
  enqueue_copy_buf_unaligned.cpp \
  test_printf.cpp \
  enqueue_fill_buf.cpp \
  builtin_kernel_max_global_size.cpp \
  image_1D_buffer.cpp \
  image_from_buffer.cpp \
  compare_image_2d_and_1d_array.cpp \
  compiler_fill_image_1d_array.cpp \
  compiler_fill_image_2d_array.cpp \
  compiler_constant_expr.cpp \
  compiler_assignment_operation_in_if.cpp \
  vload_bench.cpp \
  runtime_use_host_ptr_buffer.cpp \
  runtime_alloc_host_ptr_buffer.cpp \
  runtime_use_host_ptr_image.cpp \
  compiler_get_max_sub_group_size.cpp \
  compiler_get_sub_group_local_id.cpp \
  compiler_sub_group_shuffle.cpp

ifeq ($(EGL_FOUND),true)
LOCAL_SRC_FILES += \
    compiler_fill_gl_image.cpp
SUBDIR_CPPFLAGS += -DHAS_EGL
SUBDIR_CFLAGS += -DHAS_EGL
endif

LOCAL_SHARED_LIBRARIES := \
libcl \
libm \
libdl

LOCAL_C_INCLUDES := $(SUBDIR_C_INCLUDES)
LOCAL_CPPFLAGS := $(SUBDIR_CPPFLAGS)
LOCAL_CFLAGS := $(SURDIR_CFLAGS)
LOCAL_MODULE := libutests

#LOCAL_CLANG := true
include external/libcxx/libcxx.mk
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := utest_run
LOCAL_SRC_FILES:= utest_run.cpp

LOCAL_SHARED_LIBRARIES := \
libutests \
libm \
libdl

LOCAL_C_INCLUDES := $(SUBDIR_C_INCLUDES)
LOCAL_CPPFLAGS := $(SUBDIR_CPPFLAGS)
LOCAL_CFLAGS := $(SURDIR_CFLAGS)

LOCAL_MULTILIB := both
LOCAL_MODULE_STEM_32 := utest_run-x86
LOCAL_MODULE_STEM_64 := utest_run-x86_64


#LOCAL_CLANG := true
include external/libcxx/libcxx.mk
include $(BUILD_EXECUTABLE)


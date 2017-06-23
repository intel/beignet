ADD_EXECUTABLE(gen_bin_generater kernels/gen_bin_generater.c)
ADD_DEPENDENCIES(gen_bin_generater gbe)
set_target_properties(gen_bin_generater PROPERTIES COMPILE_DEFINITIONS COMPILER_BACKEND_OBJECT="${LOCAL_GBE_OBJECT_DIR}")
TARGET_LINK_LIBRARIES(gen_bin_generater dl ${ELF_LIB_LIBRARIES})
set(GEN_BIN_GENERATER
	env OCL_BITCODE_LIB_PATH=${LOCAL_OCL_BITCODE_BIN} OCL_HEADER_FILE_DIR=${LOCAL_OCL_HEADER_DIR} OCL_PCH_PATH=${LOCAL_OCL_PCH_OBJECT}
	OCL_BITCODE_LIB_20_PATH=${LOCAL_OCL_BITCODE_BIN_20} OCL_PCH_20_PATH=${LOCAL_OCL_PCH_OBJECT_20})
set(GEN_BIN_GENERATER ${GEN_BIN_GENERATER} ${CMAKE_CURRENT_BINARY_DIR}/gen_bin_generater)

macro (MakeKernelBinStr KERNEL_DIST KERNEL_SOURCE KERNEL_FILES)
	foreach (KF ${KERNEL_FILES})
		set (input_file ${KERNEL_SOURCE}/${KF}.cl)
		set (output_file ${KERNEL_DIST}/${KF}_str.c)
		list (APPEND KERNEL_STR_FILES ${output_file})
		if(GEN_PCI_ID)
			add_custom_command(
				OUTPUT ${output_file}
				COMMAND rm -rf ${output_file}
				COMMAND ${GEN_BIN_GENERATER} -o${output_file} -p${GEN_PCI_ID} -i${input_file}
				DEPENDS ${input_file})
		else(GEN_PCI_ID)
			add_custom_command(
				OUTPUT ${output_file}
				COMMAND rm -rf ${output_file}
				COMMAND ${GEN_BIN_GENERATER} -o${output_file} -i${input_file}
				DEPENDS ${input_file})
		endif(GEN_PCI_ID)
	endforeach (KF)
endmacro (MakeKernelBinStr)

macro (MakeBuiltInKernelStr KERNEL_PATH KERNEL_FILES)
	set (output_file ${KERNEL_PATH}/${BUILT_IN_NAME}.cl)
	set (file_content)
	file (REMOVE ${output_file})
	foreach (KF ${KERNEL_NAMES})
		set (input_file ${CMAKE_CURRENT_SOURCE_DIR}/kernels/${KF}.cl)
		file(READ ${input_file} file_content )
		STRING(REGEX REPLACE ";" "\\\\;" file_content "${file_content}")
		file(APPEND ${output_file} ${file_content})
	endforeach (KF)
endmacro (MakeBuiltInKernelStr)

set (KERNEL_STR_FILES)
set (KERNEL_NAMES cl_internal_copy_buf_align4
	cl_internal_copy_buf_align16 cl_internal_copy_buf_unalign_same_offset
	cl_internal_copy_buf_unalign_dst_offset cl_internal_copy_buf_unalign_src_offset
	cl_internal_copy_buf_rect cl_internal_copy_buf_rect_align4
	cl_internal_copy_image_1d_to_1d cl_internal_copy_image_2d_to_2d
	cl_internal_copy_image_3d_to_2d cl_internal_copy_image_2d_to_3d cl_internal_copy_image_3d_to_3d
	cl_internal_copy_image_2d_to_2d_array cl_internal_copy_image_1d_array_to_1d_array
	cl_internal_copy_image_2d_array_to_2d_array cl_internal_copy_image_2d_array_to_2d
	cl_internal_copy_image_2d_array_to_3d cl_internal_copy_image_3d_to_2d_array
	cl_internal_copy_image_2d_to_buffer cl_internal_copy_image_2d_to_buffer_align16 cl_internal_copy_image_3d_to_buffer
	cl_internal_copy_buffer_to_image_2d cl_internal_copy_buffer_to_image_2d_align16 cl_internal_copy_buffer_to_image_3d
	cl_internal_fill_buf_align8 cl_internal_fill_buf_align4
	cl_internal_fill_buf_align2 cl_internal_fill_buf_unalign
	cl_internal_fill_buf_align128 cl_internal_fill_image_1d
	cl_internal_fill_image_1d_array cl_internal_fill_image_2d
	cl_internal_fill_image_2d_array cl_internal_fill_image_3d
	cl_internal_block_motion_estimate_intel)
set(BUILT_IN_NAME  cl_internal_built_in_kernel)
MakeBuiltInKernelStr ("${CMAKE_CURRENT_BINARY_DIR}/kernels/" "${KERNEL_NAMES}")
MakeKernelBinStr ("${CMAKE_CURRENT_BINARY_DIR}/kernels/" "${CMAKE_CURRENT_SOURCE_DIR}/kernels/" "${KERNEL_NAMES}")
MakeKernelBinStr ("${CMAKE_CURRENT_BINARY_DIR}/kernels/" "${CMAKE_CURRENT_BINARY_DIR}/kernels/" "${BUILT_IN_NAME}")


set(OPENCL_GEN_SRC
	${KERNEL_STR_FILES}
	intel_batchbuffer.c
	intel_driver.c
	cl_device_id_gen.c
	cl_kernel_gen.c
	cl_program_gen.c
	cl_context_gen.c
	cl_command_queue_gen.c
	cl_mem_gen.c
	cl_image_gen.c
	cl_compiler_gen.c
	cl_event_gen.c
	cl_sampler_gen.c
	cl_printf_gen.c
	)

if (X11_FOUND)
	set(CMAKE_CXX_FLAGS "-DHAS_X11 ${CMAKE_CXX_FLAGS}")
	set(CMAKE_C_FLAGS "-DHAS_X11 ${CMAKE_C_FLAGS}")
	set(OPENCL_GEN_SRC
		${OPENCL_GEN_SRC}
		x11/dricommon.c
		x11/va_dri2.c)
endif (X11_FOUND)

if (HAVE_DRM_INTEL_USERPTR)
	SET(CMAKE_CXX_FLAGS "-DHAS_USERPTR ${CMAKE_CXX_FLAGS}")
	SET(CMAKE_C_FLAGS "-DHAS_USERPTR ${CMAKE_C_FLAGS}")
endif (HAVE_DRM_INTEL_USERPTR)

if (HAVE_DRM_INTEL_EU_TOTAL)
	SET(CMAKE_CXX_FLAGS "-DHAS_EU_TOTAL ${CMAKE_CXX_FLAGS}")
	SET(CMAKE_C_FLAGS "-DHAS_EU_TOTAL ${CMAKE_C_FLAGS}")
endif (HAVE_DRM_INTEL_EU_TOTAL)

if (HAVE_DRM_INTEL_SUBSLICE_TOTAL)
	SET(CMAKE_CXX_FLAGS "-DHAS_SUBSLICE_TOTAL ${CMAKE_CXX_FLAGS}")
	SET(CMAKE_C_FLAGS "-DHAS_SUBSLICE_TOTAL ${CMAKE_C_FLAGS}")
endif (HAVE_DRM_INTEL_SUBSLICE_TOTAL)

if (HAVE_DRM_INTEL_POOLED_EU)
	SET(CMAKE_CXX_FLAGS "-DHAS_POOLED_EU ${CMAKE_CXX_FLAGS}")
	SET(CMAKE_C_FLAGS "-DHAS_POOLED_EU ${CMAKE_C_FLAGS}")
endif (HAVE_DRM_INTEL_POOLED_EU)

if (HAVE_DRM_INTEL_MIN_EU_IN_POOL)
	SET(CMAKE_CXX_FLAGS "-DHAS_MIN_EU_IN_POOL ${CMAKE_CXX_FLAGS}")
	SET(CMAKE_C_FLAGS "-DHAS_MIN_EU_IN_POOL ${CMAKE_C_FLAGS}")
endif (HAVE_DRM_INTEL_MIN_EU_IN_POOL)

if (HAVE_DRM_INTEL_BO_SET_SOFTPIN)
	SET(CMAKE_CXX_FLAGS "-DHAS_BO_SET_SOFTPIN ${CMAKE_CXX_FLAGS}")
	SET(CMAKE_C_FLAGS "-DHAS_BO_SET_SOFTPIN ${CMAKE_C_FLAGS}")
endif (HAVE_DRM_INTEL_BO_SET_SOFTPIN)

add_library(cl_gen STATIC ${OPENCL_GEN_SRC})
set_target_properties(cl_gen PROPERTIES COMPILE_DEFINITIONS COMPILER_BACKEND_OBJECT="${GBE_OBJECT_FULL_PATH}")
ADD_DEPENDENCIES(cl_gen gen_bin_generater)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${DRM_INCLUDE_PATH}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../backend/src/backend/
                    ${CMAKE_CURRENT_SOURCE_DIR}/../include
                    ${MESA_SOURCE_INCLUDES})

macro (MakeKernelBinStr KERNEL_PATH KERNEL_FILES)
foreach (KF ${KERNEL_FILES})
  set (input_file ${KERNEL_PATH}/${KF}.cl)
  set (output_file ${KERNEL_PATH}/${KF}_str.c)
  list (APPEND KERNEL_STR_FILES ${output_file})
  add_custom_command(
    OUTPUT ${output_file}
    COMMAND rm -rf ${output_file}
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../backend/src/gbe_bin_generater -s ${input_file} -o${output_file}
    DEPENDS ${input_file} ${CMAKE_CURRENT_BINARY_DIR}/../backend/src/gbe_bin_generater)
endforeach (KF)
endmacro (MakeKernelBinStr)

set (KERNEL_STR_FILES)
set (KERNEL_NAMES cl_internal_copy_buf_align1 cl_internal_copy_buf_align4 cl_internal_copy_buf_align16)
MakeKernelBinStr ("${CMAKE_CURRENT_SOURCE_DIR}/kernels/" "${KERNEL_NAMES}")

set(OPENCL_SRC
    ${KERNEL_STR_FILES}
    cl_api.c
    cl_alloc.c
    cl_kernel.c
    cl_program.c
    cl_sampler.c
    cl_event.c
    cl_enqueue.c
    cl_image.c
    cl_mem.c
    cl_platform_id.c
    cl_extensions.c
    cl_device_id.c
    cl_context.c
    cl_command_queue.c
    cl_command_queue.h
    cl_command_queue_gen7.c
    cl_thread.c
    cl_driver.h
    cl_driver.cpp
    cl_driver_defs.c
    intel/intel_gpgpu.c
    intel/intel_batchbuffer.c
    intel/intel_driver.c
    x11/dricommon.c 
    x11/va_dri2.c)

if (EGL_FOUND AND MESA_SOURCE_FOUND)
set (OPENCL_SRC ${OPENCL_SRC} cl_mem_gl.c cl_gl_api.c x11/mesa_egl_extension.c x11/mesa_egl_res_share.c intel/intel_dri_resource_sharing.c)
SET(CMAKE_CXX_FLAGS "-DHAS_EGL ${CMAKE_CXX_FLAGS}")
SET(CMAKE_C_FLAGS "-DHAS_EGL ${CMAKE_C_FLAGS}")
SET(OPTIONAL_EGL_LIBRARY "${EGL_LIBRARY}")
else(EGL_FOUND AND MESA_SOURCE_FOUND)
SET(OPTIONAL_EGL_LIBRARY "")
endif (EGL_FOUND AND MESA_SOURCE_FOUND)

if (OCLIcd_FOUND)
set (OPENCL_SRC ${OPENCL_SRC} cl_khr_icd.c)
SET(CMAKE_CXX_FLAGS "-DHAS_OCLIcd ${CMAKE_CXX_FLAGS}")
SET(CMAKE_C_FLAGS "-DHAS_OCLIcd ${CMAKE_C_FLAGS}")
endif (OCLIcd_FOUND)

SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic,--allow-shlib-undefined")

link_directories (${LLVM_LIBRARY_DIR})
add_library(cl SHARED ${OPENCL_SRC})
target_link_libraries(
                      cl
                      gbe
                      ${XLIB_LIBRARY}
                      ${XEXT_LIBRARY}
                      ${XFIXES_LIBRARY}
                      ${DRM_INTEL_LIBRARY}
                      ${DRM_LIBRARY}
                      ${OPENGL_LIBRARIES}
                      ${OPTIONAL_EGL_LIBRARY})
install (TARGETS cl LIBRARY DESTINATION lib)

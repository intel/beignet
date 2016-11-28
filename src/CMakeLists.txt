include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${DRM_INCLUDE_DIRS}
                    ${DRM_INCLUDE_DIRS}/../
                    ${CMAKE_CURRENT_SOURCE_DIR}/../backend/src/backend/
                    ${CMAKE_CURRENT_SOURCE_DIR}/../include
                    ${LLVM_INCLUDE_DIR}
                    ${OPENGL_INCLUDE_DIRS}
                    ${EGL_INCLUDE_DIRS})

macro (MakeKernelBinStr KERNEL_PATH KERNEL_FILES)
foreach (KF ${KERNEL_FILES})
  set (input_file ${KERNEL_PATH}/${KF}.cl)
  set (output_file ${KERNEL_PATH}/${KF}_str.c)
  list (APPEND KERNEL_STR_FILES ${output_file})
  list (GET GBE_BIN_GENERATER -1 GBE_BIN_FILE)
  if(GEN_PCI_ID)
    add_custom_command(
      OUTPUT ${output_file}
      COMMAND rm -rf ${output_file}
      COMMAND ${GBE_BIN_GENERATER} -s -o${output_file} -t${GEN_PCI_ID} ${input_file}
      DEPENDS ${input_file} ${GBE_BIN_FILE})
  else(GEN_PCI_ID)
    add_custom_command(
      OUTPUT ${output_file}
      COMMAND rm -rf ${output_file}
      COMMAND ${GBE_BIN_GENERATER} -s -o${output_file} ${input_file}
      DEPENDS ${input_file} ${GBE_BIN_FILE})
  endif(GEN_PCI_ID)
endforeach (KF)
endmacro (MakeKernelBinStr)

macro (MakeBuiltInKernelStr KERNEL_PATH KERNEL_FILES)
  set (output_file ${KERNEL_PATH}/${BUILT_IN_NAME}.cl)
  set (file_content)
  file (REMOVE ${output_file})
  foreach (KF ${KERNEL_NAMES})
    set (input_file ${KERNEL_PATH}/${KF}.cl)
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
set (BUILT_IN_NAME  cl_internal_built_in_kernel)
MakeBuiltInKernelStr ("${CMAKE_CURRENT_SOURCE_DIR}/kernels/" "${KERNEL_NAMES}")
MakeKernelBinStr ("${CMAKE_CURRENT_SOURCE_DIR}/kernels/" "${KERNEL_NAMES}")
MakeKernelBinStr ("${CMAKE_CURRENT_SOURCE_DIR}/kernels/" "${BUILT_IN_NAME}")

set(OPENCL_SRC
    ${KERNEL_STR_FILES}
    cl_base_object.c
    cl_api.c
    cl_api_platform_id.c
    cl_api_device_id.c
    cl_api_mem.c
    cl_api_kernel.c
    cl_api_command_queue.c
    cl_api_event.c
    cl_api_context.c
    cl_api_sampler.c
    cl_api_program.c
    cl_alloc.c
    cl_kernel.c
    cl_program.c
    cl_gbe_loader.cpp
    cl_sampler.c
    cl_accelerator_intel.c
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
    cl_device_enqueue.c
    cl_device_enqueue.h
    cl_command_queue_gen7.c
    cl_command_queue_enqueue.c
    cl_utils.c
    cl_driver.h
    cl_driver.cpp
    cl_driver_defs.c
    intel/intel_gpgpu.c
    intel/intel_batchbuffer.c
    intel/intel_driver.c
    performance.c)

if (X11_FOUND)
  set(CMAKE_CXX_FLAGS "-DHAS_X11 ${CMAKE_CXX_FLAGS}")
  set(CMAKE_C_FLAGS "-DHAS_X11 ${CMAKE_C_FLAGS}")
  set(OPENCL_SRC
      ${OPENCL_SRC}
      x11/dricommon.c
      x11/va_dri2.c)
endif (X11_FOUND)

if (CMRT_FOUND)
  set(CMAKE_CXX_FLAGS "-DHAS_CMRT ${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "-DCMRT_PATH=${CMRT_LIBDIR}/libcmrt.so.1 ${CMAKE_CXX_FLAGS}")
  set(CMAKE_C_FLAGS "-DHAS_CMRT ${CMAKE_C_FLAGS}")
  set(OPENCL_SRC ${OPENCL_SRC} cl_cmrt.cpp)
endif (CMRT_FOUND)

if (OPENGL_FOUND AND EGL_FOUND)
  set (OPENCL_SRC ${OPENCL_SRC} cl_mem_gl.c cl_gl_api.c )
  SET(CMAKE_CXX_FLAGS "-DHAS_GL_EGL ${CMAKE_CXX_FLAGS}")
  SET(CMAKE_C_FLAGS "-DHAS_GL_EGL ${CMAKE_C_FLAGS}")
endif (OPENGL_FOUND AND EGL_FOUND)

if (OCLIcd_FOUND)
  set (OPENCL_SRC ${OPENCL_SRC} cl_khr_icd.c)
  SET(CMAKE_CXX_FLAGS "-DHAS_OCLIcd ${CMAKE_CXX_FLAGS}")
  SET(CMAKE_C_FLAGS "-DHAS_OCLIcd ${CMAKE_C_FLAGS}")
endif (OCLIcd_FOUND)

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

set(GIT_SHA1 "git_sha1.h")
add_custom_target(${GIT_SHA1} ALL
  COMMAND chmod +x ${CMAKE_CURRENT_SOURCE_DIR}/git_sha1.sh
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/git_sha1.sh ${CMAKE_CURRENT_SOURCE_DIR} ${GIT_SHA1}
)

SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic,--allow-shlib-undefined")

link_directories (${LLVM_LIBRARY_DIR} ${DRM_LIBDIR} ${OPENGL_LIBDIR} ${EGL_LIBDIR})
add_library(cl SHARED ${OPENCL_SRC})
ADD_DEPENDENCIES(cl ${GIT_SHA1})
target_link_libraries(
                      cl
                      rt
                      ${X11_LIBRARIES}
                      ${XEXT_LIBRARIES}
                      ${XFIXES_LIBRARIES}
                      ${DRM_INTEL_LIBRARIES}
                      ${DRM_LIBRARIES}
                      ${CMAKE_THREAD_LIBS_INIT}
                      ${CMAKE_DL_LIBS}
                      ${OPENGL_LIBRARIES}
                      ${EGL_LIBRARIES})
install (TARGETS cl LIBRARY DESTINATION ${BEIGNET_INSTALL_DIR})

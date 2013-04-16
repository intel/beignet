include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${DRM_INCLUDE_PATH}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../backend/src/backend/
                    ${CMAKE_CURRENT_SOURCE_DIR}/../include)

set(OPENCL_SRC
    cl_api.c
    cl_alloc.c
    cl_kernel.c
    cl_program.c
    cl_sampler.c
    cl_event.c
    cl_image.c
    cl_mem.c
    cl_platform_id.c
    cl_extensions.c
    cl_device_id.c
    cl_context.c
    cl_command_queue.c
    cl_command_queue.h
    cl_command_queue_gen7.c
    cl_driver.h
    cl_driver.cpp
    cl_driver_defs.c
    intel/intel_gpgpu.c
    intel/intel_batchbuffer.c
    intel/intel_driver.c
    x11/dricommon.c 
    x11/va_dri2.c)

if (EGL_FOUND)
set (OPENCL_SRC ${OPENCL_SRC} cl_mem_gl.c cl_gl_api.c x11/gbm_dri2_x11_platform.c)
SET(CMAKE_CXX_FLAGS "-DHAS_EGL ${CMAKE_CXX_FLAGS}")
SET(CMAKE_C_FLAGS "-DHAS_EGL ${CMAKE_C_FLAGS}")
SET(OPTIONAL_EGL_LIBRARY "${EGL_LIBRARY}")
else(EGL_FOUND)
SET(OPTIONAL_EGL_LIBRARY "")
endif (EGL_FOUND)

if (OCLIcd_FOUND)
set (OPENCL_SRC ${OPENCL_SRC} cl_khr_icd.c)
SET(CMAKE_CXX_FLAGS "-DHAS_OCLIcd ${CMAKE_CXX_FLAGS}")
SET(CMAKE_C_FLAGS "-DHAS_OCLIcd ${CMAKE_C_FLAGS}")
endif (OCLIcd_FOUND)

SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic")

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
                      ${OPTIONAL_EGL_LIBRARY}
                      ${GBM_LIBRARY})
install (TARGETS cl LIBRARY DESTINATION lib)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${DRM_INCLUDE_PATH}
                    ${GBE_INCLUDE_PATH}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../include)

SET(OPENCL_SRC
    cl_api.c
    cl_alloc.c
    cl_kernel.c
    cl_program.c
    cl_sampler.c
    cl_event.c
    cl_image.c
    cl_mem.c
    cl_platform_id.c
    cl_device_id.c
    cl_context.c
    cl_command_queue.c
    cl_command_queue.h
    cl_command_queue_gen7.c
    cl_driver.c
    cl_driver.h
    cl_driver.cpp
    sim/sim_driver.c
    sim/sim_driver.h
    sim/sim_simulator.c
    sim/sim_simulator.h
    intel/intel_gpgpu.c
    intel/intel_batchbuffer.c
    intel/intel_driver.c
    x11/dricommon.c 
    x11/va_dri2.c)

ADD_LIBRARY(cl SHARED ${OPENCL_SRC})
TARGET_LINK_LIBRARIES(cl
                      ${GBE_LIBRARY}
                      ${DRM_LIBRARY}
                      ${DRM_INTEL_LIBRARY}
                      ${XLIB_LIBRARY}
                      ${XEXT_LIBRARY}
                      ${XFIXES_LIBRARY}
                      ${DRM_LIBRARY})


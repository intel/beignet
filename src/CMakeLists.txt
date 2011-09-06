INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${DRM_INCLUDE_PATH}
  ${CMAKE_CURRENT_SOURCE_DIR}/../include)

SET(OPENCL_SRC
    cl_api.c
    cl_alloc.c
    cl_kernel.c
    cl_program.c
    cl_event.c
    cl_mem.c
    cl_platform_id.c
    cl_device_id.c
    cl_context.c
    cl_command_queue.c
    cl_command_queue_gen6.c
    cl_command_queue_gen7.c
    intel/intel_gpgpu.c
    intel/intel_batchbuffer.c
    intel/intel_driver.c
    x11/dricommon.c 
    x11/va_dri2.c)

ADD_LIBRARY(cl SHARED ${OPENCL_SRC})
TARGET_LINK_LIBRARIES(cl
                      ${DRM_LIBRARY}
                      ${DRM_INTEL_LIBRARY}
                      ${XLIB_LIBRARY}
                      ${XEXT_LIBRARY}
                      ${XFIXES_LIBRARY}
                      ${DRM_LIBRARY})

ADD_LIBRARY(cl_test STATIC
            tests/common.c
            tests/cl_test.c
            tests/cl_file_map.c)
TARGET_LINK_LIBRARIES(cl_test cl)

ADD_EXECUTABLE(test_copy_buffer tests/test_copy_buffer.c)
ADD_EXECUTABLE(test_enqueue_read tests/test_enqueue_read.c)
ADD_EXECUTABLE(test_imm_parameters tests/test_imm_parameters.c)
ADD_EXECUTABLE(test_2d_copy tests/test_2d_copy.c)
ADD_EXECUTABLE(test_barrier tests/test_barrier.c)
ADD_EXECUTABLE(test_static_local_memory tests/test_static_local_memory.c)
ADD_EXECUTABLE(test_local_memory tests/test_local_memory.c)
ADD_EXECUTABLE(test_private_memory tests/test_private_memory.c)
ADD_EXECUTABLE(test_constant_memory tests/test_constant_memory.c)
ADD_EXECUTABLE(test_memory_leak tests/test_memory_leak.c)
ADD_EXECUTABLE(test_perf_report tests/test_perf_report.c)
#ADD_EXECUTABLE(test_trigo tests/test_trigo.c)
ADD_EXECUTABLE(mersenneTwister tests/mersenneTwister.c)
ADD_EXECUTABLE(blackscholes tests/blackscholes.c)
ADD_EXECUTABLE(matmul tests/matmul.c)
ADD_EXECUTABLE(urng tests/urng.c)
ADD_EXECUTABLE(fastWalsh tests/fastWalsh.c)
ADD_EXECUTABLE(fft tests/fft.c)
ADD_EXECUTABLE(dct tests/dct.c)
ADD_EXECUTABLE(binomialOption tests/binomialOption.c)
ADD_EXECUTABLE(nbody tests/nbody.c)
ADD_EXECUTABLE(svm_test tests/svm_test.c)
TARGET_LINK_LIBRARIES(test_copy_buffer cl_test m)
TARGET_LINK_LIBRARIES(test_enqueue_read cl_test m)
TARGET_LINK_LIBRARIES(test_imm_parameters cl_test m)
TARGET_LINK_LIBRARIES(test_2d_copy cl_test m)
TARGET_LINK_LIBRARIES(test_barrier cl_test m)
TARGET_LINK_LIBRARIES(test_static_local_memory cl_test m)
TARGET_LINK_LIBRARIES(test_local_memory cl_test m)
TARGET_LINK_LIBRARIES(test_private_memory cl_test m)
TARGET_LINK_LIBRARIES(test_constant_memory cl_test m)
TARGET_LINK_LIBRARIES(test_memory_leak cl_test m)
TARGET_LINK_LIBRARIES(test_perf_report cl_test m)
#TARGET_LINK_LIBRARIES(test_trigo cl_test m)
TARGET_LINK_LIBRARIES(mersenneTwister cl_test m)
TARGET_LINK_LIBRARIES(blackscholes cl_test m)
TARGET_LINK_LIBRARIES(matmul cl_test m)
TARGET_LINK_LIBRARIES(urng cl_test m)
TARGET_LINK_LIBRARIES(fastWalsh cl_test m)
TARGET_LINK_LIBRARIES(fft cl_test m)
TARGET_LINK_LIBRARIES(dct cl_test m)
TARGET_LINK_LIBRARIES(binomialOption cl_test m)
TARGET_LINK_LIBRARIES(nbody cl_test m)
TARGET_LINK_LIBRARIES(svm_test cl_test m)


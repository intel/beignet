INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utests
                    ${CMAKE_CURRENT_SOURCE_DIR}/../include)


link_directories (${LLVM_LIBRARY_DIR} ${DRM_LIBDIR})
set (benchmark_sources
  ../utests/utest_error.c
  ../utests/utest_assert.cpp
  ../utests/utest.cpp
  ../utests/utest_file_map.cpp
  ../utests/utest_helper.cpp
  ../utests/vload_bench.cpp
  benchmark_copy_buf.cpp
  benchmark_use_host_ptr_buffer.cpp
  benchmark_read_buffer.cpp
  benchmark_read_image.cpp
  benchmark_copy_buffer_to_image.cpp
  benchmark_copy_image_to_buffer.cpp
  benchmark_copy_buffer.cpp
  benchmark_copy_image.cpp
  benchmark_workgroup.cpp
  benchmark_math.cpp)


SET(CMAKE_CXX_FLAGS "-DBUILD_BENCHMARK ${CMAKE_CXX_FLAGS}")
SET(CMAKE_C_FLAGS "-DBUILD_BENCHMARK ${CMAKE_C_FLAGS}")

ADD_LIBRARY(benchmarks SHARED ${ADDMATHFUNC} ${benchmark_sources})

#TARGET_LINK_LIBRARIES(benchmarks cl m ${OPENGL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
TARGET_LINK_LIBRARIES(benchmarks cl m)

ADD_EXECUTABLE(benchmark_run benchmark_run.cpp)
TARGET_LINK_LIBRARIES(benchmark_run benchmarks)
ADD_CUSTOM_TARGET(benchmark DEPENDS benchmarks benchmark_run)

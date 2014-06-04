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
  enqueue_copy_buf.cpp)

ADD_LIBRARY(benchmarks SHARED ${ADDMATHFUNC} ${benchmark_sources})

#TARGET_LINK_LIBRARIES(benchmarks cl m ${OPENGL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
TARGET_LINK_LIBRARIES(benchmarks cl m)

ADD_EXECUTABLE(benchmark_run benchmark_run.cpp)
TARGET_LINK_LIBRARIES(benchmark_run benchmarks)

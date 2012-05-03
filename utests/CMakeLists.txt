INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../include)

ADD_EXECUTABLE(run
               utest_error.c
               utest_helper.cpp
               utest_file_map.cpp
               utest_assert.cpp
               utest.cpp
               utest_run.cpp
               compiler_write_only.cpp
               compiler_copy_buffer.cpp
               compiler_copy_buffer_row.cpp)
TARGET_LINK_LIBRARIES(run cl m)


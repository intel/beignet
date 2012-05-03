INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../include)

ADD_LIBRARY(utest SHARED
            utest_error.c
            utest_helper.cpp
            utest_file_map.cpp
            utest_assert.cpp
            utest.cpp)
ADD_EXECUTABLE(compiler_write_only compiler_write_only.cpp)
TARGET_LINK_LIBRARIES(compiler_write_only cl m utest)


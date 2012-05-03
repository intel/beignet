INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../include)

ADD_LIBRARY(cl_test SHARED
            cl_test.c
            cl_file_map.c
            common.c
            assert.cpp
            utest.cpp)
ADD_EXECUTABLE(write_only write_only.c)
TARGET_LINK_LIBRARIES(write_only cl m cl_test)


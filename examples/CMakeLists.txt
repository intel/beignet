EXECUTE_PROCESS(COMMAND ls "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libva" OUTPUT_VARIABLE LS_RESULT)
IF ("LS_RESULT" STREQUAL "")
EXECUTE_PROCESS(COMMAND git submodule init WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)
EXECUTE_PROCESS(COMMAND git submodule update WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)
EXECUTE_PROCESS(COMMAND git checkout master WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libva)
ENDIF ("LS_RESULT" STREQUAL "")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utests
                    ${CMAKE_CURRENT_SOURCE_DIR}/../include
                    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libva/va
                    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libva/test/common
                    ${X11_INCLUDE_DIR})

link_directories (${LIBVA_LIBDIR}
                  ${LIBVA-X11_LIBDIR})

set (examples_sources
  ../utests/utest_error.c
  ../utests/utest_assert.cpp
  ../utests/utest_file_map.cpp
  ../utests/utest_helper.cpp
  ./thirdparty/libva/test/common/va_display.c
  ./thirdparty/libva/test/common/va_display_x11.c)


ADD_DEFINITIONS(-DHAVE_VA_X11)
ADD_DEFINITIONS(-DINPUT_NV12_DEFAULT="${CMAKE_CURRENT_SOURCE_DIR}/libva_buffer_sharing/256_128.nv12")

ADD_LIBRARY(va_ocl_basic SHARED ${examples_sources})

TARGET_LINK_LIBRARIES(va_ocl_basic cl m va va-x11 ${X11_X11_LIB})

ADD_EXECUTABLE(example-libva_buffer_sharing ./libva_buffer_sharing/libva_buffer_sharing.cpp)
TARGET_LINK_LIBRARIES(example-libva_buffer_sharing va_ocl_basic)

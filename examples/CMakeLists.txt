INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utests
                    ${CMAKE_CURRENT_SOURCE_DIR}/../include
                    ${X11_INCLUDE_DIR}
                    ${OPENGL_INCLUDE_DIRS}
                    ${EGL_INCLUDE_DIRS})

set (ocl_example_helper_sources
  ../utests/utest_error.c
  ../utests/utest_assert.cpp
  ../utests/utest_file_map.cpp
  ../utests/utest_helper.cpp)

IF(OPENGL_FOUND AND EGL_FOUND AND X11_FOUND)
  ADD_DEFINITIONS(-DHAS_GL_EGL_X11)
ENDIF(OPENGL_FOUND AND EGL_FOUND AND X11_FOUND)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations" )

ADD_LIBRARY(ocl_example_helper SHARED ${ocl_example_helper_sources})

TARGET_LINK_LIBRARIES(ocl_example_helper cl m ${X11_LIBRARIES} ${OPENGL_LIBRARIES} ${EGL_LIBRARIES})

IF(LIBVA_BUF_SH_DEP OR V4L2_BUF_SH_DEP)
  EXECUTE_PROCESS(COMMAND ls "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libva" OUTPUT_VARIABLE LS_RESULT)
  IF ("LS_RESULT" STREQUAL "")
    EXECUTE_PROCESS(COMMAND git submodule init WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)
    EXECUTE_PROCESS(COMMAND git submodule update WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)
    EXECUTE_PROCESS(COMMAND git checkout master WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libva)
  ENDIF ("LS_RESULT" STREQUAL "")

  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libva/va
                      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libva/test/common)

  link_directories (${LIBVA_LIBDIR}
                    ${LIBVA-X11_LIBDIR})

  set (va_display_sources
    ./thirdparty/libva/test/common/va_display.c
    ./thirdparty/libva/test/common/va_display_x11.c)

  ADD_DEFINITIONS(-DHAVE_VA_X11)

  ADD_LIBRARY(va_display SHARED ${va_display_sources})

  TARGET_LINK_LIBRARIES(va_display cl m va va-x11 ${X11_LIBRARIES})

  IF(LIBVA_BUF_SH_DEP)
    ADD_DEFINITIONS(-DINPUT_NV12_DEFAULT="${CMAKE_CURRENT_SOURCE_DIR}/libva_buffer_sharing/256_128.nv12")
    ADD_EXECUTABLE(example-libva_buffer_sharing ./libva_buffer_sharing/libva_buffer_sharing.cpp)
    TARGET_LINK_LIBRARIES(example-libva_buffer_sharing ocl_example_helper va_display)
  ENDIF(LIBVA_BUF_SH_DEP)

  IF(V4L2_BUF_SH_DEP)
    ADD_EXECUTABLE(example-v4l2_buffer_sharing ./v4l2_buffer_sharing/v4l2_buffer_sharing.cpp)
    TARGET_LINK_LIBRARIES(example-v4l2_buffer_sharing ocl_example_helper va_display)
  ENDIF(V4L2_BUF_SH_DEP)
ENDIF(LIBVA_BUF_SH_DEP OR V4L2_BUF_SH_DEP)

IF(OPENGL_FOUND AND EGL_FOUND AND X11_FOUND)
  ADD_EXECUTABLE(example-gl_buffer_sharing ./gl_buffer_sharing/gl_buffer_sharing.cpp)
  TARGET_LINK_LIBRARIES(example-gl_buffer_sharing ocl_example_helper ${OPENGL_LIBRARIES} ${EGL_LIBRARIES})
ENDIF(OPENGL_FOUND AND EGL_FOUND AND X11_FOUND)

#
# Try to find EGL library and include path.
# Once done this will define
#
# EGL_FOUND
# EGL_INCLUDE_PATH
# EGL_LIBRARY
#

FIND_PATH(EGL_INCLUDE_PATH EGL/egl.h
  ~/include/
  /usr/include/
  /usr/local/include/
  /sw/include/
  /opt/local/include/
  DOC "The directory where gen/program.h resides")
FIND_LIBRARY(EGL_LIBRARY
  NAMES EGL egl
  PATHS
  ~/lib/
  /usr/lib64
  /usr/lib
  /usr/local/lib64
  /usr/local/lib
  /sw/lib
  /opt/local/lib
  DOC "The EGL library")

IF(EGL_INCLUDE_PATH)
  SET(EGL_FOUND 1 CACHE STRING "Set to 1 if EGL is found, 0 otherwise")
ELSE(EGL_INCLUDE_PATH)
  SET(EGL_FOUND 0 CACHE STRING "Set to 1 if EGL is found, 0 otherwise")
ENDIF(EGL_INCLUDE_PATH)

MARK_AS_ADVANCED(EGL_FOUND)

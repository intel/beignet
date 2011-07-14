#
# Try to find X library and include path.
# Once done this will define
#
# XLIB_FOUND
# XLIB_INCLUDE_PATH
# XLIB_LIBRARY
# 

FIND_PATH(XLIB_INCLUDE_PATH X11/Xlib.h
  /usr/include
  /usr/local/include
  /sw/include
  /opt/local/include
  DOC "The directory where X11/Xlib.h resides")
FIND_LIBRARY(XLIB_LIBRARY
  NAMES XLIB X11
  PATHS
  /usr/lib64
  /usr/lib
  /usr/local/lib64
  /usr/local/lib
  /sw/lib
  /opt/local/lib
  DOC "The XLIB library")

IF(XLIB_INCLUDE_PATH)
  SET(XLIB_FOUND 1 CACHE STRING "Set to 1 if XLIB is found, 0 otherwise")
ELSE(XLIB_INCLUDE_PATH)
  SET(XLIB_FOUND 0 CACHE STRING "Set to 1 if XLIB is found, 0 otherwise")
ENDIF(XLIB_INCLUDE_PATH)

MARK_AS_ADVANCED(XLIB_FOUND)


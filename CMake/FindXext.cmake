#
# Try to find Xext library path.
# Once done this will define
#
# XEXT_FOUND
# XEXT_LIBRARY
# 

FIND_PATH(XEXT_INCLUDE_PATH X11/extensions/Xext.h
  /usr/include
  /usr/local/include
  /sw/include
  /opt/local/include
  DOC "The directory where Xext.h resides")

FIND_LIBRARY(XEXT_LIBRARY
  NAMES XEXT Xext
  PATHS
  /usr/lib64
  /usr/lib
  /usr/local/lib64
  /usr/local/lib
  /sw/lib
  /opt/local/lib
  DOC "The XEXT library")

IF(XEXT_INCLUDE_PATH)
  INCLUDE_DIRECTORIES(${XEXT_INCLUDE_PATH})
  SET(XEXT_FOUND 1 CACHE STRING "Set to 1 if XEXT is found, 0 otherwise")
ELSE(XEXT_INCLUDE_PATH)
  SET(XEXT_FOUND 0 CACHE STRING "Set to 1 if XEXT is found, 0 otherwise")
ENDIF(XEXT_INCLUDE_PATH)

MARK_AS_ADVANCED(XEXT_FOUND)


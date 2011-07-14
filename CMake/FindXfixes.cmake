#
# Try to find Xfixes library path.
# Once done this will define
#
# XFIXES_FOUND
# XFIXES_LIBRARY
# 

FIND_LIBRARY(XFIXES_LIBRARY
  NAMES XFIXES Xfixes
  PATHS
  /usr/lib64
  /usr/lib
  /usr/local/lib64
  /usr/local/lib
  /sw/lib
  /opt/local/lib
  DOC "The XFIXES library")

IF(XFIXES_INCLUDE_PATH)
  SET(XFIXES_FOUND 1 CACHE STRING "Set to 1 if XFIXES is found, 0 otherwise")
ELSE(XFIXES_INCLUDE_PATH)
  SET(XFIXES_FOUND 0 CACHE STRING "Set to 1 if XFIXES is found, 0 otherwise")
ENDIF(XFIXES_INCLUDE_PATH)

MARK_AS_ADVANCED(XFIXES_FOUND)


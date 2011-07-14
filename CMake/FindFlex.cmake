#
# Try to find X library and include path.
# Once done this will define
#
# FLEX_FOUND
# FLEX_LIBRARY
# 

FIND_LIBRARY(FLEX_LIBRARY
  NAMES FLEX fl
  PATHS
  /usr/lib64
  /usr/lib
  /usr/local/lib64
  /usr/local/lib
  /sw/lib
  /opt/local/lib
  DOC "The FLEX library")

IF(FLEX_INCLUDE_PATH)
  SET(FLEX_FOUND 1 CACHE STRING "Set to 1 if FLEX is found, 0 otherwise")
ELSE(FLEX_INCLUDE_PATH)
  SET(FLEX_FOUND 0 CACHE STRING "Set to 1 if FLEX is found, 0 otherwise")
ENDIF(FLEX_INCLUDE_PATH)

MARK_AS_ADVANCED(FLEX_FOUND)


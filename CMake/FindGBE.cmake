#
# Try to find X library and include path.
# Once done this will define
#
# GBE_FOUND
# GBE_INCLUDE_PATH
# GBE_LIBRARY
# 

FIND_PATH(GBE_INCLUDE_PATH gen/program.h
  ~/include/
  /usr/include/
  /usr/local/include/
  /sw/include/
  /opt/local/include/
  DOC "The directory where gen/program.h resides")
FIND_LIBRARY(GBE_LIBRARY
  NAMES GBE gbe
  PATHS
  ~/lib/
  /usr/lib64
  /usr/lib
  /usr/local/lib64
  /usr/local/lib
  /sw/lib
  /opt/local/lib
  DOC "The GBE library")

IF(GBE_INCLUDE_PATH)
  SET(GBE_FOUND 1 CACHE STRING "Set to 1 if GBE is found, 0 otherwise")
ELSE(GBE_INCLUDE_PATH)
  SET(GBE_FOUND 0 CACHE STRING "Set to 1 if GBE is found, 0 otherwise")
ENDIF(GBE_INCLUDE_PATH)

MARK_AS_ADVANCED(GBE_FOUND)


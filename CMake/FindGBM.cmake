#
# Try to find gbm library and include path.
# Once done this will define
#
# GBM_FOUND
# GBM_INCLUDE_PATH
# GBM_LIBRARY
#

FIND_PATH(GBM_INCLUDE_PATH gbm.h
  ~/include/
  /usr/include/
  /usr/local/include/
  /sw/include/
  /opt/local/include/
  DOC "The directory where gen/program.h resides")
FIND_LIBRARY(GBM_LIBRARY
  NAMES GBM gbm
  PATHS
  ~/lib/
  /usr/lib64
  /usr/lib
  /usr/local/lib64
  /usr/local/lib
  /sw/lib
  /opt/local/lib
  DOC "The GBM library")

IF(GBM_INCLUDE_PATH)
  SET(GBM_FOUND 1 CACHE STRING "Set to 1 if GBM is found, 0 otherwise")
ELSE(GBM_INCLUDE_PATH)
  SET(GBM_FOUND 0 CACHE STRING "Set to 1 if GBM is found, 0 otherwise")
ENDIF(GBM_INCLUDE_PATH)

MARK_AS_ADVANCED(GBM_FOUND)

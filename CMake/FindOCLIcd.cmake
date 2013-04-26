#
# Try to find ocl_icd library and include path.
# Once done this will define
#
# OCLIcd_FOUND
# OCLIcd_INCLUDE_PATH
#

FIND_PATH(OCLIcd_INCLUDE_PATH ocl_icd.h
  ~/include/
  /usr/include/
  /usr/local/include/
  /sw/include/
  /opt/local/include/
  DOC "The directory where ocl_icd.h resides")

IF(OCLIcd_INCLUDE_PATH)
  INCLUDE_DIRECTORIES(${OCLIcd_INCLUDE_PATH})
  SET(OCLIcd_FOUND 1 CACHE STRING "Set to 1 if OCLIcd is found, 0 otherwise")
ELSE(OCLIcd_INCLUDE_PATH)
  SET(OCLIcd_FOUND 0 CACHE STRING "Set to 1 if OCLIcd is found, 0 otherwise")
ENDIF(OCLIcd_INCLUDE_PATH)

MARK_AS_ADVANCED(OCLIcd_FOUND)

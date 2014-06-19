#
# Try to find mesa source code
# Once done this will define
#
# MESA_SOURCE_FOUND
# MESA_SOURCE_INCLUDES
#

# Find mesa source code.
FIND_PATH(MESA_SOURCE_PREFIX src/mesa/main/texobj.c
  $ENV{MESA_SOURCE_DIR}
  ${MAKE_CURRENT_SOURCE_DIR}/../mesa
  ~/mesa
  DOC "The mesa source directory which is needed for cl_khr_gl_sharing.")

IF(MESA_SOURCE_PREFIX)
SET(MESA_SOURCE_INCLUDES ${MESA_SOURCE_PREFIX}/src/mesa
                         ${MESA_SOURCE_PREFIX}/include
                         ${MESA_SOURCE_PREFIX}/src/mapi
                         ${MESA_SOURCE_PREFIX}/src/mesa/drivers/dri/i965/
                         ${MESA_SOURCE_PREFIX}/src/mesa/drivers/dri/i915/
                         ${MESA_SOURCE_PREFIX}/src/mesa/drivers/dri/common/)
SET(MESA_SOURCE_FOUND 1 CACHE STRING "Set to 1 if mesa source code is found, 0 otherwise")
ELSE(MESA_SOURCE_PREFIX)
SET(MESA_SOURCE_FOUND 0 CACHE STRING "Set to 1 if mesa source code is found, 0 otherwise")
ENDIF(MESA_SOURCE_PREFIX)

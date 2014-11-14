# compiler choose,now support ICC,GCC CLANG compiler
set (COMPILER "GCC" CACHE INT "Compiler to choose on Linux (GCC,ICC,CLANG)")
if (COMPILER STREQUAL "GCC")
elseif (COMPILER STREQUAL "CLANG")
  set (CMAKE_C_COMPILER   "clang")
  set (CMAKE_CXX_COMPILER "clang++")
  find_program(CMAKE_AR NAMES llvm-ar)
  find_program(CMAKE_LINKER NAMES llvm-ld)
elseif (COMPILER STREQUAL "ICC")
  find_program(CMAKE_C_COMPILER NAMES icc)
  find_program(CMAKE_CXX_COMPILER NAMES icpc)
  find_program(CMAKE_AR NAMES xiar)
  find_program(CMAKE_LINKER NAMES xild)
endif ()

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)
PROJECT(OCL)
set (LIBCL_DRIVER_VERSION_MAJOR 1)
set (LIBCL_DRIVER_VERSION_MINOR 0)
set (LIBCL_DRIVER_VERSION_PATCH 0)
set (LIBCL_C_VERSION_MAJOR 1)
set (LIBCL_C_VERSION_MINOR 2)

configure_file (
  "src/OCLConfig.h.in"
  "src/OCLConfig.h"
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE (FindPkgConfig)

SET(CMAKE_VERBOSE_MAKEFILE "false")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/")
if (NOT LIB_INSTALL_DIR)
  set (LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif (NOT LIB_INSTALL_DIR)
if (NOT BEIGNET_INSTALL_DIR)
  set (BEIGNET_INSTALL_DIR "${LIB_INSTALL_DIR}/beignet/")
endif (NOT BEIGNET_INSTALL_DIR)
SET(EMULATE_IVB false CACHE BOOL "To emulate IVB")
SET(EMULATE_SNB false CACHE BOOL "To emulate SNB")
SET(EMULATE_HSW false CACHE BOOL "To emulate HSW")
ADD_DEFINITIONS(-D__$(USER)__)

# Force Release with debug info
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE RelWithDebInfo)
endif (NOT CMAKE_BUILD_TYPE)
set (CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "assure config" FORCE)
message(STATUS "Building mode: " ${CMAKE_BUILD_TYPE})

IF (EMULATE_HSW)
  SET (USE_FULSIM "true")
  ADD_DEFINITIONS(-DEMULATE_GEN=75)
ELSEIF (EMULATE_IVB)
  SET (USE_FULSIM "true")
  ADD_DEFINITIONS(-DEMULATE_GEN=7)
ELSEIF (EMULATE_SNB)
  SET (USE_FULSIM "true")
  ADD_DEFINITIONS(-DEMULATE_GEN=6)
ELSE (EMULATE_IVB)
  SET (USE_FULSIM "false")
  ADD_DEFINITIONS(-DEMULATE_GEN=0)
ENDIF (EMULATE_HSW)

# XXX now hard coded to enable the clamp to border workaround for IVB.
ADD_DEFINITIONS(-DGEN7_SAMPLER_CLAMP_BORDER_WORKAROUND)

IF (USE_FULSIM)
  ADD_DEFINITIONS(-DUSE_FULSIM=1)
ELSE (USE_FULSIM)
  ADD_DEFINITIONS(-DUSE_FULSIM=0)
ENDIF (USE_FULSIM)

# compiler flag setting
if (COMPILER STREQUAL "GCC")
  set (CMAKE_C_CXX_FLAGS "${CMAKE_C_CXX_FLAGS} -funroll-loops -fstrict-aliasing -msse2 -msse3 -mssse3 -msse4.1 -fPIC -Wall -mfpmath=sse -Wcast-align -Wl,-E")
elseif (COMPILER STREQUAL "CLANG")
  set (CMAKE_C_CXX_FLAGS "${CMAKE_C_CXX_FLAGS} -funroll-loops -fstrict-aliasing -msse2 -msse3 -mssse3 -msse4.1 -fPIC -Wall")
elseif (COMPILER STREQUAL "ICC")
  set (CMAKE_C_CXX_FLAGS "${CMAKE_C_CXX_FLAGS}  -wd2928 -Wall -fPIC -fstrict-aliasing -fp-model fast -msse4.1 -Wl,-E")
endif ()
set (CMAKE_CXX_FLAGS "${CMAKE_C_CXX_FLAGS} -std=c++0x -Wno-invalid-offsetof -fno-rtti")
set (CMAKE_C_FLAGS "${CMAKE_C_CXX_FLAGS}")
set (CMAKE_CXX_FLAGS_DEBUG          "-O0 -g -DGBE_DEBUG=1")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DGBE_DEBUG=1")
set (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG -DGBE_DEBUG=0")
set (CMAKE_CXX_FLAGS_RELEASE        "-O2 -DNDEBUG -DGBE_DEBUG=0")
set (CMAKE_C_FLAGS_DEBUG          "-O0 -g -DGBE_DEBUG=1")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DGBE_DEBUG=1")
set (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG -DGBE_DEBUG=0")
set (CMAKE_C_FLAGS_RELEASE        "-O2 -DNDEBUG -DGBE_DEBUG=0")

# Front end stuff we need
#INCLUDE(CMake/FindLLVM.cmake)
Find_Package(LLVM 3.3)

set (CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined ${LLVM_LDFLAGS}")

# XLib
Find_Package(X11)
IF(X11_FOUND)
  MESSAGE(STATUS "Looking for XLib - found")
ELSE(X11_FOUND)
  MESSAGE(STATUS "Looking for XLib - not found")
ENDIF(X11_FOUND)

# DRM
pkg_check_modules(DRM REQUIRED libdrm)
IF(DRM_FOUND)
  MESSAGE(STATUS "Looking for DRM - found at ${DRM_PREFIX} ${DRM_VERSION}")
  INCLUDE_DIRECTORIES(${DRM_INCLUDE_DIRS})
ELSE(DRM_FOUND)
  MESSAGE(STATUS "Looking for DRM - not found")
ENDIF(DRM_FOUND)

# DRM Intel
pkg_check_modules(DRM_INTEL libdrm_intel>=2.4.52)
IF(DRM_INTEL_FOUND)
  INCLUDE_DIRECTORIES(${DRM_INTEL_INCLUDE_DIRS})
  MESSAGE(STATUS "Looking for DRM Intel - found at ${DRM_INTEL_PREFIX} ${DRM_INTEL_VERSION}")
  #userptr support starts from 2.4.57, but 2.4.58 is the actual stable release
  #FIXME userptr has randome fail for some cases, need further investigating.
  #IF(DRM_INTEL_VERSION VERSION_GREATER 2.4.57)
  #  MESSAGE(STATUS "Enable userptr support")
  #  SET(DRM_INTEL_USERPTR "enable")
  #ELSE(DRM_INTEL_VERSION VERSION_GREATER 2.4.57)
  #  MESSAGE(STATUS "Disable userptr support")
  #ENDIF(DRM_INTEL_VERSION VERSION_GREATER 2.4.57)
ELSE(DRM_INTEL_FOUND)
  MESSAGE(FATAL_ERROR "Looking for DRM Intel (>= 2.4.52) - not found")
ENDIF(DRM_INTEL_FOUND)

# Threads
Find_Package(Threads)

IF(X11_FOUND)
# OpenGL (not use cmake helper)
pkg_check_modules(OPENGL gl)
IF(OPENGL_FOUND)
  INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIRS})
  MESSAGE(STATUS "Looking for OpenGL - found at ${OPENGL_PREFIX}")
ELSE(OPENGL_FOUND)
  MESSAGE(STATUS "Looking for OpenGL - not found")
ENDIF(OPENGL_FOUND)

# Xext
pkg_check_modules(XEXT REQUIRED xext)
IF(XEXT_FOUND)
  INCLUDE_DIRECTORIES(${XEXT_INCLUDE_DIRS})
  MESSAGE(STATUS "Looking for Xext - found at ${XEXT_PREFIX}")
ELSE(XEXT_FOUND)
  MESSAGE(STATUS "Looking for Xext - not found")
ENDIF(XEXT_FOUND)

# Xfixes
pkg_check_modules(XFIXES REQUIRED xfixes)
IF(XFIXES_FOUND)
  INCLUDE_DIRECTORIES(${XFIXES_INCLUDE_DIRS})
  MESSAGE(STATUS "Looking for Xfixes - found at ${XFIXES_PREFIX}")
ELSE(XFIXES_FOUND)
  MESSAGE(STATUS "Looking for Xfixes - not found")
ENDIF(XFIXES_FOUND)
ENDIF(X11_FOUND)

pkg_check_modules(EGL egl)
IF(EGL_FOUND)
  MESSAGE(STATUS "Looking for EGL - found at ${EGL_PREFIX}")
ELSE(EGL_FOUND)
  MESSAGE(STATUS "Looking for EGL - not found")
ENDIF(EGL_FOUND)

# cl_khr_gl_sharing requires to build with mesa source
Find_Package(MesaSrc)
IF(MESA_SOURCE_FOUND)
  MESSAGE(STATUS "Looking for mesa source code - found at ${MESA_SOURCE_PREFIX}")
ELSE(MESA_SOURCE_FOUND)
  MESSAGE(STATUS "Looking for mesa source code - not found, cl_khr_gl_sharing will be disabled.")
ENDIF(MESA_SOURCE_FOUND)

Find_Package(OCLIcd)
IF(OCLIcd_FOUND)
  MESSAGE(STATUS "Looking for OCL ICD header file - found")
  configure_file (
    "intel-beignet.icd.in"
    "intel-beignet.icd"
  )
  install (FILES ${CMAKE_CURRENT_BINARY_DIR}/intel-beignet.icd DESTINATION /etc/OpenCL/vendors)
ELSE(OCLIcd_FOUND)
  MESSAGE(STATUS "Looking for OCL ICD header file - not found")
ENDIF(OCLIcd_FOUND)

Find_Package(PythonInterp)

ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(backend)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(utests)
ADD_SUBDIRECTORY(benchmark)

SET(CPACK_PACKAGE_VERSION_MAJOR "${LIBCL_DRIVER_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${LIBCL_DRIVER_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${LIBCL_DRIVER_VERSION_PATCH}")
SET(CPACK_SOURCE_GENERATOR "TGZ;TZ")
SET(CPACK_PACKAGE_NAME "Beignet")
SET(CPACK_PACKAGE_VENDOR "Intel Open Source Technology Center")
INCLUDE(CPack)

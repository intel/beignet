#############################################################################
#                  INTEL CORPORATION PROPRIETARY INFORMATION                #
#     This software is supplied under the terms of a license agreement or   #
#     nondisclosure agreement with Intel Corporation and may not be copied  #
#     or disclosed except in accordance with the terms of that agreement.   #
#          Copyright (C) 2009 Intel Corporation. All Rights Reserved.       #
#############################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)
PROJECT(OCL)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

SET(CMAKE_VERBOSE_MAKEFILE "false")
SET(CMAKE_BUILD_TYPE "NOOPT" CACHE STRING "Build type")
SET(USE_INTEL_COMPILER CACHE BOOL "false")
SET(USE_FULSIM CACHE BOOL "false")
ADD_DEFINITIONS(-D__$(USER)__)

IF (USE_FULSIM)
  ADD_DEFINITIONS(-DUSE_FULSIM=1)
ELSE (USE_FULSIM)
  ADD_DEFINITIONS(-DUSE_FULSIM=0)
ENDIF (USE_FULSIM)

SET(SSE "-mssse3")
SET(CMAKE_CXX_FLAGS "-Wall -Wno-invalid-offsetof -mfpmath=sse ${SSE} --no-exceptions --no-rtti -Wcast-align -std=c++0x")
SET(CMAKE_CXX_FLAGS_NOOPT "-g -DNOOPT")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -O2 -pg -DDEBUG")
SET(CMAKE_CXX_FLAGS_RELEASE "-g -O3 -DNDEBUG")
SET(CMAKE_C_FLAGS "-Wall -mfpmath=sse -msse -msse2 ${SSE4} --no-exceptions -Wcast-align")
SET(CMAKE_C_FLAGS_NOOPT "-g -DNOOPT")
SET(CMAKE_C_FLAGS_DEBUG "-g -O2 -pg -DDEBUG")
SET(CMAKE_C_FLAGS_RELEASE "-O3 -g -DNDEBUG")
SET(OCL_CMAKE_DIR "${OCL_SOURCE_DIR}/CMake")
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${OCL_CMAKE_DIR}")

# XLib
INCLUDE(CMake/FindXLib.cmake)
IF(XLIB_FOUND)
  MESSAGE(STATUS "Looking for XLib - found")
ELSE(XLIB_FOUND)
  MESSAGE(STATUS "Looking for XLib - not found")
ENDIF(XLIB_FOUND)

# DRM
INCLUDE(CMake/FindDRM.cmake)
IF(DRM_FOUND)
  MESSAGE(STATUS "Looking for DRM - found")
ELSE(DRM_FOUND)
  MESSAGE(STATUS "Looking for DRM - not found")
ENDIF(DRM_FOUND)

# DRM Intel
INCLUDE(CMake/FindDRMIntel.cmake)
IF(DRM_INTEL_FOUND)
  MESSAGE(STATUS "Looking for DRM Intel - found")
ELSE(DRM_INTEL_FOUND)
  MESSAGE(STATUS "Looking for DRM Intel - not found")
ENDIF(DRM_INTEL_FOUND)

# Xext
INCLUDE(CMake/FindXext.cmake)
IF(XEXT_FOUND)
  MESSAGE(STATUS "Looking for Xext - found")
ELSE(XEXT_FOUND)
  MESSAGE(STATUS "Looking for Xext - not found")
ENDIF(XEXT_FOUND)

# Xfixes
INCLUDE(CMake/FindXfixes.cmake)
IF(XFIXES_FOUND)
  MESSAGE(STATUS "Looking for Xfixes - found")
ELSE(XFIXES_FOUND)
  MESSAGE(STATUS "Looking for Xfixes - not found")
ENDIF(XFIXES_FOUND)

# the run-time itself
ADD_SUBDIRECTORY(src)


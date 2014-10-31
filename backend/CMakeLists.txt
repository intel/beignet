project (GBE)
set (LIBGBE_VERSION_MAJOR 0)
set (LIBGBE_VERSION_MINOR 2)
cmake_minimum_required (VERSION 2.6.0)

set (GBE_CMAKE_DIR "${GBE_SOURCE_DIR}/cmake")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${GBE_CMAKE_DIR}")

##############################################################
# Compilation directives
##############################################################

set (GBE_DEBUG_MEMORY false CACHE bool "Activate the memory debugger")
set (GBE_USE_BLOB false CACHE bool "Compile everything from one big file")


# Force Release with debug info
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE RelWithDebInfo)
endif (NOT CMAKE_BUILD_TYPE)
set (CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "assure config" FORCE)
message(STATUS "Building mode: " ${CMAKE_BUILD_TYPE})

if (GBE_DEBUG_MEMORY)
  set (GBE_DEBUG_MEMORY_FLAG "-DGBE_DEBUG_MEMORY=1")
else (GBE_DEBUG_MEMORY)
  set (GBE_DEBUG_MEMORY_FLAG "-DGBE_DEBUG_MEMORY=0")
endif (GBE_DEBUG_MEMORY)

# Hide all symbols and allows the symbols declared as visible to be exported
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CFLAGS} ${GBE_DEBUG_MEMORY_FLAG} ${GBE_COMPILE_UTESTS_FLAG} -DGBE_COMPILER_AVAILABLE=1 -fvisibility=hidden")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LLVM_CFLAGS} ${GBE_DEBUG_MEMORY_FLAG} ${GBE_COMPILE_UTESTS_FLAG} -DGBE_COMPILER_AVAILABLE=1")

include_directories (${CMAKE_CURRENT_BINARY_DIR})
##############################################################
# Project source code
##############################################################
add_subdirectory (src)
set(LOCAL_OCL_BITCODE_BIN "${LOCAL_OCL_BITCODE_BIN}" PARENT_SCOPE)
set(LOCAL_OCL_HEADER_DIR "${LOCAL_OCL_HEADER_DIR}" PARENT_SCOPE)
set(LOCAL_OCL_PCH_OBJECT "${LOCAL_OCL_PCH_OBJECT}" PARENT_SCOPE)

set(LOCAL_GBE_OBJECT_DIR ${LOCAL_GBE_OBJECT_DIR} PARENT_SCOPE)
set(LOCAL_INTERP_OBJECT_DIR ${LOCAL_INTERP_OBJECT_DIR} PARENT_SCOPE)

set (GBE_BIN_GENERATER
     env OCL_BITCODE_LIB_PATH=${LOCAL_OCL_BITCODE_BIN} OCL_HEADER_FILE_DIR=${LOCAL_OCL_HEADER_DIR} OCL_PCH_PATH=${LOCAL_OCL_PCH_OBJECT} LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src/gbe_bin_generater
     PARENT_SCOPE)


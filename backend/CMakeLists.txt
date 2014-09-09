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

##############################################################
# Compiler
##############################################################
if (UNIX)
  set (COMPILER "GCC" CACHE INT "Compiler to choose on Linux (GCC,ICC,CLANG)")
endif (UNIX)

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
set (CMAKE_C_CXX_FLAGS "-fvisibility=hidden -DGBE_COMPILER_AVAILABLE=1 ${CMAKE_C_CXX_FLAGS}")

if (COMPILER STREQUAL "GCC")
  set (CMAKE_C_CXX_FLAGS "${CMAKE_C_CXX_FLAGS} -funroll-loops -Wstrict-aliasing=2 -fstrict-aliasing -msse2 -msse3 -mssse3 -msse4.1 -fPIC -Wall")
  set (CMAKE_C_CXX_FLAGS "${CMAKE_C_CXX_FLAGS}  ${LLVM_CFLAGS}")
  set (CMAKE_CXX_FLAGS "${CMAKE_C_CXX_FLAGS}  -Wno-invalid-offsetof -fno-rtti -std=c++0x")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GBE_DEBUG_MEMORY_FLAG}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GBE_COMPILE_UTESTS_FLAG}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-E")
  set (CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined ${LLVM_LFLAGS}")
  set (CMAKE_CXX_FLAGS_DEBUG          "-g -DGBE_DEBUG=1")
  set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DGBE_DEBUG=1")
  set (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG -DGBE_DEBUG=0")
  set (CMAKE_CXX_FLAGS_RELEASE        "-O2 -DNDEBUG -DGBE_DEBUG=0")
  set (CMAKE_C_FLAGS "${CMAKE_C_CXX_FLAGS}")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GBE_DEBUG_MEMORY_FLAG}")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GBE_COMPILE_UTESTS_FLAG}")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-E")
  set (CMAKE_C_FLAGS_DEBUG          "-g -DGBE_DEBUG=1")
  set (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DGBE_DEBUG=1")
  set (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG -DGBE_DEBUG=0")
  set (CMAKE_C_FLAGS_RELEASE        "-O2 -DNDEBUG -DGBE_DEBUG=0")
elseif (COMPILER STREQUAL "CLANG")
  set (CMAKE_C_COMPILER             "clang")
  set (CMAKE_C_FLAGS                "-Wall -std=c99")
  set (CMAKE_C_FLAGS_DEBUG          "-g -DGBE_DEBUG=1")
  set (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DGBE_DEBUG=1")
  set (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG -DGBE_DEBUG=0")
  set (CMAKE_C_FLAGS_RELEASE        "-O2 -DNDEBUG -DGBE_DEBUG=0")
  set (CMAKE_CXX_COMPILER             "clang++")
  set (CMAKE_CXX_FLAGS "-fstrict-aliasing -msse2 -fPIC -Wall -Wno-format-security -Wno-invalid-offsetof -std=c++0x")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GBE_DEBUG_MEMORY_FLAG}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GBE_COMPILE_UTESTS_FLAG}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${VISIBILITY_FLAG}")
  set (CMAKE_CXX_FLAGS_DEBUG          "-g -DGBE_DEBUG=1")
  set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DGBE_DEBUG=1")
  set (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG -DGBE_DEBUG=0")
  set (CMAKE_CXX_FLAGS_RELEASE        "-O2 -DNDEBUG -DGBE_DEBUG=0")
  set (CMAKE_AR      "/usr/bin/llvm-ar")
  set (CMAKE_LINKER  "/usr/bin/llvm-ld")
  set (CMAKE_NM      "/usr/bin/llvm-nm")
  set (CMAKE_OBJDUMP "/usr/bin/llvm-objdump")
  set (CMAKE_RANLIB  "ranlib")
elseif (COMPILER STREQUAL "ICC")
  set (CMAKE_CXX_COMPILER "icpc")
  set (CMAKE_C_COMPILER "icc")
  set (CMAKE_CXX_FLAGS "-std=c++0x -wd2928 -Wall -fPIC -fstrict-aliasing -fp-model fast -xSSE2")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GBE_DEBUG_MEMORY_FLAG}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GBE_COMPILE_UTESTS_FLAG}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${VISIBILITY_FLAG} -Wl,-E")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GBE_DEBUG_MODE_FLAG}")
  set (CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DGBE_DEBUG=1")
  set (CCMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O2 -DGBE_DEBUG=1")
  set (CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O2 -DGBE_DEBUG=0")
  set (CCMAKE_CXX_FLAGS_MINSIZEREL "-Os -DGBE_DEBUG=0")
  set (CMAKE_EXE_LINKER_FLAGS "")
endif ()

include_directories (${CMAKE_CURRENT_BINARY_DIR})
##############################################################
# Project source code
##############################################################
add_subdirectory (src)
set(LOCAL_PCH_OBJECT_DIR ${LOCAL_PCH_OBJECT_DIR} PARENT_SCOPE)
set(LOCAL_PCM_OBJECT_DIR ${LOCAL_PCM_OBJECT_DIR} PARENT_SCOPE)
set(LOCAL_GBE_OBJECT_DIR ${LOCAL_GBE_OBJECT_DIR} PARENT_SCOPE)
set(LOCAL_INTERP_OBJECT_DIR ${LOCAL_INTERP_OBJECT_DIR} PARENT_SCOPE)

set (GBE_BIN_GENERATER
     OCL_PCM_PATH=${LOCAL_PCM_OBJECT_DIR} OCL_PCH_PATH=${LOCAL_PCH_OBJECT_DIR} LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src/gbe_bin_generater
     PARENT_SCOPE)


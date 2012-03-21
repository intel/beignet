project (GBE)

cmake_minimum_required (VERSION 2.6.0)

set (GBE_CMAKE_DIR "${GBE_SOURCE_DIR}/cmake")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${GBE_CMAKE_DIR}")

##############################################################
# Compilation directives
##############################################################

set (GBE_DEBUG_MEMORY false CACHE bool "Activate the memory debugger")
set (GBE_COMPILE_UTESTS false CACHE bool "Will compile the unit tests")
set (GBE_USE_BLOB false CACHE bool "Compile everything from one big file")

##############################################################
# Front end stuff we need
##############################################################
Find_Package(LLVM REQUIRED)

##############################################################
# Compiler
##############################################################
if (UNIX)
  set (COMPILER "GCC" CACHE INT "Compiler to choose on Linux (GCC,ICC,CLANG)")
endif (UNIX)

if (WIN32)
  if (MINGW)
    set (DEF "-D")
  else (MINGW)
    set (DEF "/D")
  endif (MINGW)
else (WIN32)
  set (DEF "-D")
endif (WIN32)

if (GBE_DEBUG_MEMORY)
  set (GBE_DEBUG_MEMORY_FLAG "${DEF}GBE_DEBUG_MEMORY=1")
else (GBE_DEBUG_MEMORY)
  set (GBE_DEBUG_MEMORY_FLAG "${DEF}GBE_DEBUG_MEMORY=0")
endif (GBE_DEBUG_MEMORY)

if (GBE_COMPILE_UTESTS)
  set (GBE_COMPILE_UTESTS_FLAG "${DEF}GBE_COMPILE_UTESTS=1")
else (GBE_COMPILE_UTESTS)
  set (GBE_COMPILE_UTESTS_FLAG "${DEF}GBE_COMPILE_UTESTS=0")
endif (GBE_COMPILE_UTESTS)

## Linux compilation
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # Hide all symbols and allows the symbols declared as visible to be exported
  if (NOT GBE_COMPILE_UTESTS)
    set (CMAKE_C_CXX_FLAGS "-fvisibility=hidden")
  endif (NOT GBE_COMPILE_UTESTS)

  if (COMPILER STREQUAL "GCC")
    set (CMAKE_C_CXX_FLAGS "${CMAKE_C_CXX_FLAGS} -Wstrict-aliasing=2 -fstrict-aliasing -msse2 -ffast-math -fPIC -Wall")
    set (CMAKE_CXX_FLAGS "${CMAKE_C_CXX_FLAGS}  -Wno-invalid-offsetof -fno-rtti -std=c++0x")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GBE_DEBUG_MEMORY_FLAG}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GBE_COMPILE_UTESTS_FLAG}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-E")
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
    if (NOT GBE_COMPILE_UTESTS)
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    endif (NOT GBE_COMPILE_UTESTS)
    set (CMAKE_CXX_FLAGS_DEBUG          "-g -DGBE_DEBUG=1")
    set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DGBE_DEBUG=1")
    set (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG -DGBE_DEBUG=0")
    set (CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG -DGBE_DEBUG=0")
    set (CMAKE_C_FLAGS "${CMAKE_C_CXX_FLAGS}")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GBE_DEBUG_MEMORY_FLAG}")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GBE_COMPILE_UTESTS_FLAG}")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-E")
    set (CMAKE_C_FLAGS_DEBUG          "-g -DGBE_DEBUG=1")
    set (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DGBE_DEBUG=1")
    set (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG -DGBE_DEBUG=0")
    set (CMAKE_C_FLAGS_RELEASE        "-O3 -DNDEBUG -DGBE_DEBUG=0")
  elseif (COMPILER STREQUAL "CLANG")
    set (CMAKE_C_COMPILER             "clang")
    set (CMAKE_C_FLAGS                "-Wall -std=c99")
    set (CMAKE_C_FLAGS_DEBUG          "-g -DGBE_DEBUG=1")
    set (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DGBE_DEBUG=1")
    set (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG -DGBE_DEBUG=0")
    set (CMAKE_C_FLAGS_RELEASE        "-O3 -DNDEBUG -DGBE_DEBUG=0")
    set (CMAKE_CXX_COMPILER             "clang++")
    set (CMAKE_CXX_FLAGS "-fstrict-aliasing -msse2 -ffast-math -fPIC -Wall -Wno-format-security -Wno-invalid-offsetof -std=c++0x")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GBE_DEBUG_MEMORY_FLAG}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GBE_COMPILE_UTESTS_FLAG}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${VISIBILITY_FLAG}")
    set (CMAKE_CXX_FLAGS_DEBUG          "-g -DGBE_DEBUG=1")
    set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DGBE_DEBUG=1")
    set (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG -DGBE_DEBUG=0")
    set (CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG -DGBE_DEBUG=0")
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

## Windows compilation
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  if (MINGW)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof -fstrict-aliasing -msse2 -ffast-math -Wall -fno-rtti -std=c++0x")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GBE_DEBUG_MEMORY_FLAG}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GBE_COMPILE_UTESTS_FLAG}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GBE_DEBUG_MODE_FLAG}")
    set (CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -DGBE_DEBUG=0")
  else (MINGW)
    set (COMMON_FLAGS "${GBE_DEBUG_MEMORY_FLAG} /arch:SSE2 /D_CRT_SECURE_NO_WARNINGS /DNOMINMAX /GR- /GS- /W3 /wd4275")
    set (CMAKE_CXX_FLAGS ${COMMON_FLAGS})
    set (CMAKE_C_FLAGS ${COMMON_FLAGS})
  endif (MINGW)
endif ()

##############################################################
# Project source code
##############################################################
add_subdirectory (src)


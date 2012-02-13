set (GBE_SRC
  sys/vector.hpp
  sys/hash_map.hpp
  sys/map.hpp
  sys/set.hpp
  sys/exception.hpp
  sys/assert.cpp
  sys/assert.hpp
  sys/string.cpp
  sys/string.hpp
  sys/alloc.cpp
  sys/alloc.hpp
  sys/sysinfo.cpp
  sys/sysinfo.hpp
  sys/mutex.cpp
  sys/mutex.hpp
  sys/condition.cpp
  sys/condition.hpp
  sys/platform.cpp
  sys/platform.hpp
  ir/context.cpp
  ir/context.hpp
  ir/unit.cpp
  ir/unit.hpp
  ir/constant.cpp
  ir/constant.hpp
  ir/instruction.cpp
  ir/instruction.hpp
  ir/register.cpp
  ir/register.hpp
  ir/function.cpp
  ir/function.hpp)

set (COMPILE_UTEST false CACHE bool "Compile or not the unit tests")
if (COMPILE_UTEST)
  set (GBE_SRC ${GBE_SRC})
endif (COMPILE_UTEST)

include_directories (.)
add_library (gbe SHARED ${GBE_SRC})


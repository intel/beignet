set (GBE_SRC
  sys/vector.hpp
  sys/hash_map.hpp
  sys/map.hpp
  sys/string.cpp
  sys/string.hpp
  sys/filename.cpp
  sys/filename.hpp
  sys/library.cpp
  sys/library.hpp
  sys/alloc.cpp
  sys/alloc.hpp
  sys/sysinfo.cpp
  sys/sysinfo.hpp
  sys/ref.hpp
  sys/mutex.cpp
  sys/mutex.hpp
  sys/condition.cpp
  sys/condition.hpp
  sys/platform.cpp
  sys/platform.hpp
  ir/ir_context.cpp
  ir/ir_context.hpp
  ir/ir_unit.cpp
  ir/ir_unit.hpp
  ir/ir_constant.cpp
  ir/ir_constant.hpp
  ir/ir_instruction.cpp
  ir/ir_instruction.hpp
  ir/ir_register.cpp
  ir/ir_register.hpp
  ir/ir_function.cpp
  ir/ir_function.hpp)

set (COMPILE_UTEST false CACHE bool "Compile or not the unit tests")
if (COMPILE_UTEST)
  set (GBE_SRC ${GBE_SRC})
endif (COMPILE_UTEST)

if (UNIX)
  set (EXEC_DEPENDENCIES pthread dl)
else (UNIX)
  set (EXEC_DEPENDENCIES)
endif (UNIX)

include_directories (.)
add_library (gbe SHARED ${GBE_SRC})


#include "utest_helper.hpp"

void compiler_atomic_functions(void)
{
  OCL_CREATE_KERNEL("compiler_atomic_functions");
}

MAKE_UTEST_FROM_FUNCTION(compiler_atomic_functions);



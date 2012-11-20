#include "utest_helper.hpp"

void compiler_math_builtin(void)
{
  OCL_CREATE_KERNEL("compiler_math_builtin");
}

MAKE_UTEST_FROM_FUNCTION(compiler_math_builtin);


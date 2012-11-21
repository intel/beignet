#include "utest_helper.hpp"

void compiler_integer_builtin(void)
{
  OCL_CREATE_KERNEL("compiler_integer_builtin");
}

MAKE_UTEST_FROM_FUNCTION(compiler_integer_builtin);


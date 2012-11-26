#include "utest_helper.hpp"

void compiler_relational_builtin(void)
{
  OCL_CREATE_KERNEL("compiler_relational_builtin");
}

MAKE_UTEST_FROM_FUNCTION(compiler_relational_builtin);


#include "utest_helper.hpp"

void compiler_geometric_builtin(void)
{
  OCL_CREATE_KERNEL("compiler_geometric_builtin");
}

MAKE_UTEST_FROM_FUNCTION(compiler_geometric_builtin);


#include "utest_helper.hpp"

void compiler_workitem_builtin(void)
{
  OCL_CREATE_KERNEL("compiler_workitem_builtin");
}

MAKE_UTEST_FROM_FUNCTION(compiler_workitem_builtin);


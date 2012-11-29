#include "utest_helper.hpp"

void compiler_async_copy_and_prefetch(void)
{
  OCL_CREATE_KERNEL("compiler_async_copy_and_prefetch");
}

MAKE_UTEST_FROM_FUNCTION(compiler_async_copy_and_prefetch);



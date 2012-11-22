#include "utest_helper.hpp"

void compiler_address_space(void)
{
  OCL_CREATE_KERNEL("compiler_address_space");
}

MAKE_UTEST_FROM_FUNCTION(compiler_address_space);



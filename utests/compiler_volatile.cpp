#include "utest_helper.hpp"

void compiler_volatile(void)
{
  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_volatile");
}

MAKE_UTEST_FROM_FUNCTION(compiler_volatile);

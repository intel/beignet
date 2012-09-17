#include "utest_helper.hpp"

void compiler_local_slm(void)
{
  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_local_slm");
}

MAKE_UTEST_FROM_FUNCTION(compiler_local_slm);


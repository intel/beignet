#include "utest_helper.hpp"

void test_printf(void)
{
  // Setup kernel and buffers
  OCL_CREATE_KERNEL("test_printf");
  globals[0] = 16;
  locals[0] = 16;
  globals[1] = 4;
  locals[1] = 4;
  globals[2] = 8;
  locals[2] = 2;

  // Run the kernel on GPU
  OCL_NDRANGE(3);
}

MAKE_UTEST_FROM_FUNCTION(test_printf);

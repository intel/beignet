#include "utest_helper.hpp"

void compiler_insert_vector(void)
{
  const size_t n = 2048;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_insert_vector");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int) * 4, NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_insert_vector);

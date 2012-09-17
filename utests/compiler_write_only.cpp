#include "utest_helper.hpp"

void compiler_write_only(void)
{
  const size_t n = 2048;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("test_write_only");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(0);

  // Check results
  for (uint32_t i = 0; i < n; ++i)
    OCL_ASSERT(((uint32_t*)buf_data[0])[i] == i);
}

MAKE_UTEST_FROM_FUNCTION(compiler_write_only);


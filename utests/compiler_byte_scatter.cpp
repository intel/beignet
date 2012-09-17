#include "utest_helper.hpp"

static void compiler_byte_scatter(void)
{
  const size_t n = 128;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_byte_scatter");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int8_t), NULL);

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  // Check result
  OCL_MAP_BUFFER(0);
  for (int32_t i = 0; i < (int32_t) n; ++i)
    OCL_ASSERT(((int8_t*)buf_data[0])[i] == (int8_t) i);
}

MAKE_UTEST_FROM_FUNCTION(compiler_byte_scatter);


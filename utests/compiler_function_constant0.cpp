#include "utest_helper.hpp"

void compiler_function_constant0(void)
{
  const size_t n = 2048;
  const uint32_t value = 34;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_function_constant0");
  OCL_CREATE_BUFFER(buf[0], 0, 75 * sizeof(int32_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, 1 * sizeof(char), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(3, sizeof(uint32_t), &value);

  OCL_MAP_BUFFER(0);
  for(uint32_t i = 0; i < 69; ++i)
    ((int32_t *)buf_data[0])[i] = i;
  OCL_UNMAP_BUFFER(0);

  OCL_MAP_BUFFER(1);
  ((char *)buf_data[1])[0] = 15;
  OCL_UNMAP_BUFFER(1);

  // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(2);

  // Check results
  for (uint32_t i = 0; i < n; ++i)
    OCL_ASSERT(((uint32_t *)buf_data[2])[i] == (value + 15 + i%69));

  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION(compiler_function_constant0);

#include "utest_helper.hpp"

void compiler_function_constant1(void)
{
  const size_t n = 2048;
  const uint32_t value = 34;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_function_constant");
  OCL_CREATE_BUFFER(buf[0], 0, 75 * sizeof(short), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(uint32_t), &value);

  OCL_MAP_BUFFER(0);
  for(uint32_t i = 0; i < 69; ++i)
    ((short *)buf_data[0])[i] = i;
  OCL_UNMAP_BUFFER(0);

  // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  OCL_CREATE_BUFFER(buf[2], 0, 101 * sizeof(short), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[2]);
  OCL_MAP_BUFFER(2);
  for(uint32_t i = 0; i < 69; ++i)
    ((short *)buf_data[2])[i] = 2*i;
  OCL_UNMAP_BUFFER(2);

  // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(1);

  // Check results
  for (uint32_t i = 0; i < n; ++i)
    OCL_ASSERT(((uint32_t *)buf_data[1])[i] == (value + (i%69)*2));

  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_function_constant1);

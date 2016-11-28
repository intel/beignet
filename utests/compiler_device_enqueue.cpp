#include "utest_helper.hpp"

void compiler_device_enqueue(void)
{
  const size_t n = 32;
  const uint32_t global_sz = 3;
  uint32_t result = 0;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_device_enqueue");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(uint32_t), &global_sz);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[0]);

  OCL_MAP_BUFFER(0);
  for(uint32_t i = 0; i < 69; ++i)
    ((short *)buf_data[0])[i] = 0;
  OCL_UNMAP_BUFFER(0);

  // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  for(uint32_t i = 0; i < global_sz; ++i) {
    result += i;
  }
  result *= global_sz;

  OCL_MAP_BUFFER(0);
  for (uint32_t i = 0; i < n; ++i)
    OCL_ASSERT(((uint32_t *)buf_data[0])[i] == result);
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_device_enqueue);

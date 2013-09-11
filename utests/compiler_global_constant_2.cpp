#include "utest_helper.hpp"

void compiler_global_constant_2(void)
{
  const size_t n = 2048;
  const uint32_t e = 34, r = 77;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_global_constant_2");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(uint32_t), &e);
  OCL_SET_ARG(2, sizeof(uint32_t), &r);

  // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  unsigned int m[3] = {0x15b,0x25b,0x35b};
  unsigned int t[5] = {0x45b,0x55b,0x65b,0x75b,0x85b};

  // Check results
  OCL_MAP_BUFFER(0);
  for (uint32_t i = 0; i < n; ++i)
//    std::cout << ((uint32_t *)buf_data[0])[i] << std::endl;
    OCL_ASSERT(((uint32_t *)buf_data[0])[i] == m[i%3] + t[i%5] + e + r);
  OCL_UNMAP_BUFFER(0);
}

void compiler_global_constant_2_long(void)
{
  const size_t n = 2048;
  const uint32_t e = 34, r = 77;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_global_constant_2", "compiler_global_constant_2_long");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint64_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(uint32_t), &e);
  OCL_SET_ARG(2, sizeof(uint32_t), &r);

  // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  uint64_t m[3] = {0x15b,0x25b,0xFFFFFFFFF};

  // Check results
  OCL_MAP_BUFFER(0);
  for (uint32_t i = 0; i < n; ++i)
//    std::cout << ((uint64_t *)buf_data[0])[i] << std::endl;
    OCL_ASSERT(((uint64_t *)buf_data[0])[i] == m[i%3] + e + r);
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_global_constant_2);
MAKE_UTEST_FROM_FUNCTION(compiler_global_constant_2_long);

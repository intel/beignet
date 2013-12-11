#include "utest_helper.hpp"

void compiler_global_constant(void)
{
  const size_t n = 2048;
  const uint32_t e = 34, r = 77;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_global_constant");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(uint32_t), &e);
  OCL_SET_ARG(2, sizeof(uint32_t), &r);

  // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  unsigned int m[3] = {71,72,73};

  // Check results
  OCL_MAP_BUFFER(0);
  for (uint32_t i = 0; i < n; ++i)
//    printf("%d result %d reference %d\n", i, ((uint32_t *)buf_data[0])[i], m[i%3] + e + r);
    OCL_ASSERT(((uint32_t *)buf_data[0])[i] == m[i%3] + e + r);
  OCL_UNMAP_BUFFER(0);
}

void compiler_global_constant1(void)
{
  const size_t n = 32;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_global_constant", "compiler_global_constant1");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  uint32_t data1[] = {1, 4, 7};
  uint32_t data2[]= {3, 7, 11};

  // Check results
  OCL_MAP_BUFFER(0);
  for (uint32_t i = 0; i < n; ++i)
//    printf("%d result %d reference %d\n", i, ((uint32_t *)buf_data[0])[i], data1[i%3] + data2[i%3]);
    OCL_ASSERT(((uint32_t *)buf_data[0])[i] == data1[i%3] + data2[i%3]);
  OCL_UNMAP_BUFFER(0);
}

void compiler_global_constant2(void)
{
  const size_t n = 32;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_global_constant", "compiler_global_constant2");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  // Check results
  OCL_MAP_BUFFER(0);
  for (uint32_t i = 0; i < n; ++i)
//    printf("%d result %d reference %d\n", i, ((uint32_t *)buf_data[0])[i], 6);
    OCL_ASSERT(((uint32_t *)buf_data[0])[i] == 6);
  OCL_UNMAP_BUFFER(0);
}

void compiler_global_constant3(void)
{
  const size_t n = 32;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_global_constant", "compiler_global_constant3");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  // Run the kernel
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  uint32_t data1[] = {3, 6, 9};
  char data2[]= {'c', 'f', 'j'};
  // Check results
  OCL_MAP_BUFFER(0);
  for (uint32_t i = 0; i < n; ++i)
//    printf("%d result %d reference %d\n", i, ((uint32_t *)buf_data[0])[i], data1[i%3] + (int)data2[i%3]);
    OCL_ASSERT(((uint32_t *)buf_data[0])[i] == data1[i%3] + (uint32_t)data2[i%3]);
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_global_constant, true);
MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_global_constant1, true);
MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_global_constant2, true);
MAKE_UTEST_FROM_FUNCTION(compiler_global_constant3);

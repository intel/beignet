#include "utest_helper.hpp"

void compiler_group_size1(void)
{
  const size_t n = 7*32*17;

  int group_size[] = {7, 17, 32};
  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_group_size");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  for(int i = 0; i < 3; i++) {
    // Run the kernel
    globals[0] = n;
    locals[0] = group_size[i];
    OCL_NDRANGE(1);
    OCL_MAP_BUFFER(0);

    // Check results
    for (uint32_t i = 0; i < n; ++i)
      OCL_ASSERT(((uint32_t*)buf_data[0])[i] == i);
    OCL_UNMAP_BUFFER(0);
  }
}

void compiler_group_size2(void)
{
  const uint32_t n = 4*17*8;
  int size_x[] = {2, 4, 17};
  int size_y[] = {2, 4, 4};

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_group_size");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  for(int i = 0; i < 3; i++) {
    // Run the kernel
    globals[0] = 4*17;
    globals[1] = 8;
    locals[0] = size_x[i];
    locals[1] = size_y[i];
    OCL_NDRANGE(2);
    OCL_MAP_BUFFER(0);

    // Check results
    for (uint32_t i = 0; i < n; ++i)
      OCL_ASSERT(((uint32_t*)buf_data[0])[i] == i);
    OCL_UNMAP_BUFFER(0);
  }
}

void compiler_group_size3(void)
{
  const uint32_t n = 4*17*8*4;
  int size_x[] = {2, 4, 17};
  int size_y[] = {2, 4, 4};
  int size_z[] = {2, 1, 2};

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_group_size");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  for(int i = 0; i < 3; i++) {
    // Run the kernel
    globals[0] = 4*17;
    globals[1] = 8;
    globals[2] = 4;
    locals[0] = size_x[i];
    locals[1] = size_y[i];
    locals[2] = size_z[i];
    OCL_NDRANGE(3);
    OCL_MAP_BUFFER(0);

    // Check results
    for (uint32_t i = 0; i < n; ++i)
      OCL_ASSERT(((uint32_t*)buf_data[0])[i] == i);
    OCL_UNMAP_BUFFER(0);
  }
}
MAKE_UTEST_FROM_FUNCTION(compiler_group_size1);
MAKE_UTEST_FROM_FUNCTION(compiler_group_size2);
MAKE_UTEST_FROM_FUNCTION(compiler_group_size3);


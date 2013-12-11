#include "utest_helper.hpp"
#include <string.h>

struct xyz{
  unsigned short b;
  unsigned short e;
  unsigned int o;
};

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

void compiler_group_size4(void)
{
  const size_t n = 16;
  uint32_t color = 2;
  uint32_t num = 1;
  int group_size[] = {1};
  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_group_size", "compiler_group_size4");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(struct xyz), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);

  for(uint32_t i = 0; i < num; i++) {
    // Run the kernel
    OCL_MAP_BUFFER(0);
    ((struct xyz*)buf_data[0])[0].b = 0;
    ((struct xyz*)buf_data[0])[0].e = 2;
    ((struct xyz*)buf_data[0])[0].o = 0;
    OCL_UNMAP_BUFFER(0);

    OCL_MAP_BUFFER(1);
    memset(((uint32_t*)buf_data[1]), 0x0, sizeof(uint32_t)*n);
    OCL_UNMAP_BUFFER(1);

    OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
    OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
    OCL_SET_ARG(2, sizeof(cl_int), &group_size[i]);
    OCL_SET_ARG(3, sizeof(cl_int), &color);

    globals[0] = group_size[i];
    locals[0] = group_size[i];
    OCL_NDRANGE(1);
    OCL_MAP_BUFFER(1);

    // Check results
    for (uint32_t j = 0; j < n; ++j) {
//      std::cout <<((uint32_t*)buf_data[1])[j] << "  ";
      if(j >= i && j <= i+2) {
       OCL_ASSERT(((uint32_t*)buf_data[1])[j] == color);
      } else {
       OCL_ASSERT(((uint32_t*)buf_data[1])[j] == 0);
      }

    }
    OCL_UNMAP_BUFFER(1);
  }
}
MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_group_size1, true);
MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_group_size2, true);
MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_group_size3, true);
MAKE_UTEST_FROM_FUNCTION(compiler_group_size4);


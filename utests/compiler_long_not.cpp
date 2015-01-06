#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"

void compiler_long_not_vec8(void)
{
  const size_t n = 64;
  const int v = 8; 
  int64_t src[n * v];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_not", "compiler_long_not_vec8");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int64_t) * v, NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(int64_t) * v, NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(int64_t) * v, NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n*v; ++i) {
    if (i % 3 == 0)
      src[i] = 0x0UL;
    else
      src[i] = ((int64_t)rand() << 32) + rand();

    //	printf(" 0x%lx", src[i]);
  }

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, sizeof(int64_t) * n * v);
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);
  uint64_t res;

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n*v; ++i) {
    res = 0xffffffffffffffffUL;
    if (src[i])
      res = 0x0;

    OCL_ASSERT(((uint64_t *)(buf_data[1]))[i] == res);
    //printf("ref is 0x%lx, result is 0x%lx\n", res, ((int64_t *)(buf_data[1]))[i]);
  }
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_long_not_vec8);

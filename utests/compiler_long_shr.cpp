#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"

void compiler_long_shr(void)
{
  const size_t n = 64;
  uint64_t src[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_long_shr");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint64_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint64_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  for (int32_t i = 0; i < (int32_t) n; ++i)
    src[i] = (uint64_t)1 << 63;
  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, sizeof(src));
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  uint64_t *dest = ((uint64_t *)buf_data[1]);
  for (int32_t i = 0; i < (int32_t) n; ++i)
    if (i > 7)
      OCL_ASSERT(dest[i] == src[i] >> i);
    else
      OCL_ASSERT(dest[i] == src[i] + 1);
  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION(compiler_long_shr);

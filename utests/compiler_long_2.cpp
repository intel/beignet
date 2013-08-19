#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"

void compiler_long_2(void)
{
  const size_t n = 16;
  int64_t src1[n], src2[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_long_2");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int64_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(int64_t), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(int64_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src1[i] = ((int64_t)rand() << 32) + rand();
    src2[i] = ((int64_t)rand() << 32) + rand();
  }
  src1[4] = 1;
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], src1, sizeof(src1));
  memcpy(buf_data[1], src2, sizeof(src2));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(2);
  int64_t *dest = ((int64_t *)buf_data[2]);
  //for (int32_t i = 0; i < (int32_t) n; ++i)
  //  printf("%lx\n", dest[i]);
  OCL_ASSERT(0xFEDCBA9876543210UL == (uint64_t)dest[0]);
  OCL_ASSERT((src1[1] & src2[1]) == dest[1]);
  OCL_ASSERT((src1[2] | src2[2]) == dest[2]);
  OCL_ASSERT((src1[3] ^ src2[3]) == dest[3]);
  OCL_ASSERT(0x1122334455667788L == dest[4]);
  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION(compiler_long_2);

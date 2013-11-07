#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"

void compiler_long(void)
{
  const size_t n = 16;
  int64_t src1[n], src2[n];

  int64_t zero = 0;
  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_long");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int64_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(int64_t), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(int64_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(3, sizeof(cl_long), &zero);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  src1[0] = -1L,                  src2[0] = -1L;
  src1[1] = 0x8000000000000000UL, src2[1] = 0x8000000000000000UL;
  src1[2] = 0x7FFFFFFFFFFFFFFFL,  src2[2] = 1L;
  src1[3] = 0xFFFFFFFEL,          src2[3] = 1L;
  src1[4] = 0x7FFFFFFFL,          src2[4] = 0x80000000L;
  src1[5] = 0,                    src2[5] = 0;
  src1[6] = 0,                    src2[6] = 1;
  src1[7] = -2L,                  src2[7] = -1L;
  src1[8] = 0,                    src2[8] = 0x8000000000000000UL;
  for (int32_t i = 9; i < (int32_t) n; ++i) {
    src1[i] = ((int64_t)rand() << 32) + rand();
    src2[i] = ((int64_t)rand() << 32) + rand();
  }
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
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%lx\n", ((int64_t *)buf_data[2])[i]);
    if (i < 5)
      OCL_ASSERT(src1[i] + src2[i] == ((int64_t *)buf_data[2])[i]);
    if (i > 5)
      OCL_ASSERT(src1[i] - src2[i] == ((int64_t *)buf_data[2])[i]);
  }
  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION(compiler_long);

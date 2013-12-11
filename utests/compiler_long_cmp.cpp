#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"

void compiler_long_cmp(void)
{
  const size_t n = 16;
  int64_t src1[n], src2[n];

  src1[0] = (int64_t)1 << 63, src2[0] = 0x7FFFFFFFFFFFFFFFll;
  src1[1] = (int64_t)1 << 63, src2[1] = ((int64_t)1 << 63) | 1;
  src1[2] = -1ll, src2[2] = 0;
  src1[3] = ((int64_t)123 << 32) | 0x7FFFFFFF, src2[3] = ((int64_t)123 << 32) | 0x80000000;
  src1[4] = 0x7FFFFFFFFFFFFFFFll, src2[4] = (int64_t)1 << 63;
  src1[5] = ((int64_t)1 << 63) | 1, src2[5] = (int64_t)1 << 63;
  src1[6] = 0, src2[6] = -1ll;
  src1[7] = ((int64_t)123 << 32) | 0x80000000, src2[7] = ((int64_t)123 << 32) | 0x7FFFFFFF;
  for(size_t i=8; i<n; i++) {
    src1[i] = i;
    src2[i] = i;
  }

  globals[0] = n;
  locals[0] = 16;

  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int64_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(int64_t), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(int64_t), NULL);
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], src1, sizeof(src1));
  memcpy(buf_data[1], src2, sizeof(src2));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);


  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_cmp", "compiler_long_cmp_l");
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    int64_t *dest = (int64_t *)buf_data[2];
    int64_t x = (src1[i] < src2[i]) ? 3 : 4;
    OCL_ASSERT(x == dest[i]);
  }
  OCL_UNMAP_BUFFER(2);
  OCL_DESTROY_KERNEL_KEEP_PROGRAM(true);

  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_cmp", "compiler_long_cmp_le");
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    int64_t *dest = (int64_t *)buf_data[2];
    int64_t x = (src1[i] <= src2[i]) ? 3 : 4;
    OCL_ASSERT(x == dest[i]);
  }
  OCL_UNMAP_BUFFER(2);
  OCL_DESTROY_KERNEL_KEEP_PROGRAM(true);

  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_cmp", "compiler_long_cmp_g");
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    int64_t *dest = (int64_t *)buf_data[2];
    int64_t x = (src1[i] > src2[i]) ? 3 : 4;
    OCL_ASSERT(x == dest[i]);
  }
  OCL_UNMAP_BUFFER(2);
  OCL_DESTROY_KERNEL_KEEP_PROGRAM(true);

  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_cmp", "compiler_long_cmp_ge");
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    int64_t *dest = (int64_t *)buf_data[2];
    int64_t x = (src1[i] >= src2[i]) ? 3 : 4;
    OCL_ASSERT(x == dest[i]);
  }
  OCL_UNMAP_BUFFER(2);
  OCL_DESTROY_KERNEL_KEEP_PROGRAM(true);

  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_cmp", "compiler_long_cmp_eq");
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    int64_t *dest = (int64_t *)buf_data[2];
    int64_t x = (src1[i] == src2[i]) ? 3 : 4;
    OCL_ASSERT(x == dest[i]);
  }
  OCL_UNMAP_BUFFER(2);
  OCL_DESTROY_KERNEL_KEEP_PROGRAM(true);

  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_cmp", "compiler_long_cmp_neq");
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    int64_t *dest = (int64_t *)buf_data[2];
    int64_t x = (src1[i] != src2[i]) ? 3 : 4;
    OCL_ASSERT(x == dest[i]);
  }
  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION(compiler_long_cmp);

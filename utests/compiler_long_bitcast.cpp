#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"

void compiler_bitcast_char8_to_long(void)
{
  const size_t n = 64;
  const int v = 8;
  char src[n * v];
  uint64_t *dst = (uint64_t *)src;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_bitcast", "compiler_bitcast_char8_to_long");
  OCL_CREATE_BUFFER(buf[0], 0, sizeof(src), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, sizeof(src), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n*v; ++i) {
    src[i] = (char)rand();
  }

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, sizeof(src));
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    OCL_ASSERT(((uint64_t *)(buf_data[1]))[i] == dst[i]);
    //printf("ref is 0x%lx, result is 0x%lx\n", dst[i], ((int64_t *)(buf_data[1]))[i]);
  }
  OCL_UNMAP_BUFFER(1);
}

void compiler_bitcast_long_to_char8(void)
{
  const size_t n = 64;
  const int v = 8;
  uint64_t src[n];
  char *dst = (char *)src;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_bitcast", "compiler_bitcast_char8_to_long");
  OCL_CREATE_BUFFER(buf[0], 0, sizeof(src), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, sizeof(src), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src[i] = ((int64_t)rand() << 32) + rand();
  }

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, sizeof(src));
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n*v; ++i) {
    OCL_ASSERT(((char *)(buf_data[1]))[i] == dst[i]);
//    printf("ref is 0x%2x, result is 0x%2x\n", dst[i], ((char *)(buf_data[1]))[i]);
  }
  OCL_UNMAP_BUFFER(1);
}

void compiler_bitcast_int2_to_long(void)
{
  const size_t n = 64;
  const int v = 2;
  int src[n * v];
  uint64_t *dst = (uint64_t *)src;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_bitcast", "compiler_bitcast_int2_to_long");
  OCL_CREATE_BUFFER(buf[0], 0, sizeof(src), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, sizeof(src), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n*v; ++i) {
    src[i] = (int)rand();
  }

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, sizeof(src));
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    OCL_ASSERT(((uint64_t *)(buf_data[1]))[i] == dst[i]);
    //printf("ref is 0x%lx, result is 0x%lx\n", dst[i], ((int64_t *)(buf_data[1]))[i]);
  }
  OCL_UNMAP_BUFFER(1);
}

void compiler_bitcast_long_to_int2(void)
{
  const size_t n = 64;
  const int v = 2;
  uint64_t src[n];
  uint32_t *dst = (uint32_t *)src;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_bitcast", "compiler_bitcast_long_to_int2");
  OCL_CREATE_BUFFER(buf[0], 0, sizeof(src), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, sizeof(src), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src[i] = ((int64_t)i << 32) + i;
  }

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, sizeof(src));
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n*v; ++i) {
    OCL_ASSERT(((uint32_t *)(buf_data[1]))[i] == dst[i]);
    //printf("ref is 0x%2x, result is 0x%2x\n", dst[i], ((uint32_t *)(buf_data[1]))[i]);
  }
  OCL_UNMAP_BUFFER(1);
}

void compiler_bitcast_short4_to_long(void)
{
  const size_t n = 64;
  const int v = 4;
  short src[n * v];
  uint64_t *dst = (uint64_t *)src;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_bitcast", "compiler_bitcast_short4_to_long");
  OCL_CREATE_BUFFER(buf[0], 0, sizeof(src), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, sizeof(src), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n*v; ++i) {
    src[i] = (short)rand();
  }

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, sizeof(src));
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    OCL_ASSERT(((uint64_t *)(buf_data[1]))[i] == dst[i]);
    //printf("ref is 0x%lx, result is 0x%lx\n", dst[i], ((int64_t *)(buf_data[1]))[i]);
  }
  OCL_UNMAP_BUFFER(1);
}

void compiler_bitcast_long_to_short4(void)
{
  const size_t n = 64;
  const int v = 4;
  uint64_t src[n];
  uint16_t *dst = (uint16_t *)src;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_bitcast", "compiler_bitcast_long_to_short4");
  OCL_CREATE_BUFFER(buf[0], 0, sizeof(src), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, sizeof(src), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src[i] = ((int64_t)rand() << 32) + rand();
  }

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, sizeof(src));
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n*v; ++i) {
    OCL_ASSERT(((uint16_t *)(buf_data[1]))[i] == dst[i]);
    //printf("ref is 0x%2x, result is 0x%2x\n", dst[i], ((uint16_t *)(buf_data[1]))[i]);
  }
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_bitcast_char8_to_long);
MAKE_UTEST_FROM_FUNCTION(compiler_bitcast_long_to_char8);
MAKE_UTEST_FROM_FUNCTION(compiler_bitcast_int2_to_long);
MAKE_UTEST_FROM_FUNCTION(compiler_bitcast_long_to_int2);
MAKE_UTEST_FROM_FUNCTION(compiler_bitcast_short4_to_long);
MAKE_UTEST_FROM_FUNCTION(compiler_bitcast_long_to_short4);

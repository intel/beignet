#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"

// convert shorter integer to 64-bit integer
void compiler_long_convert(void)
{
  const size_t n = 16;
  char src1[n];
  short src2[n];
  int src3[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_long_convert");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(char), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(short), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(int), NULL);
  OCL_CREATE_BUFFER(buf[3], 0, n * sizeof(int64_t), NULL);
  OCL_CREATE_BUFFER(buf[4], 0, n * sizeof(int64_t), NULL);
  OCL_CREATE_BUFFER(buf[5], 0, n * sizeof(int64_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(3, sizeof(cl_mem), &buf[3]);
  OCL_SET_ARG(4, sizeof(cl_mem), &buf[4]);
  OCL_SET_ARG(5, sizeof(cl_mem), &buf[5]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src1[i] = -i;
    src2[i] = -i;
    src3[i] = -i;
  }
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  memcpy(buf_data[0], src1, sizeof(src1));
  memcpy(buf_data[1], src2, sizeof(src2));
  memcpy(buf_data[2], src3, sizeof(src3));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(3);
  OCL_MAP_BUFFER(4);
  OCL_MAP_BUFFER(5);
  int64_t *dst1 = ((int64_t *)buf_data[3]);
  int64_t *dst2 = ((int64_t *)buf_data[4]);
  int64_t *dst3 = ((int64_t *)buf_data[5]);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%lx %lx %lx\n", dst1[i], dst2[i], dst3[i]);
    OCL_ASSERT(dst1[i] == -(int64_t)i);
    OCL_ASSERT(dst2[i] == -(int64_t)i);
    OCL_ASSERT(dst3[i] == -(int64_t)i);
  }
  OCL_UNMAP_BUFFER(3);
  OCL_UNMAP_BUFFER(4);
  OCL_UNMAP_BUFFER(5);
}

MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_long_convert, true);

// convert 64-bit integer to shorter integer
void compiler_long_convert_2(void)
{
  const size_t n = 16;
  int64_t src[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_convert", "compiler_long_convert_2");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(char), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(short), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(int), NULL);
  OCL_CREATE_BUFFER(buf[3], 0, n * sizeof(int64_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(3, sizeof(cl_mem), &buf[3]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src[i] = -i;
  }
  OCL_MAP_BUFFER(3);
  memcpy(buf_data[3], src, sizeof(src));
  OCL_UNMAP_BUFFER(3);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  char *dst1 = ((char *)buf_data[0]);
  short *dst2 = ((short *)buf_data[1]);
  int *dst3 = ((int *)buf_data[2]);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%x %x %x\n", dst1[i], dst2[i], dst3[i]);
    OCL_ASSERT(dst1[i] == -i);
    OCL_ASSERT(dst2[i] == -i);
    OCL_ASSERT(dst3[i] == -i);
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_long_convert_2, true);

// convert 64-bit integer to 32-bit float
void compiler_long_convert_to_float(void)
{
  const size_t n = 16;
  int64_t src[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_long_convert", "compiler_long_convert_to_float");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(int64_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src[i] = -(int64_t)i;
  }
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[1], src, sizeof(src));
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  float *dst = ((float *)buf_data[0]);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%f\n", dst[i]);
    OCL_ASSERT(dst[i] == src[i]);
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_long_convert_to_float);

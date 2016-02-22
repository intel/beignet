#include <cmath>
#include <string.h>
#include "utest_helper.hpp"

void compiler_double_convert_int(void)
{
  const size_t n = 16;
  double src[n];
  int32_t cpu_dst0[n];
  uint32_t cpu_dst1[n];

  if (!cl_check_double())
    return;

  memset(cpu_dst0, 0, sizeof(cpu_dst0));
  memset(cpu_dst1, 0, sizeof(cpu_dst1));
  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_double_convert", "compiler_double_convert_int");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(double), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(int32_t), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src[i] = ((double*)buf_data[0])[i] = 32.1 * (rand() & 1324135) + 1434342.73209855531;
    ((int32_t*)buf_data[1])[i] = 0;
    ((uint32_t*)buf_data[2])[i] = 0;
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    if (i%3 == 0) continue;
    cpu_dst0[i] = (int32_t)src[i];
    cpu_dst1[i] = (uint32_t)src[i];
  }

  // Compare
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("Return Int is %d, ref is %d,\t Uint is %u, ref is %u,\t double is %f\n",
    //   ((int*)buf_data[1])[i], cpu_dst0[i], ((uint32_t*)buf_data[2])[i], cpu_dst1[i], src[i]);
    OCL_ASSERT(((int32_t*)buf_data[1])[i] == cpu_dst0[i]);
    OCL_ASSERT(((uint32_t*)buf_data[2])[i] == cpu_dst1[i]);
  }
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION(compiler_double_convert_int);

void compiler_double_convert_float(void)
{
  const size_t n = 16;
  double src[n];
  float cpu_dst[n];

  if (!cl_check_double())
    return;

  memset(cpu_dst, 0, sizeof(cpu_dst));
  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_double_convert", "compiler_double_convert_float");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(double), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src[i] = ((double*)buf_data[0])[i] = 1332.1 * (rand() & 1324135) - 1434342.73209855531 * (rand() & 135);
    ((float*)buf_data[1])[i] = 0;
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    cpu_dst[i] = (float)src[i];
  }

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("Return float is %f,\t ref is %f,\t double is %f\n", ((float*)buf_data[1])[i], cpu_dst[i], src[i]);
    OCL_ASSERT(((float*)buf_data[1])[i] == cpu_dst[i]);
  }
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_double_convert_float);

void compiler_double_convert_short(void)
{
  const size_t n = 16;
  double src[n];
  int16_t cpu_dst0[n];
  uint16_t cpu_dst1[n];

  if (!cl_check_double())
    return;

  memset(cpu_dst0, 0, sizeof(cpu_dst0));
  memset(cpu_dst1, 0, sizeof(cpu_dst1));
  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_double_convert", "compiler_double_convert_short");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(double), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(int16_t), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(uint16_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src[i] = ((double*)buf_data[0])[i] = 10.3443 * (rand() & 15) + 14.8924323;
    ((int16_t*)buf_data[1])[i] = 0;
    ((uint16_t*)buf_data[2])[i] = 0;
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    if (i%3 == 0) continue;
    cpu_dst0[i] = (int16_t)src[i];
    cpu_dst1[i] = (uint16_t)src[i];
  }

  // Compare
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("Return Int is %d, ref is %d,\t Uint is %u, ref is %u,\t double is %f\n",
    //   ((int16_t*)buf_data[1])[i], cpu_dst0[i], ((uint16_t*)buf_data[2])[i], cpu_dst1[i], src[i]);
    OCL_ASSERT(((int16_t*)buf_data[1])[i] == cpu_dst0[i]);
    OCL_ASSERT(((uint16_t*)buf_data[2])[i] == cpu_dst1[i]);
  }
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION(compiler_double_convert_short);

void compiler_double_convert_char(void)
{
  const size_t n = 16;
  double src[n];
  int8_t cpu_dst0[n];
  uint8_t cpu_dst1[n];

  if (!cl_check_double())
    return;

  memset(cpu_dst0, 0, sizeof(cpu_dst0));
  memset(cpu_dst1, 0, sizeof(cpu_dst1));
  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_double_convert", "compiler_double_convert_char");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(double), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(int8_t), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(uint8_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src[i] = ((double*)buf_data[0])[i] = 10.3443 * (rand() & 7) + 2.8924323;
    ((int8_t*)buf_data[1])[i] = 0;
    ((uint8_t*)buf_data[2])[i] = 0;
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    if (i%3 == 0) continue;
    cpu_dst0[i] = (int8_t)src[i];
    cpu_dst1[i] = (uint8_t)src[i];
  }

  // Compare
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
//    printf("Return Int is %d, ref is %d,\t Uint is %u, ref is %u,\t double is %f\n",
//       ((int8_t*)buf_data[1])[i], cpu_dst0[i], ((uint8_t*)buf_data[2])[i], cpu_dst1[i], src[i]);
    OCL_ASSERT(((int8_t*)buf_data[1])[i] == cpu_dst0[i]);
    OCL_ASSERT(((uint8_t*)buf_data[2])[i] == cpu_dst1[i]);
  }
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION(compiler_double_convert_char);

void compiler_double_convert_long(void)
{
  const size_t n = 16;
  double src[n];
  int64_t cpu_dst0[n];
  uint64_t cpu_dst1[n];

  if (!cl_check_double())
    return;

  memset(cpu_dst0, 0, sizeof(cpu_dst0));
  memset(cpu_dst1, 0, sizeof(cpu_dst1));
  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_double_convert", "compiler_double_convert_long");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(double), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(int64_t), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(uint64_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src[i] = ((double*)buf_data[0])[i] = 10.3443 * (rand() & 7) + 2.8924323;
    ((int64_t*)buf_data[1])[i] = 0;
    ((uint64_t*)buf_data[2])[i] = 0;
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    if (i%3 == 0) continue;
    cpu_dst0[i] = (int64_t)src[i];
    cpu_dst1[i] = (uint64_t)src[i];
  }

  // Compare
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
//    printf("Return Int is %d, ref is %d,\t Uint is %u, ref is %u,\t double is %f\n",
//       ((int8_t*)buf_data[1])[i], cpu_dst0[i], ((uint8_t*)buf_data[2])[i], cpu_dst1[i], src[i]);
    OCL_ASSERT(((int64_t*)buf_data[1])[i] == cpu_dst0[i]);
    OCL_ASSERT(((uint64_t*)buf_data[2])[i] == cpu_dst1[i]);
  }
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION(compiler_double_convert_long);

void compiler_long_convert_double(void)
{
  const size_t n = 16;
  int64_t src0[n];
  uint64_t src1[n];
  double cpu_dst0[n];
  double cpu_dst1[n];

  if (!cl_check_double())
    return;

  memset(cpu_dst0, 0, sizeof(cpu_dst0));
  memset(cpu_dst1, 0, sizeof(cpu_dst1));

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_double_convert", "compiler_long_convert_double");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int64_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint64_t), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(double), NULL);
  OCL_CREATE_BUFFER(buf[3], 0, n * sizeof(double), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(3, sizeof(cl_mem), &buf[3]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  OCL_MAP_BUFFER(3);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src0[i] = ((int64_t*)buf_data[0])[i] = 0xABC8ABACDA00C * (rand() & 7);
    src1[i] = ((uint64_t*)buf_data[1])[i] = 0xCABC8ABACDA00C * (rand() & 15);
    ((double*)buf_data[2])[i] = 0.0;
    ((double*)buf_data[3])[i] = 0.0;
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);
  OCL_UNMAP_BUFFER(3);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    cpu_dst0[i] = (double)src0[i];
    cpu_dst1[i] = (double)src1[i];
  }

  // Compare
  OCL_MAP_BUFFER(2);
  OCL_MAP_BUFFER(3);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
//    printf("long is %ld, ref is %f, double is %f    \t"
//           "ulong is %lu, ref is %f, double is %f\n",
//           src0[i], cpu_dst0[i], ((double*)buf_data[2])[i],
//           src1[i], cpu_dst1[i], ((double*)buf_data[3])[i]);
    OCL_ASSERT(((double*)buf_data[2])[i] == cpu_dst0[i]);
    OCL_ASSERT(((double*)buf_data[3])[i] == cpu_dst1[i]);
  }
  OCL_UNMAP_BUFFER(2);
  OCL_UNMAP_BUFFER(3);
}

MAKE_UTEST_FROM_FUNCTION(compiler_long_convert_double);

void compiler_int_convert_double(void)
{
  const size_t n = 16;
  int32_t src0[n];
  uint32_t src1[n];
  double cpu_dst0[n];
  double cpu_dst1[n];

  if (!cl_check_double())
    return;

  memset(cpu_dst0, 0, sizeof(cpu_dst0));
  memset(cpu_dst1, 0, sizeof(cpu_dst1));

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_double_convert", "compiler_int_convert_double");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int32_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(double), NULL);
  OCL_CREATE_BUFFER(buf[3], 0, n * sizeof(double), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(3, sizeof(cl_mem), &buf[3]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  OCL_MAP_BUFFER(3);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src0[i] = ((int32_t*)buf_data[0])[i] = 0xCABC8A0C * (rand() & 7);
    src1[i] = ((uint32_t*)buf_data[1])[i] = 0xCACDA00C * (rand() & 15);
    ((double*)buf_data[2])[i] = 0.0;
    ((double*)buf_data[3])[i] = 0.0;
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);
  OCL_UNMAP_BUFFER(3);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    cpu_dst0[i] = (double)src0[i];
    cpu_dst1[i] = (double)src1[i];
  }

  // Compare
  OCL_MAP_BUFFER(2);
  OCL_MAP_BUFFER(3);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
//    printf("int is %d, ref is %f, double is %f    \t"
//           "uint is %u, ref is %f, double is %f\n",
//           src0[i], cpu_dst0[i], ((double*)buf_data[2])[i],
//           src1[i], cpu_dst1[i], ((double*)buf_data[3])[i]);
    OCL_ASSERT(((double*)buf_data[2])[i] == cpu_dst0[i]);
    OCL_ASSERT(((double*)buf_data[3])[i] == cpu_dst1[i]);
  }
  OCL_UNMAP_BUFFER(2);
  OCL_UNMAP_BUFFER(3);
}

MAKE_UTEST_FROM_FUNCTION(compiler_int_convert_double);

void compiler_short_convert_double(void)
{
  const size_t n = 16;
  int16_t src0[n];
  uint16_t src1[n];
  double cpu_dst0[n];
  double cpu_dst1[n];

  if (!cl_check_double())
    return;

  memset(cpu_dst0, 0, sizeof(cpu_dst0));
  memset(cpu_dst1, 0, sizeof(cpu_dst1));

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_double_convert", "compiler_short_convert_double");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int16_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint16_t), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(double), NULL);
  OCL_CREATE_BUFFER(buf[3], 0, n * sizeof(double), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(3, sizeof(cl_mem), &buf[3]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  OCL_MAP_BUFFER(3);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src0[i] = ((int16_t*)buf_data[0])[i] = 0x8A0C * (rand() & 7);
    src1[i] = ((uint16_t*)buf_data[1])[i] = 0xC00C * (rand() & 15);
    ((double*)buf_data[2])[i] = 0.0;
    ((double*)buf_data[3])[i] = 0.0;
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);
  OCL_UNMAP_BUFFER(3);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    cpu_dst0[i] = (double)src0[i];
    cpu_dst1[i] = (double)src1[i];
  }

  // Compare
  OCL_MAP_BUFFER(2);
  OCL_MAP_BUFFER(3);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
//    printf("short is %d, ref is %f, double is %f    \t"
//           "ushort is %u, ref is %f, double is %f\n",
//           src0[i], cpu_dst0[i], ((double*)buf_data[2])[i],
//           src1[i], cpu_dst1[i], ((double*)buf_data[3])[i]);
    OCL_ASSERT(((double*)buf_data[2])[i] == cpu_dst0[i]);
    OCL_ASSERT(((double*)buf_data[3])[i] == cpu_dst1[i]);
  }
  OCL_UNMAP_BUFFER(2);
  OCL_UNMAP_BUFFER(3);
}

MAKE_UTEST_FROM_FUNCTION(compiler_short_convert_double);

void compiler_char_convert_double(void)
{
  const size_t n = 16;
  int8_t src0[n];
  uint8_t src1[n];
  double cpu_dst0[n];
  double cpu_dst1[n];

  if (!cl_check_double())
    return;

  memset(cpu_dst0, 0, sizeof(cpu_dst0));
  memset(cpu_dst1, 0, sizeof(cpu_dst1));

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_double_convert", "compiler_char_convert_double");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int8_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint8_t), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(double), NULL);
  OCL_CREATE_BUFFER(buf[3], 0, n * sizeof(double), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(3, sizeof(cl_mem), &buf[3]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  OCL_MAP_BUFFER(3);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src0[i] = ((int8_t*)buf_data[0])[i] = 0x8C * (rand() & 7);
    src1[i] = ((uint8_t*)buf_data[1])[i] = 0xC0 * (rand() & 15);
    ((double*)buf_data[2])[i] = 0.0;
    ((double*)buf_data[3])[i] = 0.0;
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);
  OCL_UNMAP_BUFFER(3);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    cpu_dst0[i] = (double)src0[i];
    cpu_dst1[i] = (double)src1[i];
  }

  // Compare
  OCL_MAP_BUFFER(2);
  OCL_MAP_BUFFER(3);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
//    printf("char is %d, ref is %f, double is %f    \t"
//           "uchar is %u, ref is %f, double is %f\n",
//           src0[i], cpu_dst0[i], ((double*)buf_data[2])[i],
//           src1[i], cpu_dst1[i], ((double*)buf_data[3])[i]);
    OCL_ASSERT(((double*)buf_data[2])[i] == cpu_dst0[i]);
    OCL_ASSERT(((double*)buf_data[3])[i] == cpu_dst1[i]);
  }
  OCL_UNMAP_BUFFER(2);
  OCL_UNMAP_BUFFER(3);
}

MAKE_UTEST_FROM_FUNCTION(compiler_char_convert_double);

void compiler_float_convert_double(void)
{
  const size_t n = 16;
  float src[n];
  double cpu_dst[n];

  if (!cl_check_double())
    return;

  memset(cpu_dst, 0, sizeof(cpu_dst));

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_double_convert", "compiler_float_convert_double");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(double), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  // Run random tests
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src[i] = ((float*)buf_data[0])[i] = (float)(0x8C * (rand() & 7)) * 1342.42f;
    ((double*)buf_data[1])[i] = 0.0;
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    cpu_dst[i] = (double)src[i];
  }

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%f,   \t%f\n", ((double*)buf_data[1])[i], cpu_dst[i]);
    OCL_ASSERT(((double*)buf_data[1])[i] == cpu_dst[i]);
  }
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_float_convert_double);

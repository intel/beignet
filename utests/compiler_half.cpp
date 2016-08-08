#include <cstdint>
#include <cstring>
#include <iostream>
#include <cmath>
#include "utest_helper.hpp"

void compiler_half_basic(void)
{
  const size_t n = 16;
  uint16_t hsrc[n];
  float fsrc[n], fdst[n];
  float f = 2.5;
  uint32_t tmp_f;

  if (!cl_check_half())
    return;

  memcpy(&tmp_f, &f, sizeof(float));

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_half", "compiler_half_basic");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint16_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint16_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    fsrc[i] = 10.1 * i;
    memcpy(&tmp_f, &fsrc[i], sizeof(float));
    hsrc[i] = __float_to_half(tmp_f);
  }

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    fdst[i] = fsrc[i] + f;
    fdst[i] = fdst[i]*fdst[i];
    fdst[i] = fdst[i]/1.8;
  }

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], hsrc, sizeof(hsrc));
  memset(buf_data[1], 0, sizeof(hsrc));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    tmp_f = __half_to_float(((uint16_t *)buf_data[1])[i]);
    memcpy(&f, &tmp_f, sizeof(float));
    //printf("%f %f\n", f, fdst[i]);
    OCL_ASSERT(fabs(f - fdst[i]) <= 0.01 * fabs(fdst[i]) || (fdst[i] == 0.0 && f == 0.0));
  }
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_half_basic);

static const int half_n = 16;
static float half_test_src[half_n] = {
  -0.23455f, 1.23413f, 2.3412, 8.234f,
  -122.31f, -14.233f, 0.0023f, 99.322f,
  0.0f, 0.332f, 123.12f, -0.003f,
  16.0f, 19.22f, 128.006f, 25.032f
};

#define HALF_MATH_TEST_1ARG(NAME, CPPNAME)                              \
  void compiler_half_math_##NAME(void)                                  \
  {                                                                     \
    const size_t n = half_n;                                            \
    uint16_t hsrc[n];                                                   \
    float fsrc[n], fdst[n];                                             \
    uint32_t tmp_f;                                                     \
    float f;                                                            \
                                                                        \
    if (!cl_check_half())                                           \
      return;                                                           \
                                                                        \
    OCL_CREATE_KERNEL_FROM_FILE("compiler_half_math", "compiler_half_math_" #NAME); \
    OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint16_t), NULL);           \
    OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint16_t), NULL);           \
    OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);                            \
    OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);                            \
    globals[0] = n;                                                     \
    locals[0] = 16;                                                     \
                                                                        \
    for (int32_t i = 0; i < (int32_t) n; ++i) {                         \
      fsrc[i] = half_test_src[i];                                       \
      memcpy(&tmp_f, &fsrc[i], sizeof(float));                          \
      hsrc[i] = __float_to_half(tmp_f);                                 \
    }                                                                   \
                                                                        \
    for (int32_t i = 0; i < (int32_t) n; ++i) {                         \
      /* printf("Float is %f\n", fsrc[i]); */                           \
      fdst[i] = CPPNAME(fsrc[i]);                                       \
    }                                                                   \
                                                                        \
    OCL_MAP_BUFFER(0);                                                  \
    OCL_MAP_BUFFER(1);                                                  \
    memcpy(buf_data[0], hsrc, sizeof(hsrc));                            \
    memset(buf_data[1], 0, sizeof(hsrc));                               \
    OCL_UNMAP_BUFFER(0);                                                \
    OCL_UNMAP_BUFFER(1);                                                \
    OCL_NDRANGE(1);                                                     \
                                                                        \
    OCL_MAP_BUFFER(1);                                                  \
    for (int32_t i = 0; i < (int32_t) n; ++i) {                         \
      bool isInf, infSign;                                              \
      tmp_f = __half_to_float(((uint16_t *)buf_data[1])[i], &isInf, &infSign); \
      memcpy(&f, &tmp_f, sizeof(float));                                \
      /* printf("%.15f %.15f, diff is %f\n", f, fdst[i], (fabs(f - fdst[i])/fabs(fdst[i]))); */ \
      OCL_ASSERT(((fabs(fdst[i]) < 6e-8f) && (fabs(f) < 6e-8f)) ||      \
                 (fabs(f - fdst[i]) <= 0.03 * fabs(fdst[i])) ||         \
                 (isInf && ((infSign && fdst[i] > 65504.0f) || (!infSign && fdst[i] < -65504.0f))) || \
                 (std::isnan(f) && std::isnan(fdst[i])));               \
    }                                                                   \
    OCL_UNMAP_BUFFER(1);                                                \
  }                                                                     \
  MAKE_UTEST_FROM_FUNCTION(compiler_half_math_##NAME);

HALF_MATH_TEST_1ARG(sin, sinf);
HALF_MATH_TEST_1ARG(cos, cosf);
HALF_MATH_TEST_1ARG(sinh, sinh);
HALF_MATH_TEST_1ARG(cosh, cosh);
HALF_MATH_TEST_1ARG(tan, tanf);
HALF_MATH_TEST_1ARG(log10, log10f);
HALF_MATH_TEST_1ARG(log, logf);
HALF_MATH_TEST_1ARG(trunc, truncf);
HALF_MATH_TEST_1ARG(exp, expf);
HALF_MATH_TEST_1ARG(sqrt, sqrtf);
HALF_MATH_TEST_1ARG(ceil, ceilf);

#define HALF_MATH_TEST_2ARG(NAME, CPPNAME, RANGE_L, RANGE_H)            \
  void compiler_half_math_##NAME(void)                                  \
  {                                                                     \
    const size_t n = 16*4;                                              \
    uint16_t hsrc0[n], hsrc1[n];                                        \
    float fsrc0[n], fsrc1[n], fdst[n];                                  \
    uint32_t tmp_f;                                                     \
    float f;                                                            \
                                                                        \
    if (!cl_check_half())                                           \
      return;                                                           \
                                                                        \
    OCL_CREATE_KERNEL_FROM_FILE("compiler_half_math", "compiler_half_math_" #NAME); \
    OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint16_t), NULL);           \
    OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint16_t), NULL);           \
    OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(uint16_t), NULL);           \
    OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);                            \
    OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);                            \
    OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);                            \
    globals[0] = n;                                                     \
    locals[0] = 16;                                                     \
                                                                        \
    for (int32_t i = 0; i < (int32_t) n; ++i) {                         \
      fsrc0[i] = RANGE_L + (((RANGE_H) - (RANGE_L))/n) * i;            \
      memcpy(&tmp_f, &fsrc0[i], sizeof(float));                         \
      hsrc0[i] = __float_to_half(tmp_f);                                \
      fsrc1[i] = RANGE_L + (half_test_src[i/4] + 63) * ((RANGE_H) - (RANGE_L));            \
      memcpy(&tmp_f, &fsrc1[i], sizeof(float));                         \
      hsrc1[i] = __float_to_half(tmp_f);                                \
    }                                                                   \
                                                                        \
    for (int32_t i = 0; i < (int32_t) n; ++i) {                         \
      /* printf("Float is %f   %f\n", fsrc0[i], fsrc1[i]);*/            \
      fdst[i] = CPPNAME(fsrc0[i], fsrc1[i]);                            \
    }                                                                   \
                                                                        \
    OCL_MAP_BUFFER(0);                                                  \
    OCL_MAP_BUFFER(1);                                                  \
    OCL_MAP_BUFFER(2);                                                  \
    memcpy(buf_data[0], hsrc0, sizeof(hsrc0));                          \
    memcpy(buf_data[1], hsrc1, sizeof(hsrc1));                          \
    memset(buf_data[2], 0, sizeof(hsrc0));                              \
    OCL_UNMAP_BUFFER(0);                                                \
    OCL_UNMAP_BUFFER(1);                                                \
    OCL_UNMAP_BUFFER(2);                                                \
    OCL_NDRANGE(1);                                                     \
                                                                        \
    OCL_MAP_BUFFER(2);                                                  \
    for (int32_t i = 0; i < (int32_t) n; ++i) {                         \
    bool isInf, infSign;                                                \
    tmp_f = __half_to_float(((uint16_t *)buf_data[2])[i], &isInf, &infSign); \
    memcpy(&f, &tmp_f, sizeof(float));                                  \
    /*printf("%.15f %.15f, diff is %%%f\n", f, fdst[i], (fabs(f - fdst[i])/fabs(fdst[i]))); */ \
    OCL_ASSERT(((fabs(fdst[i]) < 6e-8f) && (fabs(f) < 6e-8f)) ||        \
               (fabs(f - fdst[i]) <= 0.03 * fabs(fdst[i])) ||           \
               (isInf && ((infSign && fdst[i] > 65504.0f) || (!infSign && fdst[i] < -65504.0f))) || \
               (std::isnan(f) && std::isnan(fdst[i])));                 \
    }                                                                   \
    OCL_UNMAP_BUFFER(2);                                                \
  }                                                                     \
  MAKE_UTEST_FROM_FUNCTION(compiler_half_math_##NAME);

HALF_MATH_TEST_2ARG(fmod, fmod, 1.0, 500.0);
HALF_MATH_TEST_2ARG(fmax, fmax, -10.0, 20.0);
HALF_MATH_TEST_2ARG(fmin, fmin, -10.0, 20.0);

void compiler_half_isnan(void)
{
  const size_t n = 16*2;
  uint16_t hsrc[n];

  if (!cl_check_half())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_half_relation", "compiler_half_isnan");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint16_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint16_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    hsrc[i] = 0xFF00;
  }

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], hsrc, sizeof(hsrc));
  memset(buf_data[1], 0, sizeof(uint16_t)*n);
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%d\n", ((uint16_t *)buf_data[1])[i]);
    OCL_ASSERT(((int16_t *)buf_data[1])[i] == -1);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_half_isnan);

void compiler_half_isinf(void)
{
  const size_t n = 16;
  uint16_t hsrc[n];

  if (!cl_check_half())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_half_relation", "compiler_half_isinf");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint16_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(int), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n/2; ++i) {
    hsrc[i] = 0x7C00;
  }
  for (int32_t i = n/2; i < (int32_t) n; ++i) {
    hsrc[i] = 0xFC00;
  }

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], hsrc, sizeof(hsrc));
  memset(buf_data[1], 0, sizeof(int)*n);
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%d\n", ((int *)buf_data[1])[i]);
    OCL_ASSERT(((int *)buf_data[1])[i] == 1);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_half_isinf);


void compiler_half_to_float(void)
{
  const size_t n = 16*4;
  uint16_t hsrc[n];
  float fdst[n];
  uint32_t tmp_f;

  if (!cl_check_half())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_half_convert", "compiler_half_to_float");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint16_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    fdst[i] = 13.1 * i;
    memcpy(&tmp_f, &fdst[i], sizeof(float));
    hsrc[i] = __float_to_half(tmp_f);
  }

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], hsrc, sizeof(hsrc));
  memset(buf_data[1], 0.0f, sizeof(fdst));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%f %f, abs is %f\n", (((float *)buf_data[1])[i]), fdst[i], fabs((((float *)buf_data[1])[i]) - fdst[i]));
    OCL_ASSERT((fabs((((float *)buf_data[1])[i]) - fdst[i]) < 0.001 * fabs(fdst[i])) ||
               (fdst[i] == 0.0 && (((float *)buf_data[1])[i]) == 0.0));
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_half_to_float);

void compiler_half_as_char2(void)
{
  const size_t n = 16;
  uint16_t hsrc[n];
  uint8_t* csrc = (uint8_t*)hsrc;

  if (!cl_check_half())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_half_convert", "compiler_half_as_char2");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint16_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint16_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    hsrc[i] = (i&0x0f)<<8 | ((i+1)&0x0f);
  }

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], hsrc, sizeof(hsrc));
  memset(buf_data[1], 0, sizeof(hsrc));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n*2; ++i) {
    //printf("%d   %d\n", (((uint8_t *)buf_data[1])[i]), csrc[i]);
    OCL_ASSERT((((uint8_t *)buf_data[1])[i]) == csrc[i]);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_half_as_char2);

void compiler_half2_as_int(void)
{
  const size_t n = 16*2;
  uint16_t hsrc[n];
  int* isrc = (int*)hsrc;

  if (!cl_check_half())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_half_convert", "compiler_half2_as_int");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint16_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint16_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    hsrc[i] = (i&0x0f)<<8 | ((i+1)&0x0f);
  }

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], hsrc, sizeof(hsrc));
  memset(buf_data[1], 0, sizeof(hsrc));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n/2; ++i) {
    //printf("%d   %d\n", (((int *)buf_data[1])[i]), isrc[i]);
    OCL_ASSERT((((int *)buf_data[1])[i]) == isrc[i]);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_half2_as_int);

void compiler_half_to_char_sat(void)
{
  const size_t n = 16;
  uint16_t hsrc[n];
  float fsrc[n];
  char dst[n];
  uint32_t tmp_f;

  if (!cl_check_half())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_half_convert", "compiler_half_to_char_sat");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint16_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(char), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    fsrc[i] = -200.1f + 30.5f * i;
    memcpy(&tmp_f, &fsrc[i], sizeof(float));
    hsrc[i] = __float_to_half(tmp_f);
    if (fsrc[i] <= -128.0f) {
      dst[i] = -128;
    } else if (fsrc[i] >= 127.0f) {
      dst[i] = 127;
    } else {
      dst[i] = (char)fsrc[i];
    }
  }

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], hsrc, sizeof(hsrc));
  memset(buf_data[1], 0, sizeof(dst));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%d     %d\n", (((char *)buf_data[1])[i]), dst[i]);
    OCL_ASSERT((((char *)buf_data[1])[i]) == dst[i]);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_half_to_char_sat);

void compiler_half_to_ushort_sat(void)
{
  const size_t n = 16;
  uint16_t hsrc[n];
  float fsrc[n];
  uint16_t dst[n];
  uint32_t tmp_f;

  if (!cl_check_half())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_half_convert", "compiler_half_to_ushort_sat");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint16_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint16_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    fsrc[i] = -100.1f + 10.3f * i;
    memcpy(&tmp_f, &fsrc[i], sizeof(float));
    hsrc[i] = __float_to_half(tmp_f);
    if (fsrc[i] <= 0.0f) {
      dst[i] = 0;
    } else {
      dst[i] = (uint16_t)fsrc[i];
    }
  }

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], hsrc, sizeof(hsrc));
  memset(buf_data[1], 0, sizeof(dst));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%u     %u\n", (((uint16_t *)buf_data[1])[i]), dst[i]);
    OCL_ASSERT((((uint16_t *)buf_data[1])[i]) == dst[i]);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_half_to_ushort_sat);

void compiler_half_to_uint_sat(void)
{
  const size_t n = 16;
  uint16_t hsrc[n];
  float fsrc[n];
  uint32_t dst[n];
  uint32_t tmp_f;

  if (!cl_check_half())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_half_convert", "compiler_half_to_uint_sat");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint16_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    fsrc[i] = -10.1f + 13.965f * i;
    memcpy(&tmp_f, &fsrc[i], sizeof(float));
    hsrc[i] = __float_to_half(tmp_f);
    if (fsrc[i] <= 0.0f) {
      dst[i] = 0;
    } else {
      dst[i] = (uint32_t)fsrc[i];
    }
  }

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], hsrc, sizeof(hsrc));
  memset(buf_data[1], 0, sizeof(dst));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%u     %u\n", (((uint32_t *)buf_data[1])[i]), dst[i]);
    OCL_ASSERT((((uint32_t *)buf_data[1])[i]) == dst[i]);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_half_to_uint_sat);

void compiler_uchar_to_half(void)
{
  const size_t n = 16;
  uint8_t hsrc[n];
  float fdst[n];
  uint32_t tmp_f;

  if (!cl_check_half())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_half_convert", "compiler_uchar_to_half");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint8_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint16_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    hsrc[i] = 5*i;
    fdst[i] = (float)hsrc[i];
  }

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], hsrc, sizeof(hsrc));
  memset(buf_data[1], 0, n*sizeof(uint16_t));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    float f;
    tmp_f = __half_to_float(((uint16_t *)buf_data[1])[i]);
    memcpy(&f, &tmp_f, sizeof(float));
    //printf("%f     %f\n", f, fdst[i]);
    OCL_ASSERT(f == fdst[i]);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_uchar_to_half);

void compiler_int_to_half(void)
{
  const size_t n = 16;
  int hsrc[n];
  float fdst[n];
  uint32_t tmp_f;

  if (!cl_check_half())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_half_convert", "compiler_int_to_half");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint16_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    hsrc[i] = 51*i;
    fdst[i] = (float)hsrc[i];
  }

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], hsrc, sizeof(hsrc));
  memset(buf_data[1], 0, n*sizeof(uint16_t));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    float f;
    tmp_f = __half_to_float(((uint16_t *)buf_data[1])[i]);
    memcpy(&f, &tmp_f, sizeof(float));
    //printf("%f     %f\n", f, fdst[i]);
    OCL_ASSERT(f == fdst[i]);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_int_to_half);

void compiler_half_to_long(void)
{
  const size_t n = 16;
  uint16_t hsrc[n];
  int64_t ldst[n];
  uint32_t tmp_f;
  float f;

  if (!cl_check_half())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_half_convert", "compiler_half_to_long");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint16_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint64_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    f = -100.1f + 10.3f * i;
    memcpy(&tmp_f, &f, sizeof(float));
    hsrc[i] = __float_to_half(tmp_f);
    ldst[i] = (int64_t)f;
  }

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], hsrc, sizeof(hsrc));
  memset(buf_data[1], 0, n*sizeof(uint64_t));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%ld	   %ld\n", (((int64_t *)buf_data[1])[i]), ldst[i]);
    OCL_ASSERT((((int64_t *)buf_data[1])[i]) == ldst[i]);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_half_to_long);

void compiler_ulong_to_half(void)
{
  const size_t n = 16;
  uint64_t src[n];
  float fdst[n];
  uint32_t tmp_f;
  float f;

  if (!cl_check_half())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_half_convert", "compiler_ulong_to_half");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint64_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint16_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    src[i] = 10 + 126*i;
    fdst[i] = (float)src[i];
  }

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], src, sizeof(src));
  memset(buf_data[1], 0, n*sizeof(uint16_t));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    tmp_f = __half_to_float(((uint16_t *)buf_data[1])[i]);
    memcpy(&f, &tmp_f, sizeof(float));
    //printf("%f    %f\n", f, fdst[i]);
    OCL_ASSERT(f == fdst[i]);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_ulong_to_half);

void compiler_half_to_long_sat(void)
{
  const size_t n = 16;
  uint16_t hsrc[n];
  int64_t ldst[n];
  uint32_t tmp_f;
  float f;

  if (!cl_check_half())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_half_convert", "compiler_half_to_long_sat");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint16_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint64_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 1; i < (int32_t) n-1; ++i) {
    f = -100.1f + 10.3f * i;
    memcpy(&tmp_f, &f, sizeof(float));
    hsrc[i] = __float_to_half(tmp_f);
    ldst[i] = (int64_t)f;
  }
  hsrc[0] = 0xFC00; //-inf;
  ldst[0] = 0x8000000000000000;
  hsrc[n-1] = 0x7C00; //inf;
  ldst[n-1] = 0x7FFFFFFFFFFFFFFF;

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], hsrc, sizeof(hsrc));
  memset(buf_data[1], 0, n*sizeof(uint64_t));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    //printf("%lx	   %lx\n", (((int64_t *)buf_data[1])[i]), ldst[i]);
    OCL_ASSERT((((int64_t *)buf_data[1])[i]) == ldst[i]);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_half_to_long_sat);

void compiler_half_to_double(void)
{
  const size_t n = 16;
  uint16_t hsrc[n];
  double ddst[n];
  uint32_t tmp_f;
  float f;

//  if (!cl_check_half())
//    return;
  if (!cl_check_double())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_half_convert", "compiler_half_to_double");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint16_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(double), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    f = -100.1f + 10.3f * i;
    memcpy(&tmp_f, &f, sizeof(float));
    hsrc[i] = __float_to_half(tmp_f);
    ddst[i] = (double)f;
  }

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], hsrc, sizeof(hsrc));
  memset(buf_data[1], 0, n*sizeof(double));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    double dd = ((double *)(buf_data[1]))[i];
//    printf("%f	   %f, diff is %%%f\n", dd, ddst[i], fabs(dd - ddst[i])/fabs(ddst[i]));
    OCL_ASSERT(fabs(dd - ddst[i]) < 0.001f * fabs(ddst[i]));
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_half_to_double);

void compiler_double_to_half(void)
{
  const size_t n = 16;
  uint16_t hdst[n];
  double src[n];
  uint32_t tmp_f;
  float f;

//  if (!cl_check_half())
//    return;
  if (!cl_check_double())
    return;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_half_convert", "compiler_double_to_half");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(double), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint16_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  for (int32_t i = 0; i < (int32_t) n; ++i) {
    f = -100.1f + 10.3f * i;
    src[i] = (double)f;
    memcpy(&tmp_f, &f, sizeof(float));
    hdst[i] = __float_to_half(tmp_f);
  }

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], src, sizeof(src));
  memset(buf_data[1], 0, n*sizeof(uint16_t));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    uint16_t hf = ((uint16_t *)(buf_data[1]))[i];
    //tmp_f = __half_to_float(hf);
    //memcpy(&f, &tmp_f, sizeof(float));
    //printf("%f, %x, %x\n", f, hf, hdst[i]);
    OCL_ASSERT(hf == hdst[i]);
  }
  OCL_UNMAP_BUFFER(1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_double_to_half);

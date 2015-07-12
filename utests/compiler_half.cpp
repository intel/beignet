#include <cstdint>
#include <cstring>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "utest_helper.hpp"

static uint32_t __half_to_float(uint16_t h, bool* isInf = NULL, bool* infSign = NULL)
{
  struct __FP32 {
    uint32_t mantissa:23;
    uint32_t exponent:8;
    uint32_t sign:1;
  };
  struct __FP16 {
    uint32_t mantissa:10;
    uint32_t exponent:5;
    uint32_t sign:1;
  };
  uint32_t f;
  __FP32 o;
  memset(&o, 0, sizeof(o));
  __FP16 i;
  memcpy(&i, &h, sizeof(uint16_t));

  if (isInf)
    *isInf = false;
  if (infSign)
    *infSign = false;

  if (i.exponent == 0 && i.mantissa == 0) // (Signed) zero
    o.sign = i.sign;
  else {
    if (i.exponent == 0) { // Denormal (converts to normalized)
      // Adjust mantissa so it's normalized (and keep
      // track of exponent adjustment)
      int e = -1;
      uint m = i.mantissa;
      do {
        e++;
        m <<= 1;
      } while ((m & 0x400) == 0);

      o.mantissa = (m & 0x3ff) << 13;
      o.exponent = 127 - 15 - e;
      o.sign = i.sign;
    } else if (i.exponent == 0x1f) { // Inf/NaN
      // NOTE: Both can be handled with same code path
      // since we just pass through mantissa bits.
      o.mantissa = i.mantissa << 13;
      o.exponent = 255;
      o.sign = i.sign;

      if (isInf) {
        *isInf = (i.mantissa == 0);
        if (infSign)
          *infSign = !i.sign;
      }
    } else { // Normalized number
      o.mantissa = i.mantissa << 13;
      o.exponent = 127 - 15 + i.exponent;
      o.sign = i.sign;
    }
  }

  memcpy(&f, &o, sizeof(uint32_t));
  return f;
}


static uint16_t __float_to_half(uint32_t x)
{
  uint16_t bits = (x >> 16) & 0x8000; /* Get the sign */
  uint16_t m = (x >> 12) & 0x07ff; /* Keep one extra bit for rounding */
  unsigned int e = (x >> 23) & 0xff; /* Using int is faster here */

  /* If zero, or denormal, or exponent underflows too much for a denormal
   * half, return signed zero. */
  if (e < 103)
    return bits;

  /* If NaN, return NaN. If Inf or exponent overflow, return Inf. */
  if (e > 142) {
    bits |= 0x7c00u;
    /* If exponent was 0xff and one mantissa bit was set, it means NaN,
     * not Inf, so make sure we set one mantissa bit too. */
    bits |= e == 255 && (x & 0x007fffffu);
    return bits;
  }

  /* If exponent underflows but not too much, return a denormal */
  if (e < 113) {
    m |= 0x0800u;
    /* Extra rounding may overflow and set mantissa to 0 and exponent
     * to 1, which is OK. */
    bits |= (m >> (114 - e)) + ((m >> (113 - e)) & 1);
    return bits;
  }

  bits |= ((e - 112) << 10) | (m >> 1);
  /* Extra rounding. An overflow will set mantissa to 0 and increment
   * the exponent, which is OK. */
  bits += m & 1;
  return bits;
}

static int check_half_device(void)
{
  std::string extStr;
  size_t param_value_size;
  OCL_CALL(clGetDeviceInfo, device, CL_DEVICE_EXTENSIONS, 0, 0, &param_value_size);
  std::vector<char> param_value(param_value_size);
  OCL_CALL(clGetDeviceInfo, device, CL_DEVICE_EXTENSIONS, param_value_size,
           param_value.empty() ? NULL : &param_value.front(), &param_value_size);
  if (!param_value.empty())
    extStr = std::string(&param_value.front(), param_value_size-1);

  if (std::strstr(extStr.c_str(), "cl_khr_fp16") == NULL) {
    printf("No cl_khr_fp16, Skip!");
    return 0;
  }

  return 1;
}

void compiler_half_basic(void)
{
  const size_t n = 16;
  uint16_t hsrc[n];
  float fsrc[n], fdst[n];
  float f = 2.5;
  uint32_t tmp_f;

  if (!check_half_device())
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
    printf("%f %f\n", f, fdst[i]);
    OCL_ASSERT(fabs(f - fdst[i]) <= 0.01 * fabs(fdst[i]) || (fdst[i] == 0.0 && f == 0.0));
  }
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_half_basic);


#define HALF_MATH_TEST_1ARG(NAME, CPPNAME, RANGE_L, RANGE_H)            \
  void compiler_half_math_##NAME(void)                                  \
  {                                                                     \
    const size_t n = 16;                                                \
    uint16_t hsrc[n];                                                   \
    float fsrc[n], fdst[n];                                             \
    uint32_t tmp_f;                                                     \
    float f;                                                            \
                                                                        \
    if (!check_half_device())                                           \
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
      fsrc[i] = RANGE_L + ((rand()%1000) / 1000.0f ) * ((RANGE_H) - (RANGE_L)); \
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
      /*printf("%.15f %.15f, diff is %%%f\n", f, fdst[i], (fabs(f - fdst[i])/fabs(fdst[i]))); */ \
      OCL_ASSERT(((fabs(fdst[i]) < 6e-8f) && (fabs(f) < 6e-8f)) ||      \
                 (fabs(f - fdst[i]) <= 0.03 * fabs(fdst[i])) ||         \
                 (isInf && ((infSign && fdst[i] > 65504.0f) || (!infSign && fdst[i] < -65504.0f))) || \
                 (isnan(f) && isnan(fdst[i])));                         \
    }                                                                   \
    OCL_UNMAP_BUFFER(1);                                                \
  }                                                                     \
  MAKE_UTEST_FROM_FUNCTION(compiler_half_math_##NAME);

HALF_MATH_TEST_1ARG(sin, sinf, -10, 10);
HALF_MATH_TEST_1ARG(cos, cosf, -10, 10);
HALF_MATH_TEST_1ARG(sinh, sinh, -10, 10);
HALF_MATH_TEST_1ARG(cosh, cosh, -10, 10);
HALF_MATH_TEST_1ARG(tan, tanf, -3.14/2, 3.14/2);
HALF_MATH_TEST_1ARG(log10, log10f, 0.1, 100);
HALF_MATH_TEST_1ARG(log, logf, 0.01, 1000);
HALF_MATH_TEST_1ARG(trunc, truncf, -1000, 1000);
HALF_MATH_TEST_1ARG(exp, expf, -19.0, 20.0);
HALF_MATH_TEST_1ARG(sqrt, sqrtf, -19.0, 10.0);
HALF_MATH_TEST_1ARG(ceil, ceilf, -19.0, 20.0);

#define HALF_MATH_TEST_2ARG(NAME, CPPNAME, RANGE_L, RANGE_H)            \
  void compiler_half_math_##NAME(void)                                  \
  {                                                                     \
    const size_t n = 16*4;                                              \
    uint16_t hsrc0[n], hsrc1[n];                                        \
    float fsrc0[n], fsrc1[n], fdst[n];                                  \
    uint32_t tmp_f;                                                     \
    float f;                                                            \
                                                                        \
    if (!check_half_device())                                           \
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
      fsrc1[i] = RANGE_L + ((rand()%1000) / 1000.0f ) * ((RANGE_H) - (RANGE_L));            \
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
               (isnan(f) && isnan(fdst[i])));                           \
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

  if (!check_half_device())
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

  if (!check_half_device())
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

  if (!check_half_device())
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

  if (!check_half_device())
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

  if (!check_half_device())
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

  if (!check_half_device())
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

  if (!check_half_device())
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

  if (!check_half_device())
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

  if (!check_half_device())
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

  if (!check_half_device())
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

  if (!check_half_device())
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

  if (!check_half_device())
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

  if (!check_half_device())
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

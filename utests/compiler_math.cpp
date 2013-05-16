#include "utest_helper.hpp"
#include <cmath>
#include <algorithm>

static void cpu_compiler_math(float *dst, float *src, int i)
{
  const float x = src[i];
  const float PI = 3.141592653589793f;
  switch (i) {
    case 0: dst[i] = cosf(x); break;
    case 1: dst[i] = sinf(x); break;
    case 2: dst[i] = log2f(x); break;
    case 3: dst[i] = sqrtf(x); break;
    case 4: dst[i] = 1.f/ sqrtf(x); break;
    case 5: dst[i] = 1.f / x; break;
    case 6: dst[i] = tanf(x); break;
    case 7: dst[i] = powf(x, 0.3333333333333333333f); break;
    case 8: dst[i] = ceilf(x); break;
    case 9: dst[i] = cosf(PI * x); break;
    case 10: dst[i] = powf(2, x); break;
    case 11: dst[i] = powf(10, x); break;
    case 12: dst[i] = expf(x) - 1; break;
    case 13: dst[i] = logf(x + 1); break;
    case 14: dst[i] = floorf(log2f(x)); break;
    case 15: dst[i] = sinf(PI * x); break;
    case 16: dst[i] = tanf(PI * x); break;
    case 17: dst[i] = 2 * roundf(x / 2); break;
    case 18: dst[i] = sinhf(x); break;
    case 19: dst[i] = coshf(x); break;
    case 20: dst[i] = tanhf(x); break;
    case 21: dst[i] = asinhf(x); break;
    case 22: dst[i] = acoshf(x); break;
    case 23: dst[i] = atanhf(x); break;
    case 24: dst[i] = asinf(x); break;
    case 25: dst[i] = acosf(x); break;
    case 26: dst[i] = atanf(x); break;
    case 27: dst[i] = asinf(x) / PI; break;
    case 28: dst[i] = acosf(x) / PI; break;
    case 29: dst[i] = atanf(x) / PI; break;
    case 30: dst[i] = erff(x); break;
    case 31: dst[i] = nanf(""); break;
    default: dst[i] = 1.f; break;
  };
}

static void compiler_math(void)
{
  const size_t n = 32;
  float cpu_dst[32], cpu_src[32];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_math");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = 16;
  locals[0] = 16;

  int j;
  for(j = 0; j < 1000; j ++) {
    OCL_MAP_BUFFER(1);
    for (uint32_t i = 0; i < 32; ++i)
      cpu_src[i] = ((float*)buf_data[1])[i] = .1f * (rand() & 15);
    OCL_UNMAP_BUFFER(1);
    OCL_NDRANGE(1);

    OCL_MAP_BUFFER(0);
    OCL_MAP_BUFFER(1);
    for (int i = 0; i < 16; ++i)
      cpu_compiler_math(cpu_dst, cpu_src, i);
    for (int i = 0; i < 16; ++i) {
      const float cpu = cpu_dst[i];
      const float gpu = ((float*)buf_data[0])[i];
      if (isinf(cpu))
        OCL_ASSERT(isinf(gpu));
      else if (isnan(cpu))
        OCL_ASSERT(isnan(gpu));
      else
        OCL_ASSERT(fabs(gpu-cpu) < 1e-3f);
    }
    OCL_UNMAP_BUFFER(0);
    OCL_UNMAP_BUFFER(1);
  }
}

MAKE_UTEST_FROM_FUNCTION(compiler_math)



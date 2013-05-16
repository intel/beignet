#include "utest_helper.hpp"
#include <cmath>
#include <algorithm>

static float rnde(float v) {
  if(v - floorf(v) > 0.5f)
    return floorf(v) + 1;
  if(v - floorf(v) < 0.5f)
    return floorf(v);
  if((int)(floorf(v)) & 1)
    return floorf(v) + 1;
  return floorf(v);
}

static void cpu_compiler_math(float *dst, float *src1, float *src2, int i)
{
  const float x = src1[i], y = src2[i];
  switch (i) {
    case 0: dst[i] = x / y; break;
    case 1: dst[i] = x > y ? x - y : 0; break;
    case 2: dst[i] = fminf(x - floorf(x), 0x1.FFFFFep-1F); break;
    case 3: dst[i] = sqrtf(x*x + y*y); break;
    case 4: dst[i] = x * powf(2, (int)y); break;
    case 5: dst[i] = powf(x, (int)y); break;
    case 6: dst[i] = x - rnde(x/y)*y; break;
    case 7: dst[i] = powf(x, 1.f/(int)(y+1)); break;
    case 8: dst[i] = x * y < 0 ? -x : x; break;
    case 9: dst[i] = fabsf(x) > fabsf(y) ? x : fabsf(y) > fabsf(x) ? y : fmaxf(x, y); break;
    case 10: dst[i] = fabsf(x) < fabsf(y) ? x : fabsf(y) < fabsf(x) ? y : fminf(x, y); break;
    default: dst[i] = 1.f; break;
  };
}

static void compiler_math_2op(void)
{
  const size_t n = 32;
  float cpu_dst[32], cpu_src1[32], cpu_src2[32];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_math_2op");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = 16;
  locals[0] = 16;

  int j;
  for(j = 0; j < 1000; j ++) {
    OCL_MAP_BUFFER(1);
    OCL_MAP_BUFFER(2);
    for (uint32_t i = 0; i < 32; ++i) {
      cpu_src1[i] = ((float*)buf_data[1])[i] = .1f * (rand() & 15);
      cpu_src2[i] = ((float*)buf_data[2])[i] = .1f * (rand() & 15);
    }
    OCL_UNMAP_BUFFER(1);
    OCL_UNMAP_BUFFER(2);
    OCL_NDRANGE(1);

    for (int i = 0; i < 16; ++i)
      cpu_compiler_math(cpu_dst, cpu_src1, cpu_src2, i);
    OCL_MAP_BUFFER(0);
    for (int i = 0; i < 16; ++i) {
      const float cpu = cpu_dst[i];
      const float gpu = ((float*)buf_data[0])[i];
      if (isinf(cpu))
        OCL_ASSERT(isinf(gpu));
      else if (isnan(cpu))
        OCL_ASSERT(isnan(gpu));
      else {
        OCL_ASSERT(fabs(gpu-cpu) < 1e-3f);
      }
    }
    OCL_UNMAP_BUFFER(0);
  }
}

MAKE_UTEST_FROM_FUNCTION(compiler_math_2op)

#include "utest_helper.hpp"
#include <cmath>
#include <algorithm>

static void cpu_compiler_math(float *dst, float *src1, float *src2, float *src3, int i)
{
  const float x = src1[i], y = src2[i], z = src3[i];
  switch (i%2) {
    case 0: dst[i] = x * y + z; break;
    case 1: dst[i] = x * y + z; break;
    default: dst[i] = 1.f; break;
  };
  dst[0] = (src1[0]*src2[0]+src3[0]);
}

static void compiler_math_3op(void)
{
  const size_t n = 32;
  float cpu_dst[32], cpu_src1[32], cpu_src2[32], cpu_src3[32];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_math_3op");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[3], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(3, sizeof(cl_mem), &buf[3]);
  globals[0] = 16;
  locals[0] = 16;

  for (int j = 0; j < 1000; j ++) {
    OCL_MAP_BUFFER(1);
    OCL_MAP_BUFFER(2);
    OCL_MAP_BUFFER(3);
    for (uint32_t i = 0; i < 32; ++i) {
      cpu_src1[i] = ((float*)buf_data[1])[i] = .001f * (rand() & 15);
      cpu_src2[i] = ((float*)buf_data[2])[i] = .002f * (rand() & 15);
      cpu_src3[i] = ((float*)buf_data[3])[i] = .003f * (rand() & 15);
    }
    OCL_UNMAP_BUFFER(1);
    OCL_UNMAP_BUFFER(2);
    OCL_UNMAP_BUFFER(3);
    OCL_NDRANGE(1);

    for (int i = 0; i < 16; ++i)
      cpu_compiler_math(cpu_dst, cpu_src1, cpu_src2, cpu_src3, i);
    OCL_MAP_BUFFER(0);
    for (int i = 0; i < 16; ++i) {
      const float cpu = cpu_dst[i];
      const float gpu = ((float*)buf_data[0])[i];
      //printf("cpu:%f, gpu:%f\n", cpu, gpu);
      if (std::isinf(cpu))
        OCL_ASSERT(std::isinf(gpu));
      else if (std::isnan(cpu))
        OCL_ASSERT(std::isnan(gpu));
      else
        OCL_ASSERT(fabs(gpu-cpu) < 1e-3f);
    }
    OCL_UNMAP_BUFFER(0);
  }
}

MAKE_UTEST_FROM_FUNCTION(compiler_math_3op)

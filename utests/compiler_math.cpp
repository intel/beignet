#include "utest_helper.hpp"
#include <cmath>
#include <algorithm>

static void cpu_compiler_math(float *dst, float *src, int get_global_id0)
{
  const float x = src[get_global_id0];
  switch (get_global_id0) {
    case 0: dst[get_global_id0] = cosf(x); break;
    case 1: dst[get_global_id0] = sinf(x); break;
    case 2: dst[get_global_id0] = log2f(x); break;
    case 3: dst[get_global_id0] = sqrtf(x); break;
    case 4: dst[get_global_id0] = 1.f/ sqrtf(x); break;
    case 5: dst[get_global_id0] = 1.f / x; break;
    case 6: dst[get_global_id0] = tanf(x); break;
    default: dst[get_global_id0] = 1.f; break;
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

  OCL_MAP_BUFFER(1);
  for (uint32_t i = 0; i < 32; ++i)
    cpu_src[i] = ((float*)buf_data[1])[i] = float(i);
  OCL_UNMAP_BUFFER(1);
  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  for (int i = 0; i < 16; ++i)
    cpu_compiler_math(cpu_dst, cpu_src, i);
  for (int i = 0; i < 16; ++i) {
    const float cpu = cpu_dst[i];
    const float gpu = ((float*)buf_data[0])[i];
    OCL_ASSERT(fabs(gpu-cpu)/std::max(fabs(cpu), fabs(gpu)) < 1e-4f);
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_math)



#include "utest_helper.hpp"
#include <algorithm>

static void compiler_insn_selection_masked_min_max(void)
{
  const size_t n = 256;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_insn_selection_masked_min_max");
  buf_data[0] = (uint32_t*) malloc(sizeof(uint32_t) * n);
  for (uint32_t i = 0; i < n; ++i)
    ((float*)buf_data[0])[i] = float(i);
  OCL_CREATE_BUFFER(buf[0], CL_MEM_COPY_HOST_PTR, n * sizeof(uint32_t), buf_data[0]);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  free(buf_data[0]);
  buf_data[0] = NULL;

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  // Check result
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  float *dst = (float*)buf_data[1];
  float *src = (float*)buf_data[0];
  for (uint32_t i = 0; i < n; ++i) {
    float cpu_dst;
    if (i % 16 > 5)
      cpu_dst = std::max(src[i], src[7]);
    else
      cpu_dst = std::min(src[i], src[10]);
    OCL_ASSERT(dst[i] == cpu_dst);
  }
}

MAKE_UTEST_FROM_FUNCTION(compiler_insn_selection_masked_min_max)



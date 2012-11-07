#include "utest_helper.hpp"
#include <algorithm>

static void compiler_insn_selection_min(void)
{
  const size_t n = 8192 * 4;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_insn_selection_min");
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
    OCL_ASSERT(dst[i] == std::min(src[i], src[0]));
  }
}

MAKE_UTEST_FROM_FUNCTION(compiler_insn_selection_min)


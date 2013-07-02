#include "utest_helper.hpp"

static void compiler_global_memory_barrier(void)
{
  const size_t n = 16*1024;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_global_memory_barrier");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

  // Run the kernel
  globals[0] = n/2;
  locals[0] = 256;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(0);

  // Check results
  uint32_t *dst = (uint32_t*)buf_data[0];
  for (uint32_t i = 0; i < n; i+=locals[0])
    for (uint32_t j = 0; j < locals[0]; ++j)
        OCL_ASSERT(dst[i+j] == locals[0] - 1 -j);
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_global_memory_barrier);

#include "utest_helper.hpp"

static void compiler_local_memory_barrier_2(void)
{
  const size_t n = 16*1024;

  globals[0] = n/2;
  locals[0] = 256;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_local_memory_barrier_2");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  //OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, locals[0] * 2 * sizeof(uint32_t), NULL);

  // Run the kernel
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(0);

  // Check results
  uint32_t *dst = (uint32_t*)buf_data[0];
  for (uint32_t i = 0; i < n; i+=locals[0])
    for (uint32_t j = 0; j < locals[0]; ++j)
        OCL_ASSERT(dst[i+j] == locals[0] - 1 -j);
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_local_memory_barrier_2);

#include "utest_helper.hpp"

static void compiler_async_copy(void)
{
  const size_t n = 1024;
  const size_t local_size = 32;
  const int copiesPerWorkItem = 5;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_async_copy");
  OCL_CREATE_BUFFER(buf[0], 0, n * copiesPerWorkItem * sizeof(int) * 2, NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * copiesPerWorkItem * sizeof(int) * 2, NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, local_size*copiesPerWorkItem*sizeof(int)*2, NULL);
  OCL_SET_ARG(3, sizeof(int), &copiesPerWorkItem);

  OCL_MAP_BUFFER(1);
  for (uint32_t i = 0; i < n * copiesPerWorkItem * 2; ++i)
      ((int*)buf_data[1])[i] = rand();
  OCL_UNMAP_BUFFER(1);

  // Run the kernel
  globals[0] = n;
  locals[0] = local_size;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);

  // Check results
  int *dst = (int*)buf_data[0];
  int *src = (int*)buf_data[1];
  for (uint32_t i = 0; i < n * copiesPerWorkItem * 2; i++)
    OCL_ASSERT(dst[i] == src[i] + 3);
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_async_copy);

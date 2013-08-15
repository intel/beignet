#include "utest_helper.hpp"

static void compiler_load_bool_imm(void)
{
  const size_t n = 1024;
  const size_t local_size = 16;
  const int copiesPerWorkItem = 5;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_load_bool_imm");
  OCL_CREATE_BUFFER(buf[0], 0, n * copiesPerWorkItem * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, local_size*copiesPerWorkItem*sizeof(int), NULL); // 16 x int
  OCL_SET_ARG(2, sizeof(int), &copiesPerWorkItem); // 16 x int

  // Run the kernel
  globals[0] = n;
  locals[0] = local_size;
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(0);

  // Check results
  int *dst = (int*)buf_data[0];
  for (uint32_t i = 0; i < n * copiesPerWorkItem; i++)
    OCL_ASSERT(dst[i] == copiesPerWorkItem);
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_load_bool_imm);

#include "utest_helper.hpp"

void compiler_get_max_sub_group_size(void)
{
  if(!cl_check_subgroups())
    return;
  const size_t n = 256;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_get_max_sub_group_size");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  globals[0] = n;
  locals[0] = 16;

  OCL_MAP_BUFFER(0);
  for (int32_t i = 0; i < (int32_t) n; ++i)
    ((int*)buf_data[0])[i] = -1;
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(0);
  int* dst = (int *)buf_data[0];
  for (int32_t i = 0; i < (int32_t) n; ++i){
    OCL_ASSERT(8 == dst[i] || 16 == dst[i] || 32 == dst[i]);
  }
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_get_max_sub_group_size);

#include "utest_helper.hpp"

static void compiler_lower_return1(void)
{
  const size_t n = 32;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_lower_return1");
  buf_data[0] = (uint32_t*) malloc(sizeof(uint32_t) * n);
  for (uint32_t i = 0; i < n; ++i) ((uint32_t*)buf_data[0])[i] = 2;
  OCL_CREATE_BUFFER(buf[0], CL_MEM_COPY_HOST_PTR, n * sizeof(uint32_t), buf_data[0]);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  free(buf_data[0]);
  buf_data[0] = NULL;

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = 16;
  locals[0] = 16;
  OCL_NDRANGE(1);

  // First control flow
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < 11; ++i)
    OCL_ASSERT(((int32_t*)buf_data[1])[i] == i);
  for (int32_t i = 11; i < 16; ++i)
    OCL_ASSERT(((int32_t*)buf_data[1])[i] == 2);

  // Second control flow
  for (uint32_t i = 0; i < 4; ++i) ((int32_t*)buf_data[0])[i] = -2;
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_NDRANGE(1);
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < 4; ++i)
    OCL_ASSERT(((int32_t*)buf_data[1])[i] == -2);
  for (int32_t i = 4; i < 11; ++i)
    OCL_ASSERT(((int32_t*)buf_data[1])[i] == i);
  for (int32_t i = 11; i < 16; ++i)
    OCL_ASSERT(((int32_t*)buf_data[1])[i] == 2);
}

MAKE_UTEST_FROM_FUNCTION(compiler_lower_return1);


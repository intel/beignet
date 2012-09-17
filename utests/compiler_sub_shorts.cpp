#include "utest_helper.hpp"

static void compiler_sub_shorts(void)
{
  const size_t n = 16;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_sub_shorts");
  buf_data[0] = (int16_t*) malloc(sizeof(int16_t) * n);
  buf_data[1] = (int16_t*) malloc(sizeof(int16_t) * n);
  for (uint32_t i = 0; i < n; ++i) ((int16_t*)buf_data[0])[i] = (int16_t) rand();
  for (uint32_t i = 0; i < n; ++i) ((int16_t*)buf_data[1])[i] = (int16_t) rand();
  OCL_CREATE_BUFFER(buf[0], CL_MEM_COPY_HOST_PTR, n * sizeof(int16_t), buf_data[0]);
  OCL_CREATE_BUFFER(buf[1], CL_MEM_COPY_HOST_PTR, n * sizeof(int16_t), buf_data[0]);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(int16_t), NULL);

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  // Check result
  OCL_MAP_BUFFER(2);
  for (uint32_t i = 0; i < n; ++i)
    OCL_ASSERT(((int16_t*)buf_data[2])[i] = ((int16_t*)buf_data[0])[i] - ((int16_t*)buf_data[1])[i]);
  free(buf_data[0]);
  free(buf_data[1]);
  buf_data[0] = buf_data[1] = NULL;
}

MAKE_UTEST_FROM_FUNCTION(compiler_sub_shorts);



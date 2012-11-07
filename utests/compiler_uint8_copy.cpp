#include "utest_helper.hpp"

static void compiler_uint8_copy(void)
{
  const size_t n = 128;

  // Setup kernel and buffers. Note that uint8 is aligned on 16 bytes
  // according to the OCL specification
  OCL_CREATE_KERNEL("compiler_uint8_copy");
  buf_data[0] = (uint32_t*) malloc(sizeof(uint32_t[8]) * n);
  for (uint32_t i = 0; i < n; ++i)
    for (uint32_t j = 0; j < 8; ++j)
      ((uint32_t*)buf_data[0])[8*i+j] = 8*i+j;

  OCL_CREATE_BUFFER(buf[0], CL_MEM_COPY_HOST_PTR, n * sizeof(uint32_t[8]), buf_data[0]);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t[8]), NULL);
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
  for (uint32_t i = 0; i < 8*n; ++i)
    OCL_ASSERT(((uint32_t*)buf_data[0])[i] == ((uint32_t*)buf_data[1])[i]);
}

MAKE_UTEST_FROM_FUNCTION(compiler_uint8_copy);


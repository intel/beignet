#include "utest_helper.hpp"

static void compiler_uint16_copy(void)
{
  const size_t n = 128;

  // Setup kernel and buffers. Note that uint16 is aligned on 16 bytes
  // according to the OCL specificatio
  OCL_CREATE_KERNEL("compiler_uint16_copy");
  buf_data[0] = (uint32_t*) malloc(sizeof(uint32_t[16]) * n);
  for (uint32_t i = 0; i < n; ++i)
    for (uint32_t j = 0; j < 16; ++j)
      ((uint32_t*)buf_data[0])[16*i+j] = 16*i+j;

  OCL_CREATE_BUFFER(buf[0], CL_MEM_COPY_HOST_PTR, n * sizeof(uint32_t[16]), buf_data[0]);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t[16]), NULL);
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
  for (uint32_t i = 0; i < 16*n; ++i)
    OCL_ASSERT(((uint32_t*)buf_data[0])[i] == ((uint32_t*)buf_data[1])[i]);
}

MAKE_UTEST_FROM_FUNCTION(compiler_uint16_copy);


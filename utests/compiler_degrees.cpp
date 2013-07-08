#include "utest_helper.hpp"

void compiler_degrees(void)
{
  const int n = 32;
  float src[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_degrees");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n;
  locals[0] = 16;

  OCL_MAP_BUFFER(0);
  for (int i = 0; i < n; ++i) {
    src[i] = ((float *)buf_data[0])[i] = rand() * 0.01f;
  }
  OCL_UNMAP_BUFFER(0);

  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(1);
  for (int i = 0; i < n; ++i) {
    OCL_ASSERT(((float *)buf_data[1])[i] == src[i] * (180 / 3.141592653589793F));
  }
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_degrees);

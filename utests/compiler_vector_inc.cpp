#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"

void compiler_vector_inc(void)
{
  const int n = 64;
  char dst[n];
  char src[n];

  OCL_CREATE_KERNEL("compiler_vector_inc");
  OCL_CREATE_BUFFER(buf[0], 0, n, NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n, NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n / 2;
  locals[0] = 16;

  for (int i = 0; i < n; ++i) {
    dst[i] = i;
    src[i] = (i / 2) % 4;
  }
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], dst, n);
  memcpy(buf_data[1], src, n);
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(0);
  char *dest = ((char *)buf_data[0]);
  for (int i=0; i<n; ++i) {
    char wish;
    if (src[i/2] < 2)
      wish = dst[i] + 1;
    else
      wish = dst[i] - 1;
    OCL_ASSERT(dest[i] == wish);
  }
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION_WITH_ISSUE(compiler_vector_inc);

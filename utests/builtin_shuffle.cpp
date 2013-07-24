#include "utest_helper.hpp"

void builtin_shuffle(void)
{
  const int n = 32;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("builtin_shuffle");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[3], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(3, sizeof(cl_mem), &buf[3]);
  globals[0] = n;
  locals[0] = 16;

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  for (int i = 0; i < n; i ++) {
    ((float *)(buf_data[0]))[i] = rand();
    ((float *)(buf_data[1]))[i] = rand();
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  OCL_MAP_BUFFER(3);
  for (int i = 0; i < n; i ++) {
    OCL_ASSERT(((float *)(buf_data[0]))[i] == ((float *)(buf_data[3]))[i]);
    OCL_ASSERT(((float *)(buf_data[1]))[i] == ((float *)(buf_data[2]))[i]);
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);
  OCL_UNMAP_BUFFER(3);
}

MAKE_UTEST_FROM_FUNCTION(builtin_shuffle);

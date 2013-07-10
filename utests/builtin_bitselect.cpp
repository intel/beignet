#include "utest_helper.hpp"

int as_int(float f) {
  void *p = &f;
  return *(int *)p;
}

int cpu(int a, int b, int c) {
  return (a & ~c) | (b & c);
}

void builtin_bitselect(void)
{
  const int n = 32;
  float src1[n], src2[n], src3[n];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("builtin_bitselect");
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
  OCL_MAP_BUFFER(2);
  for (int i = 0; i < n; ++i) {
    src1[i] = ((float*)buf_data[0])[i] = rand() * 0.1f;
    src2[i] = ((float*)buf_data[1])[i] = rand() * 0.1f;
    src3[i] = ((float*)buf_data[2])[i] = rand() * 0.1f;
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);

  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(3);
  for (int i = 0; i < n; ++i)
    OCL_ASSERT(((int*)buf_data[3])[i] == cpu(as_int(src1[i]), as_int(src2[i]), as_int(src3[i])));
  OCL_UNMAP_BUFFER(3);
}

MAKE_UTEST_FROM_FUNCTION(builtin_bitselect);

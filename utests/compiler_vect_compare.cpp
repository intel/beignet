#include "utest_helper.hpp"

typedef struct {
  int x;
  int y;
  int z;
  int w;
} int4;

void compiler_vect_compare(void)
{
  const size_t n = 16;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_vect_compare");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int4), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(int4), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

  OCL_MAP_BUFFER(0);
  for (uint32_t i = 0; i < n; ++i) {
    ((int4*)buf_data[0])[i].x = i & 0x1;
    ((int4*)buf_data[0])[i].y = i & 0x2;
    ((int4*)buf_data[0])[i].z = i & 0x4;
    ((int4*)buf_data[0])[i].w = i & 0x8;
  }
  OCL_UNMAP_BUFFER(0);

  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(1);
  for (uint32_t i = 0; i < 16; ++i) {
    OCL_ASSERT(((int4*)buf_data[1])[i].x == (int)((i&0x1)?0xffffffff:0));
    OCL_ASSERT(((int4*)buf_data[1])[i].y == (int)((i&0x2)?0xffffffff:0));
    OCL_ASSERT(((int4*)buf_data[1])[i].z == (int)((i&0x4)?0xffffffff:0));
    OCL_ASSERT(((int4*)buf_data[1])[i].w == (int)((i&0x8)?0xffffffff:0));
  }
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_vect_compare);

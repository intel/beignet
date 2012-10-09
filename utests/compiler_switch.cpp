#include "utest_helper.hpp"

static void cpu_compiler_switch(int *dst, int *src, int get_global_id0)
{
  switch (get_global_id0) {
    case 0: dst[get_global_id0] = src[get_global_id0 + 4]; break;
    case 1: dst[get_global_id0] = src[get_global_id0 + 14]; break;
    case 2: dst[get_global_id0] = src[get_global_id0 + 13]; break;
    case 6: dst[get_global_id0] = src[get_global_id0 + 11]; break;
    case 7: dst[get_global_id0] = src[get_global_id0 + 10]; break;
    case 10: dst[get_global_id0] = src[get_global_id0 + 9]; break;
    case 12: dst[get_global_id0] = src[get_global_id0 + 6]; break;
    default: dst[get_global_id0] = src[get_global_id0 + 8]; break;
  }
}

static void compiler_switch(void)
{
  const size_t n = 32;
  int cpu_dst[32], cpu_src[32];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_switch");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = 16;
  locals[0] = 16;

  OCL_MAP_BUFFER(1);
  for (uint32_t i = 0; i < 32; ++i)
    cpu_src[i] = ((int32_t*)buf_data[1])[i] = i;
  OCL_UNMAP_BUFFER(1);
  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  for (int i = 0; i < 16; ++i)
    cpu_compiler_switch(cpu_dst, cpu_src, i);
  for (int i = 0; i < 16; ++i)
    OCL_ASSERT(((int32_t*)buf_data[0])[i] == cpu_dst[i]);
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_switch)


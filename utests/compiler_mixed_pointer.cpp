#include "utest_helper.hpp"

static void cpu(int global_id, int *src1, int *src2, int *dst) {
  int * tmp = NULL;

  switch(global_id) {
    case 0:
    case 1:
    case 4:
      tmp = src1;
      break;
    default:
      tmp = src2;
      break;
  }
  dst[global_id] = tmp[global_id];

}
static void cpu1(int global_id, int *src, int *dst1, int *dst2) {
  int * tmp = global_id < 5 ? dst1 : dst2;
  tmp[global_id] = src[global_id];
}

void compiler_mixed_pointer(void)
{
  const size_t n = 16;
  int cpu_dst[16], cpu_src[16], cpu_src1[16];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_mixed_pointer");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = 16;
  locals[0] = 16;

  // Run random tests
  for (uint32_t pass = 0; pass < 1; ++pass) {
    OCL_MAP_BUFFER(0);
    OCL_MAP_BUFFER(1);
    for (int32_t i = 0; i < (int32_t) n; ++i) {
      cpu_src[i] = ((int32_t*)buf_data[0])[i] = i;
      cpu_src1[i] = ((int32_t*)buf_data[1])[i] = 65536-i;
    }
    OCL_UNMAP_BUFFER(0);
    OCL_UNMAP_BUFFER(1);

    // Run the kernel on GPU
    OCL_NDRANGE(1);

    // Run on CPU
    for (int32_t i = 0; i <(int32_t) n; ++i) cpu(i, cpu_src, cpu_src1, cpu_dst);

    // Compare
    OCL_MAP_BUFFER(2);
    for (size_t i = 0; i < n; ++i) {
//      printf(" %d  %d\n", cpu_dst[i], ((int32_t*)buf_data[2])[i]);
      OCL_ASSERT(((int32_t*)buf_data[2])[i] == cpu_dst[i]);
    }
    OCL_UNMAP_BUFFER(2);
  }
}

MAKE_UTEST_FROM_FUNCTION(compiler_mixed_pointer);

void compiler_mixed_pointer1(void)
{
  const size_t n = 16;
  int cpu_dst1[16], cpu_dst2[16], cpu_src[16];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_mixed_pointer", "compiler_mixed_pointer1");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(uint32_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = 16;
  locals[0] = 16;

  // Run random tests
  for (uint32_t pass = 0; pass < 1; ++pass) {
    OCL_MAP_BUFFER(0);
    OCL_MAP_BUFFER(1);
    OCL_MAP_BUFFER(2);
    for (int32_t i = 0; i < (int32_t) n; ++i) {
      cpu_src[i] = ((int32_t*)buf_data[0])[i] = i;
      cpu_dst1[i] = ((int32_t*)buf_data[1])[i] = 0xff;
      cpu_dst2[i] = ((int32_t*)buf_data[2])[i] = 0xff;
    }
    OCL_UNMAP_BUFFER(0);
    OCL_UNMAP_BUFFER(1);
    OCL_UNMAP_BUFFER(2);

    // Run the kernel on GPU
    OCL_NDRANGE(1);

    // Run on CPU
    for (int32_t i = 0; i <(int32_t) n; ++i) cpu1(i, cpu_src, cpu_dst1, cpu_dst2);

    // Compare
    OCL_MAP_BUFFER(1);
    OCL_MAP_BUFFER(2);
    for (size_t i = 0; i < n; ++i) {
//      printf(" %d  %d\n", cpu_dst1[i], ((int32_t*)buf_data[1])[i]);
//      printf(" %d  %d\n", ((int32_t*)buf_data[2])[i], cpu_dst2[i]);
      OCL_ASSERT(((int32_t*)buf_data[1])[i] == cpu_dst1[i]);
      OCL_ASSERT(((int32_t*)buf_data[2])[i] == cpu_dst2[i]);
    }
    OCL_UNMAP_BUFFER(1);
    OCL_UNMAP_BUFFER(2);
  }
}

MAKE_UTEST_FROM_FUNCTION(compiler_mixed_pointer1);

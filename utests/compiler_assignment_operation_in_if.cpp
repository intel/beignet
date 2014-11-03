#include "utest_helper.hpp"

typedef struct cpu_int3{
	int x;
	int y;
	int z;
}cpu_int3;

static void cpu(int gidx, int *dst) {
  cpu_int3 d1 = {gidx, gidx-1, gidx-3};
  int k = gidx % 5;
  if (k == 1){
    d1.x = d1.y;
  }
  int * addr = dst + gidx;
  *addr = d1.x;
}

void compiler_assignment_operation_in_if(void){
  const size_t n = 16;
  int cpu_dst[16];
	
  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_assignment_operation_in_if");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  globals[0] = 16;
  locals[0] = 16;

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i)
    cpu(i, cpu_dst);

  // Compare
  OCL_MAP_BUFFER(0);
  for (int32_t i = 0; i < (int32_t) n; ++i)
    OCL_ASSERT(((int *)buf_data[0])[i] == cpu_dst[i]);
  OCL_UNMAP_BUFFER(0);

}

MAKE_UTEST_FROM_FUNCTION(compiler_assignment_operation_in_if)

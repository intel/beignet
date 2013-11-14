#include "utest_helper.hpp"

static void cpu(int global_id, int *src, int *dst, int scale) {
  bool isRedRow = false;
  bool isRed;
  int val = src[global_id];
  for (int i=0; i<scale; i++, isRedRow = !isRedRow) {
    if (isRedRow) {
      isRed= false;
      for (int j=0; j < scale; j++, isRed=!isRed) {
        if (isRed) {
	  val++;
        }
      }
    }
  }
  dst[global_id] = val;
}

void compiler_bool_cross_basic_block(void){
  const size_t n = 16;
  int cpu_dst[16], cpu_src[16];
  int scale = 4;
	
  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_bool_cross_basic_block");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(int), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(int), &scale);
  globals[0] = 16;
  locals[0] = 16;

  OCL_MAP_BUFFER(0);
  for (int32_t i = 0; i < (int32_t) n; ++i)
    cpu_src[i] = ((int*)buf_data[0])[i] = i;
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Run on CPU
  for (int32_t i = 0; i < (int32_t) n; ++i)
    cpu(i, cpu_src, cpu_dst, scale);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < (int32_t) n; ++i)
    OCL_ASSERT(((int *)buf_data[1])[i] == cpu_dst[i]);
  OCL_UNMAP_BUFFER(1);

}

MAKE_UTEST_FROM_FUNCTION(compiler_bool_cross_basic_block)

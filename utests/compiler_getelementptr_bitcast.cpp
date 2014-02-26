#include "utest_helper.hpp"

void compiler_getelementptr_bitcast(void)
{
  const size_t n = 16;
  float cpu_dst[16], cpu_src[16];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_getelementptr_bitcast");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = 16;

  //must be 1 to pass the test, it is required by the special usage in the kernel
  locals[0] = 1;

  // Run random tests
  for (uint32_t pass = 0; pass < 8; ++pass) {
    OCL_MAP_BUFFER(0);
    for (int32_t i = 0; i < (int32_t) n; ++i)
      cpu_src[i] = ((float*)buf_data[0])[i] = .1f * (rand() & 15) - .75f;
    OCL_UNMAP_BUFFER(0);

    // Run the kernel on GPU
    OCL_NDRANGE(1);

    // Run on CPU
    for (int32_t i = 0; i < (int32_t) n; ++i){
      unsigned char* c = (unsigned char*)&cpu_src[i];
      cpu_dst[i] = c[2];
    }

    // Compare
    OCL_MAP_BUFFER(1);
    for (int32_t i = 0; i < (int32_t) n; ++i){
      //printf("src:%f, gpu_dst: %f, cpu_dst: %f\n", cpu_src[i], ((float *)buf_data[1])[i], cpu_dst[i]);
      OCL_ASSERT(((float *)buf_data[1])[i] == cpu_dst[i]);
    }
    OCL_UNMAP_BUFFER(1);
  }
}

MAKE_UTEST_FROM_FUNCTION(compiler_getelementptr_bitcast);

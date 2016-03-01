#include "utest_helper.hpp"

template<typename T>
void test_atomic(const char* kernelName)
{
  const int n = 16;
  T cpu_src[16];

  // Setup kernel and buffers
  OCL_CALL(cl_kernel_init, "compiler_generic_atomic.cl", kernelName, SOURCE, "-cl-std=CL2.0");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(T), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(T), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = 16;
  locals[0] = 16;

  OCL_MAP_BUFFER(0);
  for (int i = 0; i <  n; ++i)
    cpu_src[i] = ((T*)buf_data[0])[i] = (T)i;
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_NDRANGE(1);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < n; ++i) {
//    printf("i=%d dst=%d\n", i, ((T*)buf_data[1])[i]);
    OCL_ASSERT(((T*)buf_data[1])[i] == 2 * cpu_src[i]);
  }
  OCL_UNMAP_BUFFER(1);
}

#define GENERIC_ATOMIC_TEST(T)                    \
void compiler_generic_atomic_##T() {       \
  test_atomic<T>("compiler_generic_atomic_"#T);   \
} \
MAKE_UTEST_FROM_FUNCTION(compiler_generic_atomic_##T);

GENERIC_ATOMIC_TEST(int)
//GENERIC_TEST(long)




#include "utest_helper.hpp"
#include <string.h>
template<typename T>
static void compiler_vector_load_store(int elemNum, const char *kernelName)
{
  const size_t n = elemNum * 256;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_vector_load_store", kernelName);
  buf_data[0] = (T*) malloc(sizeof(T) * n);
  for (uint32_t i = 0; i < n; ++i)
    ((T*)buf_data[0])[i] = i;
  OCL_CREATE_BUFFER(buf[0], CL_MEM_COPY_HOST_PTR, n * sizeof(T), buf_data[0]);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(T), NULL);
  free(buf_data[0]);
  buf_data[0] = NULL;

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = n / elemNum;
  locals[0] = 16;
  OCL_NDRANGE(1);

  // Check result
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  for (uint32_t i = 0; i < n; ++i)
  {
    int shift = ((i % elemNum) + 1);
    if (strstr(kernelName, "double") == NULL)
      OCL_ASSERT(((T*)buf_data[1])[i] == (T)(((T*)buf_data[0])[i] + shift));
    else
      OCL_ASSERT((((T*)buf_data[1])[i] - ((T)((T*)buf_data[0])[i] + shift)) < 1e-5);
  }
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);
}

#define compiler_vector_load_store(type, n, kernel_type, keep_program) \
static void compiler_vector_ ##kernel_type ##n ##_load_store(void)\
{\
  compiler_vector_load_store<type>(n, "test_" #kernel_type #n);\
}\
MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_vector_ ## kernel_type ##n ##_load_store, keep_program);

#define test_all_vector(type, kernel_type, keep_program) \
  compiler_vector_load_store(type, 2, kernel_type, true) \
  compiler_vector_load_store(type, 3, kernel_type, true) \
  compiler_vector_load_store(type, 4, kernel_type, true) \
  compiler_vector_load_store(type, 8, kernel_type, true) \
  compiler_vector_load_store(type, 16, kernel_type, keep_program)

test_all_vector(int8_t, char, true)
test_all_vector(uint8_t, uchar, true)
test_all_vector(int16_t, short, true)
test_all_vector(uint16_t, ushort, true)
test_all_vector(int32_t, int, true)
test_all_vector(uint32_t, uint, true)
test_all_vector(float, float, true)
//test_all_vector(double, double, true)
test_all_vector(int64_t, long, true)
test_all_vector(uint64_t, ulong, false)

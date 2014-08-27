#include "utest_helper.hpp"
#include <sys/time.h>

#define N_ITERATIONS 10000

#define T uint8_t
template <typename T>
static double vload_bench(const char *kernelFunc, uint32_t N, uint32_t offset, bool benchMode)
{
  const size_t n = benchMode ? (512 * 1024) : (8 * 1024);
  struct timeval start, end;

  // Setup kernel and buffers
  std::string kernelName = kernelFunc + std::to_string(N);
  OCL_CALL (cl_kernel_init, "vload_bench.cl", kernelName.c_str(), SOURCE, NULL);
  //OCL_CREATE_KERNEL("compiler_array");
  buf_data[0] = (T*) malloc(sizeof(T) * n);
  for (uint32_t i = 0; i < n; ++i) ((T*)buf_data[0])[i] = i; //rand() & ((1LL << N) - 1);
  OCL_CREATE_BUFFER(buf[0], CL_MEM_COPY_HOST_PTR, n * sizeof(T), buf_data[0]);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(uint32_t), NULL);
  free(buf_data[0]);
  buf_data[0] = NULL;

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(uint32_t), &offset);
  globals[0] = n / ((N + 1) & ~0x1);
  locals[0] = 256;
  if (benchMode)
    gettimeofday(&start, NULL);
  OCL_NDRANGE(1);
  if (benchMode) {
    OCL_FINISH();
    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) * 1e6 + (end.tv_usec - start.tv_usec);
    double bandwidth = (globals[0] * (N_ITERATIONS) * sizeof(T) * N) / elapsed;
    printf("\t%2.1fGB/S\n", bandwidth/1000.);
    return bandwidth;
  } else {
    // Check result
    OCL_MAP_BUFFER(0);
    OCL_MAP_BUFFER(1);
    for (uint32_t i = 0; i < globals[0]; ++i) {
      OCL_ASSERT(((T*)buf_data[0])[i + offset] == ((uint32_t*)buf_data[1])[i]);
    }
    return 0;
  }
}

#define VLOAD_TEST(T, kT) \
static void vload_test_ ##kT(void) \
{ \
  uint8_t vectorSize[] = {2, 3, 4, 8, 16}; \
  for(uint32_t i = 0; i < sizeof(vectorSize); i++) { \
    for(uint32_t offset = 0; offset < vectorSize[i]; offset++) {\
      (void)vload_bench<T>("vload_bench_1" #kT, vectorSize[i], offset, false); \
    }\
  } \
}\
MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(vload_test_ ##kT, true)

#ifndef BUILD_BENCHMARK
VLOAD_TEST(uint8_t, uchar)
VLOAD_TEST(int8_t, char)
VLOAD_TEST(uint16_t, ushort)
VLOAD_TEST(int16_t, short)
VLOAD_TEST(uint32_t, uint)
VLOAD_TEST(int32_t, int)
VLOAD_TEST(float, float)
#endif

#define VLOAD_BENCH(T, kT) \
static int vload_bench_ ##kT(void) \
{ \
  uint8_t vectorSize[] = {2, 3, 4, 8, 16}; \
  double totBandwidth = 0; \
  unsigned int j = 0;\
  printf("\n");\
  for(uint32_t i = 0; i < sizeof(vectorSize); i++, j++) { \
    printf("  Vector size %d:\n", vectorSize[i]); \
    uint32_t k = 0;\
    double bandwidthForOneSize = 0;\
    for(uint32_t offset = 0; offset < vectorSize[i]; offset++, k++) {\
      printf("\tOffset %d :", offset); \
      bandwidthForOneSize += vload_bench<T>("vload_bench_10000"  #kT, vectorSize[i], offset, true); \
    }\
    totBandwidth += bandwidthForOneSize / k;\
  } \
  return totBandwidth/j;\
}\
MAKE_BENCHMARK_FROM_FUNCTION_KEEP_PROGRAM(vload_bench_ ##kT, true)

#ifdef BUILD_BENCHMARK
VLOAD_BENCH(uint8_t, uchar)
VLOAD_BENCH(uint16_t, ushort)
VLOAD_BENCH(uint32_t, uint)
#endif

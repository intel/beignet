#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"
#include <sys/time.h>
#include <iomanip>
#include <algorithm>

using namespace std;

/* work-group general settings */
#define WG_GLOBAL_SIZE          (512 * 256)
#define WG_LOCAL_SIZE           128
#define WG_LOOP_COUNT           1000

/* work-group broadcast only */
#define WG_GLOBAL_SIZE_X        1024
#define WG_GLOBAL_SIZE_Y        1024

#define WG_LOCAL_SIZE_X         32
#define WG_LOCAL_SIZE_Y         2

#define WG_LOCAL_X    5
#define WG_LOCAL_Y    0


enum WG_FUNCTION
{
  WG_BROADCAST_1D,
  WG_BROADCAST_2D,
  WG_REDUCE_ADD,
  WG_REDUCE_MIN,
  WG_REDUCE_MAX,
  WG_SCAN_EXCLUSIVE_ADD,
  WG_SCAN_EXCLUSIVE_MAX,
  WG_SCAN_EXCLUSIVE_MIN,
  WG_SCAN_INCLUSIVE_ADD,
  WG_SCAN_INCLUSIVE_MAX,
  WG_SCAN_INCLUSIVE_MIN
};

/*
 * Generic compute-expected on CPU function for any workgroup type
 * and any variable type
 */
template<class T>
static void benchmark_expected(WG_FUNCTION wg_func,
                    T* input,
                    T* expected,
                    uint32_t wg_global_size,
                    uint32_t wg_local_size)
{
  if(wg_func == WG_BROADCAST_1D)
  {
    for(uint32_t i = 0; i < wg_local_size; i++)
      expected[i] = input[WG_LOCAL_X];
  }
  else if(wg_func == WG_BROADCAST_2D)
  {
    for(uint32_t i = 0; i < wg_local_size; i++)
      expected[i] =
          input[WG_LOCAL_X +
                WG_LOCAL_Y * WG_LOCAL_SIZE_X];
  }
  else if(wg_func == WG_REDUCE_ADD)
  {
    T wg_sum = input[0];
    for(uint32_t i = 1; i < wg_local_size; i++)
      wg_sum += input[i];
    for(uint32_t i = 0; i < wg_local_size; i++)
      expected[i] = wg_sum;
  }
  else if(wg_func == WG_REDUCE_MAX)
  {
    T wg_max = input[0];
    for(uint32_t i = 1; i < wg_local_size; i++)
      wg_max = max(input[i], wg_max);
    for(uint32_t i = 0; i < wg_local_size; i++)
      expected[i] = wg_max;
  }
  else if(wg_func == WG_REDUCE_MIN)
  {
    T wg_min = input[0];
    for(uint32_t i = 1; i < wg_local_size; i++)
      wg_min = min(input[i], wg_min);
    for(uint32_t i = 0; i < wg_local_size; i++)
      expected[i] = wg_min;
  }
  else if(wg_func == WG_SCAN_INCLUSIVE_ADD)
  {
    expected[0] = input[0];
    for(uint32_t i = 1; i < wg_local_size; i++)
      expected[i] = input[i] + expected[i - 1];
  }
  else if(wg_func == WG_SCAN_INCLUSIVE_MAX)
  {
    expected[0] = input[0];
    for(uint32_t i = 1; i < wg_local_size; i++)
      expected[i] = max(input[i], expected[i - 1]);
  }
  else if(wg_func == WG_SCAN_INCLUSIVE_MIN)
  {
    expected[0] = input[0];
    for(uint32_t i = 1; i < wg_local_size; i++)
      expected[i] = min(input[i], expected[i - 1]);
  }
}

/*
 * Generic input-expected generate function for any workgroup type
 * and any variable type
 */
template<class T>
static void benchmark_data(WG_FUNCTION wg_func,
                   T* &input,
                   T* &expected,
                   uint32_t &wg_global_size,
                   uint32_t &wg_local_size)
{
  if(wg_func == WG_BROADCAST_1D)
  {
    wg_global_size = WG_GLOBAL_SIZE_X;
    wg_local_size = WG_LOCAL_SIZE_X;
  }
  else if(wg_func == WG_BROADCAST_2D)
  {
    wg_global_size = WG_GLOBAL_SIZE_X * WG_GLOBAL_SIZE_Y;
    wg_local_size = WG_LOCAL_SIZE_X * WG_LOCAL_SIZE_Y;
  }
  else
  {
    wg_global_size = WG_GLOBAL_SIZE;
    wg_local_size = WG_LOCAL_SIZE;
  }

  input = new T[wg_global_size];
  expected = new T[wg_global_size];

  /* seed for random inputs */
  srand (time(NULL));

  /* generate inputs and expected values */
  for(uint32_t gid = 0; gid < wg_global_size; gid += wg_local_size)
  {
    /* input values */
    for(uint32_t lid = 0; lid < wg_local_size; lid++)
      input[gid + lid] = (rand() % 512) / 3.1415f;

    /* expected values */
    benchmark_expected(wg_func, input + gid, expected + gid,
                       wg_global_size, wg_local_size);
  }
}

/*
 * Generic benchmark function for any workgroup type
 * and any variable type
 */
template<class T>
static double benchmark_generic(WG_FUNCTION wg_func,
                       T* input,
                       T* expected)
{
  double elapsed = 0;
  const uint32_t reduce_loop = WG_LOOP_COUNT;
  struct timeval start,stop;

  uint32_t wg_global_size = 0;
  uint32_t wg_local_size = 0;

  /* input and expected data */
  benchmark_data(wg_func, input, expected, wg_global_size, wg_local_size);

  /* prepare input for datatype */
  OCL_CREATE_BUFFER(buf[0], 0, wg_global_size * sizeof(T), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, wg_global_size * sizeof(T), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_uint), &reduce_loop);

  if(wg_func == WG_BROADCAST_1D ||
      wg_func == WG_BROADCAST_2D)
  {
    cl_uint wg_local_x = WG_LOCAL_X;
    cl_uint wg_local_y = WG_LOCAL_Y;
    OCL_SET_ARG(3, sizeof(cl_uint), &wg_local_x);
    OCL_SET_ARG(4, sizeof(cl_uint), &wg_local_y);
  }

  /* set input data for GPU */
  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], input, wg_global_size * sizeof(T));
  OCL_UNMAP_BUFFER(0);

  /* run the kernel on GPU */
  gettimeofday(&start,0);

  if(wg_func == WG_BROADCAST_1D)
  {
    globals[0] = WG_GLOBAL_SIZE_X;
    locals[0] = WG_LOCAL_SIZE_X;
    OCL_NDRANGE(1);
  }
  else if(wg_func == WG_BROADCAST_2D)
  {
    globals[0] = WG_GLOBAL_SIZE_X;
    locals[0] = WG_LOCAL_SIZE_X;
    globals[1] = WG_GLOBAL_SIZE_Y;
    locals[1] = WG_LOCAL_SIZE_Y;
    OCL_NDRANGE(2);
  }
  else
  { /* reduce, scan inclulsive, scan exclusive */
    globals[0] = WG_GLOBAL_SIZE;
    locals[0] = WG_LOCAL_SIZE;
    OCL_NDRANGE(1);
  }

  clFinish(queue);
  gettimeofday(&stop,0);
  elapsed = time_subtract(&stop, &start, 0);

  /* check if mistmatch, display execution time */
  OCL_MAP_BUFFER(1);
  uint32_t mistmatches = 0;
  for (uint32_t i = 0; i < wg_global_size; i++)
    if(((T *)buf_data[1])[i] != *(expected + i)){
      /* uncomment bellow for DEBUG */
      /* cout << "Err at " << i << ", " <<
        ((T *)buf_data[1])[i] << " != " << *(expected + i) << endl; */
      mistmatches++;
    }
  cout << endl << endl << "Mistmatches " << mistmatches << endl;
  cout << "Exec time " << elapsed << endl << endl;
  OCL_UNMAP_BUFFER(1);

  return BANDWIDTH(sizeof(T) * wg_global_size * reduce_loop, elapsed);
}

/*
 * Benchmark workgroup broadcast
 */
double benchmark_workgroup_broadcast_1D_int(void)
{
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("bench_workgroup",
                  "bench_workgroup_broadcast_1D_int");
  return benchmark_generic(WG_BROADCAST_1D, input, expected);
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_workgroup_broadcast_1D_int, "GB/S");
double benchmark_workgroup_broadcast_1D_long(void)
{
  cl_long *input = NULL;
  cl_long *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("bench_workgroup",
                  "bench_workgroup_broadcast_1D_long");
  return benchmark_generic(WG_BROADCAST_1D, input, expected);
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_workgroup_broadcast_1D_long, "GB/S");
double benchmark_workgroup_broadcast_2D_int(void)
{
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("bench_workgroup",
                  "bench_workgroup_broadcast_2D_int");
  return benchmark_generic(WG_BROADCAST_2D, input, expected);
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_workgroup_broadcast_2D_int, "GB/S");
double benchmark_workgroup_broadcast_2D_long(void)
{
  cl_long *input = NULL;
  cl_long *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("bench_workgroup",
                  "bench_workgroup_broadcast_2D_long");
  return benchmark_generic(WG_BROADCAST_2D, input, expected);
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_workgroup_broadcast_2D_long, "GB/S");

/*
 * Benchmark workgroup reduce add
 */
double benchmark_workgroup_reduce_add_int(void)
{
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("bench_workgroup",
                  "bench_workgroup_reduce_add_int");
  return benchmark_generic(WG_REDUCE_ADD, input, expected);
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_workgroup_reduce_add_int, "GB/S");
double benchmark_workgroup_reduce_add_long(void)
{
  cl_long *input = NULL;
  cl_long *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("bench_workgroup",
                  "bench_workgroup_reduce_add_long");
  return benchmark_generic(WG_REDUCE_ADD, input, expected);
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_workgroup_reduce_add_long, "GB/S");

/*
 * Benchmark workgroup reduce min
 */
double benchmark_workgroup_reduce_min_int(void)
{
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("bench_workgroup",
                  "bench_workgroup_reduce_min_int");
  return benchmark_generic(WG_REDUCE_MIN, input, expected);
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_workgroup_reduce_min_int, "GB/S");
double benchmark_workgroup_reduce_min_long(void)
{
  cl_long *input = NULL;
  cl_long *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("bench_workgroup",
                  "bench_workgroup_reduce_min_long");
  return benchmark_generic(WG_REDUCE_MIN, input, expected);
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_workgroup_reduce_min_long, "GB/S");

/*
 * Benchmark workgroup scan inclusive add
 */
double benchmark_workgroup_scan_inclusive_add_int(void)
{
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("bench_workgroup",
                  "bench_workgroup_scan_inclusive_add_int");
  return benchmark_generic(WG_SCAN_INCLUSIVE_ADD, input, expected);
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_workgroup_scan_inclusive_add_int, "GB/S");
double benchmark_workgroup_scan_inclusive_add_long(void)
{
  cl_long *input = NULL;
  cl_long *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("bench_workgroup",
                  "bench_workgroup_scan_inclusive_add_long");
  return benchmark_generic(WG_SCAN_INCLUSIVE_ADD, input, expected);
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_workgroup_scan_inclusive_add_long, "GB/S");

/*
 * Benchmark workgroup scan inclusive min
 */
double benchmark_workgroup_scan_inclusive_min_int(void)
{
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("bench_workgroup",
                  "bench_workgroup_scan_inclusive_min_int");
  return benchmark_generic(WG_SCAN_INCLUSIVE_MIN, input, expected);
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_workgroup_scan_inclusive_min_int, "GB/S");
double benchmark_workgroup_scan_inclusive_min_long(void)
{
  cl_long *input = NULL;
  cl_long *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("bench_workgroup",
                  "bench_workgroup_scan_inclusive_min_long");
  return benchmark_generic(WG_SCAN_INCLUSIVE_MIN, input, expected);
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_workgroup_scan_inclusive_min_long, "GB/S");




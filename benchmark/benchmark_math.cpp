#include "utests/utest_helper.hpp"
#include <sys/time.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"
#include <sys/time.h>

double benchmark_generic_math(const char* str_filename,
                              const char* str_kernel)
{
  double elapsed = 0;
  struct timeval start,stop;
  const size_t global_size = 1024 * 1024;
  const size_t local_size = 64;

  /* Compute math OP, loop times on global size */
  cl_float base = 1.000002;
  cl_float pwr = 1.0102003;
  uint32_t loop = 1000;

  /* Input set will be generated */
  float* src = (float*)calloc(sizeof(float), global_size);
  OCL_ASSERT(src != NULL);
  for(uint32_t i = 0; i < global_size; i++)
    src[i] = base + i * (base - 1);

  /* Setup kernel and buffers */
  OCL_CALL(cl_kernel_init, str_filename, str_kernel, SOURCE, "");

  OCL_CREATE_BUFFER(buf[0], 0, (global_size) * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, (global_size) * sizeof(float), NULL);

  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], src, global_size * sizeof(float));
  OCL_UNMAP_BUFFER(0);

  globals[0] = global_size;
  locals[0] = local_size;

  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_float), &pwr);
  OCL_SET_ARG(3, sizeof(cl_uint), &loop);

  /* Measure performance */
  gettimeofday(&start,0);
  OCL_NDRANGE(1);
  clFinish(queue);
  gettimeofday(&stop,0);
  elapsed = time_subtract(&stop, &start, 0);

  /* Show compute results */
  OCL_MAP_BUFFER(1);
  for(uint32_t i = 0; i < global_size; i += 8192)
    printf("\t%.3f", ((float*)buf_data[1])[i]);
  OCL_UNMAP_BUFFER(1);

  return BANDWIDTH(global_size * loop, elapsed);
}

double benchmark_math_pow(void){
  return benchmark_generic_math("bench_math.cl", "bench_math_pow");
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_math_pow, "Mop/s");

double benchmark_math_exp2(void){
  return benchmark_generic_math("bench_math.cl", "bench_math_exp2");
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_math_exp2, "Mop/s");

double benchmark_math_exp(void){
  return benchmark_generic_math("bench_math.cl", "bench_math_exp");
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_math_exp, "Mop/s");

double benchmark_math_exp10(void){
  return benchmark_generic_math("bench_math.cl", "bench_math_exp10");
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_math_exp10, "Mop/s");

double benchmark_math_log2(void){
  return benchmark_generic_math("bench_math.cl", "bench_math_log2");
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_math_log2, "Mop/s");

double benchmark_math_log(void){
  return benchmark_generic_math("bench_math.cl", "bench_math_log");
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_math_log, "Mop/s");

double benchmark_math_log10(void){
  return benchmark_generic_math("bench_math.cl", "bench_math_log10");
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_math_log10, "Mop/s");

double benchmark_math_sqrt(void){
  return benchmark_generic_math("bench_math.cl", "bench_math_sqrt");
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_math_sqrt, "Mop/s");

double benchmark_math_sin(void){
  return benchmark_generic_math("bench_math.cl", "bench_math_sin");
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_math_sin, "Mop/s");

double benchmark_math_cos(void){
  return benchmark_generic_math("bench_math.cl", "bench_math_cos");
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_math_cos, "Mop/s");

double benchmark_math_tan(void){
  return benchmark_generic_math("bench_math.cl", "bench_math_tan");
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_math_tan, "Mop/s");

double benchmark_math_asin(void){
  return benchmark_generic_math("bench_math.cl", "bench_math_asin");
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_math_asin, "Mop/s");

double benchmark_math_acos(void){
  return benchmark_generic_math("bench_math.cl", "bench_math_acos");
}
MAKE_BENCHMARK_FROM_FUNCTION(benchmark_math_acos, "Mop/s");

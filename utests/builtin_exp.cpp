#include "utest_helper.hpp"
#include <cmath>
#include <algorithm>

#define udebug 0

#define FLT_MAX 0x1.fffffep127f
#define FLT_MIN 0x1.0p-126f
#define FLT_ULP  (1.0e-6f)

#define printf_c(...) \
{\
  printf("\033[1m\033[40;31m");\
  printf( __VA_ARGS__ );\
  printf("\033[0m");\
}

const float input_data[] = {FLT_MAX, -FLT_MAX, FLT_MIN, -FLT_MIN, 80, -80, 3.14, -3.14, -0.5, 0.5, 1, -1, 0.0 };
const int count_input = sizeof(input_data) / sizeof(input_data[0]);
const int max_function = 5;

static void cpu_compiler_math(float *dst, const float *src)
{
  const float x = *src;

  dst[0] = exp(x);
  dst[1] = exp2(x);
  dst[2] = exp10(x);
  dst[3] = expm1(x);
  dst[4] = x;
}

static void builtin_exp(void)
{
  // Setup kernel and buffers
  int k, i, index_cur;
  float gpu_data[max_function * count_input] = {0}, cpu_data[max_function * count_input] = {0};
  float diff;
  char log[256] = {0};

  OCL_CREATE_KERNEL("builtin_exp");

  OCL_CREATE_BUFFER(buf[0], CL_MEM_READ_WRITE, count_input * max_function * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], CL_MEM_READ_WRITE, count_input * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[2], CL_MEM_READ_WRITE, sizeof(int), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);

  globals[0] = count_input;
  locals[0] = 1;

  clEnqueueWriteBuffer( queue, buf[1], CL_TRUE, 0, count_input * sizeof(float), input_data, 0, NULL, NULL);
  clEnqueueWriteBuffer( queue, buf[2], CL_TRUE, 0, sizeof(int), &max_function , 0, NULL, NULL);

   // Run the kernel
  OCL_NDRANGE( 1 );

  clEnqueueReadBuffer( queue, buf[0], CL_TRUE, 0, sizeof(float) * max_function * count_input, gpu_data, 0, NULL, NULL);

  for (k = 0; (uint)k < count_input; k++)
  {
    cpu_compiler_math( cpu_data + k * max_function, input_data + k);

    for (i = 0; i < max_function; i++)
    {
      index_cur = k * max_function + i;
      diff = fabs(gpu_data[index_cur]-cpu_data[index_cur]);
      sprintf(log, "%d/%d: %f -> gpu:%f  cpu:%f diff:%f expect:%f\n", \
         k, i, input_data[k], gpu_data[index_cur], cpu_data[index_cur], \
         diff/gpu_data[index_cur], 3 * FLT_ULP);

#if udebug
      if (isinf(cpu_data[index_cur]) && isinf(gpu_data[index_cur])){
        printf(log);
      }
      else if (isnan(cpu_data[index_cur]) && isnan(gpu_data[index_cur])){
        printf(log);
      }
      else if( diff / cpu_data[index_cur] < 3 * FLT_ULP \
        && ( gpu_data[index_cur] > FLT_ULP  || cpu_data[index_cur] > FLT_ULP )){
        printf(log);
      }
      else if ( gpu_data[index_cur] < FLT_ULP && gpu_data[index_cur] < FLT_ULP)
        printf(log);
      else
        printf_c(log);
#else
      if (isinf(cpu_data[index_cur]))
        OCL_ASSERTM(isinf(gpu_data[index_cur]), log);
      else if (isnan(cpu_data[index_cur]))
        OCL_ASSERTM(isnan(gpu_data[index_cur]), log);
      else if ( gpu_data[index_cur] > FLT_ULP || cpu_data[index_cur] > FLT_ULP)
        OCL_ASSERTM(fabs( diff / cpu_data[index_cur]) < 3 * FLT_ULP, log);
      else
        OCL_ASSERTM(fabs(diff) < 3 * FLT_ULP, log);
#endif
    }
  }
}

MAKE_UTEST_FROM_FUNCTION(builtin_exp)

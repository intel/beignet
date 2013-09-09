#include "utest_helper.hpp"
#include <cmath>
#include <algorithm>

#define udebug 0
#define printf_c(...) \
{\
  printf("\033[1m\033[40;31m");\
  printf( __VA_ARGS__ );\
  printf("\033[0m");\
}
const float ori_data[] = {-20.5, -1, -0.9, -0.01, 0, 0.01, 0.9, 1.0, 20.5};
const int count_input_ori = sizeof(ori_data) / sizeof(ori_data[0]);
const int count_input = count_input_ori * count_input_ori;

float input_data1[count_input];
float input_data2[count_input];
const int max_function = 1;

static void cpu_compiler_math(const float *src1, const float *src2, float *dst)
{
  dst[0] = powf(src1[0], src2[0]);
//  dst[1] = src1[0];
}

static void builtin_pow(void)
{
  // Setup kernel and buffers
  int k, i, index_cur;
  float gpu_data[max_function * count_input] = {0}, cpu_data[max_function * count_input] = {0};

  for(i=0; i<count_input_ori;i++)
    for(k=0; k<count_input_ori;k++)
    {
      input_data1[i*count_input_ori+k] = ori_data[i];
      input_data2[i*count_input_ori+k] = ori_data[k];
    }

  OCL_CREATE_KERNEL("builtin_pow");

  OCL_CREATE_BUFFER(buf[0], CL_MEM_READ_WRITE, count_input * max_function * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[1], CL_MEM_READ_WRITE, count_input * max_function * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[2], CL_MEM_READ_WRITE, count_input * max_function * sizeof(float), NULL);
  OCL_CREATE_BUFFER(buf[3], CL_MEM_READ_WRITE, sizeof(int), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(3, sizeof(cl_mem), &buf[3]);

  globals[0] = count_input;
  locals[0] = 1;

  clEnqueueWriteBuffer( queue, buf[1], CL_TRUE, 0, count_input * sizeof(float), input_data1, 0, NULL, NULL);
  clEnqueueWriteBuffer( queue, buf[2], CL_TRUE, 0, count_input * sizeof(float), input_data2, 0, NULL, NULL);
  clEnqueueWriteBuffer( queue, buf[3], CL_TRUE, 0, sizeof(int), &max_function, 0, NULL, NULL);

   // Run the kernel
  OCL_NDRANGE( 1 );

  clEnqueueReadBuffer( queue, buf[0], CL_TRUE, 0, sizeof(float) * max_function * count_input, gpu_data, 0, NULL, NULL);

  for (k = 0; (uint)k < count_input; k++)
  {
    cpu_compiler_math( input_data1 + k, input_data2 + k, cpu_data + k * max_function);

    for (i = 0; i < max_function; i++)
    {
      index_cur = k * max_function + i;
#if udebug
      if ( (isinf(cpu_data[index_cur]) && !isinf(gpu_data[index_cur])) ||
           (isnan(cpu_data[index_cur]) && !isnan(gpu_data[index_cur])) ||
           (fabs(gpu_data[index_cur] - cpu_data[index_cur]) > 1e-5f)   )
      {
        printf_c("%d/%d: x:%f, y:%f -> gpu:%f  cpu:%f\n", k, i, input_data1[k], input_data2[k], gpu_data[index_cur], cpu_data[index_cur]);
      }
      else
        printf("%d/%d: x:%f, y:%f -> gpu:%f  cpu:%f\n", k, i, input_data1[k], input_data2[k], gpu_data[index_cur], cpu_data[index_cur]);
#else
     if (isinf(cpu_data[index_cur]))
       OCL_ASSERT(isinf(gpu_data[index_cur]));
     else if (isnan(cpu_data[index_cur]))
       OCL_ASSERT(isnan(gpu_data[index_cur]));
     else
     {
       OCL_ASSERT(fabs(gpu_data[index_cur] - cpu_data[index_cur]) < 1e-3f);
     }
#endif
    }
  }
}

MAKE_UTEST_FROM_FUNCTION_WITH_ISSUE(builtin_pow)

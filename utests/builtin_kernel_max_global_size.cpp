#include "utest_helper.hpp"
#include <string.h>

void builtin_kernel_max_global_size(void)
{
  char* built_in_kernel_names;
  size_t built_in_kernels_size;
  cl_int err = CL_SUCCESS;
  size_t ret_sz;


  OCL_CALL (clGetDeviceInfo, device, CL_DEVICE_BUILT_IN_KERNELS, 0, 0, &built_in_kernels_size);
  if(built_in_kernels_size <= 1) { //the size of empty string is 1
    printf(" no built in kernel, Skip!");
    return;
  }

  built_in_kernel_names = (char* )malloc(built_in_kernels_size * sizeof(char) );
  OCL_CALL(clGetDeviceInfo, device, CL_DEVICE_BUILT_IN_KERNELS, built_in_kernels_size, (void*)built_in_kernel_names, &ret_sz);
  OCL_ASSERT(ret_sz == built_in_kernels_size);
  cl_program built_in_prog = clCreateProgramWithBuiltInKernels(ctx, 1, &device, built_in_kernel_names, &err);
  OCL_ASSERT(built_in_prog != NULL);
  char* first_kernel = strtok(built_in_kernel_names, ";");
  OCL_ASSERT(first_kernel);
  cl_kernel builtin_kernel_1d = clCreateKernel(built_in_prog, first_kernel,  &err);
  OCL_ASSERT(builtin_kernel_1d != NULL);
  size_t param_value_size;
  void* param_value;
  OCL_CALL(clGetKernelWorkGroupInfo, builtin_kernel_1d, device, CL_KERNEL_GLOBAL_WORK_SIZE, 0, NULL, &param_value_size);
  param_value = malloc(param_value_size);
  OCL_CALL(clGetKernelWorkGroupInfo, builtin_kernel_1d, device, CL_KERNEL_GLOBAL_WORK_SIZE, param_value_size, param_value, 0);
  OCL_ASSERT(*(size_t*)param_value == 256 * 1024 *1024);
  clReleaseKernel(builtin_kernel_1d);
  clReleaseProgram(built_in_prog);
  free(built_in_kernel_names);
  free(param_value);
}

MAKE_UTEST_FROM_FUNCTION(builtin_kernel_max_global_size);

#include "utest_helper.hpp"

void enqueue_built_in_kernels(void)
{
  char* built_in_kernel_names;
  size_t built_in_kernels_size;
  cl_int err = CL_SUCCESS;
  size_t ret_sz;


  OCL_CALL (clGetDeviceInfo, device, CL_DEVICE_BUILT_IN_KERNELS, 0, 0, &built_in_kernels_size);
  built_in_kernel_names = (char* )malloc(built_in_kernels_size * sizeof(char) );
  OCL_CALL(clGetDeviceInfo, device, CL_DEVICE_BUILT_IN_KERNELS, built_in_kernels_size, (void*)built_in_kernel_names, &ret_sz);
  OCL_ASSERT(ret_sz == built_in_kernels_size);
  cl_program built_in_prog = clCreateProgramWithBuiltInKernels(ctx, 1, &device, built_in_kernel_names, &err);
  OCL_ASSERT(built_in_prog != NULL);
}

MAKE_UTEST_FROM_FUNCTION(enqueue_built_in_kernels);

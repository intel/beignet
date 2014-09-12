#include "utest_helper.hpp"

void compiler_function_qualifiers(void)
{
  OCL_CREATE_KERNEL("compiler_function_qualifiers");

  size_t param_value_size;
  void* param_value;
  cl_int err;

  err = clGetKernelInfo(kernel, CL_KERNEL_ATTRIBUTES, 0, NULL, &param_value_size);
  OCL_ASSERT(err == CL_SUCCESS);
  param_value = malloc(param_value_size);
  err = clGetKernelInfo(kernel, CL_KERNEL_ATTRIBUTES, param_value_size, param_value, NULL);
  OCL_ASSERT(err == CL_SUCCESS);
}

MAKE_UTEST_FROM_FUNCTION(compiler_function_qualifiers);



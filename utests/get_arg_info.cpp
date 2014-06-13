#include <string.h>
#include "utest_helper.hpp"

void test_get_arg_info(void)
{
  int ret;
  uint32_t ret_val;
  cl_kernel_arg_type_qualifier type_qual;
  size_t ret_sz;
  char name[64];

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("test_get_arg_info");

  //Arg 0
  ret = clGetKernelArgInfo(kernel, 0, CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                           sizeof(ret_val), &ret_val, &ret_sz);
  OCL_ASSERT(ret == CL_SUCCESS);
  OCL_ASSERT(ret_sz == sizeof(cl_kernel_arg_address_qualifier));
  OCL_ASSERT(ret_val == CL_KERNEL_ARG_ADDRESS_GLOBAL);

  ret = clGetKernelArgInfo(kernel, 0, CL_KERNEL_ARG_ACCESS_QUALIFIER,
                           sizeof(ret_val), &ret_val, &ret_sz);
  OCL_ASSERT(ret == CL_SUCCESS);
  OCL_ASSERT(ret_sz == sizeof(cl_kernel_arg_access_qualifier));
  OCL_ASSERT(ret_val == CL_KERNEL_ARG_ACCESS_NONE);

  ret = clGetKernelArgInfo(kernel, 0, CL_KERNEL_ARG_TYPE_NAME,
                           sizeof(name), name, &ret_sz);
  OCL_ASSERT(ret == CL_SUCCESS);
  OCL_ASSERT(ret_sz == strlen("float*") + 1);
  OCL_ASSERT(!strcmp(name, "float*"));

  ret = clGetKernelArgInfo(kernel, 0, CL_KERNEL_ARG_NAME,
                           sizeof(name), name, &ret_sz);
  OCL_ASSERT(ret == CL_SUCCESS);
  OCL_ASSERT(ret_sz == strlen("src") + 1);
  OCL_ASSERT(!strcmp(name, "src"));

  ret = clGetKernelArgInfo(kernel, 0, CL_KERNEL_ARG_TYPE_QUALIFIER,
                           sizeof(type_qual), &type_qual, &ret_sz);
  OCL_ASSERT(ret == CL_SUCCESS);
  OCL_ASSERT(ret_sz == sizeof(cl_kernel_arg_type_qualifier));
  OCL_ASSERT(type_qual == (CL_KERNEL_ARG_TYPE_CONST|CL_KERNEL_ARG_TYPE_VOLATILE));

  //Arg 1
  ret = clGetKernelArgInfo(kernel, 1, CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                           sizeof(ret_val), &ret_val, &ret_sz);
  OCL_ASSERT(ret == CL_SUCCESS);
  OCL_ASSERT(ret_sz == sizeof(cl_kernel_arg_address_qualifier));
  OCL_ASSERT(ret_val == CL_KERNEL_ARG_ADDRESS_LOCAL);

  ret = clGetKernelArgInfo(kernel, 1, CL_KERNEL_ARG_ACCESS_QUALIFIER,
                           sizeof(ret_val), &ret_val, &ret_sz);
  OCL_ASSERT(ret == CL_SUCCESS);
  OCL_ASSERT(ret_sz == sizeof(cl_kernel_arg_access_qualifier));
  OCL_ASSERT(ret_val == CL_KERNEL_ARG_ACCESS_NONE);

  ret = clGetKernelArgInfo(kernel, 1, CL_KERNEL_ARG_TYPE_NAME,
                           sizeof(name), name, &ret_sz);
  OCL_ASSERT(ret == CL_SUCCESS);
  OCL_ASSERT(ret_sz == strlen("int*") + 1);
  OCL_ASSERT(!strcmp(name, "int*"));

  ret = clGetKernelArgInfo(kernel, 1, CL_KERNEL_ARG_NAME,
                           sizeof(name), name, &ret_sz);
  OCL_ASSERT(ret == CL_SUCCESS);
  OCL_ASSERT(ret_sz == strlen("dst") + 1);
  OCL_ASSERT(!strcmp(name, "dst"));

  ret = clGetKernelArgInfo(kernel, 1, CL_KERNEL_ARG_TYPE_QUALIFIER,
                           sizeof(type_qual), &type_qual, &ret_sz);
  OCL_ASSERT(ret == CL_SUCCESS);
  OCL_ASSERT(ret_sz == sizeof(cl_kernel_arg_type_qualifier));
  OCL_ASSERT(type_qual == CL_KERNEL_ARG_TYPE_NONE);

  //Arg 2
  ret = clGetKernelArgInfo(kernel, 2, CL_KERNEL_ARG_TYPE_NAME,
                           sizeof(name), name, &ret_sz);
  OCL_ASSERT(ret == CL_SUCCESS);
  OCL_ASSERT(ret_sz == strlen("test_arg_struct") + 1);
  OCL_ASSERT(!strcmp(name, "test_arg_struct"));
}

MAKE_UTEST_FROM_FUNCTION(test_get_arg_info);

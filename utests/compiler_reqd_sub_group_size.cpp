#include "utest_helper.hpp"
#include<string>
#include<sstream>
#include<iostream>

using namespace std;

void compiler_reqd_sub_group_size(void)
{
  if (!cl_check_reqd_subgroup())
    return;

  size_t param_value_size;
  OCL_CALL(clGetDeviceInfo, device, CL_DEVICE_SUB_GROUP_SIZES_INTEL,
           0, NULL, &param_value_size);

  size_t* param_value = new size_t[param_value_size];
  OCL_CALL(clGetDeviceInfo, device, CL_DEVICE_SUB_GROUP_SIZES_INTEL,
           param_value_size, param_value, NULL);

  const char* opt = "-D SIMD_SIZE=";
  for( uint32_t i = 0; i < param_value_size / sizeof(size_t) ; ++i)
  {
    ostringstream ss;
    uint32_t simd_size = param_value[i];
    ss << opt << simd_size;
    //cout << "options: " << ss.str() << endl;
    OCL_CALL(cl_kernel_init, "compiler_reqd_sub_group_size.cl", "compiler_reqd_sub_group_size",
                             SOURCE, ss.str().c_str());
    size_t SIMD_SIZE = 0;
    OCL_CALL(utestclGetKernelSubGroupInfoKHR,kernel,device, CL_KERNEL_COMPILE_SUB_GROUP_SIZE_INTEL,0, NULL,sizeof(size_t),&SIMD_SIZE,NULL);
    //cout << SIMD_SIZE << " with " << simd_size << endl;
    OCL_ASSERT(SIMD_SIZE == simd_size);

    cl_ulong SPILL_SIZE = 0xFFFFFFFF;
    OCL_CALL(clGetKernelWorkGroupInfo, kernel, device, CL_KERNEL_SPILL_MEM_SIZE_INTEL, sizeof(cl_ulong), &SPILL_SIZE, NULL);
    //cout << "spill size: " << SPILL_SIZE << endl;
    OCL_ASSERT(SPILL_SIZE == 0);

    clReleaseProgram(program);
    program = NULL;
  }
  delete[] param_value;
}

MAKE_UTEST_FROM_FUNCTION(compiler_reqd_sub_group_size);

/* test case for OpenCL 1.1 Preprocessor Directives & Macros (section 6.9) */
__kernel_exec(1, float4) void compiler_preprocessor_macros()
{
#pragma OPENCL FP_CONTRACT ON
#pragma OPENCL FP_CONTRACT OFF
#pragma OPENCL FP_CONTRACT DEFAULT
  int i = __OPENCL_VERSION__;
  i = __CL_VERSION_1_0__;
  i = __CL_VERSION_1_1__;
  i = __ENDIAN_LITTLE__;
  i = __IMAGE_SUPPORT__;
  i = __FAST_RELAXED_MATH__;
}

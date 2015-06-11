#pragma OPENCL EXTENSION cl_khr_fp16 : enable

#define MATH_KERNEL_ARG1(NAME) \
    kernel void compiler_half_math_##NAME(global half *src, global half *dst) { \
    int i = get_global_id(0); \
    dst[i] = NAME(src[i]); \
  }

MATH_KERNEL_ARG1(sin);
MATH_KERNEL_ARG1(cos);
MATH_KERNEL_ARG1(sinh);
MATH_KERNEL_ARG1(cosh);
MATH_KERNEL_ARG1(tan);
MATH_KERNEL_ARG1(log10);
MATH_KERNEL_ARG1(log);
MATH_KERNEL_ARG1(trunc);
MATH_KERNEL_ARG1(exp);
MATH_KERNEL_ARG1(sqrt);
MATH_KERNEL_ARG1(ceil);

#define MATH_KERNEL_ARG2(NAME) \
    kernel void compiler_half_math_##NAME(global half4 *src0, global half4 *src1, global half4 *dst) { \
    int i = get_global_id(0); \
    dst[i] = NAME(src0[i], src1[i]); \
  }
MATH_KERNEL_ARG2(fmod);
MATH_KERNEL_ARG2(fmax);
MATH_KERNEL_ARG2(fmin);

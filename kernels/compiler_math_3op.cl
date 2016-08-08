#ifdef HALF
#pragma OPENCL EXTENSION cl_khr_fp16 : enable
kernel void compiler_math_3op_half(global half *dst, global half *src1, global half *src2, global half *src3) {
  int i = get_global_id(0);
  const half x = src1[i], y = src2[i], z = src3[i];
  switch (i%2) {
    case 0: dst[i] = mad(x, y, z); break;
    case 1: dst[i] = fma(x, y, z); break;
    default: dst[i] = 1.f; break;
  };
  dst[0] = mad(src1[0],src2[0],src3[0]);
}
#else
kernel void compiler_math_3op_float(global float *dst, global float *src1, global float *src2, global float *src3) {

  int i = get_global_id(0);
  const float x = src1[i], y = src2[i], z = src3[i];
  switch (i%2) {
    case 0: dst[i] = mad(x, y, z); break;
    case 1: dst[i] = fma(x, y, z); break;
    default: dst[i] = 1.f; break;
  };
  dst[0] = mad(src1[0],src2[0],src3[0]);
}
#endif

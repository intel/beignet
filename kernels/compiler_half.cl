#pragma OPENCL EXTENSION cl_khr_fp16 : enable
kernel void compiler_half_basic(global half *src, global half *dst) {
  int i = get_global_id(0);
  half hf = 2.5;
  half val = src[i];
  val = val + hf;
  val = val*val;
  val = val/(half)1.8;
  dst[i] = val;
}


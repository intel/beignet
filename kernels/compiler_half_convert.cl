#pragma OPENCL EXTENSION cl_khr_fp16 : enable
kernel void compiler_half_to_long_sat(global half *src, global long *dst) {
  int i = get_global_id(0);
  dst[i] = convert_long_sat(src[i]);
}

kernel void compiler_ulong_to_half(global ulong *src, global half *dst) {
  int i = get_global_id(0);
  dst[i] = convert_half(src[i]);
}

kernel void compiler_half_to_long(global half *src, global long *dst) {
  int i = get_global_id(0);
  dst[i] = convert_long(src[i]);
}

kernel void compiler_int_to_half(global int *src, global half *dst) {
  int i = get_global_id(0);
  dst[i] = convert_half(src[i]);
}

kernel void compiler_uchar_to_half(global uchar *src, global half *dst) {
  int i = get_global_id(0);
  dst[i] = convert_half(src[i]);
}

kernel void compiler_half_to_uint_sat(global half *src, global uint *dst) {
  int i = get_global_id(0);
  dst[i] = convert_uint(src[i]);
}

kernel void compiler_half_to_ushort_sat(global half *src, global ushort *dst) {
  int i = get_global_id(0);
  dst[i] = convert_ushort(src[i]);
}

kernel void compiler_half_to_char_sat(global half *src, global char *dst) {
  int i = get_global_id(0);
  dst[i] = convert_char_sat(src[i]);
}

kernel void compiler_half2_as_int(global half2 *src, global int *dst) {
  int i = get_global_id(0);
  dst[i] = as_int(src[i]);
}

kernel void compiler_half_as_char2(global half *src, global char2 *dst) {
  int i = get_global_id(0);
  dst[i] = as_char2(src[i]);
}

kernel void compiler_half_to_float(global half4 *src, global float4 *dst) {
  int i = get_global_id(0);
  dst[i] = convert_float4(src[i]);
}

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
kernel void compiler_half_to_double(global half *src, global double *dst) {
  int i = get_global_id(0);
  dst[i] = src[i];
}
kernel void compiler_double_to_half(global double *src, global half *dst) {
  int i = get_global_id(0);
  dst[i] = src[i];
}

/*
 * Subgroup scan inclusive add functions
 */
#ifndef HALF
kernel void compiler_subgroup_scan_inclusive_add_short(global short *src, global short *dst) {
  short val = src[get_global_id(0)];
  short sum = sub_group_scan_inclusive_add(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_add_ushort(global ushort *src, global ushort *dst) {
  ushort val = src[get_global_id(0)];
  ushort sum = sub_group_scan_inclusive_add(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_add_int(global int *src, global int *dst) {
  int val = src[get_global_id(0)];
  int sum = sub_group_scan_inclusive_add(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_add_uint(global uint *src, global uint *dst) {
  uint val = src[get_global_id(0)];
  uint sum = sub_group_scan_inclusive_add(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_add_long(global long *src, global long *dst) {
  long val = src[get_global_id(0)];
  long sum = sub_group_scan_inclusive_add(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_add_ulong(global ulong *src, global ulong *dst) {
  ulong val = src[get_global_id(0)];
  ulong sum = sub_group_scan_inclusive_add(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_add_float(global float *src, global float *dst) {
  float val = src[get_global_id(0)];
  float sum = sub_group_scan_inclusive_add(val);
  dst[get_global_id(0)] = sum;
}

/*
 * Subgroup scan inclusive max functions
 */
kernel void compiler_subgroup_scan_inclusive_max_short(global short *src, global short *dst) {
  short val = src[get_global_id(0)];
  short sum = sub_group_scan_inclusive_max(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_max_ushort(global ushort *src, global ushort *dst) {
  ushort val = src[get_global_id(0)];
  ushort sum = sub_group_scan_inclusive_max(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_max_int(global int *src, global int *dst) {
  int val = src[get_global_id(0)];
  int sum = sub_group_scan_inclusive_max(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_max_uint(global uint *src, global uint *dst) {
  uint val = src[get_global_id(0)];
  uint sum = sub_group_scan_inclusive_max(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_max_long(global long *src, global long *dst) {
  long val = src[get_global_id(0)];
  long sum = sub_group_scan_inclusive_max(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_max_ulong(global ulong *src, global ulong *dst) {
  ulong val = src[get_global_id(0)];
  ulong sum = sub_group_scan_inclusive_max(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_max_float(global float *src, global float *dst) {
  float val = src[get_global_id(0)];
  float sum = sub_group_scan_inclusive_max(val);
  dst[get_global_id(0)] = sum;
}

/*
 * Subgroup scan inclusive min functions
 */
kernel void compiler_subgroup_scan_inclusive_min_short(global short *src, global short *dst) {
  short val = src[get_global_id(0)];
  short sum = sub_group_scan_inclusive_min(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_min_ushort(global ushort *src, global ushort *dst) {
  ushort val = src[get_global_id(0)];
  ushort sum = sub_group_scan_inclusive_min(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_min_int(global int *src, global int *dst) {
  int val = src[get_global_id(0)];
  int sum = sub_group_scan_inclusive_min(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_min_uint(global uint *src, global uint *dst) {
  uint val = src[get_global_id(0)];
  uint sum = sub_group_scan_inclusive_min(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_min_long(global long *src, global long *dst) {
  long val = src[get_global_id(0)];
  long sum = sub_group_scan_inclusive_min(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_min_ulong(global ulong *src, global ulong *dst) {
  ulong val = src[get_global_id(0)];
  ulong sum = sub_group_scan_inclusive_min(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_subgroup_scan_inclusive_min_float(global float *src, global float *dst) {
  float val = src[get_global_id(0)];
  float sum = sub_group_scan_inclusive_min(val);
  dst[get_global_id(0)] = sum;
}
#else
#pragma OPENCL EXTENSION cl_khr_fp16 : enable
kernel void compiler_subgroup_scan_inclusive_add_half(global half *src, global half *dst) {
  half val = src[get_global_id(0)];
  half sum = sub_group_scan_inclusive_add(val);
  dst[get_global_id(0)] = sum;
}
kernel void compiler_subgroup_scan_inclusive_max_half(global half *src, global half *dst) {
  half val = src[get_global_id(0)];
  half sum = sub_group_scan_inclusive_max(val);
  dst[get_global_id(0)] = sum;
}
kernel void compiler_subgroup_scan_inclusive_min_half(global half *src, global half *dst) {
  half val = src[get_global_id(0)];
  half sum = sub_group_scan_inclusive_min(val);
  dst[get_global_id(0)] = sum;
}
#endif

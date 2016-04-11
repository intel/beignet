/*
 * Workgroup scan inclusive add functions
 */
kernel void compiler_workgroup_scan_inclusive_add_int(global int *src, global int *dst) {
  int val = src[get_global_id(0)];
  int sum = work_group_scan_inclusive_add(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_inclusive_add_uint(global uint *src, global uint *dst) {
  uint val = src[get_global_id(0)];
  uint sum = work_group_scan_inclusive_add(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_inclusive_add_long(global long *src, global long *dst) {
  long val = src[get_global_id(0)];
  long sum = work_group_scan_inclusive_add(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_inclusive_add_ulong(global ulong *src, global ulong *dst) {
  ulong val = src[get_global_id(0)];
  ulong sum = work_group_scan_inclusive_add(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_inclusive_add_float(global float *src, global float *dst) {
  float val = src[get_global_id(0)];
  float sum = work_group_scan_inclusive_add(val);
  dst[get_global_id(0)] = sum;
}

/*
 * Workgroup scan inclusive max functions
 */
kernel void compiler_workgroup_scan_inclusive_max_int(global int *src, global int *dst) {
  int val = src[get_global_id(0)];
  int sum = work_group_scan_inclusive_max(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_inclusive_max_uint(global uint *src, global uint *dst) {
  uint val = src[get_global_id(0)];
  uint sum = work_group_scan_inclusive_max(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_inclusive_max_long(global long *src, global long *dst) {
  long val = src[get_global_id(0)];
  long sum = work_group_scan_inclusive_max(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_inclusive_max_ulong(global ulong *src, global ulong *dst) {
  ulong val = src[get_global_id(0)];
  ulong sum = work_group_scan_inclusive_max(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_inclusive_max_float(global float *src, global float *dst) {
  float val = src[get_global_id(0)];
  float sum = work_group_scan_inclusive_max(val);
  dst[get_global_id(0)] = sum;
}

/*
 * Workgroup scan inclusive min functions
 */
kernel void compiler_workgroup_scan_inclusive_min_int(global int *src, global int *dst) {
  int val = src[get_global_id(0)];
  int sum = work_group_scan_inclusive_min(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_inclusive_min_uint(global uint *src, global uint *dst) {
  uint val = src[get_global_id(0)];
  uint sum = work_group_scan_inclusive_min(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_inclusive_min_long(global long *src, global long *dst) {
  long val = src[get_global_id(0)];
  long sum = work_group_scan_inclusive_min(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_inclusive_min_ulong(global ulong *src, global ulong *dst) {
  ulong val = src[get_global_id(0)];
  ulong sum = work_group_scan_inclusive_min(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_inclusive_min_float(global float *src, global float *dst) {
  float val = src[get_global_id(0)];
  float sum = work_group_scan_inclusive_min(val);
  dst[get_global_id(0)] = sum;
}

/*
 * Workgroup scan exclusive add functions
 */
kernel void compiler_workgroup_scan_exclusive_add_int(global int *src, global int *dst) {
  int val = src[get_global_id(0)];
  int sum = work_group_scan_exclusive_add(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_exclusive_add_uint(global uint *src, global uint *dst) {
  uint val = src[get_global_id(0)];
  uint sum = work_group_scan_exclusive_add(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_exclusive_add_long(global long *src, global long *dst) {
  long val = src[get_global_id(0)];
  long sum = work_group_scan_exclusive_add(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_exclusive_add_ulong(global ulong *src, global ulong *dst) {
  ulong val = src[get_global_id(0)];
  ulong sum = work_group_scan_exclusive_add(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_exclusive_add_float(global float *src, global float *dst) {
  float val = src[get_global_id(0)];
  float sum = work_group_scan_exclusive_add(val);
  dst[get_global_id(0)] = sum;
}

/*
 * Workgroup scan exclusive max functions
 */
kernel void compiler_workgroup_scan_exclusive_max_int(global int *src, global int *dst) {
  int val = src[get_global_id(0)];
  int sum = work_group_scan_exclusive_max(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_exclusive_max_uint(global uint *src, global uint *dst) {
  uint val = src[get_global_id(0)];
  uint sum = work_group_scan_exclusive_max(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_exclusive_max_long(global long *src, global long *dst) {
  long val = src[get_global_id(0)];
  long sum = work_group_scan_exclusive_max(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_exclusive_max_ulong(global ulong *src, global ulong *dst) {
  ulong val = src[get_global_id(0)];
  ulong sum = work_group_scan_exclusive_max(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_exclusive_max_float(global float *src, global float *dst) {
  float val = src[get_global_id(0)];
  float sum = work_group_scan_exclusive_max(val);
  dst[get_global_id(0)] = sum;
}

/*
 * Workgroup scan exclusive min functions
 */
kernel void compiler_workgroup_scan_exclusive_min_int(global int *src, global int *dst) {
  int val = src[get_global_id(0)];
  int sum = work_group_scan_exclusive_min(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_exclusive_min_uint(global uint *src, global uint *dst) {
  uint val = src[get_global_id(0)];
  uint sum = work_group_scan_exclusive_min(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_exclusive_min_long(global long *src, global long *dst) {
  long val = src[get_global_id(0)];
  long sum = work_group_scan_exclusive_min(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_exclusive_min_ulong(global ulong *src, global ulong *dst) {
  ulong val = src[get_global_id(0)];
  ulong sum = work_group_scan_exclusive_min(val);
  dst[get_global_id(0)] = sum;
}

kernel void compiler_workgroup_scan_exclusive_min_float(global float *src, global float *dst) {
  float val = src[get_global_id(0)];
  float sum = work_group_scan_exclusive_min(val);
  dst[get_global_id(0)] = sum;
}

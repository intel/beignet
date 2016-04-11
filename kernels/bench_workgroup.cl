/*
 * Benchmark workgroup reduce add
 */
kernel void bench_workgroup_reduce_add_int(
  global int *src,
  global int *dst,
  int reduce_loop)
{
  int val;
  int result;

  for(; reduce_loop > 0; reduce_loop--){
    val = src[get_global_id(0)];
    result = work_group_reduce_add(val);
  }

  dst[get_global_id(0)] = result;
}

kernel void bench_workgroup_reduce_add_long(
  global long *src,
  global long *dst,
  int reduce_loop)
{
  long val;
  long result;

  for(; reduce_loop > 0; reduce_loop--){
    val = src[get_global_id(0)];
    result = work_group_reduce_add(val);
  }

  dst[get_global_id(0)] = result;
}

/*
 * Benchmark workgroup reduce min
 */
kernel void bench_workgroup_reduce_min_int(
  global int *src,
  global int *dst,
  int reduce_loop)
{
  int val;
  int result;

  for(; reduce_loop > 0; reduce_loop--){
    val = src[get_global_id(0)];
    result = work_group_reduce_min(val);
  }

  dst[get_global_id(0)] = result;
}

kernel void bench_workgroup_reduce_min_long(
  global long *src,
  global long *dst,
  int reduce_loop)
{
  long val;
  long result;

  for(; reduce_loop > 0; reduce_loop--){
    val = src[get_global_id(0)];
    result = work_group_reduce_min(val);
  }

  dst[get_global_id(0)] = result;
}

/*
 * Benchmark workgroup scan inclusive add
 */
kernel void bench_workgroup_scan_inclusive_add_int(
  global int *src,
  global int *dst,
  int reduce_loop)
{
  int val;
  int result;

  for(; reduce_loop > 0; reduce_loop--){
    val = src[get_global_id(0)];
    result = work_group_scan_inclusive_add(val);
  }

  dst[get_global_id(0)] = result;
}

kernel void bench_workgroup_scan_inclusive_add_long(
  global long *src,
  global long *dst,
  int reduce_loop)
{
  long val;
  long result;

  for(; reduce_loop > 0; reduce_loop--){
    val = src[get_global_id(0)];
    result = work_group_scan_inclusive_add(val);
  }

  dst[get_global_id(0)] = result;
}

/*
 * Benchmark workgroup scan inclusive min
 */
kernel void bench_workgroup_scan_inclusive_min_int(
  global int *src,
  global int *dst,
  int reduce_loop)
{
  int val;
  int result;

  for(; reduce_loop > 0; reduce_loop--){
    val = src[get_global_id(0)];
    result = work_group_scan_inclusive_add(val);
  }

  dst[get_global_id(0)] = result;
}

kernel void bench_workgroup_scan_inclusive_min_long(
  global long *src,
  global long *dst,
  int reduce_loop)
{
  long val;
  long result;

  for(; reduce_loop > 0; reduce_loop--){
    val = src[get_global_id(0)];
    result = work_group_scan_inclusive_add(val);
  }

  dst[get_global_id(0)] = result;
}


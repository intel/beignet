/*
 * Benchmark broadcast 1D
 */
kernel void bench_workgroup_broadcast_1D_int(global int *src,
                                  global int *dst,
                                  int reduce_loop,
                                  uint wg_local_x,
                                  uint wg_local_y)
{
  uint offset = 0;
  uint index = offset + get_global_id(0);

  int val = src[index];
  /* depending on generated ASM, volatile may be removed */
  volatile int result;

  for(; reduce_loop > 0; reduce_loop--){
    result = work_group_broadcast(val,
                                  wg_local_x);
  }

  dst[index] = result;
}

kernel void bench_workgroup_broadcast_1D_long(global long *src,
                                  global long *dst,
                                  int reduce_loop,
                                  uint wg_local_x,
                                  uint wg_local_y)
{
  uint offset = 0;
  uint index = offset + get_global_id(0);

  long val = src[index];
  /* depending on generated ASM, volatile may be removed */
  volatile long result;

  for(; reduce_loop > 0; reduce_loop--){
    result = work_group_broadcast(val,
                                  wg_local_x);
  }

  dst[index] = result;
}


/*
 * Benchmark broadcast 2D
 */
kernel void bench_workgroup_broadcast_2D_int(global int *src,
                                  global int *dst,
                                  int reduce_loop,
                                  uint wg_local_x,
                                  uint wg_local_y)
{
  uint lsize = get_local_size(0) * get_local_size(1);
  uint offset = get_group_id(0) * lsize +
      get_group_id(1) * get_num_groups(0) * lsize;
  uint index = offset + get_local_id(0) +
      get_local_id(1) * get_local_size(0);

  int val = src[index];
  /* depending on generated ASM, volatile may be removed */
  int result;

  for(; reduce_loop > 0; reduce_loop--){
    result = work_group_broadcast(val,
                                  wg_local_x,
                                  wg_local_y);
  }

  dst[index] = result;
}

kernel void bench_workgroup_broadcast_2D_long(global long *src,
                                  global long *dst,
                                  int reduce_loop,
                                  uint wg_local_x,
                                  uint wg_local_y)
{
  uint lsize = get_local_size(0) * get_local_size(1);
  uint offset = get_group_id(0) * lsize +
      get_group_id(1) * get_num_groups(0) * lsize;
  uint index = offset + get_local_id(0) +
      get_local_id(1) * get_local_size(0);

  long val = src[index];
  /* depending on generated ASM, volatile may be removed */
  long result;

  for(; reduce_loop > 0; reduce_loop--){
    result = work_group_broadcast(val,
                                  wg_local_x,
                                  wg_local_y);
  }

  dst[index] = result;
}

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


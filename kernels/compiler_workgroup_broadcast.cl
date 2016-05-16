/*
 * Workgroup broadcast 1D functions
 */

kernel void compiler_workgroup_broadcast_1D_int(global int *src,
                                                global int *dst,
                                                uint wg_local_x,
                                                uint wg_local_y,
                                                uint wg_local_z)
{
  uint offset = 0;
  uint index = offset + get_global_id(0);

  int val = src[index];
  int broadcast_val = work_group_broadcast(val,
                                            wg_local_x);
  dst[index] = broadcast_val;
}
#if 0
kernel void compiler_workgroup_broadcast_1D_long(global long *src,
                                                global long *dst,
                                                uint wg_local_x,
                                                uint wg_local_y,
                                                uint wg_local_z)
{
  uint offset = 0;
  uint index = offset + get_global_id(0);

  long val = src[index];
  long broadcast_val = work_group_broadcast(val,
                                            wg_local_x);
  dst[index] = broadcast_val;
}
#endif
/*
 * Workgroup broadcast 2D functions
 */
kernel void compiler_workgroup_broadcast_2D_int(global int *src,
                                                global int *dst,
                                                uint wg_local_x,
                                                uint wg_local_y,
                                                uint wg_local_z)
{
  uint lsize = get_local_size(0) * get_local_size(1);
  uint offset = get_group_id(0) * lsize +
      get_group_id(1) * get_num_groups(0) * lsize;
  uint index = offset + get_local_id(0) +
      get_local_id(1) * get_local_size(0);

  int val = src[index];
  int broadcast_val = work_group_broadcast(val,
                                            wg_local_x,
                                            wg_local_y);
  dst[index] = broadcast_val;
}
#if 0
kernel void compiler_workgroup_broadcast_2D_long(global long *src,
                                                global long *dst,
                                                uint wg_local_x,
                                                uint wg_local_y,
                                                uint wg_local_z)
{
  uint lsize = get_local_size(0) * get_local_size(1);
  uint offset = get_group_id(0) * lsize +
      get_group_id(1) * get_num_groups(0) * lsize;
  uint index = offset + get_local_id(0) +
      get_local_id(1) * get_local_size(0);

  long val = src[index];
  long broadcast_val = work_group_broadcast(val,
                                            wg_local_x,
                                            wg_local_y);
  dst[index] = broadcast_val;
}
#endif
/*
 * Workgroup broadcast 3D functions
 */
kernel void compiler_workgroup_broadcast_3D_int(global int *src,
                                                global int *dst,
                                                uint wg_local_x,
                                                uint wg_local_y,
                                                uint wg_local_z)
{
  uint lsize = get_local_size(0) * get_local_size(1) * get_local_size(2);
  uint offset = get_group_id(0) * lsize +
      get_group_id(1) * get_num_groups(0) * lsize +
      get_group_id(2) * get_num_groups(1) * get_num_groups(0) * lsize;
  uint index = offset + get_local_id(0) +
      get_local_id(1) * get_local_size(0) +
      get_local_id(2) * get_local_size(1) * get_local_size(0);

  int val = src[index];
  int broadcast_val = work_group_broadcast(val,
                                            wg_local_x,
                                            wg_local_y,
                                            wg_local_z);
  dst[index] = broadcast_val;
}
#if 0
kernel void compiler_workgroup_broadcast_3D_long(global long *src,
                                                global long *dst,
                                                uint wg_local_x,
                                                uint wg_local_y,
                                                uint wg_local_z)
{
  uint lsize = get_local_size(0) * get_local_size(1) * get_local_size(2);
  uint offset = get_group_id(0) * lsize +
      get_group_id(1) * get_num_groups(0) * lsize +
      get_group_id(2) * get_num_groups(0) * get_num_groups(1) * lsize;
  uint index = offset + get_local_id(0) +
      get_local_id(1) * get_local_size(0) +
      get_local_id(2) * get_local_size(1) * get_local_size(0);

  long val = src[index];
  long broadcast_val = work_group_broadcast(val,
                                            wg_local_x,
                                            wg_local_y,
                                            wg_local_z);
  dst[index] = broadcast_val;
}
#endif

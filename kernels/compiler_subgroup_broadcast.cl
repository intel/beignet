/*
 * Subgroup broadcast 1D functions
 */

kernel void compiler_subgroup_broadcast_imm_int(global int *src,
                                                global int *dst,
                                                uint simd_id)
{
  uint index = get_global_id(0);

  int val = src[index];
  int broadcast_val = sub_group_broadcast(val, 10);
  dst[index] = broadcast_val;
}
kernel void compiler_subgroup_broadcast_int(global int *src,
                                                global int *dst,
                                                uint simd_id)
{
  uint index = get_global_id(0);

  int val = src[index];
  int broadcast_val = sub_group_broadcast(val, simd_id);
  dst[index] = broadcast_val;
}
kernel void compiler_subgroup_broadcast_long(global long *src,
                                                global long *dst,
                                                uint simd_id)
{
  uint index = get_global_id(0);

  long val = src[index];
  long broadcast_val = sub_group_broadcast(val, simd_id);
  dst[index] = broadcast_val;
}

/*
 * Subgroup broadcast 1D functions
 */
#ifndef HALF
kernel void compiler_subgroup_broadcast_imm_int(global int *src,
                                                global int *dst,
                                                uint simd_id)
{
  uint index = get_global_id(0);

  int val = src[index];
  int broadcast_val = sub_group_broadcast(val, 2);
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
kernel void compiler_subgroup_broadcast_short(global short *src,
                                                global short *dst,
                                                uint simd_id)
{
  uint index = get_global_id(0);

  short val = src[index];
  short broadcast_val = sub_group_broadcast(val, simd_id);
  dst[index] = broadcast_val;
}
#else
#pragma OPENCL EXTENSION cl_khr_fp16 : enable
kernel void compiler_subgroup_broadcast_half(global half *src,
                                                global half *dst,
                                                uint simd_id)
{
  uint index = get_global_id(0);

  half val = src[index];
  half broadcast_val = sub_group_broadcast(val, simd_id);
  //printf("%d val %d is %d\n",index,as_ushort(val), as_ushort(broadcast_val));
  dst[index] = broadcast_val;
}
#endif

__kernel void compiler_sub_group_shuffle_up_int(global int *dst, int c)
{
  int i = get_global_id(0);
  if (i == 0)
    dst[0] = get_max_sub_group_size();
  dst++;

  int from = i;
  int j = get_sub_group_local_id() + 1;
  int k = get_max_sub_group_size() - get_sub_group_local_id() - 1;
  int o0 = intel_sub_group_shuffle_up(123, 456, c);
  int o1 = intel_sub_group_shuffle_up(123, from, c);
  int o2 = intel_sub_group_shuffle_up(from, -from, k);
  int o3 = intel_sub_group_shuffle_up(from, 321, j);
  dst[i*4] = o0;
  dst[i*4+1] = o1;
  dst[i*4+2] = o2;
  dst[i*4+3] = o3;
}
#ifdef SHORT
__kernel void compiler_sub_group_shuffle_up_short(global short *dst, int c)
{
  short i = get_global_id(0);
  if (i == 0)
    dst[0] = get_max_sub_group_size();
  dst++;

  short from = i;
  int j = get_sub_group_local_id() + 1;
  int k = get_max_sub_group_size() - get_sub_group_local_id() - 1;
  short o0 = intel_sub_group_shuffle_up((short)123, (short)456, c);
  short o1 = intel_sub_group_shuffle_up((short)123, from, c);
  short o2 = intel_sub_group_shuffle_up(from, (short)-from, k);
  short o3 = intel_sub_group_shuffle_up(from, (short)321, j);
  dst[i*4] = o0;
  dst[i*4+1] = o1;
  dst[i*4+2] = o2;
  dst[i*4+3] = o3;
}
#endif

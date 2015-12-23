__kernel void compiler_get_sub_group_local_id(global int *dst)
{
  int i = get_global_id(0);
  if (i == 0)
    dst[0] = get_max_sub_group_size();

  dst[i+1] = get_sub_group_local_id();
}

__kernel void compiler_get_max_sub_group_size(global int *dst)
{
  int i = get_global_id(0);
  dst[i] = get_max_sub_group_size();
}

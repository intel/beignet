__kernel void compiler_get_sub_group_size(global int *dst)
{
  int i = get_global_id(0);
  dst[i] = get_sub_group_size();
}

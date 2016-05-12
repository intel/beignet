__kernel void builtin_sub_group_id(global int *dst)
{
  int lid = get_local_linear_id();
  int lsz = get_local_size(0) * get_local_size(1) * get_local_size(2);
  int gid = lid + lsz*(get_num_groups(1) * get_num_groups(0) * get_group_id(2) + get_num_groups(0) * get_group_id(1) + get_group_id(0));
  dst[gid] = get_sub_group_id();
}

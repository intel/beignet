__kernel void compiler_subgroup_buffer_block_write1(global uint *src, global uint *dst)
{
  int id = get_global_id(0);
  global uint * p = dst + get_sub_group_id() * get_max_sub_group_size();
  intel_sub_group_block_write(p,src[id]);
}

__kernel void compiler_subgroup_buffer_block_write2(global uint2 *src, global uint *dst)
{
  int id = get_global_id(0);
  global uint * p = dst + get_sub_group_id() * get_max_sub_group_size()*2;
  intel_sub_group_block_write2(p,src[id]);
}

__kernel void compiler_subgroup_buffer_block_write4(global uint4 *src, global uint *dst)
{
  int id = get_global_id(0);
  global uint * p = dst + get_sub_group_id() * get_max_sub_group_size()*4;
  intel_sub_group_block_write4(p,src[id]);
}

__kernel void compiler_subgroup_buffer_block_write8(global uint8 *src, global uint *dst)
{
  int id = get_global_id(0);
  global uint * p = dst + get_sub_group_id() * get_max_sub_group_size()*8;
  intel_sub_group_block_write8(p,src[id]);
}

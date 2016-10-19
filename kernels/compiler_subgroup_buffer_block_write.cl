__kernel void compiler_subgroup_buffer_block_write_ui1(global uint *src, global uint *dst)
{
  int id = get_global_id(0);
  global uint * p = dst + get_sub_group_id() * get_max_sub_group_size();
  intel_sub_group_block_write_ui(p,src[id]);
}

__kernel void compiler_subgroup_buffer_block_write_ui2(global uint2 *src, global uint *dst)
{
  int id = get_global_id(0);
  global uint * p = dst + get_sub_group_id() * get_max_sub_group_size()*2;
  intel_sub_group_block_write_ui2(p,src[id]);
}

__kernel void compiler_subgroup_buffer_block_write_ui4(global uint4 *src, global uint *dst)
{
  int id = get_global_id(0);
  global uint * p = dst + get_sub_group_id() * get_max_sub_group_size()*4;
  intel_sub_group_block_write_ui4(p,src[id]);
}

__kernel void compiler_subgroup_buffer_block_write_ui8(global uint8 *src, global uint *dst)
{
  int id = get_global_id(0);
  global uint * p = dst + get_sub_group_id() * get_max_sub_group_size()*8;
  intel_sub_group_block_write_ui8(p,src[id]);
}
#ifdef SHORT
__kernel void compiler_subgroup_buffer_block_write_us1(global ushort *src, global ushort *dst)
{
  int id = get_global_id(0);
  global ushort * p = dst + get_sub_group_id() * get_max_sub_group_size();
  intel_sub_group_block_write_us(p,src[id]);
}

__kernel void compiler_subgroup_buffer_block_write_us2(global ushort2 *src, global ushort *dst)
{
  int id = get_global_id(0);
  global ushort * p = dst + get_sub_group_id() * get_max_sub_group_size()*2;
  intel_sub_group_block_write_us2(p,src[id]);
}

__kernel void compiler_subgroup_buffer_block_write_us4(global ushort4 *src, global ushort *dst)
{
  int id = get_global_id(0);
  global ushort * p = dst + get_sub_group_id() * get_max_sub_group_size()*4;
  intel_sub_group_block_write_us4(p,src[id]);
}
__kernel void compiler_subgroup_buffer_block_write_us8(global ushort8 *src, global ushort *dst)
{
  int id = get_global_id(0);
  global ushort * p = dst + get_sub_group_id() * get_max_sub_group_size()*8;
  intel_sub_group_block_write_us8(p,src[id]);
}
#endif

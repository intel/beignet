__kernel void compiler_subgroup_buffer_block_read_ui1(global uint *src, global uint *dst)
{
  int id = get_global_id(0);
  global uint * p = src + get_sub_group_id() * get_max_sub_group_size();
  uint tmp = intel_sub_group_block_read_ui(p);
  dst[id] = tmp;
}

__kernel void compiler_subgroup_buffer_block_read_ui2(global uint *src, global uint2 *dst)
{
  int id = get_global_id(0);
  global uint * p = src + get_sub_group_id() * get_max_sub_group_size()*2;
  uint2 tmp = intel_sub_group_block_read_ui2(p);
  dst[id] = tmp;
}

__kernel void compiler_subgroup_buffer_block_read_ui4(global uint *src, global uint4 *dst)
{
  int id = get_global_id(0);
  global uint * p = src + get_sub_group_id() * get_max_sub_group_size()*4;
  uint4 tmp = intel_sub_group_block_read_ui4(p);
  dst[id] = tmp;
}

__kernel void compiler_subgroup_buffer_block_read_ui8(global uint *src, global uint8 *dst)
{
  int id = get_global_id(0);
  global uint * p = src + get_sub_group_id() * get_max_sub_group_size()*8;
  uint8 tmp = intel_sub_group_block_read_ui8(p);
  dst[id] = tmp;
}
#ifdef SHORT
__kernel void compiler_subgroup_buffer_block_read_us1(global ushort *src, global ushort *dst)
{
  int id = get_global_id(0);
  global ushort * p = src + get_sub_group_id() * get_max_sub_group_size();
  ushort tmp = intel_sub_group_block_read_us(p);
  dst[id] = tmp;
}
__kernel void compiler_subgroup_buffer_block_read_us2(global ushort *src, global ushort2 *dst)
{
  int id = get_global_id(0);
  global ushort * p = src + get_sub_group_id() * get_max_sub_group_size()*2;
  ushort2 tmp = intel_sub_group_block_read_us2(p);
  dst[id] = tmp;
}
__kernel void compiler_subgroup_buffer_block_read_us4(global ushort *src, global ushort4 *dst)
{
  int id = get_global_id(0);
  global ushort * p = src + get_sub_group_id() * get_max_sub_group_size()*4;
  ushort4 tmp = intel_sub_group_block_read_us4(p);
  dst[id] = tmp;
}

__kernel void compiler_subgroup_buffer_block_read_us8(global ushort *src, global ushort8 *dst)
{
  int id = get_global_id(0);
  global ushort * p = src + get_sub_group_id() * get_max_sub_group_size()*8;
  ushort8 tmp = intel_sub_group_block_read_us8(p);
  dst[id] = tmp;
}
#endif

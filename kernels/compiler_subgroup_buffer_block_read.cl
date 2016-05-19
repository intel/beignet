__kernel void compiler_subgroup_buffer_block_read1(global uint *src, global uint *dst)
{
  int id = get_global_id(0);
  global uint * p = src + get_sub_group_id() * get_max_sub_group_size();
  uint tmp = intel_sub_group_block_read(p);
  dst[id] = tmp;
}

__kernel void compiler_subgroup_buffer_block_read2(global uint *src, global uint2 *dst)
{
  int id = get_global_id(0);
  global uint * p = src + get_sub_group_id() * get_max_sub_group_size()*2;
  uint2 tmp = intel_sub_group_block_read2(p);
  dst[id] = tmp;
}

__kernel void compiler_subgroup_buffer_block_read4(global uint *src, global uint4 *dst)
{
  int id = get_global_id(0);
  global uint * p = src + get_sub_group_id() * get_max_sub_group_size()*4;
  uint4 tmp = intel_sub_group_block_read4(p);
  dst[id] = tmp;
}

__kernel void compiler_subgroup_buffer_block_read8(global uint *src, global uint8 *dst)
{
  int id = get_global_id(0);
  global uint * p = src + get_sub_group_id() * get_max_sub_group_size()*8;
  uint8 tmp = intel_sub_group_block_read8(p);
  dst[id] = tmp;
}

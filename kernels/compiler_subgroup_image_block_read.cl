__kernel void compiler_subgroup_image_block_read1(image2d_t src, global uint *dst)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(uint),0);
  uint tmp = intel_sub_group_block_read(src,coord);
  dst[id] = tmp;
}

__kernel void compiler_subgroup_image_block_read2(image2d_t src, global uint2 *dst)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(uint),0);
  uint2 tmp = intel_sub_group_block_read2(src,coord);
  dst[id] = tmp;
}

__kernel void compiler_subgroup_image_block_read4(image2d_t src, global uint4 *dst)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(uint),0);
  uint4 tmp = intel_sub_group_block_read4(src,coord);
  dst[id] = tmp;
}

__kernel void compiler_subgroup_image_block_read8(image2d_t src, global uint8 *dst)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(uint),0);
  uint8 tmp = intel_sub_group_block_read8(src,coord);
  dst[id] = tmp;
}

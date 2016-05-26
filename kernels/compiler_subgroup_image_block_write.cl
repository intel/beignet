__kernel void compiler_subgroup_image_block_write1(image2d_t dst, global uint *src)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(uint),0);
  intel_sub_group_block_write(dst,coord, src[id]);
}

__kernel void compiler_subgroup_image_block_write2(image2d_t dst, global uint2 *src)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(uint),0);
  intel_sub_group_block_write2(dst,coord, src[id]);
}

__kernel void compiler_subgroup_image_block_write4(image2d_t dst, global uint4 *src)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(uint),0);
  intel_sub_group_block_write4(dst,coord, src[id]);
}

__kernel void compiler_subgroup_image_block_write8(image2d_t dst, global uint8 *src)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(uint),0);
  intel_sub_group_block_write8(dst,coord, src[id]);
}

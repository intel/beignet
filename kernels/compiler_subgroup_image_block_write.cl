__kernel void compiler_subgroup_image_block_write_ui1(image2d_t dst, global uint *src)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(uint),0);
  intel_sub_group_block_write_ui(dst,coord, src[id]);
}
__kernel void compiler_subgroup_image_block_write_ui2(image2d_t dst, global uint2 *src)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(uint),0);
  intel_sub_group_block_write_ui2(dst,coord, src[id]);
}

__kernel void compiler_subgroup_image_block_write_ui4(image2d_t dst, global uint4 *src)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(uint),0);
  intel_sub_group_block_write_ui4(dst,coord, src[id]);
}

__kernel void compiler_subgroup_image_block_write_ui8(image2d_t dst, global uint8 *src)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(uint),0);
  intel_sub_group_block_write_ui8(dst,coord, src[id]);
}
#ifdef SHORT
__kernel void compiler_subgroup_image_block_write_us1(image2d_t dst, global ushort *src)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(ushort),0);
  intel_sub_group_block_write_us(dst,coord, src[id]);
}

__kernel void compiler_subgroup_image_block_write_us2(image2d_t dst, global ushort2 *src)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(ushort),0);
  intel_sub_group_block_write_us2(dst,coord, src[id]);
}

__kernel void compiler_subgroup_image_block_write_us4(image2d_t dst, global ushort4 *src)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(ushort),0);
  intel_sub_group_block_write_us4(dst,coord, src[id]);
}

__kernel void compiler_subgroup_image_block_write_us8(image2d_t dst, global ushort8 *src)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(ushort),0);
  intel_sub_group_block_write_us8(dst,coord, src[id]);
}
#endif

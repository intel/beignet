__kernel void compiler_subgroup_image_block_read_ui1(image2d_t src, global uint *dst)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(uint),0);
  uint tmp = intel_sub_group_block_read_ui(src,coord);
  dst[id] = tmp;
}

__kernel void compiler_subgroup_image_block_read_ui2(image2d_t src, global uint2 *dst)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(uint),0);
  uint2 tmp = intel_sub_group_block_read_ui2(src,coord);
  dst[id] = tmp;
}

__kernel void compiler_subgroup_image_block_read_ui4(image2d_t src, global uint4 *dst)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(uint),0);
  uint4 tmp = intel_sub_group_block_read_ui4(src,coord);
  dst[id] = tmp;
}

__kernel void compiler_subgroup_image_block_read_ui8(image2d_t src, global uint8 *dst)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(uint),0);
  uint8 tmp = intel_sub_group_block_read_ui8(src,coord);
  dst[id] = tmp;
}
#ifdef SHORT
__kernel void compiler_subgroup_image_block_read_us1(image2d_t src, global ushort *dst)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(ushort),0);
  ushort tmp = intel_sub_group_block_read_us(src,coord);
  dst[id] = tmp;
}

__kernel void compiler_subgroup_image_block_read_us2(image2d_t src, global ushort2 *dst)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(ushort),0);
  ushort2 tmp = intel_sub_group_block_read_us2(src,coord);
  dst[id] = tmp;
}

__kernel void compiler_subgroup_image_block_read_us4(image2d_t src, global ushort4 *dst)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(ushort),0);
  ushort4 tmp = intel_sub_group_block_read_us4(src,coord);
  dst[id] = tmp;
}

__kernel void compiler_subgroup_image_block_read_us8(image2d_t src, global ushort8 *dst)
{
  int id = get_global_id(0);
  int2 coord = (int2)(get_simd_size()*get_sub_group_id()*sizeof(ushort),0);
  ushort8 tmp = intel_sub_group_block_read_us8(src,coord);
  dst[id] = tmp;
}
#endif

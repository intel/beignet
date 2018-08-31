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
#ifdef MEDIA_BLOCK_IO
__kernel void compiler_subgroup_media_block_write_ui1(image2d_t dst, global uint *src)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(uint) + sizeof(uint) * get_group_id(0) * get_local_size(0),yid);
  intel_sub_group_media_block_write_ui(coord, 16, 1, src[id], dst);
}
__kernel void compiler_subgroup_media_block_write_ui2(image2d_t dst, global uint2 *src)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(uint) + sizeof(uint) * get_group_id(0) * get_local_size(0),yid*2);
  intel_sub_group_media_block_write_ui2(coord, 16, 2, src[id], dst);
}
__kernel void compiler_subgroup_media_block_write_ui4(image2d_t dst, global uint4 *src)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(uint) + sizeof(uint) * get_group_id(0) * get_local_size(0),yid*4);
  intel_sub_group_media_block_write_ui4(coord, 16, 4, src[id], dst);
}
__kernel void compiler_subgroup_media_block_write_ui8(image2d_t dst, global uint8 *src)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(uint) + sizeof(uint) * get_group_id(0) * get_local_size(0),yid*8);
  intel_sub_group_media_block_write_ui8(coord, 16, 8, src[id], dst);
}
__kernel void compiler_subgroup_media_block_write_us1(image2d_t dst, global ushort *src)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(ushort) + sizeof(ushort) * get_group_id(0) * get_local_size(0),yid);
  intel_sub_group_media_block_write_us(coord, 16, 1, src[id], dst);
}
__kernel void compiler_subgroup_media_block_write_us2(image2d_t dst, global ushort2 *src)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(ushort) + sizeof(ushort) * get_group_id(0) * get_local_size(0),yid*2);
  intel_sub_group_media_block_write_us2(coord, 16, 2, src[id], dst);
}
__kernel void compiler_subgroup_media_block_write_us4(image2d_t dst, global ushort4 *src)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(ushort) + sizeof(ushort) * get_group_id(0) * get_local_size(0),yid*4);
  intel_sub_group_media_block_write_us4(coord, 16, 4, src[id], dst);
}
__kernel void compiler_subgroup_media_block_write_us8(image2d_t dst, global ushort8 *src)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(ushort) + sizeof(ushort) * get_group_id(0) * get_local_size(0),yid*8);
  intel_sub_group_media_block_write_us8(coord, 16, 8, src[id], dst);
}
__kernel void  __attribute__((intel_reqd_sub_group_size(8)))
compiler_subgroup_media_block_write_us16(image2d_t dst, global ushort16 *src)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(ushort) + sizeof(ushort) * get_group_id(0) * get_local_size(0),yid*16);
  intel_sub_group_media_block_write_us16(coord, 8, 16, src[id], dst);
}

__kernel void compiler_subgroup_media_block_write_uc1(image2d_t dst, global uchar *src)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(char) + sizeof(char) * get_group_id(0) * get_local_size(0),yid);
  intel_sub_group_media_block_write_uc(coord, 16, 1, src[id], dst);
}
__kernel void compiler_subgroup_media_block_write_uc2(image2d_t dst, global uchar2 *src)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(char) + sizeof(char) * get_group_id(0) * get_local_size(0),yid*2);
  intel_sub_group_media_block_write_uc2(coord, 16, 2, src[id], dst);
}
__kernel void compiler_subgroup_media_block_write_uc4(image2d_t dst, global uchar4 *src)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(char) + sizeof(char) * get_group_id(0) * get_local_size(0),yid*4);
  intel_sub_group_media_block_write_uc4(coord, 16, 4, src[id], dst);
}
__kernel void compiler_subgroup_media_block_write_uc8(image2d_t dst, global uchar8 *src)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(char) + sizeof(char) * get_group_id(0) * get_local_size(0),yid*8);
  intel_sub_group_media_block_write_uc8(coord, 16, 8, src[id], dst);
}
__kernel void compiler_subgroup_media_block_write_uc16(image2d_t dst, global uchar16 *src)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(char) + sizeof(char) * get_group_id(0) * get_local_size(0),yid*16);
  intel_sub_group_media_block_write_uc16(coord, 16, 16, src[id], dst);
}
#endif

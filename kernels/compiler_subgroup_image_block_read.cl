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
#ifdef MEDIA_BLOCK_IO
__kernel void compiler_subgroup_media_block_read_ui1(image2d_t src, global uint *dst)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(uint) + sizeof(uint) * get_group_id(0) * get_local_size(0),yid);
  uint tmp = intel_sub_group_media_block_read_ui(coord, 16, 1, src);
  dst[yid * get_global_size(0) + id] = tmp;
}
__kernel void compiler_subgroup_media_block_read_ui2(image2d_t src, global uint2 *dst)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(uint) + sizeof(uint) * get_group_id(0) * get_local_size(0),yid*2);
  uint2 tmp = intel_sub_group_media_block_read_ui2(coord, 16, 2, src);
  dst[yid * get_global_size(0) + id] = tmp;
}
__kernel void compiler_subgroup_media_block_read_ui4(image2d_t src, global uint4 *dst)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(uint) + sizeof(uint) * get_group_id(0) * get_local_size(0),yid*4);
  uint4 tmp = intel_sub_group_media_block_read_ui4(coord, 16, 4, src);
  dst[yid * get_global_size(0) + id] = tmp;
}
__kernel void compiler_subgroup_media_block_read_ui8(image2d_t src, global uint8 *dst)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(uint) + sizeof(uint) * get_group_id(0) * get_local_size(0),yid*8);
  uint8 tmp = intel_sub_group_media_block_read_ui8(coord, 16, 8, src);
  dst[yid * get_global_size(0) + id] = tmp;
}
__kernel void compiler_subgroup_media_block_read_us1(image2d_t src, global ushort *dst)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(ushort) + sizeof(ushort) * get_group_id(0) * get_local_size(0),yid);
  ushort tmp = intel_sub_group_media_block_read_us(coord, 16, 1, src);
  dst[yid * get_global_size(0) + id] = tmp;
}
__kernel void compiler_subgroup_media_block_read_us2(image2d_t src, global ushort2 *dst)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(ushort) + sizeof(ushort) * get_group_id(0) * get_local_size(0),yid*2);
  ushort2 tmp = intel_sub_group_media_block_read_us2(coord, 16, 2, src);
  dst[yid * get_global_size(0) + id] = tmp;
}
__kernel void compiler_subgroup_media_block_read_us4(image2d_t src, global ushort4 *dst)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(ushort) + sizeof(ushort) * get_group_id(0) * get_local_size(0),yid*4);
  ushort4 tmp = intel_sub_group_media_block_read_us4(coord, 16, 4, src);
  dst[yid * get_global_size(0) + id] = tmp;
}
__kernel void compiler_subgroup_media_block_read_us8(image2d_t src, global ushort8 *dst)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(ushort) + sizeof(ushort) * get_group_id(0) * get_local_size(0),yid*8);
  ushort8 tmp = intel_sub_group_media_block_read_us8(coord, 16, 8, src);
  dst[yid * get_global_size(0) + id] = tmp;
}
__kernel void  __attribute__((intel_reqd_sub_group_size(8)))
compiler_subgroup_media_block_read_us16(image2d_t src, global ushort16 *dst)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(ushort) + sizeof(ushort) * get_group_id(0) * get_local_size(0),yid*16);
  ushort16 tmp = intel_sub_group_media_block_read_us16(coord, 8, 16, src);
  dst[yid * get_global_size(0) + id] = tmp;
}
__kernel void compiler_subgroup_media_block_read_uc1(image2d_t src, global uchar *dst)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(char) + sizeof(char) * get_group_id(0) * get_local_size(0),yid);
  uchar tmp = intel_sub_group_media_block_read_uc(coord, 16, 1, src);
  dst[yid * get_global_size(0) + id] = tmp;
}
__kernel void compiler_subgroup_media_block_read_uc2(image2d_t src, global uchar2 *dst)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(char) + sizeof(char) * get_group_id(0) * get_local_size(0),yid*2);
  uchar2 tmp = intel_sub_group_media_block_read_uc2(coord, 16, 2, src);
  dst[yid * get_global_size(0) + id] = tmp;
}
__kernel void compiler_subgroup_media_block_read_uc4(image2d_t src, global uchar4 *dst)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(char) + sizeof(char) * get_group_id(0) * get_local_size(0),yid*4);
  uchar4 tmp = intel_sub_group_media_block_read_uc4(coord, 16, 4, src);
  dst[yid * get_global_size(0) + id] = tmp;
}
__kernel void compiler_subgroup_media_block_read_uc8(image2d_t src, global uchar8 *dst)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(char) + sizeof(char) * get_group_id(0) * get_local_size(0),yid*8);
  uchar8 tmp = intel_sub_group_media_block_read_uc8(coord, 16, 8, src);
  dst[yid * get_global_size(0) + id] = tmp;
}
__kernel void compiler_subgroup_media_block_read_uc16(image2d_t src, global uchar16 *dst)
{
  int id = get_global_id(0);
  int yid = get_global_id(1);
  int2 coord = (int2)(get_sub_group_size()*get_sub_group_id()*sizeof(char) + sizeof(char) * get_group_id(0) * get_local_size(0),yid*16);
  uchar16 tmp = intel_sub_group_media_block_read_uc16(coord, 16, 16, src);
  dst[yid * get_global_size(0) + id] = tmp;
}
#endif

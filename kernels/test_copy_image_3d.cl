__kernel void
test_copy_image_3d(__read_only image3d_t src,
                   __write_only image3d_t dst,
                   sampler_t sampler,
                   __write_only image2d_t buf0,
                   __write_only image2d_t buf1,
                   __write_only image2d_t buf2,
                   __write_only image2d_t buf3)
{
  int4 coord;
  int2 coord2;
  float4 color;
  coord.x = (int)get_global_id(0);
  coord.y = (int)get_global_id(1);
  coord.z = (int)get_global_id(2);
  coord2.x = coord.x;
  coord2.y = coord.y;
  color = read_imagef(src, sampler, coord);
  write_imagef(dst, coord, color);
  if (coord.z == 0)
    write_imagef(buf0, coord2, color);
  else if (coord.z == 1)
    write_imagef(buf1, coord2, color);
  else if (coord.z == 2)
    write_imagef(buf2, coord2, color);
  else if (coord.z == 3)
    write_imagef(buf3, coord2, color);
}

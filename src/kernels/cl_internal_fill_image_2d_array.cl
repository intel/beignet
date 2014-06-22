kernel void __cl_fill_image_2d_array( __write_only image2d_array_t image, float4 pattern,
                             unsigned int region0, unsigned int region1, unsigned int region2,
                             unsigned int origin0, unsigned int origin1, unsigned int origin2)
{
  int i = get_global_id(0);
  int j = get_global_id(1);
  int k = get_global_id(2);
  int4 coord;
  if((i >= region0) || (j>= region1) || (k>=region2))
    return;
  coord.x = origin0 + i;
  coord.y = origin1 + j;
  coord.z = origin2 + k;
  write_imagef(image, coord, pattern);

}

__kernel void
compare_image_2d_and_1d_array(image2d_t a1, image1d_array_t a2, sampler_t sampler)
{
  float2 coord;
  int4 color1;
  int4 color2;
  coord.x = (float)get_global_id(0) + 0.3f;
  coord.y = (float)get_global_id(1) + 0.3f;
  color1 = read_imagei(a1, sampler, coord);
  color2 = read_imagei(a2, sampler, coord);
//  printf("########## x y is (%f, %f), color1 is (%d %d %d %d), color2 is (%d %d %d %d)\n",
//	  coord.x, coord.y, color1.x, color1.y, color1.z, color1.w, color2.x, color2.y, color2.z, color2.w);
}

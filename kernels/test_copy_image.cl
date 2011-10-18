__constant sampler_t s = CLK_NORMALIZED_COORDS_FALSE |
                         CLK_ADDRESS_CLAMP |
                         CLK_FILTER_NEAREST;
__kernel void
test_copy_image(__read_only image2d_t src, __global uchar4 *dst)
{
  const int x = (int) get_global_id(0);
  const int y = (int) get_global_id(1);
  const int id = x + y * get_image_width(src);
  const uchar4 from = convert_uchar4(read_imageui(src, s, (int2)(x,y)));
  dst[id] = from;
}


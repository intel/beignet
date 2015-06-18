__kernel void
runtime_yuy2_processing(__global uchar *src,
                        int image_height,
                        int image_pitch)
{
  int gx = get_global_id(0);
  int gy = get_global_id(1);

  int src_y = image_height / 2 + gy;
  int mirror_y = image_height - src_y;

  uchar4 mirror_val = *(__global uchar4*)(src + mirror_y*image_pitch + gx*4);
  *(__global uchar4*)(src + src_y*image_pitch + gx*4) = mirror_val;

}

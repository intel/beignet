__kernel void
test_movforphi_undef(__read_only image2d_t src, __write_only image2d_t dst, sampler_t sampler)
{
  int2 coord, dstCoord;
  int4 color;
  int x = get_global_id(0);
  int y = get_global_id(1);
  dstCoord.x = x;
  dstCoord.y = y;
  coord.y = y;
  for(int j = -8; j < 2; j++)
  {
    coord.x = j + x;
    color = read_imagei(src, sampler, coord);
    if (j == 1 + x)
      write_imagei(dst, dstCoord, color);
  }
}

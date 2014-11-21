__kernel void
runtime_climage_from_boname(__write_only image2d_t dst)
{
  int2 coord;
  coord.x = (int)get_global_id(0);
  coord.y = (int)get_global_id(1);
  write_imagef(dst, coord, (float4)(0.34f));
}

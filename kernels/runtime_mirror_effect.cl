__kernel void
runtime_mirror_effect(__read_only image2d_t src_y,
                      __read_only image2d_t src_uv,
                      __write_only image2d_t dst_y,
                      __write_only image2d_t dst_uv,
                      int src_height)
{
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST;
  int2 loc = (int2)(get_global_id(0), get_global_id(1));
  uint4 color_y, color_uv;

  if (loc.y < src_height/2) {
    color_y = read_imageui(src_y, sampler, loc);
    color_uv = read_imageui(src_uv, sampler, loc/2);
  } else {
    int2 newloc = (int2)(loc.x, src_height - loc.y);
    color_y = read_imageui(src_y, sampler, newloc);
    color_uv = read_imageui(src_uv, sampler, newloc/2);
  }

  write_imageui(dst_y, loc, color_y);
  write_imageui(dst_uv, loc/2, color_uv);

}

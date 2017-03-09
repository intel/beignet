__kernel void
image_planar_total(__read_write image2d_t srcNV, __read_write image2d_t dstNV)
{
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST;
  int2 loc = (int2)(get_global_id(0), get_global_id(1));
  float4 color = read_imagef(srcNV, sampler, loc);
  //typed write to surface format planar_420_8 is not supported yet.
  //write_imagef(srcNV, loc, color);
}

__kernel void image_planar_seperate(__read_only image2d_t YPlane,
                                    __read_only image2d_t UVPlane,
                                    __write_only image2d_t YPlaneOut,
                                    __write_only image2d_t UVPlaneOut) {
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST;
  int2 loc = (int2)(get_global_id(0), get_global_id(1));
  float4 colorY = read_imagef(YPlane, sampler, loc);
  float4 colorUV = read_imagef(UVPlane, sampler, loc/2);
  colorY.x += 1/255.0f;
  colorUV += 1/255.0f;
  write_imagef(YPlaneOut, loc, colorY);
  write_imagef(UVPlaneOut, loc/2, colorUV);
}


#define S(A,B,C) CLK_NORMALIZED_COORDS_##A | CLK_ADDRESS_##B | CLK_FILTER_##C

#define COPY_IMAGE(_dst, _sampler, scoord, dcoord) \
  color = read_imagei(src, _sampler, scoord);\
  write_imagei(_dst, dcoord, color)

__kernel void
test_copy_image1(__read_only image2d_t src,
                 __write_only image2d_t dst0,
                 sampler_t sampler0,
                 __write_only image2d_t dst1,
                 __write_only image2d_t dst2,
                 __write_only image2d_t dst3,
                 __write_only image2d_t dst4,
                 float w_inv, float h_inv)
{
  const sampler_t sampler1 = S(FALSE, REPEAT, NEAREST);
  const sampler_t sampler2 = S(FALSE, CLAMP, NEAREST);
  const sampler_t sampler3 = S(FALSE, MIRRORED_REPEAT, NEAREST);
  const sampler_t sampler4 = S(TRUE, REPEAT, NEAREST);
  int2 coord;
  float2 fcoord;
  int4 color;
  coord.x = (int)get_global_id(0);
  coord.y = (int)get_global_id(1);
  fcoord.x = coord.x * w_inv;
  fcoord.y = coord.y * h_inv;
  COPY_IMAGE(dst0, sampler0, coord, coord);
  COPY_IMAGE(dst1, sampler1, coord, coord);
  COPY_IMAGE(dst2, sampler2, coord, coord);
  COPY_IMAGE(dst3, sampler3, coord, coord);
  COPY_IMAGE(dst4, sampler4, fcoord, coord);
}

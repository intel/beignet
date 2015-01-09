#define X_COUNT 4
#define Y_COUNT 4

__kernel void
compiler_read_image(__read_only image2d_t src0, __read_only image2d_t src1, __global float4* dst)
{
  float4 sum = 0;
  int2 coord;
  int x_sz = (int)get_global_size(0);
  int y_sz = (int)get_global_size(1);
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE| CLK_ADDRESS_CLAMP| CLK_FILTER_NEAREST;
  int i, j;

  int x = (int)get_global_id(0);
  int y = (int)get_global_id(1);

  for(i=0; i<X_COUNT; i++) {
    coord.x = x + i * x_sz;
    for(j=0; j<Y_COUNT; j++) {
      coord.y = y + j * y_sz;
      sum = sum + read_imagef(src0, sampler, coord) + read_imagef(src1, sampler, coord);
    }
  }
  dst[y * x_sz + x] = sum;
}

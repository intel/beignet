__kernel void test_cmp(__global bool *dst, int x, int y, float z, float w)
{
  dst[0] = (x < y) + (z > w);
}


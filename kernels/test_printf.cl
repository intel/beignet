__kernel void
test_printf(void)
{
  int x = (int)get_global_id(0);
  int y = (int)get_global_id(1);
  int z = (int)get_global_id(2);

  if (x % 15 == 0)
    if (y % 3 == 0)
      if (z % 7 == 0)
        printf("######## global_id(x, y, z) = (%d, %d, %d), global_size(d0, d1, d3) = (%d, %d, %d)\n",
                x, y, z, get_global_size(0), get_global_size(1), get_global_size(2));
}

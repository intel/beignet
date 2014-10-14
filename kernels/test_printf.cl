__kernel void
test_printf(void)
{
  int x = (int)get_global_id(0);
  int y = (int)get_global_id(1);
  int z = (int)get_global_id(2);
  int g0 = (int)get_global_size(0);
  int g1 = (int)get_global_size(1);
  uint a = 'x';
  float f = 5.0f;
  int3 vec;
  ulong cc = 1004294967296;
  vec.x = x;
  vec.y = y;
  vec.z = z;

  if (x == 0 && y == 0 && z == 0) {
    printf("--- Welcome to the printf test of %s ---\n", "Intel Beignet");
    printf("### output a char is %c\n", a);
    printf("@@@ A long value is %ld\n", cc);
  }

  for(int i = 0; i < g0/2; i++)
    for(int j = 0; j < g1/2; j++)
      if(x == 0 && y == 0 && z == 0)
        printf("loops: i = %d, j = %d\n", i, j);

  if (x == 0) {
    if (y == 0) {
      if (z % 2 == 0)
          printf("!!! output a float is %f\n", f);
      else
          printf("!!! output a float to int is %d\n", f);
    }
  }

  if (x % 15 == 0)
    if (y % 3 == 0)
      if (z % 7 == 0)
        printf("######## global_id(x, y, z) = %v3d, global_size(d0, d1, d3) = (%d, %d, %d)\n",
                vec, get_global_size(0), get_global_size(1), get_global_size(2));

  if (x == 0 && y == 0 && z == 0) {
    printf("--- End to the printf test ---\n");
  }
}

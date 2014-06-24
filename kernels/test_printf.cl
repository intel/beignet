__kernel void
test_printf(void)
{
  int x = (int)get_global_id(0);
  int y = (int)get_global_id(1);
  int z = (int)get_global_id(2);
  uint a = 'x';
  float f = 5.0f;
  int3 vec;
  vec.x = x;
  vec.y = y;
  vec.z = z;

  if (x == 0 && y == 0 && z == 0) {
    printf("--- Welcome to the printf test of %s ---\n", "Intel Beignet");

    printf("### output a char is %c\n", a);
  }

  if (x % 15 == 0)
    if (y % 3 == 0)
      if (z % 7 == 0)
        printf("######## global_id(x, y, z) = %v3d, global_size(d0, d1, d3) = (%d, %d, %d)\n",
                vec, get_global_size(0), get_global_size(1), get_global_size(2));

  if (x == 1)
    if (y == 0) {
      if (z % 2 == 0)
          printf("#### output a float is %f\n", f);
      else
          printf("#### output a float to int is %d\n", f);
    }

  if (x == 0 && y == 0 && z == 0) {
    printf("--- End to the printf test ---\n");
  }

}

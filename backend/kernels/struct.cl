struct my_struct {
  int a;
  int b[2];
};

const __constant int g[4] = {0,1,2,3};

__kernel void struct_cl (struct my_struct s, int x, __global int *mem, int y)
{
  __local struct my_struct array[256];
  for (int i = 0; i < 256; ++i) {
    array[i].a = i;
    array[i].b[0] = i;
    array[i].b[0] = i+1;
  }
  array[0] = array[y];
  mem[0] = s.a + array[x].a + array[x+1].b[0] + g[x] + g[3];
}


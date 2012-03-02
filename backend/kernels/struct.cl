struct my_struct {
  int a;
  int b[2];
};

__constant int g[4] = {0,1,2,3};

__kernel void struct_cl (struct my_struct s, int x, __global int *mem)
{
  __local int array[256];
  for (int i = 0; i < 256; ++i)
    array[i] = i;
  mem[0] = s.a + array[x] + g[x];
}


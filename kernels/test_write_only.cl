__kernel void
test_write_only(__global int *dst)
{
  int id = (int)get_global_id(0);
  dst[id] = id;
}

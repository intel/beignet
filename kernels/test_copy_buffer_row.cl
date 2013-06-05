__kernel void
test_copy_buffer_row(__global int *src, __global int *dst, __global int *data)
{
  int row = data[0];
  int size = data[1];
  int id = (int) get_global_id(0);
  for (; id < size; id += row) dst[id] = src[id];
}

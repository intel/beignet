__kernel void
runtime_alloc_host_ptr_buffer(__global int* buf)
{
  int id = (int)get_global_id(0);
  buf[id] = id / 2;
}

__kernel void
compiler_write_only_bytes(__global char *dst)
{
    int id = (int)get_global_id(0);
    dst[id] = 2;
}

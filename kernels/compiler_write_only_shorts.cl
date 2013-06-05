__kernel void
compiler_write_only_shorts(__global short *dst)
{
    int id = (int)get_global_id(0);
    dst[id] = 2;
}

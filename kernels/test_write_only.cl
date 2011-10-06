__kernel void
test_write_only(__global float* dst )
{
    int id = (int)get_global_id(0);
    dst[id] = 1;
}


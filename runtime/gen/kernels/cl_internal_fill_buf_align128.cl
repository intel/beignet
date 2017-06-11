kernel void __cl_fill_region_align128 ( global float16* dst, float16 pattern0,
                                        unsigned int offset, unsigned int size, float16 pattern1)
{
    int i = get_global_id(0);
    if (i < size) {
        dst[i*2+offset] = pattern0;
        dst[i*2+offset+1] = pattern1;
    }
}

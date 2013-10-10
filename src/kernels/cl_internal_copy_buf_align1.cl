kernel void __cl_cpy_region_align1 ( global char* src, unsigned int src_offset,
                                     global char* dst, unsigned int dst_offset,
				     unsigned int size)
{
    int i = get_global_id(0);
    if (i < size)
        dst[i+dst_offset] = src[i+src_offset];
}

kernel void __cl_copy_region_align4 ( global float* src, unsigned int src_offset,
                                     global float* dst, unsigned int dst_offset,
				     unsigned int size)
{
    int i = get_global_id(0);
    if (i < size)
        dst[i+dst_offset] = src[i+src_offset];
}

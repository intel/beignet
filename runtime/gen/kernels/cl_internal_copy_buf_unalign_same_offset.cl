kernel void __cl_copy_region_unalign_same_offset ( global int* src, unsigned int src_offset,
                                     global int* dst, unsigned int dst_offset,
				     unsigned int size,
				     unsigned int first_mask, unsigned int last_mask)
{
    int i = get_global_id(0);
    if (i > size -1)
       return;

    if (i == 0) {
        dst[dst_offset] = (dst[dst_offset] & first_mask)
             | (src[src_offset] & (~first_mask));
    } else if (i == size - 1) {
        dst[i+dst_offset] = (src[i+src_offset] & last_mask)
            | (dst[i+dst_offset] & (~last_mask));
    } else {
        dst[i+dst_offset] = src[i+src_offset];
    }
}

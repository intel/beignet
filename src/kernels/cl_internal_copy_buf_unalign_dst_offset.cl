kernel void __cl_copy_region_unalign_dst_offset ( global int* src, unsigned int src_offset,
                                     global int* dst, unsigned int dst_offset,
				     unsigned int size,
				     unsigned int first_mask, unsigned int last_mask,
				     unsigned int shift, unsigned int dw_mask)
{
    int i = get_global_id(0);
    unsigned int tmp = 0;

    if (i > size -1)
        return;

    /* last dw, need to be careful, not to overflow the source. */
    if ((i == size - 1) && ((last_mask & (~(~dw_mask >> shift))) == 0)) {
        tmp = ((src[src_offset + i] & ~dw_mask) >> shift);
    } else {
        tmp = ((src[src_offset + i] & ~dw_mask) >> shift)
             | ((src[src_offset + i + 1] & dw_mask) << (32 - shift));
    }

    if (i == 0) {
        dst[dst_offset] = (dst[dst_offset] & first_mask) | (tmp & (~first_mask));
    } else if (i == size - 1) {
        dst[i+dst_offset] = (tmp & last_mask) | (dst[i+dst_offset] & (~last_mask));
    } else {
        dst[i+dst_offset] = tmp;
    }
}

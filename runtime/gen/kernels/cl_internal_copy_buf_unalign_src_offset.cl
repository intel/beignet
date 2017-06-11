kernel void __cl_copy_region_unalign_src_offset ( global int* src, unsigned int src_offset,
                                     global int* dst, unsigned int dst_offset,
				     unsigned int size,
				     unsigned int first_mask, unsigned int last_mask,
				     unsigned int shift, unsigned int dw_mask, int src_less)
{
    int i = get_global_id(0);
    unsigned int tmp = 0;

    if (i > size -1)
        return;

    if (i == 0) {
        tmp = ((src[src_offset + i] & dw_mask) << shift);
    } else if (src_less && i == size - 1) { // not exceed the bound of source
        tmp = ((src[src_offset + i - 1] & ~dw_mask) >> (32 - shift));
    } else {
        tmp = ((src[src_offset + i - 1] & ~dw_mask) >> (32 - shift))
             | ((src[src_offset + i] & dw_mask) << shift);
    }

    if (i == 0) {
        dst[dst_offset] = (dst[dst_offset] & first_mask) | (tmp & (~first_mask));
    } else if (i == size - 1) {
        dst[i+dst_offset] = (tmp & last_mask) | (dst[i+dst_offset] & (~last_mask));
    } else {
        dst[i+dst_offset] = tmp;
    }
}

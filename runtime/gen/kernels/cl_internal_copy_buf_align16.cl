kernel void __cl_copy_region_align16 ( global float* src, unsigned int src_offset,
                                      global float* dst, unsigned int dst_offset,
				      unsigned int size)
{
    int i = get_global_id(0) * 4;
    if (i < size*4) {
        dst[i+dst_offset] = src[i+src_offset];
        dst[i+dst_offset + 1] = src[i+src_offset + 1];
        dst[i+dst_offset + 2] = src[i+src_offset + 2];
        dst[i+dst_offset + 3] = src[i+src_offset + 3];
    }
}

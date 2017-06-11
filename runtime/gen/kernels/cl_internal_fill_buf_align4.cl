kernel void __cl_fill_region_align4 ( global float* dst, float pattern,
			             unsigned int offset, unsigned int size)
{
    int i = get_global_id(0);
    if (i < size) {
        dst[i+offset] = pattern;
    }
}

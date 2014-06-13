kernel void __cl_fill_region_align2 ( global char2 * dst, char2 pattern,
			             unsigned int offset, unsigned int size)
{
    int i = get_global_id(0);
    if (i < size) {
        dst[i+offset] = pattern;
    }
}

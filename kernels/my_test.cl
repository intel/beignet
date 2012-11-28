__kernel void
my_test(__global int2 *src, __global int *offsets, __global uint2 *dst, int w)
{
	int i, index, j;
	uint2 out;
	unsigned int a, b, c, d;
	int2 rle;
	int gid = get_global_id(0);
	index = offsets[gid];
	int i0 = 0;
	rle = src[index];
	for (i = 0; i < w; i++, i0 += 8) {
			if (i0+0 >= rle.x) { index++; rle = src[index]; } a = rle.y;
			if (i0+1 >= rle.x) { index++; rle = src[index]; } b = rle.y;
			if (i0+2 >= rle.x) { index++; rle = src[index]; } c = rle.y;
			if (i0+3 >= rle.x) { index++; rle = src[index]; } d = rle.y;
			out.x = (d<<24)|(c<<16)|(b<<8)|(a);
			if (i0+4 >= rle.x) { index++; rle = src[index]; } a = rle.y;
			if (i0+5 >= rle.x) { index++; rle = src[index]; } b = rle.y;
			if (i0+6 >= rle.x) { index++; rle = src[index]; } c = rle.y;
			if (i0+7 >= rle.x) { index++; rle = src[index]; } d = rle.y;
			out.y = (d<<24)|(c<<16)|(b<<8)|(a);

		dst[gid*w + i] = out;
	}
}

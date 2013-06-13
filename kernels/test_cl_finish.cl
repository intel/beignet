

__kernel void
test_cl_finish(__global int *src, __global int *dst, int n, int num_threads)
{
	int tid, pos;

	tid = get_global_id(0);
	for (pos=tid; pos < n; pos+=num_threads) {
		dst[pos] = src[pos];
	}
}

__kernel void vadd_gpu(
	__global const float* a, 
	__global const float* b, 
	__global float* c, 
	int n)
{
	const int i = get_global_id(0);
	c[i] = a[i] + b[i];
}


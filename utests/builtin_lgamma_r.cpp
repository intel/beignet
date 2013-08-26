#include <cmath>
#include "utest_helper.hpp"

void builtin_lgamma_r(void) {
	const int n = 1024;
	float src[n];

	// Setup kernel and buffers
	OCL_CREATE_KERNEL("builtin_lgamma_r");
	OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
	OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
	OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(int), NULL);
	OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
	OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
	OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
	globals[0] = n;
	locals[0] = 16;

	for (int j = 0; j < 1024; j++) {
		OCL_MAP_BUFFER(0);
		for (int i = 0; i < n; ++i) {
			src[i] = ((float*) buf_data[0])[i] = (j * n + i + 1) * 0.001f;
		}
		OCL_UNMAP_BUFFER(0);

		OCL_NDRANGE(1);

		OCL_MAP_BUFFER(1);
		OCL_MAP_BUFFER(2);
		float *dst = (float*) buf_data[1];
		for (int i = 0; i < n; ++i) {
			int cpu_signp;
			float cpu = lgamma_r(src[i], &cpu_signp);
			int gpu_signp = ((int*)buf_data[2])[i];
			float gpu = dst[i];
			if (cpu_signp != gpu_signp || fabsf(cpu - gpu) >= 1e-3) {
				printf("%f %f %f\n", src[i], cpu, gpu);
				OCL_ASSERT(0);
			}
		}
		OCL_UNMAP_BUFFER(1);
		OCL_UNMAP_BUFFER(2);
	}
}

MAKE_UTEST_FROM_FUNCTION (builtin_lgamma_r);

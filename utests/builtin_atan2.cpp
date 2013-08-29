#include <cmath>
#include "utest_helper.hpp"

void builtin_atan2(void) {
	const int n = 1024;
	float y[n], x[n];

	// Setup kernel and buffers
	OCL_CREATE_KERNEL("builtin_atan2");
	OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
	OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
	OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(float), NULL);
	OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
	OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
	OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
	globals[0] = n;
	locals[0] = 16;

	OCL_MAP_BUFFER(0);
	OCL_MAP_BUFFER(1);
	for (int i = 0; i < n; ++i) {
		y[i] = ((float*) buf_data[0])[i] = (rand()&255) * 0.01f;
		x[i] = ((float*) buf_data[1])[i] = (rand()&255) * 0.01f;
	}
	OCL_UNMAP_BUFFER(0);
	OCL_UNMAP_BUFFER(1);

	OCL_NDRANGE(1);

	OCL_MAP_BUFFER(2);
	float *dst = (float*) buf_data[2];
	for (int i = 0; i < n; ++i) {
		float cpu = atan2f(y[i], x[i]);
		float gpu = dst[i];
		if (fabsf(cpu - gpu) >= 1e-2) {
			printf("%f %f %f %f\n", y[i], x[i], cpu, gpu);
			OCL_ASSERT(0);
		}
	}
	OCL_UNMAP_BUFFER(2);
}

MAKE_UTEST_FROM_FUNCTION (builtin_atan2);

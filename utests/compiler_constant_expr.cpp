#include "utest_helper.hpp"
#include <math.h>

static void compiler_constant_expr(void)
{
  const size_t n = 48;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_constant_expr");
  buf_data[0] = (uint32_t*) malloc(sizeof(float) * n);
  for (uint32_t i = 0; i < n; ++i) ((float*)buf_data[0])[i] = i;
  OCL_CREATE_BUFFER(buf[0], CL_MEM_COPY_HOST_PTR, n * sizeof(float), buf_data[0]);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
  free(buf_data[0]);
  buf_data[0] = NULL;

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = 16;
  locals[0] = 16;
  OCL_NDRANGE(1);

  // Check result
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  for (uint32_t i = 0; i < n; ++i) {
    float expect = pow(((float*)buf_data[0])[i], (i % 3) + 1);
    float err = fabs(((float*)buf_data[1])[i] - expect);
    OCL_ASSERT(err <= 100 * cl_FLT_ULP(expect));
  }
}

MAKE_UTEST_FROM_FUNCTION(compiler_constant_expr);


#include "utest_helper.hpp"

struct sfloat8 {
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;
    float g;
    float h;
};

void compiler_function_argument3(void)
{
  sfloat8 arg6;

  arg6.a = 3.0f;
  arg6.h = 4.0f;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("compiler_function_argument3");
  OCL_CREATE_BUFFER(buf[0], 0, sizeof(struct sfloat8) * 8, NULL);

  OCL_SET_ARG(0, sizeof(arg6), &arg6);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[0]);

  // Run the kernel
  globals[0] = 1;
  locals[0] = 1;
  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(0);

  /* Check results */
  sfloat8 *dst = (sfloat8*)buf_data[0];

  OCL_ASSERT(dst[0].a == 3.0f);
  OCL_ASSERT(dst[0].b == 12.0f);
  OCL_ASSERT(dst[0].h == 7.0f);

  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_function_argument3);

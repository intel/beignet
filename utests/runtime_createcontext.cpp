#include "utest_helper.hpp"

void runtime_createcontextfromtype(void) {
  cl_int status;

  cl_context ctx;
  ctx = clCreateContextFromType(NULL, CL_DEVICE_TYPE_GPU, NULL, NULL, &status);
  if (ctx == NULL) {
    OCL_THROW_ERROR("runtime_createcontextfromtype", status);
  }
  clReleaseContext(ctx);
}

MAKE_UTEST_FROM_FUNCTION(runtime_createcontextfromtype);

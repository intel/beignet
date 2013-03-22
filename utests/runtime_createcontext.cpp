#include "utest_helper.hpp"

void runtime_createcontextfromtype(void) {
  cl_int status;

  if (clCreateContextFromType(NULL, CL_DEVICE_TYPE_GPU, NULL, NULL, &status) == NULL) {
    OCL_THROW_ERROR("runtime_createcontextfromtype", status);
  }
}

MAKE_UTEST_FROM_FUNCTION(runtime_createcontextfromtype);

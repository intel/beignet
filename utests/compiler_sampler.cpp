/* test OpenCL 1.1 Sampler Objects (section 5.5) */
#include "utest_helper.hpp"

void compiler_sampler(void)
{
  OCL_CREATE_KERNEL("compiler_sampler");

  OCL_ASSERT(ctx != 0);
  cl_sampler s;
  cl_int err;
  int a1[] = {CL_TRUE, CL_FALSE},
      a2[] = {CL_ADDRESS_MIRRORED_REPEAT,
              CL_ADDRESS_REPEAT,
              CL_ADDRESS_CLAMP_TO_EDGE,
              CL_ADDRESS_CLAMP,
              CL_ADDRESS_NONE},
      a3[] = {CL_FILTER_NEAREST, CL_FILTER_LINEAR},
      a4[] = {CL_SAMPLER_REFERENCE_COUNT,
              CL_SAMPLER_CONTEXT,
              CL_SAMPLER_NORMALIZED_COORDS,
              CL_SAMPLER_ADDRESSING_MODE,
              CL_SAMPLER_FILTER_MODE};
  char pv[1000];
  size_t pv_size;
  int i, j, k, l;
  for(i=0; i<2; i++)
    for(j=0; j<5; j++)
      for(k=0; k<2; k++) {
        s = clCreateSampler(ctx, a1[i], a2[j], a3[k], &err);
        OCL_ASSERT(err == CL_SUCCESS);
        OCL_CALL(clRetainSampler, s);
        OCL_CALL(clReleaseSampler, s);
        for(l=0; l<5; l++)
          OCL_CALL(clGetSamplerInfo, s, a4[l], 1000, pv, &pv_size);
        OCL_CALL(clReleaseSampler, s);
      }
}

MAKE_UTEST_FROM_FUNCTION(compiler_sampler);



/* test OpenCL 1.1 Program Objects (section 5.6)
 * test creating program objects,
 *      build program executable,
 *      build options
 *      query program objects */

#include "utest_helper.hpp"

void compiler_program_objects(void)
{
    OCL_CREATE_KERNEL("empty"); // set up global vars
    OCL_CALL(clRetainProgram, program);
    OCL_CALL(clReleaseProgram, program);
    OCL_CALL(clBuildProgram,
                 program,
                 1,
                 &device,
                 "-Dname -Dname2=def -ldir "
                 "-cl-opt-disable -cl-strict-aliasing -cl-mad-enable -cl-no-signed-zeros "
                 "-cl-finite-math-only -cl-fast-relaxed-math -cl-unsafe-math-optimizations "
                 "-cl-single-precision-constant -cl-denorms-are-zero "
                 "-w -Werror -cl-std=CL1.1",
                 NULL,
                 NULL);
    const int pi[] = {CL_PROGRAM_REFERENCE_COUNT,
                      CL_PROGRAM_CONTEXT,
                      CL_PROGRAM_NUM_DEVICES,
                      CL_PROGRAM_DEVICES,
                      CL_PROGRAM_SOURCE,
                      CL_PROGRAM_BINARY_SIZES,
                      CL_PROGRAM_BINARIES,};
    const int pbi[] = {CL_PROGRAM_BUILD_STATUS,
                       CL_PROGRAM_BUILD_OPTIONS,
                       CL_PROGRAM_BUILD_LOG,};
    char param_value[1024];
    size_t pv_size;
    int i;
    for(i=0; i<sizeof(pi) / sizeof(pi[0]); i++)
        OCL_CALL(clGetProgramInfo,
                      program,
                      pi[i],
                      sizeof(param_value),
                      param_value,
                      &pv_size);
    for(i=0; i<sizeof(pbi) / sizeof(pbi[0]); i++)
        OCL_CALL(clGetProgramBuildInfo,
                      program,
                      device,
                      pbi[i],
                      sizeof(param_value),
                      param_value,
                      &pv_size);
    std::cout<<platform<<' '
             <<device<<' '
             <<ctx<<' '
             <<program<<' '
             <<kernel<<' '
             <<queue<<std::endl;

    puts("Test clUnloadCompiler");
    OCL_CALL(clUnloadCompiler);
}

MAKE_UTEST_FROM_FUNCTION(compiler_program_objects);

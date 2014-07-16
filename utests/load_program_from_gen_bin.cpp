#include "utest_helper.hpp"
#include "utest_file_map.hpp"
#include <cmath>
#include <algorithm>

using namespace std;

static void cpu(int global_id, float *src, float *dst) {
    dst[global_id] = ceilf(src[global_id]);
}

static void test_load_program_from_gen_bin(void)
{
    const size_t n = 16;
    float cpu_dst[16], cpu_src[16];
    cl_int status;
    cl_int binary_status;
    char *ker_path = NULL;

    cl_file_map_t *fm = cl_file_map_new();
    ker_path = cl_do_kiss_path("compiler_ceil.cl", device);
    OCL_ASSERT (cl_file_map_open(fm, ker_path) == CL_FILE_MAP_SUCCESS);

    const char *src = (const char *)cl_file_map_begin(fm);

    program =clCreateProgramWithSource(ctx, 1, &src, NULL, &status);

    OCL_ASSERT(program && status == CL_SUCCESS);

    /* OCL requires to build the program even if it is created from a binary */
    OCL_ASSERT(clBuildProgram(program, 1, &device, NULL, NULL, NULL) == CL_SUCCESS);

    size_t      binarySize;
    unsigned char *binary = NULL;

    status = clGetProgramInfo( program, CL_PROGRAM_BINARY_SIZES, sizeof( binarySize ), &binarySize, NULL );
    OCL_ASSERT(status == CL_SUCCESS);
    // Create a buffer and get the gen binary
    binary = (unsigned char*)malloc(sizeof(unsigned char)*binarySize);
    OCL_ASSERT(binary != NULL);

    status = clGetProgramInfo( program, CL_PROGRAM_BINARIES, sizeof( &binary), &binary, NULL );
    OCL_ASSERT(status == CL_SUCCESS);

    cl_program bin_program = clCreateProgramWithBinary(ctx, 1,
              &device, &binarySize, (const unsigned char**)&binary, &binary_status, &status);
    OCL_ASSERT(bin_program && status == CL_SUCCESS);
    /* OCL requires to build the program even if it is created from a binary */
    OCL_ASSERT(clBuildProgram(bin_program, 1, &device, NULL, NULL, NULL) == CL_SUCCESS);

    kernel = clCreateKernel(bin_program, "compiler_ceil", &status);
    OCL_ASSERT(status == CL_SUCCESS);

    OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
    OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
    OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
    OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
    globals[0] = 16;
    locals[0] = 16;

    // Run random tests
    for (uint32_t pass = 0; pass < 8; ++pass) {
        OCL_MAP_BUFFER(0);
        for (int32_t i = 0; i < (int32_t) n; ++i)
            cpu_src[i] = ((float*)buf_data[0])[i] = .1f * (rand() & 15) - .75f;
        OCL_UNMAP_BUFFER(0);

        // Run the kernel on GPU
        OCL_NDRANGE(1);

        // Run on CPU
        for (int32_t i = 0; i < (int32_t) n; ++i) cpu(i, cpu_src, cpu_dst);

        // Compare
        OCL_MAP_BUFFER(1);

#if 0
        printf("#### GPU:\n");
        for (int32_t i = 0; i < (int32_t) n; ++i)
            printf(" %f", ((float *)buf_data[1])[i]);
        printf("\n#### CPU:\n");
        for (int32_t i = 0; i < (int32_t) n; ++i)
            printf(" %f", cpu_dst[i]);
        printf("\n");
#endif

        for (int32_t i = 0; i < (int32_t) n; ++i)
            OCL_ASSERT(((float *)buf_data[1])[i] == cpu_dst[i]);
        OCL_UNMAP_BUFFER(1);
    }
}

MAKE_UTEST_FROM_FUNCTION(test_load_program_from_gen_bin);

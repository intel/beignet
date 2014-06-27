#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"
#include "utest_file_map.hpp"

#define BUFFERSIZE  32*1024

int init_program(const char* name, cl_context ctx, cl_program *pg )
{
  cl_int err;
  char* ker_path = cl_do_kiss_path(name, device);

  cl_file_map_t *fm = cl_file_map_new();
  err = cl_file_map_open(fm, ker_path);
  if(err != CL_FILE_MAP_SUCCESS)
    OCL_ASSERT(0);
  const char *src = cl_file_map_begin(fm);

  *pg = clCreateProgramWithSource(ctx, 1, &src, NULL, &err);
  free(ker_path);
  cl_file_map_delete(fm);
  return 0;

}

void runtime_compile_link(void)
{

  cl_int err;

  const char* header_file_name="runtime_compile_link.h";
  cl_program foo_pg;
  init_program(header_file_name, ctx, &foo_pg);

  const char* myinc_file_name="include/runtime_compile_link_inc.h";
  cl_program myinc_pg;
  init_program(myinc_file_name, ctx, &myinc_pg);

  const char* file_name_A="runtime_compile_link_a.cl";
  cl_program program_A;
  init_program(file_name_A, ctx, &program_A);

  cl_program input_headers[2] = { foo_pg, myinc_pg};
  const char * input_header_names[2] = {header_file_name, myinc_file_name}; 

  err = clCompileProgram(program_A,
                                0, NULL, // num_devices & device_list
                                NULL, // compile_options
                                2, // num_input_headers
                                input_headers,
                                input_header_names,
                                NULL, NULL);

  OCL_ASSERT(err==CL_SUCCESS);
  const char* file_name_B="runtime_compile_link_b.cl";
  cl_program program_B;
  init_program(file_name_B, ctx, &program_B);

  err = clCompileProgram(program_B,
                                0, NULL, // num_devices & device_list
                                NULL, // compile_options
                                2, // num_input_headers
                                input_headers,
                                input_header_names,
                                NULL, NULL);

  OCL_ASSERT(err==CL_SUCCESS);
  cl_program input_programs[2] = { program_A, program_B};
  cl_program linked_program = clLinkProgram(ctx, 0, NULL, "-create-library", 2, input_programs, NULL, NULL, &err);

  OCL_ASSERT(linked_program != NULL);
  OCL_ASSERT(err == CL_SUCCESS);
  size_t      binarySize;
  unsigned char *binary;

  // Get the size of the resulting binary (only one device)
  err= clGetProgramInfo( linked_program, CL_PROGRAM_BINARY_SIZES, sizeof( binarySize ), &binarySize, NULL );
  OCL_ASSERT(err==CL_SUCCESS);

  // Create a buffer and get the actual binary
  binary = (unsigned char*)malloc(sizeof(unsigned char)*binarySize);
  if (binary == NULL) {
    OCL_ASSERT(0);
    return ;
  }

  unsigned char *buffers[ 1 ] = { binary };
  // Do another sanity check here first
  size_t size;
  cl_int loadErrors[ 1 ];
  err = clGetProgramInfo( linked_program, CL_PROGRAM_BINARIES, 0, NULL, &size );
  OCL_ASSERT(err==CL_SUCCESS);
  if( size != sizeof( buffers ) ){
    free(binary);
    return ;
  }

  err = clGetProgramInfo( linked_program, CL_PROGRAM_BINARIES, sizeof( buffers ), &buffers, NULL );
  OCL_ASSERT(err==CL_SUCCESS);

  cl_device_id deviceID;
  err = clGetProgramInfo( linked_program, CL_PROGRAM_DEVICES, sizeof( deviceID), &deviceID, NULL );
  OCL_ASSERT(err==CL_SUCCESS);

  cl_program program_with_binary = clCreateProgramWithBinary(ctx, 1, &deviceID, &binarySize, (const unsigned char**)buffers, loadErrors, &err);
  OCL_ASSERT(err==CL_SUCCESS);

  cl_program new_linked_program = clLinkProgram(ctx, 1, &deviceID, NULL, 1, &program_with_binary, NULL, NULL, &err);
  OCL_ASSERT(err==CL_SUCCESS);
  // link success, run this kernel.

  const size_t n = 16;
  int64_t src1[n], src2[n];

  src1[0] = (int64_t)1 << 63, src2[0] = 0x7FFFFFFFFFFFFFFFll;
  src1[1] = (int64_t)1 << 63, src2[1] = ((int64_t)1 << 63) | 1;
  src1[2] = -1ll, src2[2] = 0;
  src1[3] = ((int64_t)123 << 32) | 0x7FFFFFFF, src2[3] = ((int64_t)123 << 32) | 0x80000000;
  src1[4] = 0x7FFFFFFFFFFFFFFFll, src2[4] = (int64_t)1 << 63;
  src1[5] = ((int64_t)1 << 63) | 1, src2[5] = (int64_t)1 << 63;
  src1[6] = 0, src2[6] = -1ll;
  src1[7] = ((int64_t)123 << 32) | 0x80000000, src2[7] = ((int64_t)123 << 32) | 0x7FFFFFFF;
  for(size_t i=8; i<n; i++) {
    src1[i] = i;
    src2[i] = i;
  }

  globals[0] = n;
  locals[0] = 16;

  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int64_t), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(int64_t), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(int64_t), NULL);
  OCL_MAP_BUFFER(0);
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[0], src1, sizeof(src1));
  memcpy(buf_data[1], src2, sizeof(src2));
  OCL_UNMAP_BUFFER(0);
  OCL_UNMAP_BUFFER(1);

  kernel = clCreateKernel(new_linked_program, "runtime_compile_link_a", &err);

  OCL_ASSERT(err == CL_SUCCESS);

  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);

  clEnqueueNDRangeKernel(queue, kernel, 1, NULL, globals, locals, 0, NULL, NULL);

  OCL_MAP_BUFFER(2);
  for (int32_t i = 0; i < (int32_t) n; ++i) {
    int64_t *dest = (int64_t *)buf_data[2];
    int64_t x = (src1[i] < src2[i]) ? 3 : 4;
    OCL_ASSERT(x == dest[i]);
  }
  OCL_UNMAP_BUFFER(2);
  OCL_DESTROY_KERNEL_KEEP_PROGRAM(true);
}

MAKE_UTEST_FROM_FUNCTION(runtime_compile_link);

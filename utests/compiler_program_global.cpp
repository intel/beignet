#include "utest_helper.hpp"
#include "utest_file_map.hpp"

static int init_program(const char* name, cl_context ctx, cl_program *pg )
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

void compiler_program_global()
{
  const int n = 16;
  int cpu_src[16];
  cl_int err;

  // Setup kernel and buffers
  cl_program program;
  init_program("compiler_program_global.cl", ctx, &program);
  OCL_CALL (clBuildProgram, program, 1, &device, "-cl-std=CL2.0", NULL, NULL);

  cl_kernel k0 = clCreateKernel(program, "compiler_program_global0", &err);
  assert(err == CL_SUCCESS);
  cl_kernel k1 = clCreateKernel(program, "compiler_program_global1", &err);
  assert(err == CL_SUCCESS);

  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(int), NULL);

  OCL_CALL (clSetKernelArg, k0, 0, sizeof(cl_mem), &buf[0]);
  OCL_CALL (clSetKernelArg, k1, 0, sizeof(cl_mem), &buf[1]);

  int dynamic = 1;
  OCL_CALL (clSetKernelArg, k0, 1, sizeof(cl_int), &dynamic);
  OCL_CALL (clSetKernelArg, k1, 1, sizeof(cl_int), &dynamic);

  globals[0] = 16;
  locals[0] = 16;

  OCL_MAP_BUFFER(0);
  for (int i = 0; i <  n; ++i)
    cpu_src[i] = ((int*)buf_data[0])[i] = i;
  OCL_UNMAP_BUFFER(0);

  // Run the kernel on GPU
  OCL_CALL (clEnqueueNDRangeKernel, queue, k0, 1, NULL, globals, locals, 0, NULL, NULL);
  OCL_CALL (clEnqueueNDRangeKernel, queue, k1, 1, NULL, globals, locals, 0, NULL, NULL);

  // Compare
  OCL_MAP_BUFFER(1);
  for (int32_t i = 0; i < n; ++i) {
//    printf("i=%d dst=%d\n", i, ((int*)buf_data[1])[i]);
    switch(i) {
      default: OCL_ASSERT(((int*)buf_data[1])[i] == i); break;
      case 11: OCL_ASSERT(((int*)buf_data[1])[i] == 7); break;
      case 12: OCL_ASSERT(((int*)buf_data[1])[i] == 4); break;
      case 13: OCL_ASSERT(((int*)buf_data[1])[i] == 2); break;
      case 14: OCL_ASSERT(((int*)buf_data[1])[i] == 3); break;
      case 15: OCL_ASSERT(((int*)buf_data[1])[i] == 2); break;
    }
  }
  OCL_UNMAP_BUFFER(1);
  clReleaseKernel(k0);
  clReleaseKernel(k1);
  clReleaseProgram(program);
}

MAKE_UTEST_FROM_FUNCTION(compiler_program_global);


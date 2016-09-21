#include "utest_helper.hpp"

#define THREAD_SIZE 8
pthread_t tid[THREAD_SIZE];
static cl_command_queue all_queues[THREAD_SIZE];
static cl_event enqueue_events[THREAD_SIZE];
static cl_event user_event;
static cl_kernel the_kernel;
static char source_str[] =
  "kernel void assgin_work_dim( __global int *ret, int i) { \n"
  "if (i == 0) ret[i] = 10; \n"
  "else ret[i] = ret[i - 1] + 1; \n"
  "}\n";
static size_t the_globals[3] = {16, 1, 1};
static size_t the_locals[3] = {16, 1, 1};
static size_t the_goffsets[3] = {0, 0, 0};

static void *thread_function(void *arg)
{
  int num = *((int *)arg);
  cl_int ret;
  cl_event dep_event[2];

  ret = clSetKernelArg(the_kernel, 1, sizeof(cl_int), &num);
  OCL_ASSERT(ret == CL_SUCCESS);

  if (num == 0) {
    dep_event[0] = user_event;
    ret = clEnqueueNDRangeKernel(all_queues[num], the_kernel, 1, the_goffsets, the_globals, the_locals,
                                 1, dep_event, &enqueue_events[num]);
  } else {
    dep_event[0] = user_event;
    dep_event[1] = enqueue_events[num - 1];
    ret = clEnqueueNDRangeKernel(all_queues[num], the_kernel, 1, the_goffsets, the_globals, the_locals,
                                 2, dep_event, &enqueue_events[num]);
  }

  OCL_ASSERT(ret == CL_SUCCESS);
  return NULL;
}

void multi_queue_events(void)
{
  cl_int ret;
  size_t source_size = sizeof(source_str);
  const char *source = source_str;
  cl_program program = NULL;
  int i;

  /* Create Kernel Program from the source */
  program = clCreateProgramWithSource(ctx, 1, &source, &source_size, &ret);
  OCL_ASSERT(ret == CL_SUCCESS);

  /* Build Kernel Program */
  ret = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  OCL_ASSERT(ret == CL_SUCCESS);

  the_kernel = clCreateKernel(program, "assgin_work_dim", NULL);
  OCL_ASSERT(the_kernel != NULL);

  int buffer_content[16] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  };
  cl_mem buf = clCreateBuffer(ctx, CL_MEM_COPY_HOST_PTR, 16 * 4, buffer_content, &ret);
  OCL_ASSERT(buf != NULL);

  ret = clSetKernelArg(the_kernel, 0, sizeof(cl_mem), &buf);
  OCL_ASSERT(ret == CL_SUCCESS);

  for (i = 0; i < THREAD_SIZE; i++) {
    all_queues[i] = clCreateCommandQueue(ctx, device, 0, &ret);
    OCL_ASSERT(ret == CL_SUCCESS);
  }

  user_event = clCreateUserEvent(ctx, &ret);
  OCL_ASSERT(ret == CL_SUCCESS);

  for (i = 0; i < THREAD_SIZE; i++) {
    pthread_create(&tid[i], NULL, thread_function, &i);
    pthread_join(tid[i], NULL);
  }

  cl_event map_event;

  void *map_ptr = clEnqueueMapBuffer(all_queues[0], buf, 0, CL_MAP_READ, 0, 32,
                                     THREAD_SIZE, enqueue_events, &map_event, NULL);

  OCL_ASSERT(map_ptr != NULL);

  cl_event all_event[10];
  for (i = 0; i < THREAD_SIZE; i++) {
    all_event[i] = enqueue_events[i];
  }
  all_event[8] = user_event;
  all_event[9] = map_event;

  //printf("before Waitfor events ##\n");
  clSetUserEventStatus(user_event, CL_COMPLETE);
  ret = clWaitForEvents(10, all_event);
  OCL_ASSERT(ret == CL_SUCCESS);
  //printf("After Waitfor events ##\n");

  //printf("#############     Finish Setting   ################\n");

  printf("\n");
  for (i = 0; i < 8; i++) {
    //printf(" %d", ((int *)map_ptr)[i]);
    OCL_ASSERT(((int *)map_ptr)[i] == 10 + i);
  }

  //printf("\n");

  ret = clEnqueueUnmapMemObject(all_queues[0], buf, map_ptr, 1, &map_event, NULL);
  OCL_ASSERT(ret == CL_SUCCESS);

  //printf("------------------------- End -------------------------------\n");

  clReleaseKernel(the_kernel);
  clReleaseProgram(program);
  clReleaseMemObject(buf);
  for (i = 0; i < THREAD_SIZE; i++) {
    clReleaseCommandQueue(all_queues[i]);
    clReleaseEvent(enqueue_events[i]);
  }
  clReleaseEvent(user_event);
  clReleaseEvent(map_event);
}

MAKE_UTEST_FROM_FUNCTION(multi_queue_events);

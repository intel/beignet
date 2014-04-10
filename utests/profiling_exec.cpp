#include "utest_helper.hpp"
#include "string.h"

static void cpu_exec (int n, float* src, float* dst)
{
    int i = 0;
    for (; i < n; i++) {
	float f = src[i];
	f = f < 0 ? -f : f;
	dst[i] = f;
    }
}

#define QUEUE_SECONDS_LIMIT 10
#define SUBMIT_SECONDS_LIMIT 20
#define COMMAND_SECONDS_LIMIT 10

static void check_profiling_time(cl_ulong queued, cl_ulong submit, cl_ulong start, cl_ulong end)
{
    size_t profiling_resolution = 0;
    OCL_CALL(clGetDeviceInfo, device, CL_DEVICE_PROFILING_TIMER_RESOLUTION,
             sizeof(profiling_resolution), &profiling_resolution, NULL);

    /* Convert the time to second. */
    double queue_to_submit = (double)(submit - queued)*1e-9;
    double submit_to_start = (double)(start - submit)*1e-9;
    double start_to_end = (double)(end - start)*1e-9;

    //printf("Profiling info:\n");
    //printf("Time from queue to submit : %fms\n", (double)(queue_to_submit) * 1000.f );
    //printf( "Time from submit to start : %fms\n", (double)(submit_to_start) * 1000.f );
    //printf( "Time from start to end: %fms\n", (double)(start_to_end) * 1000.f );

    OCL_ASSERTM(queued <= submit, "Enqueue time is later than submit time, invalid\n");
    OCL_ASSERTM(submit <= start, "Submit time is later than start time, invalid\n");
    OCL_ASSERTM(start <= end, "Start time is later than end time, invalid\n");

    OCL_ASSERTM(queue_to_submit <= QUEUE_SECONDS_LIMIT, "Too large time from queue to submit\n");
    OCL_ASSERTM(submit_to_start <= QUEUE_SECONDS_LIMIT, "Too large time from submit to start\n");
    OCL_ASSERTM(start_to_end <= QUEUE_SECONDS_LIMIT, "Too large time from start to end\n");
}

static void profiling_exec(void)
{
    const size_t n = 512;
    cl_int status = CL_SUCCESS;
    cl_command_queue profiling_queue = NULL;
    cl_command_queue tmp_queue = NULL;
    float* cpu_src = (float *)malloc(n*sizeof(float));
    float* cpu_dst = (float *)malloc(n*sizeof(float));
    cl_event exec_event;
    cl_ulong time_queue, time_submit, time_start, time_end;


    /* Because the profiling prop, we can not use default queue. */
    profiling_queue = clCreateCommandQueue(ctx, device, CL_QUEUE_PROFILING_ENABLE, &status);
    OCL_ASSERT(status == CL_SUCCESS);

    /* save the default queue. */
    tmp_queue = queue;
    queue = profiling_queue;

    OCL_CREATE_KERNEL("compiler_fabs");

    OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(float), NULL);
    OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(float), NULL);
    OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
    OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
    globals[0] = n;
    locals[0] = 256;

    OCL_MAP_BUFFER(0);
    for (int32_t i = 0; i < (int32_t) n; ++i)
	cpu_src[i] = ((float*)buf_data[0])[i] = .1f * (rand() & 15) - .75f;
    OCL_UNMAP_BUFFER(0);

    cpu_exec(n, cpu_src, cpu_dst);

    // Run the kernel on GPU
    OCL_CALL(clEnqueueNDRangeKernel, queue, kernel, 1, NULL, globals, locals, 0, NULL, &exec_event);
    OCL_CALL(clWaitForEvents, 1, &exec_event);

    OCL_CALL(clGetEventProfilingInfo, exec_event, CL_PROFILING_COMMAND_QUEUED, sizeof(cl_ulong), &time_queue, NULL);
    OCL_CALL(clGetEventProfilingInfo, exec_event, CL_PROFILING_COMMAND_SUBMIT, sizeof(cl_ulong), &time_submit, NULL);
    OCL_CALL(clGetEventProfilingInfo, exec_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &time_start, NULL);
    OCL_CALL(clGetEventProfilingInfo, exec_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &time_end, NULL);

    check_profiling_time(time_queue, time_submit, time_start, time_end);

    // Compare
    OCL_MAP_BUFFER(1);
    for (int32_t i = 0; i < (int32_t) n; ++i)
	OCL_ASSERT(((float *)buf_data[1])[i] == cpu_dst[i]);
    OCL_UNMAP_BUFFER(1);

    queue = tmp_queue;
    clReleaseCommandQueue(profiling_queue);
    free(cpu_dst);
    free(cpu_src);
}

MAKE_UTEST_FROM_FUNCTION(profiling_exec);

#ifndef __PERFORMANCE_H__
#define __PERFORMANCE_H__
#include "CL/cl.h"


extern int b_output_kernel_perf;
void time_start(cl_context context, const char * kernel_name, cl_command_queue cq);
void time_end(cl_context context, const char * kernel_name, const char * build_opt, cl_command_queue cq);
void initialize_env_var();


#endif

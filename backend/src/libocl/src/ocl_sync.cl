#include "ocl_sync.h"

void __gen_ocl_barrier_local(void);
void __gen_ocl_barrier_global(void);
void __gen_ocl_barrier_local_and_global(void);

void mem_fence(cl_mem_fence_flags flags) {
}

void read_mem_fence(cl_mem_fence_flags flags) {
}

void write_mem_fence(cl_mem_fence_flags flags) {
}

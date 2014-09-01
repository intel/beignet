#ifndef __OCL_SYNC_H__
#define __OCL_SYNC_H__

#include "ocl_types.h"

/////////////////////////////////////////////////////////////////////////////
// Synchronization functions
/////////////////////////////////////////////////////////////////////////////
#define CLK_LOCAL_MEM_FENCE  (1 << 0)
#define CLK_GLOBAL_MEM_FENCE (1 << 1)

typedef uint cl_mem_fence_flags;
void barrier(cl_mem_fence_flags flags);
void mem_fence(cl_mem_fence_flags flags);
void read_mem_fence(cl_mem_fence_flags flags);
void write_mem_fence(cl_mem_fence_flags flags);

#endif  /* __OCL_SYNC_H__ */

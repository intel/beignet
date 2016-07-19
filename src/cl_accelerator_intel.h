#ifndef __CL_ACCELERATOR_INTEL_H__
#define __CL_ACCELERATOR_INTEL_H__

#include "cl_base_object.h"
#include "CL/cl.h"
#include "CL/cl_ext.h"
#include <stdint.h>

struct _cl_accelerator_intel {
  _cl_base_object base;
  cl_accelerator_intel prev, next;     /* We chain in the allocator, why chain? */
  cl_context ctx;            /* Context it belongs to */
  cl_accelerator_type_intel type;
  union {
    cl_motion_estimation_desc_intel me;
  } desc;                     /* save desc before we decide how to handle it */
};

#define CL_OBJECT_ACCELERATOR_INTEL_MAGIC 0x7e6a08c9a7ac3e3fLL
#define CL_OBJECT_IS_ACCELERATOR_INTEL(obj) \
    (((cl_base_object)obj)->magic == CL_OBJECT_ACCELERATOR_INTEL_MAGIC)

cl_accelerator_intel cl_accelerator_intel_new(cl_context ctx,
                         cl_accelerator_type_intel accel_type,
                         size_t desc_sz,
                         const void* desc,
                         cl_int* errcode_ret);

void cl_accelerator_intel_add_ref(cl_accelerator_intel accel);
void cl_accelerator_intel_delete(cl_accelerator_intel accel);

#endif

#include "cl_context.h"
#include "cl_accelerator_intel.h"
#include "cl_utils.h"
#include "cl_alloc.h"
#include "cl_khr_icd.h"
#include "cl_kernel.h"

#include <assert.h>

LOCAL cl_accelerator_intel
cl_accelerator_intel_new(cl_context ctx,
                         cl_accelerator_type_intel accel_type,
                         size_t desc_sz,
                         const void* desc,
                         cl_int* errcode_ret)
{
  cl_accelerator_intel accel = NULL;
  cl_int err = CL_SUCCESS;

  /* Allocate and inialize the structure itself */
  TRY_ALLOC(accel, CALLOC(struct _cl_accelerator_intel));
  SET_ICD(accel->dispatch)
  accel->ref_n = 1;
  accel->magic = CL_MAGIC_ACCELERATOR_INTEL_HEADER;

  if (accel_type != CL_ACCELERATOR_TYPE_MOTION_ESTIMATION_INTEL) {
    err = CL_INVALID_ACCELERATOR_TYPE_INTEL;
    goto error;
  }
  accel->type = accel_type;

  if (desc == NULL) {   //  and check inside desc
    err = CL_INVALID_ACCELERATOR_DESCRIPTOR_INTEL;
    goto error;
  }
  accel->desc.me = *(cl_motion_estimation_desc_intel*)desc;

  /* Append the accelerator_intel in the context accelerator_intel list */
  /* does this really needed? */
  pthread_mutex_lock(&ctx->accelerator_intel_lock);
    accel->next = ctx->accels;
    if (ctx->accels != NULL)
      ctx->accels->prev = accel;
    ctx->accels = accel;
  pthread_mutex_unlock(&ctx->accelerator_intel_lock);

  accel->ctx = ctx;
  cl_context_add_ref(ctx);

exit:
  if (errcode_ret)
    *errcode_ret = err;
  return accel;
error:
  cl_accelerator_intel_delete(accel);
  accel = NULL;
  goto exit;
}

LOCAL void
cl_accelerator_intel_add_ref(cl_accelerator_intel accel)
{
  atomic_inc(&accel->ref_n);
}

LOCAL void
cl_accelerator_intel_delete(cl_accelerator_intel accel)
{
  if (UNLIKELY(accel == NULL))
    return;
  if (atomic_dec(&accel->ref_n) > 1)
    return;

  /* Remove the accelerator_intel in the context accelerator_intel list */
  pthread_mutex_lock(&accel->ctx->accelerator_intel_lock);
    if (accel->prev)
      accel->prev->next = accel->next;
    if (accel->next)
      accel->next->prev = accel->prev;
    if (accel->ctx->accels == accel)
      accel->ctx->accels = accel->next;
  pthread_mutex_unlock(&accel->ctx->accelerator_intel_lock);

  cl_context_delete(accel->ctx);
  cl_free(accel);
}

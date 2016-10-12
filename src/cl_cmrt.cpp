#include "cl_cmrt.h"
#include "cl_device_id.h"
#include "intel/intel_defines.h"
#include "cl_command_queue.h"

#include "cm_rt.h"      //header file of libcmrt.so
typedef INT (*CreateCmDeviceFunc)(CmDevice * &pDevice, UINT & version,
			    CmDriverContext * drivercontext, UINT DevCreateOption);
typedef INT (*DestroyCmDeviceFunc)(CmDevice * &pDevice);

#include <dlfcn.h>

static void* dlhCMRT = NULL;
static CreateCmDeviceFunc pfnCreateCmDevice = NULL;
static DestroyCmDeviceFunc pfnDestroyCmDevice = NULL;

#define XSTR(x) #x
#define STR(x) XSTR(x)

class CmrtCleanup
{
public:
  CmrtCleanup(){}
  ~CmrtCleanup()
  {
    if (dlhCMRT != NULL)
      dlclose(dlhCMRT);
  }
};

enum CMRT_MEM_TYPE
{
    CMRT_BUFFER,
    CMRT_SURFACE2D,
};

static CmrtCleanup cmrtCleanup;

static bool LoadCmrtLibrary()
{
  if (dlhCMRT == NULL) {
    dlhCMRT = dlopen(STR(CMRT_PATH), RTLD_LAZY | RTLD_LOCAL);

    if (dlhCMRT == NULL)
      return false;

    pfnCreateCmDevice = (CreateCmDeviceFunc)dlsym(dlhCMRT, "CreateCmDevice");
    if (pfnCreateCmDevice == NULL)
      return false;

    pfnDestroyCmDevice = (DestroyCmDeviceFunc)dlsym(dlhCMRT, "DestroyCmDevice");
    if (pfnDestroyCmDevice == NULL)
      return false;
  }
  return true;
}

cl_int cmrt_build_program(cl_program p, const char *options)
{
  CmDevice*& cmrt_device = (CmDevice*&)(p->ctx->device->cmrt_device);
  int result;
  if (cmrt_device == NULL)
  {
    if (!LoadCmrtLibrary())
      return CL_DEVICE_NOT_AVAILABLE;   //yes, the error is not accurate, but i do not find a bettere one

    CmDriverContext ctx;
    ctx.shared_bufmgr = 1;
    ctx.bufmgr = (drm_intel_bufmgr*)cl_context_get_bufmgr(p->ctx);
    ctx.userptr_enabled = 0;
    ctx.deviceid = p->ctx->device->device_id;
    ctx.device_rev = -1;
    UINT version = 0;
    result = (*pfnCreateCmDevice)(cmrt_device, version, &ctx, CM_DEVICE_CREATE_OPTION_DEFAULT);
    if (result != CM_SUCCESS)
      return CL_DEVICE_NOT_AVAILABLE;
  }

  CmProgram* cmrt_program = NULL;
  result = cmrt_device->LoadProgram(p->binary, p->binary_sz, cmrt_program, options);
  if (result != CM_SUCCESS)
    return CL_COMPILE_PROGRAM_FAILURE;

  p->cmrt_program = cmrt_program;
  cmrt_program->GetKernelCount(p->ker_n);
  return CL_SUCCESS;
}

cl_int cmrt_destroy_program(cl_program p)
{
  CmDevice* cmrt_device = (CmDevice*)(p->ctx->device->cmrt_device);
  CmProgram*& cmrt_program = (CmProgram*&)(p->cmrt_program);
  if (cmrt_device->DestroyProgram(cmrt_program) != CM_SUCCESS)
    return CL_INVALID_PROGRAM;
  return CL_SUCCESS;
}

cl_int cmrt_destroy_device(cl_device_id device)
{
  CmDevice*& cmrt_device = (CmDevice*&)(device->cmrt_device);
  if ((*pfnDestroyCmDevice)(cmrt_device) != CM_SUCCESS)
    return CL_INVALID_DEVICE;
  return CL_SUCCESS;
}

void* cmrt_create_kernel(cl_program p, const char *name)
{
  CmDevice* cmrt_device = (CmDevice*)(p->ctx->device->cmrt_device);
  CmKernel* cmrt_kernel = NULL;
  int result = cmrt_device->CreateKernel((CmProgram*)(p->cmrt_program), name, cmrt_kernel);
  if (result != CM_SUCCESS)
    return NULL;

  return cmrt_kernel;
}

cl_int cmrt_destroy_kernel(cl_kernel k)
{
  CmDevice* cmrt_device = (CmDevice*)(k->program->ctx->device->cmrt_device);
  CmKernel*& cmrt_kernel = (CmKernel*&)(k->cmrt_kernel);
  if (cmrt_device->DestroyKernel(cmrt_kernel) != CM_SUCCESS)
    return CL_INVALID_KERNEL;
  return CL_SUCCESS;
}

cl_int cmrt_enqueue(cl_command_queue cq, cl_kernel k, const size_t* global_work_size, const size_t* local_work_size)
{
  CmDevice* cmrt_device = (CmDevice*)(k->program->ctx->device->cmrt_device);
  CmKernel* cmrt_kernel = (CmKernel*)(k->cmrt_kernel);

  int result = 0;

  cmrt_kernel->SetThreadCount(global_work_size[0]*global_work_size[1]);

  //no need to destory queue explicitly,
  //and there is only one queue instance within each device,
  //CreateQueue always returns the same instance
  CmQueue* pCmQueue = NULL;
  cmrt_device->CreateQueue(pCmQueue);

  CmTask *pKernelArray = NULL;
  cmrt_device->CreateTask(pKernelArray);

  pKernelArray->AddKernel(cmrt_kernel);

  CmEvent* e = NULL;

  if (local_work_size == NULL) {
    CmThreadSpace* pTS = NULL;
    cmrt_device->CreateThreadSpace(global_work_size[0], global_work_size[1], pTS);
    result = pCmQueue->Enqueue(pKernelArray, e, pTS);
  } else {
    CmThreadGroupSpace* pTGS = NULL;
	cmrt_device->CreateThreadGroupSpace(global_work_size[0], global_work_size[1], local_work_size[0], local_work_size[1], pTGS);
    result = pCmQueue->EnqueueWithGroup(pKernelArray, e, pTGS);
    cmrt_device->DestroyThreadGroupSpace(pTGS);
  }

  if (result != CM_SUCCESS)
    return CL_INVALID_OPERATION;

  cmrt_device->DestroyTask(pKernelArray);

  CmEvent*& olde = (CmEvent*&)cq->cmrt_event;
  if (olde != NULL)
    pCmQueue->DestroyEvent(e);

  cq->cmrt_event = e;

  return CL_SUCCESS;
}

static VA_CM_FORMAT GetCmrtFormat(_cl_mem_image* image)
{
    switch (image->intel_fmt)
    {
    case I965_SURFACEFORMAT_B8G8R8A8_UNORM:
      return VA_CM_FMT_A8R8G8B8;
    case I965_SURFACEFORMAT_B8G8R8X8_UNORM:
      return VA_CM_FMT_X8R8G8B8;
    case I965_SURFACEFORMAT_A8_UNORM:
      return VA_CM_FMT_A8;
    case I965_SURFACEFORMAT_R10G10B10A2_UNORM:
      return VA_CM_FMT_A2B10G10R10;
    case I965_SURFACEFORMAT_R16G16B16A16_UNORM:
      return VA_CM_FMT_A16B16G16R16;
    case I965_SURFACEFORMAT_L8_UNORM:
      return VA_CM_FMT_L8;
    case I965_SURFACEFORMAT_R16_UINT:
      return VA_CM_FMT_R16U;
    case I965_SURFACEFORMAT_R8_UNORM:
      return VA_CM_FMT_R8U;
    case I965_SURFACEFORMAT_L16_UNORM:
      return VA_CM_FMT_L16;
    case I965_SURFACEFORMAT_R32_FLOAT:
      return VA_CM_FMT_R32F;
    default:
      return VA_CM_FMT_UNKNOWN;
    }
}

static bool CreateCmrtMemory(cl_mem mem)
{
  if (mem->cmrt_mem != NULL)
    return true;

  CmDevice* cmrt_device = (CmDevice*)(mem->ctx->device->cmrt_device);
  int result;
  CmOsResource osResource;
  osResource.bo_size = mem->size;
  osResource.bo_flags = DRM_BO_HANDLE;
  osResource.bo = (drm_intel_bo*)mem->bo;
  if (IS_IMAGE(mem)) {
    _cl_mem_image* image = cl_mem_image(mem);
    if (CL_MEM_OBJECT_IMAGE2D != image->image_type)
      return CL_INVALID_ARG_VALUE;
    osResource.format = GetCmrtFormat(image);
    if (osResource.format == VA_CM_FMT_UNKNOWN)
      return false;
    osResource.aligned_width = image->row_pitch;
    osResource.aligned_height = mem->size / image->row_pitch;
    osResource.pitch = image->row_pitch;
    osResource.tile_type = image->tiling;
    osResource.orig_width = image->w;
    osResource.orig_height = image->h;
    CmSurface2D*& cmrt_surface2d = (CmSurface2D*&)(mem->cmrt_mem);
    result = cmrt_device->CreateSurface2D(&osResource, cmrt_surface2d);
    mem->cmrt_mem_type = CMRT_SURFACE2D;
  } else {
    osResource.format = VA_CM_FMT_BUFFER;
    osResource.buf_bytes = mem->size;
    CmBuffer*& cmrt_buffer = (CmBuffer*&)(mem->cmrt_mem);
    result = cmrt_device->CreateBuffer(&osResource, cmrt_buffer);
    mem->cmrt_mem_type = CMRT_BUFFER;
  }

  if (result != CM_SUCCESS)
    return false;

  return true;
}

cl_int cmrt_set_kernel_arg(cl_kernel k, cl_uint index, size_t sz, const void *value)
{
  if(value == NULL)
    return CL_INVALID_ARG_VALUE;

  CmKernel* cmrt_kernel = (CmKernel*)(k->cmrt_kernel);

  WORD argKind = -1;
  if (cmrt_kernel->GetArgKind(index, argKind) != CM_SUCCESS)
    return CL_INVALID_ARG_INDEX;

  int result;
  if (argKind == ARG_KIND_GENERAL)
    result = cmrt_kernel->SetKernelArg(index, sz, value);
  else {
    cl_mem mem = *(cl_mem*)value;
    if (((cl_base_object)mem)->magic == CL_MAGIC_MEM_HEADER) {
      if (!CreateCmrtMemory(mem))
        return CL_INVALID_ARG_VALUE;

      SurfaceIndex * memIndex = NULL;
      if (mem->cmrt_mem_type == CMRT_BUFFER) {
        CmBuffer* cmrt_buffer = (CmBuffer*)(mem->cmrt_mem);
        cmrt_buffer->GetIndex(memIndex);
      } else {
        CmSurface2D* cmrt_surface2d = (CmSurface2D*)(mem->cmrt_mem);
        cmrt_surface2d->GetIndex(memIndex);
      }
      result = cmrt_kernel->SetKernelArg(index, sizeof(SurfaceIndex), memIndex);
    } else
      return CL_INVALID_ARG_VALUE;
  }

  if (result != CM_SUCCESS)
    return CL_INVALID_KERNEL_ARGS;

  return CL_SUCCESS;
}

cl_int cmrt_destroy_memory(cl_mem mem)
{
  CmDevice* cmrt_device = (CmDevice*)(mem->ctx->device->cmrt_device);
  if (mem->cmrt_mem_type == CMRT_BUFFER) {
    CmBuffer*& cmrt_buffer = (CmBuffer*&)(mem->cmrt_mem);
    cmrt_device->DestroySurface(cmrt_buffer);
  } else {
    CmSurface2D*& cmrt_surface2d = (CmSurface2D*&)(mem->cmrt_mem);
    cmrt_device->DestroySurface(cmrt_surface2d);
  }
  return CL_SUCCESS;
}

cl_int cmrt_destroy_event(cl_command_queue cq)
{
  CmEvent*& cmrt_event = (CmEvent*&)(cq->cmrt_event);
  CmDevice* cmrt_device = (CmDevice*)(cq->ctx->device->cmrt_device);
  CmQueue* pCmQueue = NULL;
  cmrt_event->WaitForTaskFinished();
  cmrt_device->CreateQueue(pCmQueue);
  pCmQueue->DestroyEvent(cmrt_event);
  return CL_SUCCESS;
}

cl_int cmrt_wait_for_task_finished(cl_command_queue cq)
{
  CmEvent* cmrt_event = (CmEvent*)(cq->cmrt_event);
  cmrt_event->WaitForTaskFinished();
  return CL_SUCCESS;
}

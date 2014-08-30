/* 
 * Copyright © 2013 Simon Richter
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */
#include <ocl_icd.h>

#include "cl_platform_id.h"

/* The interop functions are not implemented in Beignet */
#define CL_GL_INTEROP(x) NULL
/* OpenCL 1.2 is not implemented in Beignet */
#define CL_1_2_NOTYET(x) NULL

/** Return platform list through ICD interface
 * This code is used only if a client is linked directly against the library
 * instead of using the ICD loader. In this case, no other implementations
 * should exist in the process address space, so the call is equivalent to
 * clGetPlatformIDs().
 *
 * @param[in]   num_entries     Number of entries allocated in return buffer
 * @param[out]  platforms       Platform identifiers supported by this implementation
 * @param[out]  num_platforms   Number of platform identifiers returned
 * @return      OpenCL error code
 * @retval      CL_SUCCESS                      Successful execution
 * @retval      CL_PLATFORM_NOT_FOUND_KHR       No platforms provided
 * @retval      CL_INVALID_VALUE                Invalid parameters
 */
cl_int
clIcdGetPlatformIDsKHR(cl_uint          num_entries,
                 cl_platform_id * platforms,
                 cl_uint *        num_platforms)
{
  return clGetPlatformIDs(num_entries, platforms, num_platforms);
}

struct _cl_icd_dispatch const cl_khr_icd_dispatch = {
  clGetPlatformIDs,
  clGetPlatformInfo,
  clGetDeviceIDs,
  clGetDeviceInfo,
  clCreateContext,
  clCreateContextFromType,
  clRetainContext,
  clReleaseContext,
  clGetContextInfo,
  clCreateCommandQueue,
  clRetainCommandQueue,
  clReleaseCommandQueue,
  clGetCommandQueueInfo,
  (void *) NULL, /* clSetCommandQueueProperty */
  clCreateBuffer,
  clCreateImage2D,
  clCreateImage3D,
  clRetainMemObject,
  clReleaseMemObject,
  clGetSupportedImageFormats,
  clGetMemObjectInfo,
  clGetImageInfo,
  clCreateSampler,
  clRetainSampler,
  clReleaseSampler,
  clGetSamplerInfo,
  clCreateProgramWithSource,
  clCreateProgramWithBinary,
  clRetainProgram,
  clReleaseProgram,
  clBuildProgram,
  clUnloadCompiler,
  clGetProgramInfo,
  clGetProgramBuildInfo,
  clCreateKernel,
  clCreateKernelsInProgram,
  clRetainKernel,
  clReleaseKernel,
  clSetKernelArg,
  clGetKernelInfo,
  clGetKernelWorkGroupInfo,
  clWaitForEvents,
  clGetEventInfo,
  clRetainEvent,
  clReleaseEvent,
  clGetEventProfilingInfo,
  clFlush,
  clFinish,
  clEnqueueReadBuffer,
  clEnqueueWriteBuffer,
  clEnqueueCopyBuffer,
  clEnqueueReadImage,
  clEnqueueWriteImage,
  clEnqueueCopyImage,
  clEnqueueCopyImageToBuffer,
  clEnqueueCopyBufferToImage,
  clEnqueueMapBuffer,
  clEnqueueMapImage,
  clEnqueueUnmapMemObject,
  clEnqueueNDRangeKernel,
  clEnqueueTask,
  clEnqueueNativeKernel,
  clEnqueueMarker,
  clEnqueueWaitForEvents,
  clEnqueueBarrier,
  clGetExtensionFunctionAddress,
  CL_GL_INTEROP(clCreateFromGLBuffer),
  CL_GL_INTEROP(clCreateFromGLTexture2D),
  CL_GL_INTEROP(clCreateFromGLTexture3D),
  CL_GL_INTEROP(clCreateFromGLRenderbuffer),
  CL_GL_INTEROP(clGetGLObjectInfo),
  CL_GL_INTEROP(clGetGLTextureInfo),
  CL_GL_INTEROP(clEnqueueAcquireGLObjects),
  CL_GL_INTEROP(clEnqueueReleaseGLObjects),
  CL_GL_INTEROP(clGetGLContextInfoKHR),
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  clSetEventCallback,
  clCreateSubBuffer,
  clSetMemObjectDestructorCallback,
  clCreateUserEvent,
  clSetUserEventStatus,
  clEnqueueReadBufferRect,
  clEnqueueWriteBufferRect,
  clEnqueueCopyBufferRect,
  CL_1_2_NOTYET(clCreateSubDevicesEXT),
  CL_1_2_NOTYET(clRetainDeviceEXT),
  CL_1_2_NOTYET(clReleaseDeviceEXT),
#ifdef CL_VERSION_1_2
  (void *) NULL,
  clCreateSubDevices,
  clRetainDevice,
  clReleaseDevice,
  clCreateImage,
  clCreateProgramWithBuiltInKernels,
  clCompileProgram,
  clLinkProgram,
  clUnloadPlatformCompiler,
  clGetKernelArgInfo,
  clEnqueueFillBuffer,
  clEnqueueFillImage,
  clEnqueueMigrateMemObjects,
  clEnqueueMarkerWithWaitList,
  clEnqueueBarrierWithWaitList,
  clGetExtensionFunctionAddressForPlatform,
  CL_GL_INTEROP(clCreateFromGLTexture),
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL,
  (void *) NULL
#endif
};


/* 
 * Copyright © 2012 Intel Corporation
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
 *
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

/* $Revision$ on $Date$ */

#ifndef __OPENCL_CL_D3D9_H
#define __OPENCL_CL_D3D9_H

#include <CL/cl_platform.h>
#include <d3d9.h>

#ifdef __cplusplus
extern "C" {
#endif

/* cl_khr_d3d9_sharing extension    */
#define cl_khr_d3d9_sharing 1

/* cl_context_properties            */
#define CL_CONTEXT_D3D9_DEVICE 0x1085

extern CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromD3D9BufferKHR(
    cl_context           /* context */,
    cl_mem_flags         /* flags */,
    IDirect3DResource9 * /* resource */,
    HANDLE               /* shared_handle */,
    cl_int *             /* errcode_ret */);


extern CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromD3D9TextureKHR(
    cl_context          /* context */,
    cl_mem_flags        /* flags */,
    IDirect3DTexture9 * /* texture */,
    HANDLE              /* shared_handle */,
    UINT                /* miplevel */,
    cl_int *            /* errcode_ret */);

extern CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromD3D9VolumeTextureKHR(
    cl_context                /* context */,
    cl_mem_flags              /* flags */,
    IDirect3DVolumeTexture9 * /* resource */,
    HANDLE                    /* shared_handle */,
    UINT                      /* miplevel */,
    cl_int *                  /* errcode_ret */);

extern CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromD3D9CubeTextureKHR(
    cl_context                /* context */,
    cl_mem_flags              /* flags */,
    IDirect3DCubeTexture9 *   /* resource */,                            
    HANDLE                    /* shared_handle */,
    D3DCUBEMAP_FACES Facetype /* face */,
    UINT                      /* miplevel */,
    cl_int *                  /* errcode_ret */);

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueAcquireD3D9ObjectsKHR(
    cl_command_queue /* command_queue */,
    cl_uint          /* num_objects */,
    const cl_mem *   /* mem_objects */,
    cl_uint          /* num_events_in_wait_list */,
    const cl_event * /* event_wait_list */,
    cl_event *       /* event */);

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReleaseD3D9ObjectsKHR(
    cl_command_queue /* command_queue */,
    cl_uint          /* num_objects */,
    const cl_mem *   /* mem_objects */,
    cl_uint          /* num_events_in_wait_list */,
    const cl_event * /* event_wait_list */,
    cl_event *       /* event */);

#ifdef __cplusplus
}
#endif

#endif  /* __OPENCL_CL_D3D9_H   */


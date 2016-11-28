/*
 * Copyright © 2012 - 2014 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __OCL_H__
#define __OCL_H__

/* LLVM 3.9 has these pre defined undef them first */
#ifdef cl_khr_3d_image_writes
#undef cl_khr_3d_image_writes
#endif
#ifdef cl_khr_byte_addressable_store
#undef cl_khr_byte_addressable_store
#endif
#ifdef cl_khr_fp16
#undef cl_khr_fp16
#endif
#ifdef cl_khr_fp64
#undef cl_khr_fp64
#endif
#ifdef cl_khr_global_int32_base_atomics
#undef cl_khr_global_int32_base_atomics
#endif
#ifdef cl_khr_global_int32_extended_atomics
#undef cl_khr_global_int32_extended_atomics
#endif
#ifdef cl_khr_gl_sharing
#undef cl_khr_gl_sharing
#endif
#ifdef cl_khr_icd
#undef cl_khr_icd
#endif
#ifdef cl_khr_local_int32_base_atomics
#undef cl_khr_local_int32_base_atomics
#endif
#ifdef cl_khr_local_int32_extended_atomics
#undef cl_khr_local_int32_extended_atomics
#endif

#ifdef cl_khr_d3d10_sharing
#undef cl_khr_d3d10_sharing
#endif
#ifdef cl_khr_gl_event
#undef cl_khr_gl_event
#endif
#ifdef cl_khr_int64_base_atomics
#undef cl_khr_int64_base_atomics
#endif
#ifdef cl_khr_int64_extended_atomics
#undef cl_khr_int64_extended_atomics
#endif

#ifdef cl_khr_d3d11_sharing
#undef cl_khr_d3d11_sharing
#endif
#ifdef cl_khr_depth_images
#undef cl_khr_depth_images
#endif
#ifdef cl_khr_dx9_media_sharing
#undef cl_khr_dx9_media_sharing
#endif
#ifdef cl_khr_gl_depth_images
#undef cl_khr_gl_depth_images
#endif
#ifdef cl_khr_spir
#undef cl_khr_spir
#endif

#include "ocl_defines.h"
#include "ocl_types.h"
#include "ocl_as.h"
#include "ocl_async.h"
#include "ocl_common.h"
#include "ocl_convert.h"
#include "ocl_float.h"
#include "ocl_geometric.h"
#include "ocl_image.h"
#include "ocl_integer.h"
#include "ocl_memcpy.h"
#include "ocl_memset.h"
#include "ocl_misc.h"
#include "ocl_printf.h"
#include "ocl_relational.h"
#include "ocl_sync.h"
#if (__OPENCL_C_VERSION__ >= 200)
#include "ocl_vload_20.h"
#include "ocl_atom_20.h"
#include "ocl_pipe.h"
#include "ocl_math_20.h"
#include "ocl_enqueue.h"
#else
#include "ocl_vload.h"
#include "ocl_atom.h"
#include "ocl_math.h"
#endif
#include "ocl_workitem.h"
#include "ocl_simd.h"
#include "ocl_work_group.h"

/* Move these out from ocl_defines.h for only one define */
#define cl_khr_global_int32_base_atomics
#define cl_khr_global_int32_extended_atomics
#define cl_khr_local_int32_base_atomics
#define cl_khr_local_int32_extended_atomics
#define cl_khr_byte_addressable_store
#define cl_khr_icd
#define cl_khr_gl_sharing
#define cl_khr_spir
#define cl_khr_fp16
#define cl_khr_3d_image_writes
#define cl_intel_subgroups
#define cl_intel_subgroups_short

#pragma OPENCL EXTENSION cl_khr_fp64 : disable
#pragma OPENCL EXTENSION cl_khr_fp16 : disable
#endif

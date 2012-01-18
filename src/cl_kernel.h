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

#ifndef __CL_KERNEL_H__
#define __CL_KERNEL_H__

#include "cl_defs.h"
#include "cl_internals.h"
#include "CL/cl.h"

#include <stdint.h>
#include <stdlib.h>

/***************************************************************************/
/* XXX Structures extracted from the WINDOWS CODE BASE                     */
/***************************************************************************/

// Some fields went from 1 to 4 bytes with the new compiler
#if USE_OLD_COMPILER
typedef uint8_t cl_compiler_boolean_t;
#else
typedef uint32_t cl_compiler_boolean_t;
#endif /* USE_OLD_COMPILER */

typedef struct cl_program_header {
  uint32_t magic;
  uint32_t version;
  uint32_t device;
  uint32_t ker_n;
} cl_program_header_t;

typedef struct cl_arg_info {
  uint32_t arg_index;
  uint32_t type;
  cl_compiler_boolean_t is_null;
  uint32_t offset;
  uint32_t sz;
  void *obj;
  cl_compiler_boolean_t is_patched;
  struct cl_arg_info *next;
} cl_arg_info_t;

typedef struct cl_curbe_patch_info {
  uint64_t key;
  uint32_t last;
  uint32_t offsets[OCLRT_CURBE_MAX_OFFSETS];
  uint32_t type;
  uint32_t arg_index;
  uint32_t sz;
  uint32_t src_offset;
  cl_compiler_boolean_t is_patched;
  cl_compiler_boolean_t is_local;
  struct cl_curbe_patch_info *next;
} cl_curbe_patch_info_t;

typedef struct cl_kernel_header {
  uint32_t check_sum;
  uint32_t kernel_name_sz;
  uint32_t patch_list_sz;
} cl_kernel_header_t;

typedef struct cl_kernel_header75 {
  cl_kernel_header_t header;
  uint32_t kernel_heap_sz;
  uint32_t general_state_heap_sz;
  uint32_t dynamic_state_heap_sz;
  uint32_t surface_state_heap_sz;
} cl_kernel_header75_t;

typedef struct cl_kernel_header7 {
  cl_kernel_header_t header;
  uint32_t kernel_heap_sz;
  uint32_t general_state_heap_sz;
  uint32_t dynamic_state_heap_sz;
  uint32_t surface_state_heap_sz;
} cl_kernel_header7_t;

typedef struct cl_kernel_header6 {
  cl_kernel_header_t header;
  uint32_t kernel_heap_sz;
  uint32_t general_state_heap_sz;
  uint32_t dynamic_state_heap_sz;
  uint32_t surface_state_heap_sz;
  uint32_t indirect_object__heap_sz;
} cl_kernel_header6_t;

typedef struct cl_patch_item_header {
  uint32_t token;
  uint32_t size;
} cl_patch_item_header_t;

typedef struct cl_global_memory_object_arg {
  cl_patch_item_header_t header;
  uint32_t index;
  uint32_t offset;
} cl_global_memory_object_arg_t;

#if USE_OLD_COMPILER == 0
typedef struct cl_image_memory_object_arg {
  cl_patch_item_header_t header;
  uint32_t index;
  uint32_t image_type;
  uint32_t offset;
} cl_image_memory_object_arg_t;
#endif

typedef struct cl_patch_constant_memory_object_arg {
  uint32_t index;
  uint32_t offset;
} cl_patch_constant_memory_object_arg_t;

typedef struct cl_patch_sampler_kernel_arg {
  cl_patch_item_header_t header;
  uint32_t index;
  uint32_t offset;
} cl_patch_sampler_kernel_arg_t;

typedef struct cl_patch_data_parameter_buffer {
  cl_patch_item_header_t header;
  uint32_t type;
  uint32_t index;
  uint32_t offset;
  uint32_t data_sz;
  uint32_t src_offset;
} cl_patch_data_parameter_buffer_t;

typedef struct cl_patch_data_parameter_stream {
  cl_patch_item_header_t header;
  uint32_t data_parameter_stream_sz;
} cl_patch_data_parameter_stream_t;

typedef struct cl_patch_sip {
  cl_patch_item_header_t header;
  uint32_t sip_offset;
} cl_patch_sip_t;

typedef struct cl_patch_sampler_state_array {
  cl_patch_item_header_t header;
  uint32_t offset;
  uint32_t count;
  uint32_t border_color_offset;
} cl_patch_sampler_state_array_t;

typedef struct cl_patch_binding_table_state {
  cl_patch_item_header_t header;
  uint32_t offset;
  uint32_t count;
  uint32_t surface_state_offset;
} cl_patch_binding_table_state_t;

typedef struct cl_patch_alloc_scratch_surf {
  cl_patch_item_header_t header;
  uint32_t offset;
  uint32_t size;
} cl_patch_alloc_scratch_surf_t;

typedef struct cl_patch_alloc_private_memory_surf {
  cl_patch_item_header_t header;
  uint32_t offset;
  uint32_t size;
} cl_patch_alloc_private_memory_surf_t;

typedef struct cl_patch_alloc_system_thread_surf {
  cl_patch_item_header_t header;
  uint32_t offset;
  uint32_t sz;
} cl_patch_alloc_system_thread_surf_t;

typedef struct cl_patch_alloc_surf_with_init {
  cl_patch_item_header_t header;
  uint32_t offset;
  uint32_t sz;
  char* data;
} cl_patch_alloc_surf_with_init_t;

typedef struct cl_patch_alloc_local_surf {
  cl_patch_item_header_t header;
  uint32_t offset;
  uint32_t sz;
} cl_patch_alloc_local_surf_t;

typedef struct cl_patch_thread_payload {
  cl_patch_item_header_t header;
  uint8_t header_present;
  uint8_t local_idx_present;
  uint8_t local_idy_present;
  uint8_t local_idz_present;
} cl_patch_thread_payload_t;

typedef struct cl_patch_exec_env {
  cl_patch_item_header_t header;
  uint32_t required_wgr_sz_x;
  uint32_t required_wgr_sz_y;
  uint32_t required_wgr_sz_z;
  uint32_t largest_compiled_simd_sz;
  uint8_t has_barriers;
  uint8_t compiled_simd8;
  uint8_t compiled_simd16;
  uint8_t compiled_simd32;
} cl_patch_exec_env_t;

typedef struct cl_patch_vfe_state {
  cl_patch_item_header_t header;
  uint32_t scratch_offset;
} cl_patch_vfe_state_t;

typedef struct cl_patch_curbe_load {
  cl_patch_item_header_t header;
  uint32_t offset;
  uint32_t sz;
} cl_patch_curbe_load_t;

typedef struct cl_patch_interface_desc_load {
  cl_patch_item_header_t header;
  uint32_t offset;
} cl_patch_interface_desc_load_t;

typedef struct cl_patch_interface_desc_data {
  cl_patch_item_header_t header;
  uint32_t offset;
  uint32_t sampler_state_offset;
  uint32_t kernel_offset;
  uint32_t binding_table_offset;
} cl_patch_interface_desc_data_t;

typedef struct cl_kernel_patch_info {
  cl_patch_sip_t sip;
  cl_patch_sampler_state_array_t sampler_state;
  cl_patch_binding_table_state_t binding_table;
  cl_patch_alloc_scratch_surf_t scratch;
  cl_patch_alloc_private_memory_surf_t private_surf;
  cl_patch_alloc_system_thread_surf_t sys_thread_surf;
  cl_patch_alloc_surf_with_init_t surf_with_init;
  cl_patch_alloc_local_surf_t local_surf;
  cl_patch_thread_payload_t thread_payload;
  cl_patch_exec_env_t exec_env;
  cl_patch_vfe_state_t vfe;
  cl_patch_curbe_load_t curbe;
  cl_patch_interface_desc_load_t idrt;
  cl_patch_interface_desc_data_t surf_desc;
} cl_kernel_patch_info_t;

struct _cl_kernel {
  uint64_t magic;                /* To identify it as a kernel */
  volatile int ref_n;            /* We reference count this object */
  struct _drm_intel_bo *bo;      /* The code itself */
  struct _drm_intel_bo *const_bo;/* Buffer for all __constants values in the OCL program */
  cl_program program;            /* Owns this structure (and pointers) */
  cl_arg_info_t *arg_info;       /* List of arguments */
  cl_curbe_patch_info_t *curbe_info; /* List of patch locations for the curbe */
  char *name;                   /* User defined name */
  char *cst_buffer;             /* (user provided) NDrange kernel parameters */
  void **args;                  /* (user provided) arguments which are cl_mem / cl_image / cl_sampler */
  uint8_t *is_provided;         /* Tell us if all arguments have been provided by the user */
  const char *patch_list;       /* Defines where the data are in the heaps */
  const char *kernel_heap;      /* Contains instructions */
  const char *general_heap;     /* Contains scratch space */
  const char *surface_heap;     /* Contains surface state and binding table */
  const char *dynamic_heap;     /* Contains IDRT and sampler states */
  size_t patch_list_sz;         /* Total size of the patch list */
  size_t kernel_heap_sz;        /* Size of the kernel heap */
  size_t general_heap_sz;       /* Should be 0 */
  size_t surface_heap_sz;       /* Size of the surface state heap */
  size_t dynamic_heap_sz;       /* Size of the dynamic heap */
  cl_kernel_patch_info_t patch; /* Got from the patch list */
  uint32_t arg_info_n;          /* Number of argument info */
  uint32_t curbe_info_n;        /* Number of curbe info */
  uint32_t arg_n;               /* Number of arguments in the function */
  uint32_t const_bo_index;      /* Index in the binding table for const_bo */
  uint8_t has_local_buffer;     /* Is there any __local * as function argument? */
  uint8_t ref_its_program;      /* True only for the user kernel (those created by clCreateKernel) */
};

/* Size of the surface state as encoded in the binary blob */
#define SURFACE_SZ 64

/* Allocate an empty kernel */
extern cl_kernel cl_kernel_new(void);

/* Destroy and deallocate an empty kernel */
extern void cl_kernel_delete(cl_kernel);

/* When a kernel is created from outside, we just duplicate the structure we
 * have internally and give it back to the user
 */
extern cl_kernel cl_kernel_dup(cl_kernel);

/* Add one more reference on the kernel object */
extern void cl_kernel_add_ref(cl_kernel);

/* Setup a kernel from a binary blob */
extern int cl_kernel_setup(cl_kernel, const char*);

/* Set the argument before kernel execution */
extern int cl_kernel_set_arg(cl_kernel,
                             uint32_t    arg_index,
                             size_t      arg_size,
                             const void *arg_value);

/* Check that all arguments are set before running the kernel */
extern cl_int cl_kernel_check_args(cl_kernel);

/* Get the size of shared local memory bound to the kernel */
extern uint32_t cl_kernel_local_memory_sz(cl_kernel);

/* Return a curbe entry if it exists. NULL otherwise */
extern cl_curbe_patch_info_t *cl_kernel_get_curbe_info(cl_kernel, uint64_t);

/* To look up the sorted curbe array */
static inline uint64_t
cl_curbe_key(uint32_t type, uint32_t index, uint32_t src_offset)
{
  return ((uint64_t) type << 48)  |
         ((uint64_t) index << 32) |
          (uint64_t) src_offset;
}

/* Allocate, fill and return the CURBE */
extern char*
cl_kernel_create_cst_buffer(cl_kernel k,
                            const size_t *global_wk_off,
                            const size_t *global_wk_sz,
                            const size_t *local_wk_sz,
                            cl_uint wk_dim,
                            cl_uint thread_n);

/* Compute and check the work group size from the user provided local size */
extern cl_int
cl_kernel_work_group_sz(cl_kernel ker,
                        const size_t *local_wk_sz,
                        cl_uint wk_dim,
                        size_t *wk_grp_sz);

#endif /* __CL_KERNEL_H__ */


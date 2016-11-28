/* 
 * Copyright © 2012 Intel Corporation
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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __CL_DRIVER_H__
#define __CL_DRIVER_H__

#include <stdint.h>
#include <stdlib.h>
#include "cl_driver_type.h"
#include "CL/cl_ext.h"
/* Various limitations we should remove actually */
#define GEN_MAX_SURFACES 256
#define GEN_MAX_SAMPLERS 16
#define GEN_MAX_VME_STATES 8

/**************************************************************************
 * cl_driver:
 * Hide behind some call backs the buffer allocation / deallocation ... This
 * will allow us to make the use of a software performance simulator easier and
 * to minimize the code specific for the HW and for the simulator
 **************************************************************************/
/* Create a new driver */
typedef cl_driver (cl_driver_new_cb)(cl_context_prop);
extern cl_driver_new_cb *cl_driver_new;

/* Delete the driver */
typedef void (cl_driver_delete_cb)(cl_driver);
extern cl_driver_delete_cb *cl_driver_delete;

/* Get the buffer manager from the driver */
typedef cl_buffer_mgr (cl_driver_get_bufmgr_cb)(cl_driver);
extern cl_driver_get_bufmgr_cb *cl_driver_get_bufmgr;

/* Get the Gen version from the driver */
typedef uint32_t (cl_driver_get_ver_cb)(cl_driver);
extern cl_driver_get_ver_cb *cl_driver_get_ver;

/* enlarge stack size from the driver */
typedef void (cl_driver_enlarge_stack_size_cb)(cl_driver, int32_t*);
extern cl_driver_enlarge_stack_size_cb *cl_driver_enlarge_stack_size;

typedef enum cl_self_test_res{
  SELF_TEST_PASS = 0,
  SELF_TEST_SLM_FAIL  = 1,
  SELF_TEST_ATOMIC_FAIL = 2,
  SELF_TEST_OTHER_FAIL = 3,
} cl_self_test_res;
/* Set the atomic enable/disable flag in the driver */
typedef void (cl_driver_set_atomic_flag_cb)(cl_driver, int);
extern cl_driver_set_atomic_flag_cb *cl_driver_set_atomic_flag;
/**************************************************************************
 * GPGPU command streamer
 **************************************************************************/
/* Describe texture tiling */
typedef enum cl_gpgpu_tiling {
  GPGPU_NO_TILE = 0,
  GPGPU_TILE_X  = 1,
  GPGPU_TILE_Y  = 2,
} cl_gpgpu_tiling;

/* Cache control options for gen7 */
typedef enum cl_cache_control {
  cc_gtt      = 0x0,
  cc_l3       = 0x1,
  cc_llc      = 0x2,
  cc_llc_l3   = 0x3
} cl_cache_control;

/* L3 Cache control options for gen75 */
typedef enum cl_l3_cache_control {
  l3cc_uc      = 0x0,
  l3cc_ec       = 0x1
} cl_l3_cache_control;

/* LLCCC Cache control options for gen75 */
typedef enum cl_llccc_cache_control {
  llccc_pte      = 0x0<<1,
  llccc_uc       = 0x1<<1,
  llccc_ec       = 0x2<<1,
  llccc_ucllc    = 0x3<<1
} cl_llccc_cache_control;

/* Target Cache control options for gen8 */
typedef enum cl_target_cache_control {
  tcc_ec_only    = 0x0<<3,
  tcc_llc_only   = 0x1<<3,
  tcc_llc_ec     = 0x2<<3,
  tcc_llc_ec_l3  = 0x3<<3
} cl_target_cache_control;

/* Memory type LLC/ELLC Cache control options for gen8 */
typedef enum cl_mtllc_cache_control {
  mtllc_pte      = 0x0<<5,
  mtllc_none     = 0x1<<5,
  mtllc_wt       = 0x2<<5,
  mtllc_wb       = 0x3<<5
} cl_mtllc_cache_control;

typedef enum gpu_command_status {
  command_queued    = 3,
  command_submitted = 2,
  command_running   = 1,
  command_complete  = 0
} gpu_command_status;

/* Use this structure to bind kernels in the gpgpu state */
typedef struct cl_gpgpu_kernel {
  const char *name;        /* kernel name and bo name */
  uint32_t grf_blocks;     /* register blocks kernel wants (in 8 reg blocks) */
  uint32_t curbe_sz;       /* total size of all curbes */
  cl_buffer bo;            /* kernel code in the proper addr space */
  int32_t barrierID;       /* barrierID for _this_ kernel */
  uint32_t use_slm:1;      /* For gen7 (automatic barrier management) */
  uint32_t thread_n:15;    /* For gen7 (automatic barrier management) */
  uint32_t slm_sz;         /* For gen7 (automatic SLM allocation) */
} cl_gpgpu_kernel;

/* Create a new gpgpu state */
typedef cl_gpgpu (cl_gpgpu_new_cb)(cl_driver);
extern cl_gpgpu_new_cb *cl_gpgpu_new;

/* Delete the gpgpu state */
typedef void (cl_gpgpu_delete_cb)(cl_gpgpu);
extern cl_gpgpu_delete_cb *cl_gpgpu_delete;

/* Synchonize GPU with CPU */
typedef void (cl_gpgpu_sync_cb)(void*);
extern cl_gpgpu_sync_cb *cl_gpgpu_sync;

/* Bind a regular unformatted buffer */
typedef void (cl_gpgpu_bind_buf_cb)(cl_gpgpu, cl_buffer, uint32_t offset, uint32_t internal_offset, size_t size, uint8_t bti);
extern cl_gpgpu_bind_buf_cb *cl_gpgpu_bind_buf;

typedef void (cl_gpgpu_set_kernel_cb)(cl_gpgpu, void *);
extern cl_gpgpu_set_kernel_cb *cl_gpgpu_set_kernel;

typedef void* (cl_gpgpu_get_kernel_cb)(cl_gpgpu);
extern cl_gpgpu_get_kernel_cb *cl_gpgpu_get_kernel;

/* bind samplers defined in both kernel and kernel args. */
typedef void (cl_gpgpu_bind_sampler_cb)(cl_gpgpu, uint32_t *samplers, size_t sampler_sz);
extern cl_gpgpu_bind_sampler_cb *cl_gpgpu_bind_sampler;

typedef void (cl_gpgpu_bind_vme_state_cb)(cl_gpgpu, cl_accelerator_intel accel);
extern cl_gpgpu_bind_vme_state_cb *cl_gpgpu_bind_vme_state;

/* get the default cache control value. */
typedef uint32_t (cl_gpgpu_get_cache_ctrl_cb)();
extern cl_gpgpu_get_cache_ctrl_cb *cl_gpgpu_get_cache_ctrl;
/* Set a 2d texture */
typedef void (cl_gpgpu_bind_image_cb)(cl_gpgpu state,
                                      uint32_t id,
                                      cl_buffer obj_bo,
                                      uint32_t obj_bo_offset,
                                      uint32_t format,
                                      uint32_t bpp,
                                      uint32_t type,
                                      int32_t w,
                                      int32_t h,
                                      int32_t depth,
                                      int pitch,
                                      int32_t slice_pitch,
                                      cl_gpgpu_tiling tiling);

extern cl_gpgpu_bind_image_cb *cl_gpgpu_bind_image;

typedef void (cl_gpgpu_bind_image_for_vme_cb)(cl_gpgpu state,
                                              uint32_t id,
                                              cl_buffer obj_bo,
                                              uint32_t obj_bo_offset,
                                              uint32_t format,
                                              uint32_t bpp,
                                              uint32_t type,
                                              int32_t w,
                                              int32_t h,
                                              int32_t depth,
                                              int pitch,
                                              int32_t slice_pitch,
                                              cl_gpgpu_tiling tiling);

extern cl_gpgpu_bind_image_for_vme_cb *cl_gpgpu_bind_image_for_vme;

/* Setup a stack */
typedef void (cl_gpgpu_set_stack_cb)(cl_gpgpu, uint32_t offset, uint32_t size, uint32_t cchint);
extern cl_gpgpu_set_stack_cb *cl_gpgpu_set_stack;

/* Setup scratch */
typedef int (cl_gpgpu_set_scratch_cb)(cl_gpgpu, uint32_t per_thread_size);
extern cl_gpgpu_set_scratch_cb *cl_gpgpu_set_scratch;

/* Configure internal state */
typedef int (cl_gpgpu_state_init_cb)(cl_gpgpu, uint32_t max_threads, uint32_t size_cs_entry, int profiling);
extern cl_gpgpu_state_init_cb *cl_gpgpu_state_init;

/* Set the buffer object where to report performance counters */
typedef void (cl_gpgpu_set_perf_counters_cb)(cl_gpgpu, cl_buffer perf);
extern cl_gpgpu_set_perf_counters_cb *cl_gpgpu_set_perf_counters;

/* Fills current curbe buffer with data */
typedef int (cl_gpgpu_upload_curbes_cb)(cl_gpgpu, const void* data, uint32_t size);
extern cl_gpgpu_upload_curbes_cb *cl_gpgpu_upload_curbes;

typedef cl_buffer (cl_gpgpu_alloc_constant_buffer_cb)(cl_gpgpu, uint32_t size, uint8_t bti);
extern cl_gpgpu_alloc_constant_buffer_cb *cl_gpgpu_alloc_constant_buffer;

/* Setup all indirect states */
typedef void (cl_gpgpu_states_setup_cb)(cl_gpgpu, cl_gpgpu_kernel *kernel);
extern cl_gpgpu_states_setup_cb *cl_gpgpu_states_setup;

/* Upload the constant samplers as specified inside the OCL kernel */
typedef void (cl_gpgpu_upload_samplers_cb)(cl_gpgpu *state, const void *data, uint32_t n);
extern cl_gpgpu_upload_samplers_cb *cl_gpgpu_upload_samplers;

/* Set a sampler */
typedef void (cl_gpgpu_set_sampler_cb)(cl_gpgpu, uint32_t index, uint32_t non_normalized);
extern cl_gpgpu_set_sampler_cb *cl_gpgpu_set_sampler;

/* Allocate the batch buffer and return the BO used for the batch buffer */
typedef int (cl_gpgpu_batch_reset_cb)(cl_gpgpu, size_t sz);
extern cl_gpgpu_batch_reset_cb *cl_gpgpu_batch_reset;

/* Atomic begin, pipeline select, urb, pipeline state and constant buffer */
typedef void (cl_gpgpu_batch_start_cb)(cl_gpgpu);
extern cl_gpgpu_batch_start_cb *cl_gpgpu_batch_start;

/* atomic end with possibly inserted flush */
typedef void (cl_gpgpu_batch_end_cb)(cl_gpgpu, int32_t flush_mode);
extern cl_gpgpu_batch_end_cb *cl_gpgpu_batch_end;

/* Flush the command buffer */
typedef int (cl_gpgpu_flush_cb)(cl_gpgpu);
extern cl_gpgpu_flush_cb *cl_gpgpu_flush;

/* new a event for a batch buffer */
typedef cl_gpgpu_event (cl_gpgpu_event_new_cb)(cl_gpgpu);
extern cl_gpgpu_event_new_cb *cl_gpgpu_event_new;

/* update the batch buffer of this event */
typedef int (cl_gpgpu_event_update_status_cb)(cl_gpgpu_event, int);
extern cl_gpgpu_event_update_status_cb *cl_gpgpu_event_update_status;

/* flush the batch buffer of this event */
typedef void (cl_gpgpu_event_flush_cb)(cl_gpgpu_event);
extern cl_gpgpu_event_flush_cb *cl_gpgpu_event_flush;

/* cancel exec batch buffer of this event */
typedef void (cl_gpgpu_event_cancel_cb)(cl_gpgpu_event);
extern cl_gpgpu_event_cancel_cb *cl_gpgpu_event_cancel;

/* delete a gpgpu event */
typedef void (cl_gpgpu_event_delete_cb)(cl_gpgpu_event);
extern cl_gpgpu_event_delete_cb *cl_gpgpu_event_delete;

/* Get a event time stamp */
typedef void (cl_gpgpu_event_get_exec_timestamp_cb)(cl_gpgpu, int, uint64_t*);
extern cl_gpgpu_event_get_exec_timestamp_cb *cl_gpgpu_event_get_exec_timestamp;

/* Get current GPU time stamp */
typedef void (cl_gpgpu_event_get_gpu_cur_timestamp_cb)(cl_driver, uint64_t*);
extern cl_gpgpu_event_get_gpu_cur_timestamp_cb *cl_gpgpu_event_get_gpu_cur_timestamp;

/* Get current batch buffer handle */
typedef void* (cl_gpgpu_ref_batch_buf_cb)(cl_gpgpu);
extern cl_gpgpu_ref_batch_buf_cb *cl_gpgpu_ref_batch_buf;

/* Get release batch buffer handle */
typedef void (cl_gpgpu_unref_batch_buf_cb)(void*);
extern cl_gpgpu_unref_batch_buf_cb *cl_gpgpu_unref_batch_buf;

/* Set the profiling buffer */
typedef int (cl_gpgpu_set_profiling_buffer_cb)(cl_gpgpu, uint32_t, uint32_t, uint8_t);
extern cl_gpgpu_set_profiling_buffer_cb *cl_gpgpu_set_profiling_buffer;

typedef int (cl_gpgpu_set_profiling_info_cb)(cl_gpgpu, void *);
extern cl_gpgpu_set_profiling_info_cb *cl_gpgpu_set_profiling_info;

typedef void* (cl_gpgpu_get_profiling_info_cb)(cl_gpgpu);
extern cl_gpgpu_get_profiling_info_cb *cl_gpgpu_get_profiling_info;

typedef void* (cl_gpgpu_map_profiling_buffer_cb)(cl_gpgpu);
extern cl_gpgpu_map_profiling_buffer_cb *cl_gpgpu_map_profiling_buffer;

typedef void (cl_gpgpu_unmap_profiling_buffer_cb)(cl_gpgpu);
extern cl_gpgpu_unmap_profiling_buffer_cb *cl_gpgpu_unmap_profiling_buffer;

/* Set the printf buffer */
typedef int (cl_gpgpu_set_printf_buffer_cb)(cl_gpgpu, uint32_t, uint8_t);
extern cl_gpgpu_set_printf_buffer_cb *cl_gpgpu_set_printf_buffer;

/* get the printf buffer offset in the apeture*/
typedef unsigned long (cl_gpgpu_reloc_printf_buffer_cb)(cl_gpgpu, uint32_t, uint32_t);
extern cl_gpgpu_reloc_printf_buffer_cb *cl_gpgpu_reloc_printf_buffer;

/* map the printf buffer */
typedef void* (cl_gpgpu_map_printf_buffer_cb)(cl_gpgpu);
extern cl_gpgpu_map_printf_buffer_cb *cl_gpgpu_map_printf_buffer;

/* unmap the printf buffer */
typedef void (cl_gpgpu_unmap_printf_buffer_cb)(cl_gpgpu);
extern cl_gpgpu_unmap_printf_buffer_cb *cl_gpgpu_unmap_printf_buffer;

/* release the printf buffer */
typedef unsigned long (cl_gpgpu_release_printf_buffer_cb)(cl_gpgpu);
extern cl_gpgpu_release_printf_buffer_cb *cl_gpgpu_release_printf_buffer;

/* Set the last printfset pointer */
typedef int (cl_gpgpu_set_printf_info_cb)(cl_gpgpu, void *);
extern cl_gpgpu_set_printf_info_cb *cl_gpgpu_set_printf_info;

/* Get the last printfset pointer */
typedef void* (cl_gpgpu_get_printf_info_cb)(cl_gpgpu);
extern cl_gpgpu_get_printf_info_cb *cl_gpgpu_get_printf_info;

/* Will spawn all threads */
typedef void (cl_gpgpu_walker_cb)(cl_gpgpu,
                                  uint32_t simd_sz,
                                  uint32_t thread_n,
                                  const size_t global_wk_off[3],
                                  const size_t global_dim_off[3],
                                  const size_t global_wk_sz[3],
                                  const size_t local_wk_sz[3]);
extern cl_gpgpu_walker_cb *cl_gpgpu_walker;
/**************************************************************************
 * Buffer
 **************************************************************************/
/* Allocate a buffer */
typedef cl_buffer (cl_buffer_alloc_cb)(cl_buffer_mgr, const char*, size_t, size_t);
extern cl_buffer_alloc_cb *cl_buffer_alloc;

typedef cl_buffer (cl_buffer_alloc_userptr_cb)(cl_buffer_mgr, const char*, void *, size_t, unsigned long);
extern cl_buffer_alloc_userptr_cb *cl_buffer_alloc_userptr;

typedef int (cl_buffer_set_softpin_offset_cb)(cl_buffer, uint64_t);
extern cl_buffer_set_softpin_offset_cb *cl_buffer_set_softpin_offset;

typedef int (cl_buffer_set_bo_use_full_range_cb)(cl_buffer, uint32_t);
extern cl_buffer_set_bo_use_full_range_cb *cl_buffer_set_bo_use_full_range;

typedef int (cl_buffer_disable_reuse_cb)(cl_buffer);
extern cl_buffer_disable_reuse_cb *cl_buffer_disable_reuse;

/* Set a buffer's tiling mode */
typedef int (cl_buffer_set_tiling_cb)(cl_buffer, int tiling, size_t stride);
extern cl_buffer_set_tiling_cb *cl_buffer_set_tiling;

#include "cl_context.h"
#include "cl_mem.h"

typedef cl_buffer (cl_buffer_alloc_from_texture_cb)(cl_context, unsigned int, int, unsigned int,
                                                    struct _cl_mem_image *gl_image);
extern cl_buffer_alloc_from_texture_cb *cl_buffer_alloc_from_texture;

typedef void (cl_buffer_release_from_texture_cb)(cl_context, struct _cl_mem_gl_image *);
extern cl_buffer_release_from_texture_cb *cl_buffer_release_from_texture;

typedef cl_buffer (cl_buffer_get_buffer_from_libva_cb)(cl_context ctx, unsigned int bo_name, size_t *sz);
extern cl_buffer_get_buffer_from_libva_cb *cl_buffer_get_buffer_from_libva;

typedef cl_buffer (cl_buffer_get_image_from_libva_cb)(cl_context ctx, unsigned int bo_name, struct _cl_mem_image *image);
extern cl_buffer_get_image_from_libva_cb *cl_buffer_get_image_from_libva;

/* Unref a buffer and destroy it if no more ref */
typedef int (cl_buffer_unreference_cb)(cl_buffer);
extern cl_buffer_unreference_cb *cl_buffer_unreference;

/* Add one more ref on a buffer */
typedef void (cl_buffer_reference_cb)(cl_buffer);
extern cl_buffer_reference_cb *cl_buffer_reference;

/* Map a buffer */
typedef int (cl_buffer_map_cb)(cl_buffer, uint32_t write_enable);
extern cl_buffer_map_cb *cl_buffer_map;

/* Unmap a buffer */
typedef int (cl_buffer_unmap_cb)(cl_buffer);
extern cl_buffer_unmap_cb *cl_buffer_unmap;

/* Map a buffer in the GTT domain */
typedef int (cl_buffer_map_gtt_cb)(cl_buffer);
extern cl_buffer_map_gtt_cb *cl_buffer_map_gtt;

/* Map a buffer in the GTT domain, non waiting the GPU read or write*/
typedef int (cl_buffer_map_gtt_unsync_cb)(cl_buffer);
extern cl_buffer_map_gtt_unsync_cb *cl_buffer_map_gtt_unsync;

/* Unmap a buffer in the GTT domain */
typedef int (cl_buffer_unmap_gtt_cb)(cl_buffer);
extern cl_buffer_unmap_gtt_cb *cl_buffer_unmap_gtt;

/* Get the virtual address (when mapped) */
typedef void* (cl_buffer_get_virtual_cb)(cl_buffer);
extern cl_buffer_get_virtual_cb *cl_buffer_get_virtual;

/* Get the size of the buffer */
typedef size_t (cl_buffer_get_size_cb)(cl_buffer);
extern cl_buffer_get_size_cb *cl_buffer_get_size;

/* Pin a buffer */
typedef int (cl_buffer_pin_cb)(cl_buffer, uint32_t alignment);
extern cl_buffer_pin_cb *cl_buffer_pin;

/* Unpin a buffer */
typedef int (cl_buffer_unpin_cb)(cl_buffer);
extern cl_buffer_unpin_cb *cl_buffer_unpin;

/* Fill data in the buffer */
typedef int (cl_buffer_subdata_cb)(cl_buffer, unsigned long, unsigned long, const void*);
extern cl_buffer_subdata_cb *cl_buffer_subdata;

/* Get data from buffer */
typedef int (cl_buffer_get_subdata_cb)(cl_buffer, unsigned long, unsigned long, void*);
extern cl_buffer_get_subdata_cb *cl_buffer_get_subdata;

/* Wait for all pending rendering for this buffer to complete */
typedef int (cl_buffer_wait_rendering_cb) (cl_buffer);
extern cl_buffer_wait_rendering_cb *cl_buffer_wait_rendering;

typedef int (cl_buffer_get_fd_cb)(cl_buffer, int *fd);
extern cl_buffer_get_fd_cb *cl_buffer_get_fd;

typedef int (cl_buffer_get_tiling_align_cb)(cl_context ctx, uint32_t tiling_mode, uint32_t dim);
extern cl_buffer_get_tiling_align_cb *cl_buffer_get_tiling_align;

typedef cl_buffer (cl_buffer_get_buffer_from_fd_cb)(cl_context ctx, int fd, int size);
extern cl_buffer_get_buffer_from_fd_cb *cl_buffer_get_buffer_from_fd;

typedef cl_buffer (cl_buffer_get_image_from_fd_cb)(cl_context ctx, int fd, int size, struct _cl_mem_image *image);
extern cl_buffer_get_image_from_fd_cb *cl_buffer_get_image_from_fd;

/* Get the device id */
typedef int (cl_driver_get_device_id_cb)(void);
extern cl_driver_get_device_id_cb *cl_driver_get_device_id;

/* Update the device info */
typedef void (cl_driver_update_device_info_cb)(cl_device_id device);
extern cl_driver_update_device_info_cb *cl_driver_update_device_info;

#endif /* __CL_DRIVER_H__ */


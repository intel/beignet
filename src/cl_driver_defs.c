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
#include "cl_driver.h"
#include "cl_utils.h"
#include <stdlib.h>

/* Driver */
LOCAL cl_driver_new_cb *cl_driver_new = NULL;
LOCAL cl_driver_delete_cb *cl_driver_delete = NULL;
LOCAL cl_driver_get_bufmgr_cb *cl_driver_get_bufmgr = NULL;
LOCAL cl_driver_get_ver_cb *cl_driver_get_ver = NULL;
LOCAL cl_driver_get_device_id_cb *cl_driver_get_device_id = NULL;

/* Buffer */
LOCAL cl_buffer_alloc_cb *cl_buffer_alloc = NULL;
LOCAL cl_buffer_alloc_userptr_cb *cl_buffer_alloc_userptr = NULL;
LOCAL cl_buffer_set_tiling_cb *cl_buffer_set_tiling = NULL;
LOCAL cl_buffer_alloc_from_texture_cb *cl_buffer_alloc_from_texture = NULL;
LOCAL cl_buffer_release_from_texture_cb *cl_buffer_release_from_texture = NULL;
LOCAL cl_buffer_reference_cb *cl_buffer_reference = NULL;
LOCAL cl_buffer_unreference_cb *cl_buffer_unreference = NULL;
LOCAL cl_buffer_map_cb *cl_buffer_map = NULL;
LOCAL cl_buffer_unmap_cb *cl_buffer_unmap = NULL;
LOCAL cl_buffer_map_gtt_cb *cl_buffer_map_gtt = NULL;
LOCAL cl_buffer_map_gtt_unsync_cb *cl_buffer_map_gtt_unsync = NULL;
LOCAL cl_buffer_unmap_gtt_cb *cl_buffer_unmap_gtt = NULL;
LOCAL cl_buffer_get_virtual_cb *cl_buffer_get_virtual = NULL;
LOCAL cl_buffer_get_size_cb *cl_buffer_get_size = NULL;
LOCAL cl_buffer_pin_cb *cl_buffer_pin = NULL;
LOCAL cl_buffer_unpin_cb *cl_buffer_unpin = NULL;
LOCAL cl_buffer_subdata_cb *cl_buffer_subdata = NULL;
LOCAL cl_buffer_get_subdata_cb *cl_buffer_get_subdata = NULL;
LOCAL cl_buffer_wait_rendering_cb *cl_buffer_wait_rendering = NULL;
LOCAL cl_buffer_get_buffer_from_libva_cb *cl_buffer_get_buffer_from_libva = NULL;
LOCAL cl_buffer_get_image_from_libva_cb *cl_buffer_get_image_from_libva = NULL;
LOCAL cl_buffer_get_fd_cb *cl_buffer_get_fd = NULL;
LOCAL cl_buffer_get_tiling_align_cb *cl_buffer_get_tiling_align = NULL;

/* cl_khr_gl_sharing */
LOCAL cl_gl_acquire_texture_cb *cl_gl_acquire_texture = NULL;
LOCAL cl_gl_release_texture_cb *cl_gl_release_texture = NULL;
LOCAL cl_gl_acquire_buffer_object_cb *cl_gl_acquire_buffer_object = NULL;
LOCAL cl_gl_release_buffer_object_cb *cl_gl_release_buffer_object = NULL;
LOCAL cl_gl_acquire_render_buffer_cb *cl_gl_acquire_render_buffer = NULL;
LOCAL cl_gl_release_render_buffer_cb *cl_gl_release_render_buffer = NULL;
/* GPGPU */
LOCAL cl_gpgpu_new_cb *cl_gpgpu_new = NULL;
LOCAL cl_gpgpu_delete_cb *cl_gpgpu_delete = NULL;
LOCAL cl_gpgpu_sync_cb *cl_gpgpu_sync = NULL;
LOCAL cl_gpgpu_bind_buf_cb *cl_gpgpu_bind_buf = NULL;
LOCAL cl_gpgpu_set_stack_cb *cl_gpgpu_set_stack = NULL;
LOCAL cl_gpgpu_set_scratch_cb *cl_gpgpu_set_scratch = NULL;
LOCAL cl_gpgpu_bind_image_cb *cl_gpgpu_bind_image = NULL;
LOCAL cl_gpgpu_get_cache_ctrl_cb *cl_gpgpu_get_cache_ctrl = NULL;
LOCAL cl_gpgpu_state_init_cb *cl_gpgpu_state_init = NULL;
LOCAL cl_gpgpu_alloc_constant_buffer_cb * cl_gpgpu_alloc_constant_buffer = NULL;
LOCAL cl_gpgpu_set_perf_counters_cb *cl_gpgpu_set_perf_counters = NULL;
LOCAL cl_gpgpu_upload_curbes_cb *cl_gpgpu_upload_curbes = NULL;
LOCAL cl_gpgpu_states_setup_cb *cl_gpgpu_states_setup = NULL;
LOCAL cl_gpgpu_upload_samplers_cb *cl_gpgpu_upload_samplers = NULL;
LOCAL cl_gpgpu_batch_reset_cb *cl_gpgpu_batch_reset = NULL;
LOCAL cl_gpgpu_batch_start_cb *cl_gpgpu_batch_start = NULL;
LOCAL cl_gpgpu_batch_end_cb *cl_gpgpu_batch_end = NULL;
LOCAL cl_gpgpu_flush_cb *cl_gpgpu_flush = NULL;
LOCAL cl_gpgpu_walker_cb *cl_gpgpu_walker = NULL;
LOCAL cl_gpgpu_bind_sampler_cb *cl_gpgpu_bind_sampler = NULL;
LOCAL cl_gpgpu_event_new_cb *cl_gpgpu_event_new = NULL;
LOCAL cl_gpgpu_event_update_status_cb *cl_gpgpu_event_update_status = NULL;
LOCAL cl_gpgpu_event_flush_cb *cl_gpgpu_event_flush = NULL;
LOCAL cl_gpgpu_event_delete_cb *cl_gpgpu_event_delete = NULL;
LOCAL cl_gpgpu_event_get_exec_timestamp_cb *cl_gpgpu_event_get_exec_timestamp = NULL;
LOCAL cl_gpgpu_event_get_gpu_cur_timestamp_cb *cl_gpgpu_event_get_gpu_cur_timestamp = NULL;
LOCAL cl_gpgpu_ref_batch_buf_cb *cl_gpgpu_ref_batch_buf = NULL;
LOCAL cl_gpgpu_unref_batch_buf_cb *cl_gpgpu_unref_batch_buf = NULL;
LOCAL cl_gpgpu_set_printf_buffer_cb *cl_gpgpu_set_printf_buffer = NULL;
LOCAL cl_gpgpu_reloc_printf_buffer_cb *cl_gpgpu_reloc_printf_buffer = NULL;
LOCAL cl_gpgpu_map_printf_buffer_cb *cl_gpgpu_map_printf_buffer = NULL;
LOCAL cl_gpgpu_unmap_printf_buffer_cb *cl_gpgpu_unmap_printf_buffer = NULL;
LOCAL cl_gpgpu_set_printf_info_cb *cl_gpgpu_set_printf_info = NULL;
LOCAL cl_gpgpu_get_printf_info_cb *cl_gpgpu_get_printf_info = NULL;
LOCAL cl_gpgpu_release_printf_buffer_cb *cl_gpgpu_release_printf_buffer = NULL;


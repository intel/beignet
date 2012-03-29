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

#ifndef __CL_BUFFER_H__
#define __CL_BUFFER_H__

#include <stdint.h>

/* Hide behind some call backs the buffer allocation / deallocation ... This
 * will allow us to make the use of a software performance simulator easier and
 * to minimize the code specific for the HW and for the simulator
 */

/* Encapsulates command buffer / data buffer / kernels */
typedef struct cl_buffer cl_buffer;

/* Encapsulates buffer manager */
typedef struct cl_buffer_mgr cl_buffer_mgr;

/* Encapsulates the driver backend functionalities */
typedef struct cl_driver cl_driver;

/* Create a new driver */
typedef cl_driver* (cl_driver_new_cb)(void);
extern cl_driver_new_cb cl_driver_new;

/* Delete the driver */
typedef cl_driver* (cl_driver_delete_cb)(void);
extern cl_driver_delete_cb cl_driver_delete;

/* Get the buffer manager from the driver */
typedef cl_buffer_mgr* (cl_driver_get_bufmgr_cb)(cl_driver*);
extern cl_driver_get_bufmgr_cb *cl_driver_get_bufmgr;

/* Get the Gen version from the driver */
typedef uint32_t (cl_driver_get_ver_cb)(cl_driver*);
extern cl_driver_get_ver_cb *cl_driver_get_ver;

/* Allocate a buffer */
typedef cl_buffer* (cl_buffer_alloc_cb)(cl_buffer_mgr*, const char*, unsigned long, unsigned long);
extern cl_buffer_alloc_cb *cl_buffer_alloc;

/* Unref a buffer and destroy it if no more ref */
typedef void (cl_buffer_unreference_cb)(cl_buffer*);
extern cl_buffer_unreference_cb *cl_buffer_unreference;

/* Map a buffer */
typedef void* (cl_buffer_map_cb)(cl_buffer*);
extern cl_buffer_map_cb *cl_buffer_map;

/* Unmap a buffer */
typedef void* (cl_buffer_unmap_cb)(cl_buffer*);
extern cl_buffer_unmap_cb *cl_buffer_unmap;

/* Pin a buffer */
typedef int (cl_buffer_pin_cb)(cl_buffer*);
extern cl_buffer_pin_cb *cl_buffer_pin;

/* Unpin a buffer */
typedef int (cl_buffer_unpin_cb)(cl_buffer*);
extern cl_buffer_unpin_cb *cl_buffer_unpin;

/* Fill data in the buffer */
typedef int (cl_buffer_subdata_cb)(cl_buffer*, unsigned long, unsigned long, const void*);
extern cl_buffer_subdata_cb *cl_buffer_subdata;

/* Emit relocation */
typedef int (cl_buffer_emit_reloc_cb) (cl_buffer *, uint32_t, cl_buffer*, uint32_t, uint32_t, uint32_t);
extern cl_buffer_emit_reloc_cb *cl_buffer_emit_reloc;

#endif /* __CL_BUFFER_H__ */


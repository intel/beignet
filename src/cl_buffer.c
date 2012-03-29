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

#include "cl_buffer.h"
#include <stdlib.h>

cl_buffer_alloc_cb *cl_buffer_alloc = NULL;
cl_buffer_unreference_cb *cl_buffer_unreference = NULL;
cl_buffer_map_cb *cl_buffer_map = NULL;
cl_buffer_unmap_cb *cl_buffer_unmap = NULL;
cl_buffer_pin_cb *cl_buffer_pin = NULL;
cl_buffer_unpin_cb *cl_buffer_unpin = NULL;
cl_buffer_subdata_cb *cl_buffer_subdata = NULL;
cl_buffer_emit_reloc_cb *cl_buffer_emit_reloc = NULL;
cl_driver_get_bufmgr_cb *cl_driver_get_bufmgr = NULL;
cl_driver_get_ver_cb *cl_driver_get_ver = NULL;


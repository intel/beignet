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
 * Author: Rong Yang<rong.r.yang@intel.com>
 */

#ifndef __CL_DEVICE_ENQUEUE_H__
#define __CL_DEVICE_ENQUEUE_H__

#include "cl_internals.h"
#include "cl_driver.h"
#include "CL/cl.h"
#include <stdint.h>

extern cl_int cl_device_enqueue_bind_buffer(cl_gpgpu gpgpu, cl_kernel ker,
                                                     uint32_t *max_bti, cl_gpgpu_kernel *kernel);
extern cl_int cl_device_enqueue_parse_result(cl_command_queue queue, cl_gpgpu gpgpu);
#endif /* __CL_DEVICE_ENQUEUE_H__ */

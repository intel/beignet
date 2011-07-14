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

#ifndef __OPENCL_CL_FULSIM_H
#define __OPENCL_CL_FULSIM_H

#include "CL/cl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Once a command queue has been executed, we output a buffer */
extern CL_API_ENTRY cl_int CL_API_CALL
clFulsimSetOutputBuffer(cl_command_queue, cl_mem);

#ifdef __cplusplus
}
#endif

#endif /* __OPENCL_CL_FULSIM_H */



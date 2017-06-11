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
 * Author: He Junyan <junyan.he@intel.com>
 */

#include "cl_compiler.h"
#include "cl_device_id.h"

LOCAL cl_int
cl_compiler_check_available(cl_device_id device)
{
  if (device->compiler.available)
    return CL_SUCCESS;

  return CL_COMPILER_NOT_AVAILABLE;
}

LOCAL cl_int
cl_compiler_unload(cl_device_id device)
{
  if (device->compiler.available == CL_FALSE)
    return CL_SUCCESS;

  return device->api.compiler_unload(device);
}

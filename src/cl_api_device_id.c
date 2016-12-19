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
 */

#include "cl_device_id.h"
#include "cl_platform_id.h"

cl_int
clGetDeviceIDs(cl_platform_id platform,
               cl_device_type device_type,
               cl_uint num_entries,
               cl_device_id *devices,
               cl_uint *num_devices)
{
  const cl_device_type valid_type = CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU |
                                    CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT |
                                    CL_DEVICE_TYPE_CUSTOM;

  /* Check parameter consistency */
  if (UNLIKELY(devices == NULL && num_devices == NULL))
    return CL_INVALID_VALUE;
  if (UNLIKELY(platform && platform != cl_get_platform_default()))
    return CL_INVALID_PLATFORM;
  if (UNLIKELY(devices && num_entries == 0))
    return CL_INVALID_VALUE;
  if ((device_type & valid_type) == 0)
    return CL_INVALID_DEVICE_TYPE;

  return cl_get_device_ids(platform, device_type, num_entries, devices, num_devices);
}

cl_int
clGetDeviceInfo(cl_device_id device,
                cl_device_info param_name,
                size_t param_value_size,
                void *param_value,
                size_t *param_value_size_ret)
{
  if (!CL_OBJECT_IS_DEVICE(device)) {
    return CL_INVALID_DEVICE;
  }

  return cl_get_device_info(device, param_name, param_value_size,
                            param_value, param_value_size_ret);
}

cl_int
clRetainDevice(cl_device_id device)
{
  // XXX stub for C++ Bindings
  return CL_SUCCESS;
}

cl_int
clReleaseDevice(cl_device_id device)
{
  // XXX stub for C++ Bindings
  return CL_SUCCESS;
}

cl_int
clCreateSubDevices(cl_device_id in_device,
                   const cl_device_partition_property *properties,
                   cl_uint num_devices,
                   cl_device_id *out_devices,
                   cl_uint *num_devices_ret)
{
  /* Check parameter consistency */
  if (UNLIKELY(out_devices == NULL && num_devices_ret == NULL))
    return CL_INVALID_VALUE;
  if (UNLIKELY(in_device == NULL && properties == NULL))
    return CL_INVALID_VALUE;

  *num_devices_ret = 0;
  return CL_INVALID_DEVICE_PARTITION_COUNT;
}

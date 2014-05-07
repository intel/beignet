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
 *         Alexei Soupikov <alexei.soupikov@intel.com>
 */

#ifndef __INTEL_GPGPU_H__
#define __INTEL_GPGPU_H__

#include "cl_utils.h"
#include "cl_driver.h"

#include <stdlib.h>
#include <stdint.h>

/* Set the gpgpu related call backs */
extern void intel_set_gpgpu_callbacks(int device_id);

#endif /* __INTEL_GPGPU_H__ */


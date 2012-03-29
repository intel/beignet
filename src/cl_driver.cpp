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

extern "C" {
#include "sim/sim_buffer.h"
#include "intel/intel_buffer.h"
#include "cl_utils.h"
#include <stdlib.h>
#include <string.h>
}

/* Just use c++ pre-main to initialize the call-backs */
struct CallBackInitializer
{
  CallBackInitializer(void) {
    const char *run_it = getenv("OCL_SIMULATOR");
    if (run_it != NULL && !strcmp(run_it, "2"))
      sim_setup_callbacks();
    else
      intel_setup_callbacks();
  }
};

/* Set the call backs at pre-main time */
LOCAL CallBackInitializer cbInitializer;


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

#include "intel/genx_gpgpu.h"
#include "intel/genx_driver.h"
#include "x11/dricommon.h"

#include "cl_utils.h"
#include "cl_alloc.h"
#include "cl_genx_driver.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

LOCAL void
genx_driver_delete(genx_driver_t *driver)
{
  if (driver == NULL)
    return;
  cl_free(driver);
}

LOCAL genx_driver_t*
genx_driver_new(void)
{
  genx_driver_t *driver = NULL;

  TRY_ALLOC_NO_ERR (driver, CALLOC(genx_driver_t));
  driver->intel.fd = -1;

exit:
  return driver;
error:
  genx_driver_delete(driver);
  driver = NULL;
  goto exit;
}

LOCAL void
genx_driver_init(genx_driver_t *genx)
{
  int cardi;
  genx->x11_display = XOpenDisplay(":0.0");

  if(genx->x11_display) {
    if((genx->dri_ctx = getDRI2State(genx->x11_display,
                                     DefaultScreen(genx->x11_display),
                                     NULL)))
      intel_driver_init_shared(&genx->intel, genx->dri_ctx);
    else
      printf("X server found. dri2 connection failed! \n");
  } else {
    printf("Can't find X server!\n");
  }

  if(!intel_driver_is_active(&genx->intel)) {
    printf("Trying to open directly...");
    char card_name[20];
    for(cardi = 0; cardi < 16; cardi++) {
      sprintf(card_name, "/dev/dri/card%d", cardi);
      if(intel_driver_init_master(&genx->intel, card_name)) {
        printf("Success at %s.\n", card_name);
        break;
      }
    }
  }
  if(!intel_driver_is_active(&genx->intel)) {
    printf("Device open failed.\n");
    exit(-1);
  }
}

LOCAL void
genx_driver_terminate(genx_driver_t *genx)
{
  if(genx->dri_ctx) dri_state_release(genx->dri_ctx);
  if(genx->x11_display) XCloseDisplay(genx->x11_display);
  genx->dri_ctx = NULL;
  genx->x11_display = NULL;
}

LOCAL int
cl_intel_get_device_id(void)
{
  genx_driver_t *driver = NULL;
  int intel_device_id;

  driver = genx_driver_new();
  assert(driver != NULL);
  genx_driver_init(driver);
  intel_device_id = driver->intel.device_id;
  genx_driver_terminate(driver);
  genx_driver_delete(driver);

  return intel_device_id;
}

LOCAL genx_driver_t*
cl_genx_driver_new(void)
{
  genx_driver_t *driver = NULL;
  TRY_ALLOC_NO_ERR (driver, genx_driver_new());
  genx_driver_init(driver);

exit:
  return driver;
error:
  cl_genx_driver_delete(driver);
  driver = NULL;
  goto exit;
}

LOCAL void
cl_genx_driver_delete(genx_driver_t *driver)
{
  if (driver == NULL)
    return;
  genx_driver_terminate(driver);
  genx_driver_delete(driver);
}


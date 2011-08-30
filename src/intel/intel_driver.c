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

#include "intel_driver.h"
#include "intel_batchbuffer.h"
#include "x11/dricommon.h"

#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <xf86drm.h>

#include "cl_utils.h"
#include "cl_alloc.h"
#include "cl_genx_driver.h"

#define SET_BLOCKED_SIGSET(DRIVER)   do {                     \
  sigset_t bl_mask;                                           \
  sigfillset(&bl_mask);                                       \
  sigdelset(&bl_mask, SIGFPE);                                \
  sigdelset(&bl_mask, SIGILL);                                \
  sigdelset(&bl_mask, SIGSEGV);                               \
  sigdelset(&bl_mask, SIGBUS);                                \
  sigdelset(&bl_mask, SIGKILL);                               \
  pthread_sigmask(SIG_SETMASK, &bl_mask, &(DRIVER)->sa_mask); \
} while (0)

#define RESTORE_BLOCKED_SIGSET(DRIVER) do {                   \
  pthread_sigmask(SIG_SETMASK, &(DRIVER)->sa_mask, NULL);     \
} while (0)

#define PPTHREAD_MUTEX_LOCK(DRIVER) do {                      \
  SET_BLOCKED_SIGSET(DRIVER);                                 \
  pthread_mutex_lock(&(DRIVER)->ctxmutex);                    \
} while (0)

#define PPTHREAD_MUTEX_UNLOCK(DRIVER) do {                    \
  pthread_mutex_unlock(&(DRIVER)->ctxmutex);                  \
  RESTORE_BLOCKED_SIGSET(DRIVER);                             \
} while (0)


LOCAL intel_driver_t*
intel_driver_new(void)
{
  intel_driver_t *driver = NULL;

  TRY_ALLOC_NO_ERR (driver, CALLOC(intel_driver_t));
  driver->fd = -1;

exit:
  return driver;
error:
  intel_driver_delete(driver);
  driver = NULL;
  goto exit;
}

LOCAL void
intel_driver_delete(intel_driver_t *driver)
{
  if (driver == NULL)
    return;
  cl_free(driver);
}

/* just used for maximum relocation number in drm_intel */
#define BATCH_SIZE 0x1000

static void
intel_driver_memman_init(intel_driver_t *driver)
{
  driver->bufmgr = intel_bufmgr_gem_init(driver->fd, BATCH_SIZE);
  assert(driver->bufmgr);
  intel_bufmgr_gem_enable_reuse(driver->bufmgr);
}

static void 
intel_driver_init(intel_driver_t *driver, int dev_fd)
{
  driver->fd = dev_fd;
  driver->locked = 0;
  pthread_mutex_init(&driver->ctxmutex, NULL);
#ifndef NDEBUG
  int res =
#endif /* NDEBUG */
  intel_driver_get_param(driver, I915_PARAM_CHIPSET_ID, &driver->device_id);
  assert(res);
  intel_driver_memman_init(driver);

#if EMULATE_GEN
  driver->gen_ver = EMULATE_GEN;
  if (EMULATE_GEN == 7)
    driver->device_id = PCI_CHIP_IVYBRIDGE_GT2; /* we pick GT2 for IVB */
  else if (EMULATE_GEN == 6)
    driver->device_id = PCI_CHIP_SANDYBRIDGE_GT2; /* we pick GT2 for SNB */
  else
    FATAL ("Unsupported Gen for emulation");
#else
  if (IS_GEN7(driver->device_id))
    driver->gen_ver = 7;
  else if (IS_GEN6(driver->device_id))
    driver->gen_ver = 6;
  else if(IS_IGDNG(driver->device_id))
    driver->gen_ver = 5;
  else
    driver->gen_ver = 4;
#endif /* EMULATE_GEN */
}

LOCAL void
intel_driver_open(intel_driver_t *intel)
{
  int cardi;
  intel->x11_display = XOpenDisplay(":0.0");

  if(intel->x11_display) {
    if((intel->dri_ctx = getDRI2State(intel->x11_display,
                                     DefaultScreen(intel->x11_display),
                                     NULL)))
      intel_driver_init_shared(intel, intel->dri_ctx);
    else
      printf("X server found. dri2 connection failed! \n");
  } else {
    printf("Can't find X server!\n");
  }

  if(!intel_driver_is_active(intel)) {
    printf("Trying to open directly...");
    char card_name[20];
    for(cardi = 0; cardi < 16; cardi++) {
      sprintf(card_name, "/dev/dri/card%d", cardi);
      if(intel_driver_init_master(intel, card_name)) {
        printf("Success at %s.\n", card_name);
        break;
      }
    }
  }
  if(!intel_driver_is_active(intel)) {
    printf("Device open failed\n");
    exit(-1);
  }
}

LOCAL void
intel_driver_close(intel_driver_t *intel)
{
  if(intel->dri_ctx) dri_state_release(intel->dri_ctx);
  if(intel->x11_display) XCloseDisplay(intel->x11_display);
  if(intel->fd) close(intel->fd);
  intel->dri_ctx = NULL;
  intel->x11_display = NULL;
  intel->fd = NULL;
}

LOCAL int
intel_driver_get_param(intel_driver_t *driver, int param, int *value)
{
  int ret;
  struct drm_i915_getparam gp;

  gp.param = param;
  gp.value = value;

  ret = drmCommandWriteRead(driver->fd, DRM_I915_GETPARAM, &gp, sizeof(gp));
  return ret == 0;
}

LOCAL int
intel_driver_is_active(intel_driver_t *driver) {
  return driver->fd >= 0;
}

LOCAL int 
intel_driver_init_shared(intel_driver_t *driver, dri_state_t *state)
{
  assert(state);
  if(state->driConnectedFlag != DRI2)
    return 0;
  intel_driver_init(driver, state->fd);
  driver->master = 0;
  return 1;
}

LOCAL int
intel_driver_init_master(intel_driver_t *driver, const char* dev_name)
{
  int dev_fd;

  drm_client_t client;

  // usually dev_name = "/dev/dri/card%d"
  dev_fd = open(dev_name, O_RDWR);
  if (dev_fd == -1) return 0;

  // Check that we're authenticated and the only opener
  client.idx = 0;
  int ret = ioctl(dev_fd, DRM_IOCTL_GET_CLIENT, &client);
  assert (ret == 0);

  if (!client.auth) {
    close(dev_fd);
    return 0;
  }

  client.idx = 1;
  ret = ioctl(dev_fd, DRM_IOCTL_GET_CLIENT, &client);
  if (ret != -1 || errno != EINVAL) {
    close(dev_fd);
    return 0;
  }

  intel_driver_init(driver, dev_fd);
  driver->master = 1;

  return 1;
}

LOCAL int 
intel_driver_terminate(intel_driver_t *driver)
{
  pthread_mutex_destroy(&driver->ctxmutex);

  if(driver->master)
    close(driver->fd);
  driver->fd = -1;
  return 1;
}

LOCAL void
intel_driver_lock_hardware(intel_driver_t *driver)
{

  PPTHREAD_MUTEX_LOCK(driver);
  assert(!driver->locked);
  driver->locked = 1;
}

LOCAL void 
intel_driver_unlock_hardware(intel_driver_t *driver)
{
  driver->locked = 0;
  PPTHREAD_MUTEX_UNLOCK(driver);
}

LOCAL dri_bo*
intel_driver_share_buffer(intel_driver_t *driver, uint32_t name)
{
  assert(!driver->master);
  dri_bo *bo = intel_bo_gem_create_from_name(driver->bufmgr,
                                             "rendering buffer",
                                             name);
  return bo;
}

LOCAL uint32_t
intel_driver_shared_name(intel_driver_t *driver, dri_bo *bo)
{
  uint32_t name;
  assert(!driver->master);
  assert(bo);
  dri_bo_flink(bo, &name);
  return name;
}

extern drm_intel_bufmgr* intel_driver_get_buf(intel_driver_t*);

LOCAL drm_intel_bufmgr*
intel_driver_get_buf(intel_driver_t *drv)
{
  return drv->bufmgr;
}

LOCAL int
cl_intel_get_device_id(void)
{
  intel_driver_t *driver = NULL;
  int intel_device_id;

  driver = intel_driver_new();
  assert(driver != NULL);
  intel_driver_open(driver);
  intel_device_id = driver->device_id;
  intel_driver_close(driver);
  intel_driver_delete(driver);

  return intel_device_id;
}

LOCAL void
cl_intel_driver_delete(intel_driver_t *driver)
{
  if (driver == NULL)
    return;
  intel_driver_close(driver);
  intel_driver_terminate(driver);
  intel_driver_delete(driver);
}

LOCAL intel_driver_t*
cl_intel_driver_new(void)
{
  intel_driver_t *driver = NULL;
  TRY_ALLOC_NO_ERR (driver, intel_driver_new());
  intel_driver_open(driver);

exit:
  return driver;
error:
  cl_intel_driver_delete(driver);
  driver = NULL;
  goto exit;
}


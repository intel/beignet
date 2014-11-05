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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

/*
 * Copyright 2009 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#ifndef _INTEL_DRIVER_H_
#define _INTEL_DRIVER_H_

#include "cl_device_data.h"

#include <stdint.h>
#include <pthread.h>
#include <signal.h>

#include <xf86drm.h>
#include <drm.h>
#include <i915_drm.h>
#include <intel_bufmgr.h>
#include <intel/intel_gpgpu.h>

#define CMD_MI                                  (0x0 << 29)
#define CMD_2D                                  (0x2 << 29)

#define MI_NOOP                                 (CMD_MI | 0)
#define MI_BATCH_BUFFER_END                     (CMD_MI | (0xA << 23))
#define MI_FLUSH                                (CMD_MI | (0x4 << 23))
#define STATE_INSTRUCTION_CACHE_INVALIDATE      (0x1 << 0)

#define XY_COLOR_BLT_CMD                        (CMD_2D | (0x50 << 22) | 0x04)
#define XY_COLOR_BLT_WRITE_ALPHA                (1 << 21)
#define XY_COLOR_BLT_WRITE_RGB                  (1 << 20)
#define XY_COLOR_BLT_DST_TILED                  (1 << 11)

/* BR13 */
#define BR13_565                                (0x1 << 24)
#define BR13_8888                               (0x3 << 24)

struct dri_state;
struct intel_gpgpu_node;
typedef struct _XDisplay Display;

typedef struct intel_driver
{
  dri_bufmgr *bufmgr;
  drm_intel_context *ctx;
  int fd;
  int device_id;
  int gen_ver;
  sigset_t sa_mask;
  pthread_mutex_t ctxmutex;
  int locked;
  int need_close;
  Display *x11_display;
  struct dri_state *dri_ctx;
  struct intel_gpgpu_node *gpgpu_list;
} intel_driver_t;

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

/* device control */
extern void intel_driver_lock_hardware(intel_driver_t*);
extern void intel_driver_unlock_hardware(intel_driver_t*);

/* methods working in shared mode */
extern dri_bo* intel_driver_share_buffer(intel_driver_t*, const char *sname, uint32_t name);
extern uint32_t intel_driver_shared_name(intel_driver_t*, dri_bo*);

/* init driver shared with X using dri state, acquired from X Display */
extern int intel_driver_init_shared(intel_driver_t*, struct dri_state*);

/* init driver in master mode (when X is not using the card) 
 * usually dev_name = "/dev/dri/card0"
 */
extern int intel_driver_init_master(intel_driver_t*, const char* dev_name);

/* init driver for render node */
extern int intel_driver_init_render(intel_driver_t*, const char* dev_name);

/* terminate driver and all underlying structures */
extern int intel_driver_terminate(intel_driver_t*);

/* simple check if driver was initialized (checking fd should suffice) */
extern int intel_driver_is_active(intel_driver_t*);

/* query device parameters using driver ioctl */
extern int intel_driver_get_param(intel_driver_t*, int param, int *value);

/* init the call backs used by the ocl driver */
extern void intel_setup_callbacks(void);

#endif /* _INTEL_DRIVER_H_ */

